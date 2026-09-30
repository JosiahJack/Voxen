#!/usr/bin/env python3
"""
voxout - fit a Voxen SfxDef synth preset to a .wav file and print it to stdout.

    voxout ./Audio/hud/select.wav

Reads one wav, derives the sine/noise components of the sound effect, searches
a ladder of synth models from simplest upward, and prints the simplest C
struct initializer that reproduces the source within a target fidelity gap.
Exit 0 = accepted, exit 1 = below gate (caller should keep the .wav).

The printed struct is plain C designated-initializer syntax so a shell/awk
consumer can concatenate the output straight into a .h file.  The struct
definition itself belongs in audio.c; this tool never emits one.

Model (what the engine must implement to accept this output):

    tone    sum of nHarm harmonics of f0, amp = hAmp / k^tilt,
            odd=1 keeps odd harmonics only (sine->saw->square continuum),
            envelope exp(-hDec * (1 + hDTilt*(k-1)) * t)
    bands   up to nBand noise generators, each through a 2-pole resonant
            bandpass (fc, q), envelope amp * exp(-dec * t)
    am      tremolo, multiply by (1-amDepth + amDepth*cos(TAU*amRate*t))
    drive   tanh waveshaper for clipped/buzzy sources

Fidelity is scored as 1/3-octave band log-energy distance, band-limited to
the source's own Nyquist and floored 60 dB below peak.  Because a noise
generator can never reproduce the source's phase, the score has a non-zero
floor: we measure that floor (target magnitude spectrum, random phase) and
report the gap to it, so a rung is only accepted when it gets meaningfully
close to what is physically possible rather than against an arbitrary target.
"""

import argparse
import copy
import json
import math
import os
import subprocess
import sys
import time

import numpy as np

# --- must match the engine: common.h AUDIO_RATE ------------------------------
RATE = 48000

# Voice/band filter modes.  0=bandpass (a resonance), 1=lowpass (the body),
# 2=highpass.  A voice is one filtered noise burst with its own onset and decay.
MODE_BP, MODE_LP, MODE_HP = 0, 1, 2
_NBAND_FIELDS = 6          # per-voice: amp fc q dec mode delay
_N_SCALARS = 12
TAU = 2.0 * math.pi
FLOOR_DB = 90.0          # dynamic range kept in the band spectra (60 clips real
                       # broadband content on dense signals, making a lone sine
                       # look like an acceptable match)
BAND_OCTAVES = 3.0       # 1/3-octave band spacing for the timing term
SHAPE_PER_OCT = 24.0     # log-freq points per octave for the timbre term
SHAPE_FLOOR_DB = 60.0    # dynamic range kept by the shape term
SHAPE_W = 0.45           # weight of timbre vs timing in the combined loss
FINE_W = 0.35            # weight of the fine-frequency (pitch) term
PEAK_W = 6.0             # scales energy-weighted spectral error into the fine term
FINE_FLOOR_DB = 45.0     # dynamic range kept by the fine term
# Q means different things per filter mode.  A high-Q bandpass is a wanted
# resonance; a high-Q *lowpass* is a resonant peak that whistles (frob_hardware
# came out as a dog whistle from an LP band at Q=31), so cap it per mode.
Q_MAX = {MODE_LP: 4.0, MODE_HP: 4.0, MODE_BP: 80.0}
FMAX_HARD = 20000.0      # never analyse above this
C_HELPER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "voxout_render")


# ---------------------------------------------------------------------------
# audio input
# ---------------------------------------------------------------------------

def _load_via_soundfile(path):
    import soundfile as sf
    data, sr = sf.read(path, always_2d=True)
    return data.mean(axis=1), sr


def _load_via_sox(path):
    """Fallback when libsndfile is unavailable.  Emits 32-bit f32 mono."""
    info = subprocess.run(["soxi", "-r", "-c", path],
                          capture_output=True, text=True, check=True).stdout.split()
    sr, ch = int(info[0]), int(info[1])
    raw = subprocess.run(["sox", path, "-t", "f32", "-c", "1", "-"],
                         capture_output=True, check=True).stdout
    return np.frombuffer(raw, dtype="<f4").astype(np.float64), sr


def load_audio(path):
    """-> (mono, src_rate, src_nyquist).  Reads from stdin if path == '-'."""
    if path == "-":
        head = sys.stdin.buffer.read(4096)
        rest = sys.stdin.buffer.read()
        blob = head + rest
        try:
            import soundfile as sf
            data, sr = sf.read(__import__("io").BytesIO(blob), always_2d=True)
            return data.mean(axis=1), sr, sr / 2.0
        except Exception:
            sys.stderr.write("voxout: stdin requires libsndfile\n")
            raise SystemExit(2)
    try:
        mono, sr = _load_via_soundfile(path)
    except Exception:
        mono, sr = _load_via_sox(path)
    return mono, sr, sr / 2.0


def _gcd(a, b):
    while b:
        a, b = b, a % b
    return a


def resample_to_engine_rate(mono, sr):
    """Linear resample to RATE.  Good enough for analysis and bit-faithful to
    the engine's own resample_stereo(), which is also linear."""
    if sr == RATE:
        return mono.astype(np.float64)
    from math import gcd
    d = _gcd(int(sr), RATE)
    up, down = RATE // d, int(sr) // d
    n = int(math.ceil(len(mono) * up / down))
    t = np.arange(n) * (down / up)
    i0 = np.floor(t).astype(np.int64)
    frac = t - i0
    i1 = np.minimum(i0 + 1, len(mono) - 1)
    i0 = np.minimum(i0, len(mono) - 1)
    return mono[i0] * (1.0 - frac) + mono[i1] * frac


def trim_silence(mono, rel=0.01, tail=0.005):
    """Strip leading/trailing near-silence; keep a short tail so decays finish."""
    a = np.abs(mono)
    peak = a.max()
    if peak <= 0.0:
        return mono
    hit = np.flatnonzero(a > peak * rel)
    if hit.size == 0:
        return mono
    tail_n = int(RATE * tail)
    return mono[max(0, hit[0] - 64): min(len(mono), hit[-1] + tail_n)]


# ---------------------------------------------------------------------------
# metric: 1/3-octave band log-energy, band-limited and floored
# ---------------------------------------------------------------------------

def make_band_grid(fmax, nps):
    edges = [30.0]
    while edges[-1] * (2.0 ** (1.0 / BAND_OCTAVES)) < fmax:
        edges.append(edges[-1] * (2.0 ** (1.0 / BAND_OCTAVES)))
    edges.append(fmax)
    freqs = np.arange(nps // 2 + 1) * (RATE / nps)
    idx = [np.flatnonzero((freqs >= edges[i]) & (freqs < edges[i + 1]))
           for i in range(len(edges) - 1)]
    return edges, idx


def band_spectrum(sig, nps, hop, idx):
    """-> (nband, nframe) dB, peak-referenced and floored."""
    from scipy.signal import stft as _stft
    nps = min(nps, len(sig))
    hop = max(1, min(hop, nps // 2))
    _, _, z = _stft(sig, fs=RATE, nperseg=nps, noverlap=nps - hop, window="hann")
    p = np.abs(z) ** 2
    if p.shape[0] != nps // 2 + 1:
        p = p.T
    out = np.zeros((len(idx), p.shape[1]))
    for i, ix in enumerate(idx):
        if ix.size:
            out[i] = p[ix].sum(axis=0)
    db = 10.0 * np.log10(out + 1e-12)
    db -= db.max()
    return np.maximum(db, -FLOOR_DB)


def band_distance(a, b):
    n = min(a.shape[1], b.shape[1])
    return float(np.sqrt(np.mean((a[:, :n] - b[:, :n]) ** 2)))


def adaptive_nps(nsamples, dur):
    """Window size trades frequency resolution against time resolution.  Short
    effects are almost pure transients, and a long window destroys the time
    structure that dominates them -- this is the single biggest fidelity lever
    in the whole tool, so it is derived from the file rather than fixed."""
    if dur < 0.35:
        return 256
    if dur < 0.9:
        return 512
    if dur < 2.2:
        return 1024
    return 2048


def estimate_floor(mono, nps, hop, idx, seeds=(0x51, 0xA7, 0xC3)):
    """Distance attainable by a perfect magnitude match with random phase.
    This is the hard limit for any model whose noise has the wrong phase."""
    from scipy.signal import stft as _stft, istft
    nps = min(nps, len(mono))
    hop = max(1, min(hop, nps // 2))
    _, _, z = _stft(mono, fs=RATE, nperseg=nps, noverlap=nps - hop, window="hann")
    acc = []
    for s in seeds:
        rng = np.random.default_rng(s)
        _, y = istft(np.abs(z) * np.exp(1j * TAU * rng.random(z.shape)),
                     fs=RATE, nperseg=nps, noverlap=nps - hop, window="hann")
        y = y[:len(mono)]
        peak = np.abs(y).max()
        if peak < 1e-12:
            continue
        acc.append(band_distance(band_spectrum(mono, nps, hop, idx),
                                 band_spectrum(y / peak, nps, hop, idx)))
    return float(np.mean(acc)) if acc else 0.0


def dominant_freq(mono, nps, hop):
    """Strongest partial above 40 Hz, from a fine whole-signal FFT with
    parabolic interpolation.

    Deriving this from the STFT mean profile was far too coarse: at nps=256 the
    bins are 187 Hz apart, which seeded f0 at 1312 for a source whose actual
    tone is 1362 Hz -- a 60-cent error the coarse band metric cannot see at all,
    and the reason impact_pipe came out audibly pitch-shifted.
    """
    n = len(mono)
    nps = 1 << max(10, int(math.ceil(math.log2(max(n, 2048)))))
    nps = min(nps, 1 << 16)
    mag = np.abs(np.fft.rfft(mono * np.hanning(n), nps))
    bin_hz = RATE / nps
    lo = int(40.0 / bin_hz)
    if lo >= len(mag) or mag[lo:].max() <= 0:
        return 440.0
    i = lo + int(np.argmax(mag[lo:]))
    delta = 0.0
    if 0 < i < len(mag) - 1:
        a, b, c = mag[i - 1], mag[i], mag[i + 1]
        den = a - 2.0 * b + c
        if abs(den) > 1e-20:
            delta = float(min(max(0.5 * (a - c) / den, -0.5), 0.5))
    return float((i + delta) * bin_hz)


def shape_spectrum(sig, per_oct=24.0, fmax=FMAX_HARD, fill=0.35):
    """Spectral *envelope* on a log-frequency grid, as a smoothed power average.

    Two things forced this design.

    First, band_spectrum() averages ~230 Hz at 1 kHz, so a narrow Q=75 whistle
    and a broad hiss in the same 1/3-octave band have identical energy.  They
    sound nothing alike, and the optimizer will trade broadband body for a
    screaming resonance because the coarse metric cannot tell.

    Second, the grid values must be *averaged over a window*, not sampled
    pointwise.  A single FFT bin of a noise signal fluctuates by ~10 dB, so
    pointwise sampling makes even a perfect match score badly and inflates the
    estimated floor until everything looks like a pass -- which is exactly the
    failure that let a fit missing 26 dB at 1 kHz through the gate.  Averaging
    power over a window proportional to the log-frequency spacing gives a
    stable envelope, and a random-phase reconstruction of the target then
    genuinely reproduces it, so the floor estimate becomes meaningful.
    """
    n = len(sig)
    nps = 1 << max(10, int(math.ceil(math.log2(max(n, 1024)))))
    nps = min(nps, 1 << 16)
    w = np.hanning(n) if n > 1 else np.ones(1)
    mag = np.abs(np.fft.rfft(sig * w, nps))
    p = mag * mag
    bin_hz = RATE / nps
    npts = max(8, int(per_oct * math.log2(fmax / 30.0)))
    lf = 30.0 * (fmax / 30.0) ** (np.arange(npts) / (npts - 1.0))
    # half-width proportional to the local log-frequency spacing
    spacing = lf * (2.0 ** (1.0 / per_oct) - 1.0)
    half = np.maximum(1, (fill * spacing / bin_hz).astype(np.int64))
    cen = np.clip((lf / bin_hz).astype(np.int64), 0, len(p) - 1)
    csum = np.concatenate(([0.0], np.cumsum(p)))
    lo = np.maximum(0, cen - half)
    hi = np.minimum(len(p), cen + half + 1)
    n_in = np.maximum(1, hi - lo)
    val = np.sqrt((csum[hi] - csum[lo]) / n_in)
    db = 20.0 * np.log10(val + 1e-12)
    db -= db.max()
    return np.maximum(db, -SHAPE_FLOOR_DB)


def shape_distance(a, b):
    n = min(len(a), len(b))
    return float(np.sqrt(np.mean((a[:n] - b[:n]) ** 2)))


def fine_spectrum(sig, fmax=FMAX_HARD, smooth=3, floor=FINE_FLOOR_DB):
    """Full-resolution log magnitude, lightly averaged.

    The band grid is ~240 Hz wide at 1.4 kHz, so a fit sitting 1.5 semitones
    flat scored as a perfect match -- impact_pipe fitted 1313 Hz against a
    1363 Hz source and was accepted.  Pitch only shows up at fine frequency
    resolution, which is what this term adds.

    The averaging window stays small (a few bins) and the floor shallow: unlike
    shape_spectrum this must *not* become a smooth envelope, or a detuned tone
    would average away into the noise floor and the check would be worthless.
    """
    n = len(sig)
    nps = 1 << max(10, int(math.ceil(math.log2(max(n, 2048)))))
    nps = min(nps, 1 << 16)
    w = np.hanning(n) if n > 1 else np.ones(1)
    mag = np.abs(np.fft.rfft(sig * w, nps))
    fr = np.fft.rfftfreq(nps, 1.0 / RATE)
    hi = int(min(len(mag), int(fmax / (RATE / nps)) + 1))
    mag = mag[:hi]
    if smooth > 1 and len(mag) > smooth:
        k = np.ones(smooth) / smooth
        mag = np.convolve(mag, k, mode="same")
    lin = mag.copy()
    db = 20.0 * np.log10(mag + 1e-12)
    db -= db.max()
    return np.maximum(db, -floor), fr[:hi], lin


def fine_distance(a, fa, b, fb):
    """Compare fine spectra over their common frequency range only."""
    hi = fa[-1] if fa[-1] < fb[-1] else fb[-1]
    n = min(len(a), len(b), int(hi / (fa[1] - fa[0])) + 1)
    if n < 8:
        return 0.0
    return float(np.sqrt(np.mean((a[:n] - b[:n]) ** 2)))


def peak_distance(la, fa, lb, fb):
    """Relative L2 error of the *linear* magnitudes, where the loud parts of
    the spectrum dominate.

    A log-domain RMS spreads its weight evenly over every bin, so on a signal
    with a strong partial the peak position barely registers: impact_pipe kept
    landing ~60 cents sharp even with a fine log term, because thousands of
    quiet bins outweighed the one bin that had moved.  Comparing linear
    magnitudes instead weights by energy, which is what makes pitch and the
    relative level of the partials actually matter."""
    hi = fa[-1] if fa[-1] < fb[-1] else fb[-1]
    n = min(len(la), len(lb), int(hi / (fa[1] - fa[0])) + 1)
    if n < 8:
        return 0.0
    a = la[:n]
    b = lb[:n]
    denom = float(np.sqrt((a * a).sum()))
    if denom < 1e-20:
        return 0.0
    return float(np.sqrt(((a - b) ** 2).sum()) / denom)


def spectral_loss(target_b, target_s, cand, nps, hop, idx, fmax,
                 target_f=None):
    """Combined score: coarse bands keep the timing, the envelope keeps the
    timbre, and the fine term keeps pitch honest.  Any one alone is exploitable.
    """
    n = len(cand)
    peak = np.abs(cand).max()
    if peak < 1e-7 or float((cand * cand).sum()) / (n * peak * peak) < 1e-6:
        return 1e6, 1e6, 1e6, 1e6
    c = cand / peak
    b = band_distance(target_b, band_spectrum(c, nps, hop, idx))
    s = shape_distance(target_s, shape_spectrum(c, per_oct=SHAPE_PER_OCT, fmax=fmax))
    if target_f is not None:
        cf, cfr, clin = fine_spectrum(c, fmax=fmax)
        f = (fine_distance(target_f[0], target_f[1], cf, cfr)
             + PEAK_W * peak_distance(target_f[2], target_f[1], clin, cfr))
    else:
        f = 0.0
    total = (SHAPE_W * s + (1.0 - SHAPE_W) * b + FINE_W * f)
    return total, b, s, f


# ---------------------------------------------------------------------------
# synth kernel -- must stay identical to Tools/voxout_render.c
# ---------------------------------------------------------------------------

def _rbj_bandpass(fc, q, fs=RATE):
    w0 = TAU * min(max(fc, 10.0), fs * 0.49) / fs
    alpha = math.sin(w0) / (2.0 * max(q, 0.3))
    a0 = 1.0 + alpha
    return (alpha / a0, 0.0, -alpha / a0,
            -2.0 * math.cos(w0) / a0, (1.0 - alpha) / a0)


def _rbj_lowpass(fc, q, fs=RATE):
    """A lowpass band is what carries the broadband body of a sound.  Most of
    this corpus is broadband -- metal_step1 carries energy from 40 Hz to 5 kHz
    within 20 dB -- and a model limited to bandpass resonators cannot fill
    that at all, which is what made the fits come out shrill and empty."""
    w0 = TAU * min(max(fc, 10.0), fs * 0.49) / fs
    alpha = math.sin(w0) / (2.0 * max(q, 0.3))
    a0 = 1.0 + alpha
    c = math.cos(w0)
    return ((1.0 - c) * 0.5 / a0, (1.0 - c) / a0, (1.0 - c) * 0.5 / a0,
            -2.0 * c / a0, (1.0 - alpha) / a0)


def _rbj_highpass(fc, q, fs=RATE):
    w0 = TAU * min(max(fc, 10.0), fs * 0.49) / fs
    alpha = math.sin(w0) / (2.0 * max(q, 0.3))
    a0 = 1.0 + alpha
    c = math.cos(w0)
    return ((1.0 + c) * 0.5 / a0, -(1.0 + c) / a0, (1.0 + c) * 0.5 / a0,
            -2.0 * c / a0, (1.0 - alpha) / a0)


def _biquad(mode, fc, q):
    if mode == MODE_LP:
        return _rbj_lowpass(fc, q)
    if mode == MODE_HP:
        return _rbj_highpass(fc, q)
    return _rbj_bandpass(fc, q)


def _hash_noise(n, seed):
    """Counter-based hash (murmur3 finalizer) in uint32, uniform in [-1,1].
    Vectorized, and identical to hash_noise() in Tools/voxout_render.c so the
    Python fitter and the C reference produce bit-identical noise.  Uniform
    rather than gaussian on purpose: the engine's existing Gen* generators all
    draw from random_range(-1,1), so uniform is what the engine can build."""
    idx = np.arange(n, dtype=np.uint32)
    h = (idx + np.uint32(seed & 0xFFFFFFFF)) * np.uint32(2654435761)
    h ^= h >> np.uint32(15)
    h = h * np.uint32(0x85EBCA6B)
    h ^= h >> np.uint32(13)
    h = h * np.uint32(0xC2B2AE35)
    h ^= h >> np.uint32(16)
    r = (h >> np.uint32(8)).astype(np.int32) - np.int32(0x800000)   # centre on zero
    return r.astype(np.float64) * (1.0 / 8388608.0)


def _band_env(t, dec, delay, ramp=0.0015):
    """Per-band envelope: silent until `delay`, then an exponential decay.

    The delay is what lets one preset carry several distinct hits -- a
    click-clack-tuck button sound is three transients, and a single decaying
    generator can only ever be one of them, which is why those files scored
    30-38 dB against everything else."""
    u = t - delay
    env = np.where(u > 0.0, np.exp(-dec * np.maximum(u, 0.0)), 0.0)
    # short fade-in: raw noise starting at full scale is itself a click
    r = max(1, int(RATE * ramp))
    idx = np.arange(len(t))
    start = int(delay * RATE)
    fade = (idx - start)
    env = np.where((fade >= 0) & (fade < r), env * (fade / float(r)), env)
    return env


class Kernel:
    """Field order here defines the emitted struct field order."""

    FIELDS = ("dur", "vol", "f0", "tilt", "hAmp", "hDec", "hDTilt",
              "drive", "amRate", "amDepth", "nHarm", "odd",
              "bAmp", "bFC", "bQ", "bDec", "bMode", "bDelay", "nBand")

    def __init__(self, **kw):
        for f in self.FIELDS:
            setattr(self, f, kw.get(f, 0.0))
        self.nHarm = int(self.nHarm)
        self.nBand = int(self.nBand)
        self.odd = int(self.odd)

    def to_dict(self):
        return {f: getattr(self, f) for f in self.FIELDS}

    def to_vector(self):
        """Flat vector for the optimizer: scalars first, then per-band runs."""
        v = [self.dur, self.vol, self.f0, self.tilt, self.hAmp, self.hDec,
             self.hDTilt, self.drive, self.amRate, self.amDepth,
             float(self.nHarm), float(self.odd)]
        for i in range(self.nBand):
            v += [self.bAmp[i], self.bFC[i], self.bQ[i], self.bDec[i],
                  float(self.bMode[i]), self.bDelay[i]]
        return np.array(v, dtype=np.float64)

    @staticmethod
    def from_vector(v, nharm, nband):
        k = Kernel(dur=v[0], vol=v[1], f0=v[2], tilt=v[3], hAmp=v[4],
                   hDec=v[5], hDTilt=v[6], drive=v[7], amRate=v[8],
                   amDepth=v[9], nHarm=nharm, odd=v[11])
        k.bAmp = [0.0] * nband
        k.bFC = [0.0] * nband
        k.bQ = [0.0] * nband
        k.bDec = [0.0] * nband
        k.bMode = [0] * nband
        k.bDelay = [0.0] * nband
        for i in range(nband):
            o = _N_SCALARS + _NBAND_FIELDS * i
            (k.bAmp[i], k.bFC[i], k.bQ[i], k.bDec[i],
             k.bMode[i], k.bDelay[i]) = v[o:o + 6]
            k.bMode[i] = int(round(float(k.bMode[i])))
        k.nBand = nband
        return k

    def render(self, nsamples, seed=0):
        """Render exactly nsamples.  The caller always passes the target's
        length: rendering shorter would let the metric compare only the
        leading frames (it truncates to the shorter of the two), which is a
        loophole an optimizer will happily exploit by shrinking dur."""
        n = max(1, int(nsamples))
        t = np.arange(n, dtype=np.float64) / RATE
        out = np.zeros(n)

        if self.nHarm > 0 and self.hAmp > 1e-9:
            for h in range(1, self.nHarm + 1):
                if self.odd and (h % 2) == 0:
                    continue
                fk = self.f0 * h
                if fk > 0.47 * RATE:
                    break
                env = np.exp(-self.hDec * (1.0 + self.hDTilt * (h - 1)) * t)
                out += (self.hAmp / (h ** self.tilt)) * env * np.sin(TAU * fk * t)

        if self.nBand > 0:
            from scipy.signal import lfilter
            for i in range(self.nBand):
                if self.bAmp[i] <= 1e-6:
                    continue
                b0, b1, b2, a1, a2 = _biquad(self.bMode[i], self.bFC[i],
                                            self.bQ[i])
                # Filter only from the band's onset, with the biquad state
                # starting at zero.  Filtering the whole array and masking
                # afterwards would leave the filter already running at the
                # onset, so a band's timbre would depend on when it fires.
                start = min(n, max(0, int(self.bDelay[i] * RATE)))
                x = _hash_noise(n - start, seed + i * 0x9E3779B9)
                # lfilter's denominator is 1 + a1 z^-1 + a2 z^-2, which is
                # exactly the normalized RBJ filter returned above.
                y = lfilter([b0, b1, b2], [1.0, a1, a2], x)
                seg = np.zeros(n)
                if start < n:
                    seg[start:] = y
                    env = _band_env(t, self.bDec[i], self.bDelay[i])
                    out += self.bAmp[i] * env * seg

        if self.amDepth > 1e-4:
            out *= (1.0 - self.amDepth
                    + self.amDepth * np.cos(TAU * self.amRate * t))

        if self.drive > 1.05:
            out = np.tanh(out * self.drive) / math.tanh(self.drive)
        return out


# ---------------------------------------------------------------------------
# model ladder
# ---------------------------------------------------------------------------

# (label, nHarm, odd, nBand, useAM, useDrive)
# Ordered simplest -> most complex.  The first rung that lands inside the gap
# gate wins, so a sound that a plain sine can carry never grows a noise
# generator it does not need.  Kept short on purpose: every rung costs a slice
# of the per-file budget, so near-duplicates are merged.
LADDER = [
    ("1harm",         1, 0, 0, False, False),
    ("1harm+1v",      1, 0, 1, False, False),
    ("3odd+1v",       3, 1, 1, False, False),
    ("3odd+2v",       3, 1, 2, False, False),
    ("5odd+2v",       5, 1, 2, False, False),
    ("3odd+3v",       3, 1, 3, False, False),
    ("5odd+3v+am",    5, 1, 3, True,  False),
    ("5odd+4v+am",    5, 1, 4, True,  True),
    ("7odd+5v+am",    7, 1, 5, True,  True),
    ("7odd+6v+am",    7, 1, 6, True,  True),
    ("7odd+8v+am",    7, 1, 8, True,  True),
]

def _bounds(nharm, nband, fmax, useAM, useDrive, dmax=12.0, tmax=8.0):
    """12 scalar slots (dur vol f0 tilt hAmp hDec hDTilt drive amRate amDepth
    nHarm odd) then 4 per band.  Must match Kernel.from_vector exactly."""
    b = [(0.05, 8.0), (0.0, 4.0), (20.0, min(20000.0, fmax * 1.5)),
         (0.0, 3.0), (0.0, 4.0), (0.0, 900.0), (0.0, 2.0),
         (1.0, dmax) if useDrive else (1.0, 1.0),
         (0.0, 400.0) if useAM else (0.0, 0.0),
         (0.0, 0.95) if useAM else (0.0, 0.0),
         (float(nharm), float(nharm)), (0.0, 1.0)]
    for _ in range(nband):
        b += [(0.0, 4.0), (20.0, fmax * 1.05), (0.3, 80.0), (0.0, 900.0),
              (0.0, 2.0), (0.0, dmax)]
    return b


def apply_q_caps(bounds, x0, nband):
    """Q means different things per mode, so once the mode is frozen the Q
    ceiling must follow: a high-Q lowpass is a resonant peak, which is how
    frob_hardware came out as a dog whistle."""
    for i in range(nband):
        o = _N_SCALARS + _NBAND_FIELDS * i
        mode = int(round(x0[o + 4]))
        qo = o + 2
        bounds[qo] = (0.3, Q_MAX.get(mode, 80.0))


def clamp_voices(k):
    """Final tidy-up before emission."""
    clamp_q(k)
    lim = max(0.0, k.dur - 0.002)
    for i in range(len(k.bDelay)):
        k.bDelay[i] = min(max(k.bDelay[i], 0.0), lim)
    return k


def clamp_q(k):
    """Keep Q inside what the band mode can express.

    A high-Q bandpass is a wanted resonance; a high-Q lowpass is a resonant
    peak, which is exactly the dog whistle frob_hardware turned into."""
    for i in range(k.nBand):
        k.bQ[i] = min(max(k.bQ[i], 0.3), Q_MAX[k.bMode[i]])
    return k
    return b


def _band_response(mode, fc, q, lf):
    """Magnitude response of a band's biquad at log-frequency points lf."""
    w0 = TAU * min(max(fc, 10.0), RATE * 0.49) / RATE
    alpha = math.sin(w0) / (2.0 * max(q, 0.3))
    a0 = 1.0 + alpha
    c = math.cos(w0)
    if mode == MODE_LP:
        b0, b1, b2 = (1 - c) * 0.5 / a0, (1 - c) / a0, (1 - c) * 0.5 / a0
    elif mode == MODE_HP:
        b0, b1, b2 = (1 + c) * 0.5 / a0, -(1 + c) / a0, (1 + c) * 0.5 / a0
    else:
        b0, b1, b2 = alpha / a0, 0.0, -alpha / a0
    a1, a2 = -2.0 * c / a0, (1.0 - alpha) / a0
    z = np.exp(-2j * np.pi * lf / RATE)
    return np.abs((b0 + b1 * z + b2 * z * z) / (1.0 + a1 * z + a2 * z * z))


_Q_CANDIDATES = (0.4, 0.8, 1.6, 3.0, 6.0, 12.0)


def refit_gains(k, an):
    """Re-solve the band gains for the current band shapes by non-negative
    least squares against the source envelope.

    The pursuit finds a good band *structure*, but a free-running optimizer
    over 20-odd parameters reliably collapses it to a single band -- every
    model rung ended up with one band and 10-25 dB holes elsewhere.  Solving
    the gains in closed form and pinning them during optimization keeps the
    structure and drops the search dimension by one per band.
    """
    from scipy.optimize import nnls
    if k.nBand <= 0:
        return k
    lf = an.logfreq()
    tgt = 10.0 ** (an.shape / 20.0)
    try:
        A = np.array([_band_response(m, f, q, lf)
                      for m, f, q in zip(k.bMode, k.bFC, k.bQ)]).T
        g, _ = nnls(A, tgt)
    except Exception:
        return k
    if g.size != k.nBand or g.max() <= 0:
        return k
    g = g / g.max()
    for i in range(k.nBand):
        k.bAmp[i] = float(g[i])
    return k


def spectral_match(an, nband, ncand=9):
    """Greedy non-negative matching pursuit in the spectral-envelope domain.

    Handing a 20-parameter black-box optimizer a hand-picked starting point
    does not work: every rung of the ladder collapsed onto the same local
    optimum, one narrow band with the rest of the spectrum 10-25 dB short, and
    adding a band by hand made the score *worse* because it put energy where
    the fit was already too loud.  The problem is the wrong shape, not the
    wrong size, so it is solved directly here.

    For fixed band shapes the rendered envelope is a non-negative combination
    of the bands' responses, so the gains follow from a non-negative least
    squares fit to the source envelope.  Candidates are added greedily at the
    largest remaining residual, trying every filter mode and a few Q values,
    and the gains are refit after each addition.
    """
    from scipy.optimize import nnls
    lf = an.logfreq()
    tgt = 10.0 ** (an.shape / 20.0)          # linear envelope, peak-referenced
    lfmax = an.fmax
    # candidate centres: strongest parts of the residual spectrum
    order = np.argsort(an.shape)[::-1]
    cands = []
    for i in order:
        f = lf[i]
        if f < 40.0 or f > lfmax * 0.97:
            continue
        if any(abs(math.log2(f / c)) < 1.0 / 6.0 for c in cands):
            continue
        cands.append(float(f))
        if len(cands) >= ncand:
            break
    if not cands:
        return [], [], []

    chosen = []                                # (mode, fc, q)
    cols = []
    gains = np.zeros(0)
    best_res = float(np.sum(tgt * tgt))
    # the first band is always a broad lowpass: nearly every effect needs a
    # body, and without one the pursuit happily builds a stack of thin peaks
    for mode in (MODE_LP,):
        for fc in (min(lfmax * 0.35, 2000.0), min(lfmax * 0.6, 3500.0)):
            for q in (0.4, 0.8):
                r = _band_response(mode, fc, q, lf)
                if np.any(r > 0.05):
                    trial = cols + [r]
                    A = np.array(trial).T
                    try:
                        g, _res = nnls(A, tgt)
                    except Exception:
                        continue
                    resid = float(np.sum((tgt - A @ g) ** 2))
                    if resid < best_res:
                        best_res, chosen, cols, gains = resid, chosen + [(mode, fc, q)], trial, g

    for _ in range(nband - len(chosen)):
        cand_best = None
        for f in cands:
            for mode in (MODE_BP, MODE_LP, MODE_HP):
                for q in _Q_CANDIDATES:
                    if any(abs(math.log2(f / fc)) < 1.0 / 12.0 and m == mode
                           for m, fc, _q in chosen):
                        continue
                    r = _band_response(mode, f, q, lf)
                    if not np.any(r > 0.05):
                        continue
                    A = np.array(cols + [r]).T
                    try:
                        g, _ = nnls(A, tgt)
                    except Exception:
                        continue
                    resid = float(np.sum((tgt - A @ g) ** 2))
                    if cand_best is None or resid < cand_best[0]:
                        cand_best = (resid, mode, f, q, r, g)
        if cand_best is None:
            break
        resid, mode, f, q, r, g = cand_best
        if resid > best_res * 0.995:      # no useful gain left
            break
        best_res = resid
        chosen.append((mode, f, q))
        cols.append(r)
        gains = g

    if not chosen:
        return [], [], [], []
    # nnls only recovers relative gain; scale so the loudest band is O(1)
    g = gains / max(gains.max(), 1e-9)
    return ([c[1] for c in chosen], list(g), [c[0] for c in chosen],
            [c[2] for c in chosen])


def solve_gains(an, fcs, qs, modes):
    """Non-negative least squares for the voice gains given their shapes.

    The time-frequency pursuit works in STFT-magnitude units; the render works
    in signal units, so the amplitudes have to be re-solved against the source
    envelope once the shapes and onsets are known."""
    from scipy.optimize import nnls
    if not fcs:
        return []
    lf = an.logfreq()
    tgt = 10.0 ** (an.shape / 20.0)
    try:
        A = np.array([_band_response(m, f, q, lf)
                      for f, q, m in zip(fcs, qs, modes)]).T
        g, _ = nnls(A, tgt)
    except Exception:
        return [1.0] * len(fcs)
    if g.size != len(fcs) or g.max() <= 0:
        return [1.0] * len(fcs)
    g = g / g.max()
    return [float(v) for v in g]


def tf_pursuit(an, nvoice):
    """Time-frequency matching pursuit: place voices where the energy actually
    is, in both time and frequency.

    A sound effect is a stack of overlapping events -- a metal footstep is a
    scrape, a ring and a thud that all overlap; a crate breaking is three or
    four separate bangs.  One decaying generator per filter band cannot express
    that, and pruning the model back down to a single voice (which is what
    happened to every fit) threw the layering away entirely.  This places each
    voice at a distinct (frequency, onset) so the envelopes overlap on purpose.

    For each step it takes the largest remaining energy cell, estimates that
    voice's decay from the slope of its own envelope, and subtracts the voice's
    expected contribution (its filter response across frequency, decaying
    across time) before looking again.
    """
    n = an.n
    nps = min(an.nps, n)
    hop = max(1, nps // 4)
    _, tt, Z = stft_nps(an.mono, nps, hop)
    P = np.abs(Z)
    if P.shape[0] != nps // 2 + 1:
        P = P.T                                   # (frames, bins) -> (bins, frames)
    fr = np.arange(P.shape[0]) * (RATE / nps)
    lf = an.logfreq()
    # energy per (log-freq, time) cell, floored so silence cannot be picked
    E = np.zeros((len(lf), P.shape[1]))
    for i, f in enumerate(lf):
        j = int(round(f / (RATE / nps)))
        if 0 <= j < P.shape[0]:
            E[i] = P[j]
    E = np.maximum(E, E.max() * 1e-4 if E.max() > 0 else 0.0)
    times = tt[:P.shape[1]] if len(tt) >= P.shape[1] else np.arange(P.shape[1]) * hop / RATE

    voices = []
    for _ in range(nvoice):
        if E.size == 0:
            break
        i, k = np.unravel_index(np.argmax(E), E.shape)
        if E[i, k] <= E.max() * 0.02:
            break
        f0 = lf[i]
        t0 = float(times[k])
        # decay from this voice's own envelope after its onset
        seg = E[i, k:]
        lg = np.log(np.maximum(seg, E.max() * 1e-4))
        if len(lg) >= 4:
            dt = float(times[min(k + 1, len(times) - 1)] - times[k]) or (hop / RATE)
            sl = np.polyfit(np.arange(len(lg)) * dt, lg, 1)[0]
            dec = float(min(max(-sl, 0.5), 400.0))
        else:
            dec = 20.0
        # shape: narrow if it is a local peak in frequency, broad otherwise
        lo = max(0, i - 3)
        hi = min(len(lf), i + 4)
        prof = E[lo:hi, k]
        pk = float(prof.max())
        half = prof >= pk * 0.5
        wid = max(1, int(np.flatnonzero(half)[-1] - np.flatnonzero(half)[0] + 1)) \
            if half.any() else 1
        # wid is in 1/24-octave cells
        octw = wid / SHAPE_PER_OCT
        q = 1.0 / (2.0 * math.sin(math.pi * max(octw, 1e-3)))
        q = min(max(q, 0.4), 12.0)
        mode = MODE_BP if wid <= 3 else MODE_LP
        if f0 < 120.0:
            mode = MODE_LP
        voices.append([f0, q, mode, t0, dec, float(E[i, k])])

        # subtract this voice's contribution: exp decay in time, filter
        # response across frequency
        resp = _band_response(mode, f0, q, lf)
        u = times[None, :] - t0
        # >= 0, not > 0: the cell this voice was taken from has u == 0, and
        # excluding it left that cell untouched so the pursuit kept re-picking
        # the same voice instead of moving on to the next event.
        env = np.where(u >= -1e-9, np.exp(-dec * np.maximum(u, 0.0)), 0.0)
        E = E - voices[-1][5] * resp[:, None] * env
        np.maximum(E, 0.0, out=E)
    return voices


def _seed_vector(an, nharm, nband, useAM, useDrive, odd):
    """Start from the source's own spectrum.

    Seeding the noise generators from a hand-picked list of centre frequencies
    is what let the fits collapse onto a single narrow resonance and starve
    everything else -- they came out shrill and thin.  The centres, modes and
    gains come from a matching pursuit against the source envelope instead."""
    fmax = an.fmax
    # Place the voices in the time-frequency plane first: that is what gives a
    # preset several overlapping events instead of one long decay.
    tv = tf_pursuit(an, nband)
    if tv:
        fcs = [v[0] for v in tv]
        modes = [int(v[2]) for v in tv]
        qs = [float(v[1]) for v in tv]
        delays = [float(v[3]) for v in tv]
        decays = [float(v[4]) for v in tv]
        # Amplitudes come from the pursuit's own energy at each (freq, onset)
        # cell, not from a spectral least-squares fit.  Two voices at the same
        # centre with different onsets are the *same* filter, so their columns
        # are identical and NNLS is degenerate: it handed all the gain to one
        # and zeroed the rest, silently deleting the layering this is here to
        # create.  The measured cell energy already encodes "how loud is this
        # event", which is exactly the relative amplitude we need.
        amps = [float(v[5]) for v in tv]
    else:
        fcs, modes, qs, delays, decays = [], [], [], [], []
        amps = []
    while len(fcs) < nband:                 # pad if the pursuit came up short
        fcs.append(min(fmax * 0.4, 800.0))
        amps.append(0.0 if not amps else 0.0)
        modes.append(MODE_BP); qs.append(2.0)
        delays.append(0.0); decays.append(14.0)
    if amps and max(amps) > 0:
        g = max(amps)
        amps = [a / g for a in amps]
    v = [an.dur, 0.5, min(max(an.f0seed, 20.0), min(20000.0, fmax * 1.5)),
         0.6, 0.35, 12.0, 0.15, 3.0 if (useDrive and an.drive_max > 1.0) else 1.0,
         36.0 if useAM else 0.0, 0.0,
         float(nharm), float(odd)]
    for i in range(nband):
        v += [amps[i], fcs[i], max(qs[i], 0.3), decays[i], float(modes[i]),
              min(delays[i], max(an.dur - 0.001, 0.0))]
    return np.array(v, dtype=np.float64)


class Analysis:
    """Everything a fit needs about one source file, computed once."""

    def __init__(self, mono, nps, fmax):
        self.mono = mono
        self.n = len(mono)
        self.nps = nps
        self.hop = max(1, nps // 4)
        self.fmax = fmax
        self.dur = self.n / RATE
        self.edges, self.idx = make_band_grid(fmax, nps)
        self.target_b = band_spectrum(mono, nps, self.hop, self.idx)
        self.shape = shape_spectrum(mono, per_oct=SHAPE_PER_OCT, fmax=fmax)
        self.target_s = self.shape
        self.fine = fine_spectrum(mono, fmax=fmax)
        self.f0seed = dominant_freq(mono, nps, self.hop)
        self.floor = self.estimate_floor()
        # Fraction of samples sitting on full scale.  Only a genuinely clipped
        # source justifies saturating the output: tanh(x*drive) at high drive
        # is a square wave, and letting the optimizer reach for it just to
        # manufacture harmonics is what made the fits come out shrill.
        a = np.abs(mono)
        pk = float(a.max())
        self.clip_frac = float(np.mean(a >= 0.999 * pk)) if pk > 0 else 0.0
        self.drive_max = 12.0 if self.clip_frac > 0.02 else 1.0

    def logfreq(self):
        """The log-frequency grid shape_spectrum() sampled."""
        n = len(self.shape)
        if n < 2:
            return np.array([30.0])
        return 30.0 * (self.fmax / 30.0) ** (np.arange(n) / (n - 1.0))

    def freq_at(self, i):
        n = len(self.target_s)
        if n < 2:
            return 30.0
        return 30.0 * (self.fmax / 30.0) ** (i / (n - 1.0))

    def shape_index(self, f):
        n = len(self.target_s)
        if n < 2 or f <= 30.0:
            return 0
        t = math.log(f / 30.0) / math.log(self.fmax / 30.0)
        return int(min(n - 1, max(0, round(t * (n - 1)))))

    def estimate_floor(self, seeds=(0x51, 0xA7, 0xC3)):
        """Combined distance attainable by a perfect magnitude match with random
        phase: the hard limit for any model whose noise has the wrong phase."""
        from scipy.signal import istft
        nps = min(self.nps, self.n)
        hop = self.hop
        _, _, z = stft_nps(self.mono, nps, hop)
        acc = []
        for s in seeds:
            rng = np.random.default_rng(s)
            _, y = istft(np.abs(z) * np.exp(1j * TAU * rng.random(z.shape)),
                         fs=RATE, nperseg=nps, noverlap=nps - hop, window="hann")
            y = y[:self.n]
            if np.abs(y).max() < 1e-12:
                continue
            y = y / np.abs(y).max()
            cb = band_distance(self.target_b,
                              band_spectrum(y, self.nps, self.hop, self.idx))
            cs = shape_distance(self.target_s,
                                shape_spectrum(y, per_oct=SHAPE_PER_OCT,
                                               fmax=self.fmax))
            cf, cfr, clin = fine_spectrum(y, fmax=self.fmax)
            cg = (fine_distance(self.fine[0], self.fine[1], cf, cfr)
                  + PEAK_W * peak_distance(self.fine[2], self.fine[1], clin, cfr))
            acc.append(SHAPE_W * cs + (1.0 - SHAPE_W) * cb + FINE_W * cg)
        return float(np.mean(acc)) if acc else 0.0

    def loss(self, cand):
        return spectral_loss(self.target_b, self.target_s, cand,
                            self.nps, self.hop, self.idx, self.fmax, self.fine)


def stft_nps(sig, nps, hop):
    from scipy.signal import stft as _st
    nps = min(nps, len(sig))
    hop = max(1, min(hop, nps // 2))
    return _st(sig, fs=RATE, nperseg=nps, noverlap=nps - hop, window="hann")


def fit_rung(an, nharm, nband, odd, useAM, useDrive, deadline,
             x_init=None, max_powell=6000):
    from scipy.optimize import minimize
    n = an.n
    # dur and vol are pinned, not optimized: dur is the trimmed source length
    # and vol is solved analytically from the peak ratio afterwards.  Letting
    # the optimizer touch either is either meaningless or exploitable.
    bounds = _bounds(nharm, nband, an.fmax, useAM, useDrive, an.drive_max,
                     an.dur)
    bounds[0] = (an.dur, an.dur)        # dur
    bounds[1] = (1.0, 1.0)              # vol, solved after the fit
    bounds[10] = (float(nharm), float(nharm))
    bounds[11] = (float(odd), float(odd))
    if x_init is not None and len(x_init) == len(bounds):
        x0 = np.array([min(max(v, b[0]), b[1]) for v, b in zip(x_init, bounds)])
    else:
        x0 = _seed_vector(an, nharm, nband, useAM, useDrive, odd)
    x0[0] = an.dur
    x0[1] = 1.0
    x0[10] = float(nharm)
    x0[11] = float(odd)
    # Freeze each voice's centre and filter mode; leave its gain, Q, onset and
    # decay adjustable.  Freeing the centres collapsed a layered model back to
    # a single voice and threw away the overlapping envelopes the pursuit had
    # just placed, which is the whole point of the voice model.
    for i in range(nband):
        for fld in (1, 4):                    # fc, mode
            o = _N_SCALARS + _NBAND_FIELDS * i + fld
            bounds[o] = (x0[o], x0[o])
    apply_q_caps(bounds, x0, nband)
    assert len(x0) == len(bounds), (len(x0), len(bounds))
    assert len(x0) == _N_SCALARS + _NBAND_FIELDS * nband
    evals = [0]
    best = [1e6, np.array(x0)]

    def cost(v):
        if evals[0] % 32 == 0 and time.time() > deadline:
            # Out of time: report the best score seen so the line search finds
            # no improvement and unwinds cleanly.  Raising here would discard
            # the rung's entire result.
            return best[0] + 1e-3
        evals[0] += 1
        k = Kernel.from_vector(v, nharm, nband)
        s = an.loss(k.render(n, seed=0))[0]
        if s < best[0]:
            best[0], best[1] = s, np.array(v)
        return s

    r = minimize(cost, x0, method="L-BFGS-B", bounds=bounds,
                 options={"maxiter": 400, "maxfun": 8000})
    if max_powell > 0 and time.time() < deadline:
        r = minimize(cost, r.x, method="Powell", bounds=bounds,
                     options={"maxiter": max_powell, "xtol": 1e-3,
                              "ftol": 1e-3})
    vec = best[1] if best[0] < float(r.fun) else r.x
    k = Kernel.from_vector(vec, nharm, nband)
    k.odd = odd
    k.dur = an.dur
    return (k, best[0]), evals[0]


def solve_volume(k, an):
    """The loss is peak-normalized, so loudness is matched here instead.  The
    engine clamps preset vol through AppliedFXVol to [0,1], so this is a ratio
    capped at unity."""
    y = k.render(an.n, seed=11)
    yp = np.abs(y).max()
    if yp < 1e-9:
        return 1.0
    return float(min(1.0, 1.0 / yp))


def rescore(k, an, seeds=(11, 12, 13)):
    """Held-out seeds.  The fit optimizes one noise realization, so its
    fit-time score is optimistic; this is the number to trust."""
    acc = []
    for s in seeds:
        s_loss = an.loss(k.render(an.n, seed=s))[0]
        acc.append(s_loss)
    return float(np.mean(acc)) if acc else 1e6


def prune(k, an):
    """Drop components that do not measurably contribute, then compact.

    The optimizer will keep a tone stack whose harmonics have decayed to
    nothing if it started there, so the emitted preset would carry dead fields.
    Only a removal that does not worsen the held-out score is kept, so pruning
    can never cost fidelity.  This is what keeps the number of sines and noise
    generators minimal."""
    import copy as _copy
    base = rescore(k, an)
    tol = 1e-3
    changed = True
    while changed:
        changed = False
        cands = [("band", i) for i, a in enumerate(k.bAmp) if a > 1e-6]
        if k.nHarm > 0 and k.hAmp > 1e-9:
            cands.append(("tone", -1))
        for kind, i in cands:
            t = _copy.deepcopy(k)
            if kind == "band":
                t.bAmp[i] = 0.0
            else:
                t.hAmp = 0.0
            s = rescore(t, an)
            if s <= base + tol:
                k, base, changed = t, min(s, base), True
                break
    clamp_q(k)
    # a voice that starts after the sound ends is dead weight
    lim = max(0.0, k.dur - 0.002)
    for i in range(len(k.bDelay)):
        k.bDelay[i] = min(max(k.bDelay[i], 0.0), lim)
    keep = [i for i, a in enumerate(k.bAmp) if a > 1e-6]
    k.bAmp = [k.bAmp[i] for i in keep]
    k.bFC = [k.bFC[i] for i in keep]
    k.bQ = [k.bQ[i] for i in keep]
    k.bDec = [k.bDec[i] for i in keep]
    k.bMode = [k.bMode[i] for i in keep]
    k.bDelay = [k.bDelay[i] for i in keep]
    k.nBand = len(k.bAmp)
    if k.hAmp <= 1e-9:
        k.nHarm = 0
        k.odd = 0
    if k.amDepth <= 1e-4:
        k.amDepth = 0.0
        k.amRate = 0.0
    if k.drive <= 1.05:
        k.drive = 1.0
    return k, base


def search_ladder(an, gap_gate, budget, max_rung, deadline=None,
                  eps=0.03):
    """Two phases, then select on fidelity first and simplicity second.

    Phase 1 gives every rung a short gradient-only fit to rank them.  Phase 2
    spends most of the budget on the best few, each restarted from its own
    probe.  Splitting the budget evenly instead fails: complex rungs get
    scored while still noise and the search settles on a simpler model that
    fits worse.

    Selection then prefers the *lowest score*, and among everything within
    `eps` dB of it prefers the fewest components.  Stopping at the first rung
    that merely clears the gate -- the obvious reading of "climb until good
    enough" -- throws away most of the available fidelity, which is what made
    the fits thin.
    """
    if deadline is None:
        deadline = time.time() + budget
    rungs = [(i, r) for i, r in enumerate(LADDER) if i < max_rung]
    if not rungs:
        return None, []

    probe_each = max((budget * 0.28) / len(rungs), 0.8)
    cands = []          # (score, ncomp, rung_index, vector)
    # tuple order is (label, nHarm, odd, nBand, useAM, useDrive)
    for i, (label, nharm, odd, nband, useAM, useDrive) in rungs:
        if time.time() > deadline:
            break
        res, ev = fit_rung(an, nharm, nband, odd, useAM, useDrive,
                           min(deadline, time.time() + probe_each),
                           max_powell=0)
        if res is None:
            continue
        k, _ = res
        k.vol = solve_volume(k, an)
        k, sc = polish_modes(k, an)
        cands.append((sc, _complexity(k), i, k.to_vector()))

    if not cands:
        return None, []

    cands.sort(key=lambda c: c[0])
    # Refine the best-scoring rungs *and* the most complex ones.  Phase 1 is
    # gradient-only, so a many-voice model is scored while still unconverged
    # and looks worse than a one-voice model that happens to be near its
    # optimum -- which then kept it out of the finals entirely, and every
    # emitted preset came back as a single voice with no layering.
    finalists = cands[:min(2, len(cands))]
    for c in sorted(cands, key=lambda c: -c[1])[:2]:
        if c not in finalists:
            finalists.append(c)
    left = max(deadline - time.time(), 0.5)
    for j, (sc, ncomp, i, vec) in enumerate(finalists):
        if time.time() > deadline:
            break
        label, nharm, odd, nband, useAM, useDrive = LADDER[i]
        share = left / (len(finalists) - j)
        res, ev = fit_rung(an, nharm, nband, odd, useAM, useDrive,
                           min(deadline, time.time() + share),
                           x_init=pad_vector(vec, nband))
        if res is None:
            continue
        k, _ = res
        k.vol = solve_volume(k, an)
        k, s2 = polish_modes(k, an)
        cands.append((s2, _complexity(k), i, k.to_vector()))

    # Fidelity first, then richness.  A voice costs six floats, so when two
    # models are within eps of each other (0.03 dB is inaudible) the layered
    # one is the better description of how the effect is actually built -- a
    # footstep or a breaking crate *is* several overlapping events, and a
    # single decaying generator is a worse model of it even when it happens to
    # score the same.  Preferring the simpler model here was throwing away the
    # very structure the pursuit had just found.
    best_score = min(c[0] for c in cands)
    tol = [c for c in cands if c[0] <= best_score + eps]
    tol.sort(key=lambda c: (-c[1], c[0]))
    pick = tol[0]
    return (pick[2], pick[3]), cands


def _complexity(k):
    return int(k.nHarm) + int(k.nBand)


def _vec_nband(vec):
    """A vector's length determines how many bands it actually carries --
    pruning can strip a rung down to none, so the rung's nominal width is not
    a reliable guide."""
    return max(0, (len(np.asarray(vec)) - _N_SCALARS) // _NBAND_FIELDS)


def pad_vector(vec, nband):
    """Grow a pruned vector back out to a rung's nominal band count with
    neutral bands, so a phase-2 refinement can restart from the probe's result
    instead of falling back to a cold seed."""
    v = list(np.asarray(vec, dtype=np.float64))
    have = _vec_nband(v)
    for i in range(have, nband):
        v += [0.0, 1000.0, 2.5, 10.0, 0.0, 0.0]
    return np.array(v[:_N_SCALARS + _NBAND_FIELDS * nband], dtype=np.float64)


def polish_modes(k, an):
    """Try every filter mode for every band and keep whatever helps.

    A band is only useful as a body generator if it is a lowpass, and only
    useful as a resonance if it is a bandpass.  Powell on an integer mode
    parameter is unreliable, so the choice is brute-forced here instead --
    three evaluations per band, cheap, and it guarantees the fit actually gets
    broadband coverage instead of a stack of narrow peaks."""
    import copy as _copy
    base = rescore(k, an)
    changed = True
    while changed:
        changed = False
        for i in range(k.nBand):
            for m in (MODE_BP, MODE_LP, MODE_HP):
                if k.bMode[i] == m:
                    continue
                t = _copy.deepcopy(k)
                t.bMode[i] = m
                t = clamp_q(t)
                s = rescore(t, an)
                if s < base - 1e-3:
                    k, base, changed = t, s, True
                    break
            if changed:
                break
    return k, base


def _rebuild(vec, nharm, odd, dur):
    k = Kernel.from_vector(np.asarray(vec), nharm, _vec_nband(vec))
    k.odd = odd
    k.dur = dur
    return k


# ---------------------------------------------------------------------------
# output
# ---------------------------------------------------------------------------

def fmt(x, nd=5):
    if x == 0:
        return "0"
    a = abs(x)
    if a >= 1e5 or a < 1e-3:
        return f"{x:.3e}f"
    s = f"{x:.{nd}f}".rstrip("0").rstrip(".")
    return s + "f" if s not in ("", "-") else "0"


def emit_c(k, path, name, floor, dist, gap, tier, nps, evals, seconds,
           clipfrac=0.0):
    k = clamp_voices(k)
    band = lambda seq: "{" + ",".join(fmt(v) for v in seq) + "}"
    fields = [
        f".dur={fmt(k.dur)}", f".vol={fmt(k.vol, 4)}",
        f".f0={fmt(k.f0, 2)}", f".tilt={fmt(k.tilt, 4)}",
        f".hAmp={fmt(k.hAmp, 5)}", f".hDec={fmt(k.hDec, 3)}",
        f".hDTilt={fmt(k.hDTilt, 4)}", f".drive={fmt(k.drive, 3)}",
        f".amRate={fmt(k.amRate, 2)}", f".amDepth={fmt(k.amDepth, 4)}",
        f".nHarm={k.nHarm}", f".odd={k.odd}",
        f".nBand={k.nBand}",
        f".bAmp={band(k.bAmp)}", f".bFC={band(k.bFC)}",
        f".bQ={band(k.bQ)}", f".bDec={band(k.bDec)}",
        f".bMode={{{','.join(str(int(m)) for m in k.bMode)}}}",
        f".bDelay={band(k.bDelay)}",
    ]
    ident = f" /* {name} */" if name else ""
    print(f"/* voxout path={path} status=pass tier={tier} nps={nps} "
          f"dur={k.dur:.4f} floor={floor:.3f} dist={dist:.3f} gap={gap:+.3f} "
          f"nHarm={k.nHarm} nBand={k.nBand} odd={k.odd} tilt={k.tilt:.4f} "
          f"drive={k.drive:.3f} clipfrac={clipfrac:.4f} evals={evals} "
          f"t={seconds:.1f} */")
    print("{" + ", ".join(fields) + "}" + ident)


def emit_reject(path, name, floor, dist, gap, tier, nps, reason, seconds,
                clipfrac=0.0):
    print(f"/* voxout path={path} status=reject tier={tier} nps={nps} "
          f"floor={floor:.3f} dist={dist:.3f} gap={gap:+.2f} "
          f"clipfrac={clipfrac:.4f} t={seconds:.1f} reason={reason} */")


def emit_tsv(rec):
    keys = ("path", "name", "tier", "status", "nps", "dur", "floor", "dist",
            "gap", "gate", "nHarm", "nBand", "odd", "tilt", "drive",
            "clipfrac", "seconds")
    print("\t".join(keys))
    print("\t".join(str(rec.get(k, "")) for k in keys))


def emit_json(rec):
    print(json.dumps(rec))


def emit_ladder(rows):
    print("/* ladder: rung  dist  gap  nHarm  nBand  evals */")
    for r in rows:
        if r["dist"] is None:
            print(f"/*   {r['label']:18s}  (budget exhausted) */")
        else:
            print(f"/*   {r['label']:18s}  {r['dist']:6.2f}  "
                  f"{r['gap']:+6.2f}  {r['nharm']:5d}  {r['nband']:5d}  "
                  f"{r['evals']:5d} */")


# ---------------------------------------------------------------------------
# optional C cross-check
# ---------------------------------------------------------------------------

def render_via_c(k, nsamples):
    if not os.path.exists(C_HELPER):
        return None
    argv = [C_HELPER,
            f"{nsamples}", f"{k.vol}", f"{k.f0}", f"{k.tilt}", f"{k.hAmp}",
            f"{k.hDec}", f"{k.hDTilt}", f"{k.drive}", f"{k.amRate}",
            f"{k.amDepth}", str(k.nHarm), str(k.odd),
            f"{k.nBand}",
            ",".join(str(v) for v in k.bAmp),
            ",".join(str(v) for v in k.bFC),
            ",".join(str(v) for v in k.bQ),
            ",".join(str(v) for v in k.bDec),
            ",".join(str(int(m)) for m in k.bMode),
            ",".join(str(v) for v in k.bDelay), "0"]
    try:
        out = subprocess.run(argv, capture_output=True, check=True).stdout
    except Exception:
        return None
    if not out:
        return None
    return np.frombuffer(out, dtype="<f4").astype(np.float64)


def write_wav(path, sig, gain=0.9):
    import wave
    p = np.clip(sig * gain, -1.0, 1.0)
    pcm = (p * 32767.0).astype("<i2")
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(pcm.tobytes())


def write_ab(path, original, fitted, sr):
    """original | gap | fitted, loudness-matched so A/B is fair."""
    def prep(x):
        p = np.abs(x).max()
        return x / p if p > 1e-12 else x
    a, b = prep(original), prep(fitted)
    gap = np.zeros(int(RATE * 0.30))
    write_wav(path, np.concatenate([a, gap, b]), 0.9)


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main(argv=None):
    ap = argparse.ArgumentParser(
        prog="voxout",
        description="Fit a Voxen SfxDef synth preset to a wav file; print C to stdout.")
    ap.add_argument("wav", help="input .wav (or '-' for stdin)")
    ap.add_argument("--budget", type=float, default=90.0,
                    help="seconds of optimization for this file (default 90)")
    ap.add_argument("--tier", choices=("strict", "loose"), default="strict",
                    help="fidelity tier: strict=max-dist 16, loose=24 (mixed-unit "
                         "score: band + envelope + pitch terms)")
    ap.add_argument("--max-dist", type=float, default=None,
                    help="override the tier's absolute distance gate, in dB")
    ap.add_argument("--gap", type=float, default=None,
                    help="deprecated alias for --max-dist")
    ap.add_argument("--max-rung", type=int, default=len(LADDER),
                    help="climb at most this far up the model ladder")
    ap.add_argument("--name", default=None,
                    help="identifier to attach to the emitted initializer")
    ap.add_argument("--ladder", action="store_true",
                    help="print the per-rung results to stderr")
    ap.add_argument("--ab", metavar="WAV", default=None,
                    help="also write an A/B wav (original, gap, fitted)")
    ap.add_argument("--format", choices=("c", "tsv", "json"), default="c")
    ap.add_argument("--nps", type=int, default=0,
                    help="override the analysis window size")
    ap.add_argument("--nps-sweep", action="store_true",
                    help="try several analysis window sizes and keep the best "
                         "(costs ~4x, typically worth 0.2-1.0 dB)")
    ap.add_argument("--duration-cap", type=float, default=2.5,
                    help="truncate analysis at this many seconds")
    args = ap.parse_args(argv)

    started = time.time()
    # Acceptance is an *absolute* distance, not a gap to an estimated floor.
    # The floor (a magnitude-perfect, random-phase reconstruction) is only a
    # valid bound for the coarse band term; the whole-signal envelope term is a
    # global statistic that such a reconstruction does not preserve, which
    # inflated the floor until a fit missing 26 dB at 1 kHz still "passed".
    dist_gate = (args.max_dist if args.max_dist is not None
                 else args.gap if args.gap is not None
                 else (16.0 if args.tier == "strict" else 24.0))

    mono, sr, nyq = load_audio(args.wav)
    peak = np.abs(mono).max()
    if peak < 1e-9:
        sys.stderr.write("voxout: silent input\n")
        return 2
    mono = mono / peak
    # The engine upsamples every wav to AUDIO_RATE via resample_stereo() before
    # it ever reaches the mixer, so analysis has to happen at the same rate or
    # every derived frequency is wrong by the resample ratio.
    mono = resample_to_engine_rate(mono, sr)
    mono = trim_silence(mono)
    if len(mono) > int(RATE * args.duration_cap):
        mono = mono[:int(RATE * args.duration_cap)]
    dur = len(mono) / RATE
    fmax = min(nyq, FMAX_HARD)

    name = args.name or os.path.splitext(os.path.basename(args.wav))[0]

    # Window size trades time resolution against frequency resolution, and the
    # right point depends on the material rather than the duration alone.  A
    # sweep is optional because it multiplies cost by the number of windows
    # tried; on a sound-effect corpus it is typically worth 0.2-1.0 dB.
    if args.nps > 0:
        nps_candidates = [args.nps]
    elif args.nps_sweep:
        nps_candidates = [n for n in (256, 512, 1024, 2048) if n <= len(mono)]
    else:
        nps_candidates = [adaptive_nps(len(mono), dur)]

    per = args.budget / len(nps_candidates)
    best = None
    rows = []
    an_clip = 0.0
    for nps_try in nps_candidates:
        an = Analysis(mono, nps_try, fmax)
        an_clip = an.clip_frac
        pick, probed = search_ladder(an, dist_gate, per, args.max_rung)
        for sc, _nc, ri, _v in probed:
            rows.append({"label": f"nps{nps_try}/{LADDER[ri][0]}", "dist": sc,
                         "gap": sc - an.floor, "nharm": LADDER[ri][1],
                         "nband": LADDER[ri][3], "evals": 0})
        if pick is None:
            continue
        bi, bvec = pick
        k = _rebuild(bvec, LADDER[bi][1], LADDER[bi][2], an.dur)
        k.vol = solve_volume(k, an)
        k, _ = polish_modes(k, an)
        k.vol = solve_volume(k, an)
        sc = rescore(k, an)
        if best is None or sc < best[1]:
            best = (k, sc, nps_try, an.floor)

    seconds = time.time() - started
    if args.ladder:
        emit_ladder(rows)

    if best is None:
        rec = {"path": args.wav, "name": name, "tier": args.tier,
               "status": "reject", "nps": 0, "dur": round(dur, 4),
               "floor": None, "dist": None, "gap": None,
               "reason": "budget", "seconds": round(seconds, 2)}
        if args.format == "c":
            emit_reject(args.wav, name, 99.0, 99.0, 99.0, args.tier,
                        0, "budget", seconds, 0.0)
        elif args.format == "tsv":
            emit_tsv(rec)
        else:
            emit_json(rec)
        return 1

    k, dist, nps, floor = best
    gap = dist - floor
    status = "pass" if dist <= dist_gate else "reject"
    rec = {"path": args.wav, "name": name, "tier": args.tier,
           "status": status, "nps": nps, "dur": round(dur, 4),
           "floor": round(floor, 3), "dist": round(dist, 3),
           "gap": round(gap, 3), "nHarm": k.nHarm, "nBand": k.nBand,
           "odd": k.odd, "tilt": round(k.tilt, 4),
           "drive": round(k.drive, 3), "clipfrac": round(an_clip, 4),
           "seconds": round(seconds, 2)}

    if args.ab:
        fitted = render_via_c(k, len(mono))
        if fitted is None:
            fitted = k.render(len(mono), seed=11)
        # Prefix the A/B name with the verdict so a rejected fit is never
        # mistaken for a candidate replacement when reviewing by ear.
        d, b = os.path.split(args.ab)
        prefix = "ok_" if status == "pass" else "REJECT_"
        if not b.startswith("ok_") and not b.startswith("REJECT_"):
            b = prefix + b
        write_ab(os.path.join(d, b) if d else b, mono, fitted, sr)

    an_clip = 0.0
    if args.format == "c":
        if status == "pass":
            emit_c(k, args.wav, name, floor, dist, gap, args.tier,
                   nps, 0, seconds, an_clip)
        else:
            emit_reject(args.wav, name, floor, dist, gap, args.tier,
                        nps, f"dist>{dist_gate}", seconds, an_clip)
    elif args.format == "tsv":
        emit_tsv(rec)
    else:
        emit_json(rec)
    return 0 if status == "pass" else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.exit(130)