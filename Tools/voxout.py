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
import json
import math
import os
import subprocess
import sys
import time

import numpy as np

# --- must match the engine: common.h AUDIO_RATE ------------------------------
RATE = 48000
TAU = 2.0 * math.pi
FLOOR_DB = 90.0          # dynamic range kept in the band spectra (60 clips real
                       # broadband content on dense signals, making a lone sine
                       # look like an acceptable match)
BAND_OCTAVES = 3.0       # 1/3-octave band spacing for the metric
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
    from scipy.signal import stft as _stft
    nps = min(nps, len(mono))
    hop = max(1, min(hop, nps // 2))
    _, _, z = _stft(mono, fs=RATE, nperseg=nps, noverlap=nps - hop, window="hann")
    m = np.abs(z)
    if m.shape[0] != nps // 2 + 1:
        m = m.T
    prof = m.mean(axis=1)
    if prof.max() <= 0:
        return 440.0
    f = np.arange(nps // 2 + 1) * (RATE / nps)
    # ignore the sub-audio rumble and prefer the strongest peak above 40 Hz
    mask = f > 40.0
    return float(f[mask][int(np.argmax(prof[mask]))]) if mask.any() else 440.0


# ---------------------------------------------------------------------------
# synth kernel -- must stay identical to Tools/voxout_render.c
# ---------------------------------------------------------------------------

def _rbj_bandpass(fc, q, fs=RATE):
    w0 = TAU * min(max(fc, 10.0), fs * 0.49) / fs
    alpha = math.sin(w0) / (2.0 * max(q, 0.3))
    a0 = 1.0 + alpha
    return (alpha / a0, 0.0, -alpha / a0,
            -2.0 * math.cos(w0) / a0, (1.0 - alpha) / a0)


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


class Kernel:
    """Field order here defines the emitted struct field order."""

    FIELDS = ("dur", "vol", "f0", "tilt", "hAmp", "hDec", "hDTilt",
              "drive", "amRate", "amDepth", "nHarm", "odd",
              "bAmp", "bFC", "bQ", "bDec", "nBand")

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
            v += [self.bAmp[i], self.bFC[i], self.bQ[i], self.bDec[i]]
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
        for i in range(nband):
            o = 12 + 4 * i
            k.bAmp[i], k.bFC[i], k.bQ[i], k.bDec[i] = v[o:o + 4]
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
                b0, b1, b2, a1, a2 = _rbj_bandpass(self.bFC[i], self.bQ[i])
                x = _hash_noise(n, seed + i * 0x9E3779B9)
                # lfilter's denominator is 1 + a1 z^-1 + a2 z^-2, which is
                # exactly the normalized RBJ bandpass returned above.
                y = lfilter([b0, b1, b2], [1.0, a1, a2], x)
                out += self.bAmp[i] * np.exp(-self.bDec[i] * t) * y

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
    ("1harm+1band",   1, 0, 1, False, False),
    ("3odd+1band",    3, 1, 1, False, False),
    ("3odd+2band",    3, 1, 2, False, False),
    ("5odd+2band",    5, 1, 2, False, False),
    ("7odd+3band+am", 7, 1, 3, True,  True),
]


def _bounds(nharm, nband, fmax, useAM, useDrive):
    """12 scalar slots (dur vol f0 tilt hAmp hDec hDTilt drive amRate amDepth
    nHarm odd) then 4 per band.  Must match Kernel.from_vector exactly."""
    b = [(0.05, 8.0), (0.0, 4.0), (20.0, min(20000.0, fmax * 1.5)),
         (0.0, 3.0), (0.0, 4.0), (0.0, 900.0), (0.0, 2.0),
         (1.0, 60.0) if useDrive else (1.0, 1.0),
         (0.0, 400.0) if useAM else (0.0, 0.0),
         (0.0, 0.95) if useAM else (0.0, 0.0),
         (float(nharm), float(nharm)), (0.0, 1.0)]
    for _ in range(nband):
        b += [(0.0, 4.0), (20.0, fmax * 1.05), (0.3, 80.0), (0.0, 900.0)]
    return b


def _seed_vector(nharm, nband, f0seed, fmax, useAM, useDrive):
    v = [0.05, 0.5, min(max(f0seed, 20.0), min(20000.0, fmax * 1.5)),
         0.6, 0.35, 12.0, 0.15, 3.0 if useDrive else 1.0,
         36.0 if useAM else 0.0, 0.0,
         float(nharm), 0.0]
    centres = [1600.0, 250.0, 5200.0, 5000.0]
    for i in range(nband):
        fc = centres[i] if i < len(centres) else 1200.0
        v += [0.25, min(fc, fmax * 0.95), 5.0, 14.0]
    return np.array(v, dtype=np.float64)


def fit_rung(mono, nps, hop, idx, target, f0seed, fmax, dur,
             nharm, nband, useAM, useDrive, deadline):
    from scipy.optimize import minimize
    n = len(mono)
    # dur and vol are pinned, not optimized: dur is the trimmed source length
    # and vol is solved analytically from the peak ratio afterwards.  Letting
    # the optimizer touch either is either meaningless or exploitable.
    bounds = _bounds(nharm, nband, fmax, useAM, useDrive)
    bounds[0] = (dur, dur)            # dur
    bounds[1] = (1.0, 1.0)            # vol placeholder, recomputed after fit
    x0 = _seed_vector(nharm, nband, f0seed, fmax, useAM, useDrive)
    x0[0] = dur
    x0[1] = 1.0
    assert len(x0) == len(bounds), (len(x0), len(bounds))
    assert len(x0) == 12 + 4 * nband
    evals = [0]

    def cost(v):
        if evals[0] % 32 == 0 and time.time() > deadline:
            raise TimeoutError
        evals[0] += 1
        k = Kernel.from_vector(v, nharm, nband)
        y = k.render(n, seed=0)
        peak = np.abs(y).max()
        if peak < 1e-9:
            return 1e6
        return band_distance(target, band_spectrum(y / peak, nps, hop, idx))

    try:
        r = minimize(cost, x0, method="L-BFGS-B", bounds=bounds,
                     options={"maxiter": 400, "maxfun": 6000})
        r = minimize(cost, r.x, method="Powell", bounds=bounds,
                     options={"maxiter": 6000, "xtol": 1e-3, "ftol": 1e-3})
        vec = r.x
    except TimeoutError:
        return None, evals[0]
    k = Kernel.from_vector(vec, nharm, nband)
    k.dur = dur
    return (k, r.fun), evals[0]


def solve_volume(k, mono, nsamples):
    """The metric is peak-normalized, so loudness has to be matched here.
    The engine clamps preset vol through AppliedFXVol to [0,1], so this is a
    ratio capped at unity."""
    y = k.render(nsamples, seed=11)
    yp = np.abs(y).max()
    if yp < 1e-9:
        return 1.0
    # peak-normalized mono was already divided by its own peak, so the source
    # peak is 1.0 and vol is simply the inverse of the render's peak
    return float(min(1.0, 1.0 / yp))


def rescore(k, mono, nps, hop, idx, target, seeds=(11, 12, 13)):
    """Held-out seeds.  The fit itself optimizes one noise realization, so the
    fit-time score is optimistic; this is the number to trust."""
    n = len(mono)
    acc = []
    for s in seeds:
        y = k.render(n, seed=s)
        peak = np.abs(y).max()
        if peak < 1e-7 or (float((y * y).sum()) / (n * peak * peak)) < 1e-6:
            acc.append(1e6)
            continue
        acc.append(band_distance(target, band_spectrum(y / peak, nps, hop, idx)))
    return float(np.mean(acc)) if acc else 1e6


def prune(k, mono, nps, hop, idx, target):
    """Drop components that do not measurably contribute, then compact.

    The optimizer will happily keep a tone stack whose harmonics have decayed
    to nothing if it started there, so the emitted preset would carry dead
    fields.  Only a removal that does not worsen the held-out score is kept, so
    pruning can never cost fidelity.  This is what keeps the number of sines
    and noise generators minimal."""
    import copy as _copy
    base = rescore(k, mono, nps, hop, idx, target)
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
            s = rescore(t, mono, nps, hop, idx, target)
            if s <= base + tol:
                k, base, changed = t, min(s, base), True
                break
    # compact: pull non-zero bands to the front
    keep = [i for i, a in enumerate(k.bAmp) if a > 1e-6]
    k.bAmp = [k.bAmp[i] for i in keep]
    k.bFC = [k.bFC[i] for i in keep]
    k.bQ = [k.bQ[i] for i in keep]
    k.bDec = [k.bDec[i] for i in keep]
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


def fit_rung(mono, nps, hop, idx, target, f0seed, fmax, dur,
             nharm, nband, odd, useAM, useDrive, deadline, x_init=None,
             max_powell=6000):
    from scipy.optimize import minimize
    n = len(mono)
    # dur and vol are pinned, not optimized: dur is the trimmed source length
    # and vol is solved analytically from the peak ratio afterwards.  Letting
    # the optimizer touch either is either meaningless or exploitable.
    bounds = _bounds(nharm, nband, fmax, useAM, useDrive)
    bounds[0] = (dur, dur)            # dur
    bounds[1] = (1.0, 1.0)            # vol placeholder, recomputed after fit
    bounds[10] = (float(nharm), float(nharm))
    bounds[11] = (float(odd), float(odd))
    if x_init is not None and len(x_init) == len(bounds):
        x0 = np.array(x_init, dtype=np.float64)
        x0 = np.array([min(max(v, b[0]), b[1]) for v, b in zip(x0, bounds)])
        x0[0] = dur
    else:
        x0 = _seed_vector(nharm, nband, f0seed, fmax, useAM, useDrive)
        x0[0] = dur
        x0[1] = 1.0
        x0[11] = float(odd)
    assert len(x0) == len(bounds), (len(x0), len(bounds))
    assert len(x0) == 12 + 4 * nband
    evals = [0]
    best = [1e6, np.array(x0)]

    def cost(v):
        if evals[0] % 32 == 0 and time.time() > deadline:
            # Out of time.  Report the best score seen so far so the line
            # search finds no improvement and unwinds cleanly; raising here
            # instead would throw away a rung's entire result.
            return best[0] + 1e-3
        evals[0] += 1
        k = Kernel.from_vector(v, nharm, nband)
        y = k.render(n, seed=0)
        peak = np.abs(y).max()
        if peak < 1e-7 or (float((y * y).sum()) / (n * peak * peak)) < 1e-6:
            return 1e6
        s = band_distance(target, band_spectrum(y / peak, nps, hop, idx))
        if s < best[0]:
            best[0], best[1] = s, np.array(v)
        return s

    r = minimize(cost, x0, method="L-BFGS-B", bounds=bounds,
                 options={"maxiter": 400, "maxfun": 6000})
    if max_powell > 0 and time.time() < deadline:
        r = minimize(cost, r.x, method="Powell", bounds=bounds,
                     options={"maxiter": max_powell, "xtol": 1e-3,
                              "ftol": 1e-3})
    vec = r.x
    if best[0] < float(r.fun):
        vec = best[1]
    k = Kernel.from_vector(vec, nharm, nband)
    k.odd = odd
    k.dur = dur
    return (k, best[0]), evals[0]


def search_ladder(mono, nps, hop, idx, target, f0seed, fmax, dur,
                  gap_gate, floor, budget, max_rung, rows_out):
    """Two phases.  A short probe of every rung ranks them, then most of the
    budget goes to the best rungs, each restarted from its own probe result.

    Splitting the budget evenly instead is what makes this fail: the complex
    rungs need more than a second to converge, so they get scored while still
    noise and the search settles on a simpler model that fits worse."""
    deadline = time.time() + budget
    rungs = [(i, r) for i, r in enumerate(LADDER) if i < max_rung]
    if not rungs:
        return None, 0, []

    # phase 1: rank every rung with a cheap gradient-only fit (no Powell)
    probe_each = max((budget * 0.30) / len(rungs), 0.8)
    probed = []
    for i, (label, nharm, nband, odd, useAM, useDrive) in rungs:
        if time.time() > deadline:
            break
        res, ev = fit_rung(mono, nps, hop, idx, target, f0seed, fmax, dur,
                           nharm, nband, odd, useAM, useDrive,
                           min(deadline, time.time() + probe_each),
                           max_powell=0)
        if res is None:
            continue
        k, _ = res
        k.vol = solve_volume(k, mono, len(mono))
        k, sc = prune(k, mono, nps, hop, idx, target)
        probed.append((sc, i, (label, nharm, nband, odd, useAM, useDrive),
                       k.to_vector()))
    if not probed:
        return None, 0, []

    probed.sort(key=lambda r: r[0])
    best_score, best_i, best_spec, best_vec = probed[0]
    if best_score - floor <= gap_gate:
        return (best_i, best_vec), 0, probed

    # phase 2: refine the strongest few, budget-weighted toward the complex end
    finalists = probed[:min(3, len(probed))]
    left = max(deadline - time.time(), 0.5)
    for j, (sc, i, spec, vec) in enumerate(finalists):
        if time.time() > deadline:
            break
        label, nharm, nband, odd, useAM, useDrive = spec
        share = left / (len(finalists) - j)
        res, ev = fit_rung(mono, nps, hop, idx, target, f0seed, fmax, dur,
                           nharm, nband, odd, useAM, useDrive,
                           min(deadline, time.time() + share),
                           x_init=pad_vector(vec, nband))
        if res is None:
            continue
        k, _ = res
        k.vol = solve_volume(k, mono, len(mono))
        k, s2 = prune(k, mono, nps, hop, idx, target)
        if s2 < best_score:
            best_score, best_i, best_vec = s2, i, k.to_vector()
    return (best_i, best_vec), 0, probed


def _vec_nband(vec):
    """A vector's length determines how many bands it actually carries --
    pruning can strip a rung down to none, so the rung's nominal width is not
    a reliable guide."""
    return max(0, (len(np.asarray(vec)) - 12) // 4)


def pad_vector(vec, nband):
    """Grow a pruned vector back out to a rung's nominal band count with
    neutral bands, so a phase-2 refinement can restart from the probe's
    result instead of falling back to a cold seed."""
    v = list(np.asarray(vec, dtype=np.float64))
    have = _vec_nband(v)
    for i in range(have, nband):
        v += [0.0, 1000.0, 3.0, 10.0]
    return np.array(v[:12 + 4 * nband], dtype=np.float64)


def _rebuild(vec, nharm, odd, dur):
    nband = _vec_nband(vec)
    k = Kernel.from_vector(np.asarray(vec), nharm, nband)
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


def emit_c(k, path, name, floor, dist, gap, tier, nps, evals, seconds):
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
    ]
    ident = f" /* {name} */" if name else ""
    print(f"/* {path} nps={nps} floor={floor:.2f} dist={dist:.2f} "
          f"gap={gap:+.2f} nHarm={k.nHarm} nBand={k.nBand} "
          f"tier={tier} evals={evals} t={seconds:.1f}s PASS */")
    print("{" + ", ".join(fields) + "}" + ident)


def emit_reject(path, name, floor, dist, gap, tier, nps, reason, seconds):
    print(f"/* {path} nps={nps} floor={floor:.2f} dist={dist:.2f} "
          f"gap={gap:+.2f} tier={tier} t={seconds:.1f}s REJECT {reason} */")


def emit_tsv(rec):
    keys = ("path", "name", "tier", "status", "nps", "dur", "floor", "dist",
            "gap", "nHarm", "nBand", "odd", "tilt", "drive", "evals", "seconds")
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
            ",".join(str(v) for v in k.bDec), "0"]
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
    ap.add_argument("--budget", type=float, default=45.0,
                    help="seconds of optimization for this file (default 45)")
    ap.add_argument("--tier", choices=("strict", "loose"), default="strict",
                    help="fidelity tier: strict=gate 3.0 dB, loose=8.0 dB")
    ap.add_argument("--gap", type=float, default=None,
                    help="override the tier's gap gate, in dB")
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
    ap.add_argument("--duration-cap", type=float, default=2.5,
                    help="truncate analysis at this many seconds")
    args = ap.parse_args(argv)

    started = time.time()
    gap_gate = args.gap if args.gap is not None else (3.0 if args.tier == "strict" else 8.0)

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
    nps = args.nps or adaptive_nps(len(mono), dur)
    hop = max(1, nps // 4)
    edges, idx = make_band_grid(fmax, nps)
    target = band_spectrum(mono, nps, hop, idx)
    floor = estimate_floor(mono, nps, hop, idx)
    f0seed = dominant_freq(mono, nps, hop)

    name = args.name or os.path.splitext(os.path.basename(args.wav))[0]
    rows = []
    best = None
    evals_total = 0
    deadline = started + args.budget

    pick, _, probed = search_ladder(mono, nps, hop, idx, target, f0seed,
                                    fmax, dur, gap_gate, floor, args.budget,
                                    args.max_rung, rows)
    rows = [{"label": spec[0], "dist": sc, "gap": sc - floor,
             "nharm": spec[1], "nband": spec[2], "evals": 0}
            for sc, _i, spec, _v in probed]
    best = None
    if pick is not None:
        bi, bvec = pick
        k = _rebuild(bvec, LADDER[bi][1], LADDER[bi][2], dur)
        k.vol = solve_volume(k, mono, len(mono))
        k, _ = prune(k, mono, nps, hop, idx, target)
        k.vol = solve_volume(k, mono, len(mono))
        best = (k, rescore(k, mono, nps, hop, idx, target))

    seconds = time.time() - started
    if args.ladder:
        emit_ladder(rows)

    if best is None:
        rec = {"path": args.wav, "name": name, "tier": args.tier,
               "status": "reject", "nps": nps, "dur": round(dur, 4),
               "floor": round(floor, 3), "dist": None, "gap": None,
               "reason": "budget", "evals": evals_total,
               "seconds": round(seconds, 2)}
        if args.format == "c":
            emit_reject(args.wav, name, floor, 99.0, 99.0, args.tier,
                        nps, "budget", seconds)
        elif args.format == "tsv":
            emit_tsv(rec)
        else:
            emit_json(rec)
        return 1

    k, dist = best
    gap = dist - floor
    status = "pass" if gap <= gap_gate else "reject"
    rec = {"path": args.wav, "name": name, "tier": args.tier,
           "status": status, "nps": nps, "dur": round(dur, 4),
           "floor": round(floor, 3), "dist": round(dist, 3),
           "gap": round(gap, 3), "nHarm": k.nHarm, "nBand": k.nBand,
           "odd": k.odd, "tilt": round(k.tilt, 4),
           "drive": round(k.drive, 3), "evals": evals_total,
           "seconds": round(seconds, 2)}

    if args.ab:
        fitted = render_via_c(k, len(mono))
        if fitted is None:
            fitted = k.render(len(mono), seed=11)
        write_ab(args.ab, mono, fitted, sr)

    if args.format == "c":
        if status == "pass":
            emit_c(k, args.wav, name, floor, dist, gap, args.tier,
                   nps, evals_total, seconds)
        else:
            emit_reject(args.wav, name, floor, dist, gap, args.tier,
                        nps, f"gap>{gap_gate}", seconds)
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