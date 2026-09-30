/*
 * voxout_render.c - standalone reference renderer for the SfxDef synth kernel.
 *
 * Reads one preset's parameters from argv and writes mono 48kHz float32 PCM to
 * stdout, so the fitter can confirm its Python model matches a C
 * implementation of the same kernel.  This file is deliberately independent
 * of the engine (no common.h / audio.c): it is a reference, and the engine's
 * own GenAdditive() must be written to match it.
 *
 * Built by Tools/voxout_build.sh.  Not part of the voxen binary.
 *
 *   argv: nsamples vol f0 tilt hAmp hDec hDTilt drive amRate amDepth nHarm odd \
 *         nBand bAmp,csv bFC,csv bQ,csv bDec,csv noiseSeed
 *
 * nsamples is a sample count, not a duration: the fitter always renders the
 * target's exact length so the metric cannot be gamed by truncating.  The
 * preset's dur lives in the engine, which cuts the voice after AUDIO_RATE*dur
 * frames; the envelope shapes here are governed entirely by the decay terms.
 *
 * Kernel:
 *   tone   sum over h=1..nHarm of (hAmp / h^tilt)
 *                            * exp(-hDec * (1 + hDTilt*(h-1)) * t)
 *                            * sin(2*pi * f0 * h * t),  odd==1 skips even h
 *   band   noise through a 2-pole RBJ bandpass at (fc,q), scaled by
 *                            bAmp * exp(bDec * t)
 *   am     multiply by (1-amDepth + amDepth*cos(2*pi*amRate*t))
 *   drive  tanh waveshaper, normalized by tanh(drive)
 *
 * Noise is a plain LCG so it is bit-reproducible across the Python and C
 * implementations and needs no libc RNG state.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUDIO_RATE 48000
#define MAX_HARM 16
#define MAX_BAND 8
#define TWO_PI 6.28318530717958647692

typedef struct {
    float vol, f0, tilt, hAmp, hDec, hDTilt;
    float drive, amRate, amDepth;
    int nHarm, odd, nBand;
    float bAmp[MAX_BAND], bFC[MAX_BAND], bQ[MAX_BAND], bDec[MAX_BAND];
    unsigned noiseSeed;
} SfxDef;

static void split_csv(const char *s, float *out, int n) {
    for (int i = 0; i < n; ++i) out[i] = 0.0f;
    if (!s || !*s) return;
    const char *p = s;
    for (int i = 0; i < n && *p; ++i) {
        out[i] = (float)atof(p);
        const char *c = strchr(p, ',');
        if (!c) break;
        p = c + 1;
    }
}

/* Counter-based hash (murmur3 finalizer) for the noise source.  Chosen over an
 * LCG because it is stateless per index, so Python can generate the exact same
 * stream with a vectorized numpy expression and the two implementations can be
 * compared bit for bit.  Uniform in [-1,1], matching the engine's existing
 * random_range(-1,1) idiom in the Gen* generators. */
static inline float hash_noise(unsigned i, unsigned seed) {
    unsigned h = (i + seed) * 2654435761u;
    h ^= h >> 15; h *= 0x85ebca6bu;
    h ^= h >> 13; h *= 0xc2b2ae35u;
    h ^= h >> 16;
    int r = (int)(h >> 8) - 0x800000;   /* centre on zero -> [-1,1) */
    return (float)r * (1.0f / 8388608.0f);
}

static void render(const SfxDef *d, int n, FILE *out) {
    if (n < 1) n = 1;
    float *buf = (float *)malloc(sizeof(float) * (size_t)n);
    if (!buf) return;
    memset(buf, 0, sizeof(float) * (size_t)n);

    float inv = 1.0f / (float)AUDIO_RATE;

    /* tone stack */
    if (d->nHarm > 0 && d->hAmp > 1e-9f) {
        for (int h = 1; h <= d->nHarm && h <= MAX_HARM; ++h) {
            if (d->odd && (h % 2) == 0) continue;
            float fk = d->f0 * (float)h;
            if (fk > 0.47f * (float)AUDIO_RATE) break;
            float amp = d->hAmp / powf((float)h, d->tilt);
            float dec = d->hDec * (1.0f + d->hDTilt * (float)(h - 1));
            float w = TWO_PI * fk * inv;
            for (int i = 0; i < n; ++i) {
                float t = (float)i * inv;
                buf[i] += amp * expf(-dec * t) * sinf(w * (float)i);
            }
        }
    }

    /* resonant noise bands */
    for (int b = 0; b < d->nBand && b < MAX_BAND; ++b) {
        if (d->bAmp[b] <= 1e-6f) continue;
        float fc = d->bFC[b], q = d->bQ[b] < 0.3f ? 0.3f : d->bQ[b];
        if (fc > (float)AUDIO_RATE * 0.49f) fc = (float)AUDIO_RATE * 0.49f;
        if (fc < 10.0f) fc = 10.0f;
        float w0 = TWO_PI * fc * inv;
        float alpha = sinf(w0) / (2.0f * q);
        float a0 = 1.0f + alpha;
        /* numerator alpha + 0 - alpha*z^-2 ; denominator a0 - 2cos(w0) + (1-alpha) */
        float b0 = alpha / a0, b2 = -alpha / a0;
        float a1 = -2.0f * cosf(w0) / a0, a2 = (1.0f - alpha) / a0;
        /* Per-band seed offset keeps bands independent and reproducible. */
        unsigned s = d->noiseSeed + (unsigned)b * 0x9E3779B9u;
        float d1 = 0.0f, d2 = 0.0f, yprev = 0.0f, xprev = 0.0f;
        for (int i = 0; i < n; ++i) {
            float x = hash_noise((unsigned)i, s);
            /* Transposed direct form II for
             *   H(z) = (b0 + 0*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
             * b1 is zero so the d1 update drops its b1*x term. */
            float y = b0 * x + d1;
            /* d2 must be advanced first: d1[n+1] needs the freshly formed
             * d2[n+1] = b2*x[n-1] - a2*y[n-1], not the previous d2.  Using the
             * stale value leaves the two states a sample apart and the filter
             * diverges even when it is nominally stable. */
            float d2n = b2 * xprev - a2 * yprev;
            float d1n = -a1 * y + d2n;
            d2 = d2n; d1 = d1n; xprev = x; yprev = y;
            float t = (float)i * inv;
            buf[i] += d->bAmp[b] * expf(-d->bDec[b] * t) * y;
        }
    }

    /* amplitude modulation */
    if (d->amDepth > 1e-4f) {
        float w = TWO_PI * d->amRate * inv;
        for (int i = 0; i < n; ++i)
            buf[i] *= (1.0f - d->amDepth
                       + d->amDepth * cosf(w * (float)i));
    }

    /* saturation */
    if (d->drive > 1.05f) {
        float k = tanhf(d->drive);
        for (int i = 0; i < n; ++i)
            buf[i] = tanhf(buf[i] * d->drive) / k;
    }

    for (int i = 0; i < n; ++i) buf[i] *= d->vol;
    fwrite(buf, sizeof(float), (size_t)n, out);
    free(buf);
}

int main(int argc, char **argv) {
    if (argc < 19) {
        fprintf(stderr,
            "usage: voxout_render nsamples vol f0 tilt hAmp hDec hDTilt drive "
            "amRate amDepth nHarm odd nBand bAmp bFC bQ bDec seed\n");
        return 2;
    }
    SfxDef d;
    memset(&d, 0, sizeof(d));
    int nsamples = atoi(argv[1]);
    d.vol    = (float)atof(argv[2]);
    d.f0     = (float)atof(argv[3]);
    d.tilt   = (float)atof(argv[4]);
    d.hAmp   = (float)atof(argv[5]);
    d.hDec   = (float)atof(argv[6]);
    d.hDTilt = (float)atof(argv[7]);
    d.drive  = (float)atof(argv[8]);
    d.amRate = (float)atof(argv[9]);
    d.amDepth= (float)atof(argv[10]);
    d.nHarm  = atoi(argv[11]);
    d.odd    = atoi(argv[12]);
    d.nBand  = atoi(argv[13]);
    if (d.nBand > MAX_BAND) d.nBand = MAX_BAND;
    if (d.nHarm > MAX_HARM) d.nHarm = MAX_HARM;
    split_csv(argv[14], d.bAmp, d.nBand);
    split_csv(argv[15], d.bFC,  d.nBand);
    split_csv(argv[16], d.bQ,   d.nBand);
    split_csv(argv[17], d.bDec, d.nBand);
    d.noiseSeed = (argc > 18) ? (unsigned)strtoul(argv[18], NULL, 10) : 1u;
    render(&d, nsamples, stdout);
    return 0;
}