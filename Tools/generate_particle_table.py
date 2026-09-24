#!/usr/bin/env python3
"""Generate the compiled ParticleTypeDef table in particles.c from extractor JSON."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


NUMBER = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"
PAIR_RE = re.compile(rf"({NUMBER}):({NUMBER})")
COLOR_RE = re.compile(rf"c({NUMBER}):\(({NUMBER}),({NUMBER}),({NUMBER}),({NUMBER})\)")
ALPHA_RE = re.compile(rf"a({NUMBER}):({NUMBER})")


def cfloat(value: float) -> str:
    if abs(value) < 0.0000005:
        value = 0.0
    text = f"{value:.7g}"
    if text == "-0":
        text = "0"
    if "e" not in text and "." not in text:
        text += ".0"
    return text + "f"


def cstr(value: str) -> str:
    return json.dumps(value, ensure_ascii=True)


def pairs(value: str) -> list[tuple[float, float]]:
    return [(float(t), float(v)) for t, v in PAIR_RE.findall(value)]


def curve(value: str) -> list[tuple[float, float]]:
    if value.startswith("curve[") and value.endswith("]"):
        return pairs(value[6:-1])
    if value in ("", "off"):
        return []
    try:
        scalar = float(value)
    except ValueError:
        return []
    return [(0.0, scalar), (1.0, scalar)]


def reduce_keys(keys: list[tuple[float, float]], limit: int = 16) -> list[tuple[float, float]]:
    if len(keys) <= limit:
        return keys
    return [keys[round(i * (len(keys) - 1) / (limit - 1))] for i in range(limit)]


def range_bounds(value: str) -> tuple[float, float]:
    if value in ("", "off"):
        return 0.0, 0.0
    if value.startswith("curve["):
        values = [v for _, v in curve(value)]
        return (min(values), max(values)) if values else (0.0, 0.0)
    if ".." in value:
        low, high = value.split("..", 1)
        return float(low), float(high)
    try:
        scalar = float(value)
    except ValueError:
        return 0.0, 0.0
    return scalar, scalar


def average_bounds(value: str) -> float:
    low, high = range_bounds(value)
    return (low + high) * 0.5


def array_initializer(keys: list[tuple[float, float]], prefix: str) -> str:
    keys = reduce_keys(keys)
    if not keys:
        return ""
    values = ",".join(cfloat(v) for _, v in keys)
    times = ",".join(cfloat(t) for t, _ in keys)
    count = len(keys)
    return f".{prefix}Keys={{{values}}},.{prefix}Times={{{times}}},.{prefix}Count={count}"


def interpolate(keys: list[tuple[float, float]], time: float, fallback: float) -> float:
    if not keys:
        return fallback
    if time <= keys[0][0]:
        return keys[0][1]
    for (t0, v0), (t1, v1) in zip(keys, keys[1:]):
        if time <= t1:
            if t1 == t0:
                return v1
            return v0 + (v1 - v0) * ((time - t0) / (t1 - t0))
    return keys[-1][1]


def interpolate_color(keys: list[tuple[float, tuple[float, float, float, float]]],
                      time: float, fallback: tuple[float, float, float, float]) -> tuple[float, float, float, float]:
    if not keys:
        return fallback
    if time <= keys[0][0]:
        return keys[0][1]
    for (t0, v0), (t1, v1) in zip(keys, keys[1:]):
        if time <= t1:
            if t1 == t0:
                return v1
            amount = (time - t0) / (t1 - t0)
            return tuple(a + (b - a) * amount for a, b in zip(v0, v1))
    return keys[-1][1]


def gradient(value: str) -> list[tuple[float, tuple[float, float, float, float]]]:
    if value in ("", "off"):
        return []
    colors = [(float(t), (float(r), float(g), float(b), float(a)))
              for t, r, g, b, a in COLOR_RE.findall(value)]
    alphas = [(float(t), float(a)) for t, a in ALPHA_RE.findall(value)]
    times = sorted({t for t, _ in colors} | {t for t, _ in alphas})
    if not times:
        return []
    default = colors[0][1] if colors else (1.0, 1.0, 1.0, 1.0)
    return [(t, (interpolate_color(colors, t, default)[0],
                 interpolate_color(colors, t, default)[1],
                 interpolate_color(colors, t, default)[2],
                 interpolate(alphas, t, default[3])))
            for t in times[:16]]


def color_initializer(value: str) -> str:
    keys = gradient(value)
    if not keys:
        return ".colStart=(Color){1,1,1,1},.colEnd=(Color){1,1,1,1}"
    colors = ",".join(f"(Color){{{','.join(cfloat(v) for v in color)}}}" for _, color in keys)
    times = ",".join(cfloat(t) for t, _ in keys)
    first, last = keys[0][1], keys[-1][1]
    return (
        f".colStart=(Color){{{','.join(cfloat(v) for v in first)}}},"
        f".colEnd=(Color){{{','.join(cfloat(v) for v in last)}}},"
        f".rampColors={{{colors}}},.rampTimes={{{times}}},.rampCount={len(keys)}"
    )


def texture_initializer(value: str) -> tuple[str, int]:
    indices = []
    unresolved = 0
    for raw in value.split("|"):
        if not raw:
            continue
        if raw == "?":
            indices.append("MAX_TXRS")
            unresolved += 1
        else:
            indices.append(raw)
    if not indices:
        indices = ["MAX_TXRS"]
        unresolved += 1
    indices = indices[:16]
    first_valid = next((item for item in indices if item != "MAX_TXRS"), "0")
    return ",".join(indices + ["MAX_TXRS"] * (16 - len(indices))), int(first_valid)


def velocity_curve(value: str) -> list[tuple[float, float]]:
    if value in ("", "off"):
        return []
    candidates = []
    for axis in ("x", "y", "z"):
        match = re.search(rf"(?:^|;){axis}:([^;]+)", value)
        if match:
            candidate = curve(match.group(1))
            if candidate:
                candidates.append(candidate)
    if not candidates:
        return []
    # A zero Unity velocity module means "no additional velocity"; retaining the
    # runtime's default multiplier of 1 is safer than freezing the particle.
    if all(abs(v) < 0.000001 for candidate in candidates for _, v in candidate):
        return []
    return max(candidates, key=lambda candidate: max(v for _, v in candidate) - min(v for _, v in candidate))


def entry(row: dict[str, str]) -> str:
    duration = average_bounds(row["duration"])
    if row["looping"] == "1":
        duration = 1000000000.0
    emit = row["emit_rate"]
    emission_keys = curve(emit) if emit.startswith("curve[") else []
    emit_rate = 1.0 if emission_keys else average_bounds(emit)
    texture_values, trail_texture = texture_initializer(row["voxen_texture_indices"])
    lifetime_min, lifetime_max = range_bounds(row["lifetime"])
    speed_min, speed_max = range_bounds(row["speed"])
    size_min, size_max = range_bounds(row["size"])
    gravity = average_bounds(row["gravity"])
    trail_lifetime = average_bounds(row["trail_lifetime"])
    trail_width = average_bounds(row["trail_width"])
    rotation = [(t, v * 0.0174532925199433) for t, v in curve(row["rotation_over_lifetime"])]
    velocity = velocity_curve(row["velocity_over_lifetime"])
    anim_window = 1.0
    if row["texture_sheet"] != "off":
        frame = re.search(r"frame=(curve\[.*\])", row["texture_sheet"])
        if frame:
            frame_keys = curve(frame.group(1))
            if frame_keys:
                anim_window = max(t for t, _ in frame_keys) or 1.0
    fields = [
        ".pos=(V3){0,0,0}",
        f".textures={{{texture_values}}}",
        f".emitRate={cfloat(emit_rate)}",
        f".duration={cfloat(duration)}",
        f".sizeMin={cfloat(size_min)}",
        f".sizeMax={cfloat(size_max)}",
        f".speedMin={cfloat(speed_min)}",
        f".speedMax={cfloat(speed_max)}",
        f".lifetimeMin={cfloat(lifetime_min)}",
        f".lifetimeMax={cfloat(lifetime_max)}",
        f".gravity={cfloat(gravity)}",
        f".animWindow={cfloat(anim_window)}",
        ".softness=1.0f",
        f".shapeRadius={cfloat(float(row['shape_radius'] or 0))}",
        f".shapeAngle={cfloat(float(row['shape_angle'] or 0))}",
        f".shapeType={row['shape_type'] or 0}",
        color_initializer(row["color_over_lifetime"]),
    ]
    scale = curve(row["size_over_lifetime"])
    if scale:
        fields.append(array_initializer(scale, "scale"))
    if velocity:
        fields.append(array_initializer(velocity, "vel"))
    if rotation:
        fields.append(array_initializer(rotation, "rot"))
    if emission_keys:
        fields.append(array_initializer(emission_keys, "emiss"))
    if "trail" in row["features"].split(","):
        fields.extend([
            ".trail=1",
            f".trailTexture={trail_texture}",
            ".trailColorStart=(Color){1,1,1,1}",
            ".trailColorEnd=(Color){1,1,1,1}",
            f".trailLifetime={cfloat(trail_lifetime)}",
            f".trailWidthStart={cfloat(trail_width)}",
            f".trailWidthEnd={cfloat(trail_width)}",
        ])
    return (
        "    {\n"
        f"        .prefab={cstr(row['prefab'])}, .gameObject={cstr(row['game_object'])}, "
        f".sourceId={int(row['psys_id'])}ULL,\n"
        "        .def={"
        + ",".join(fields)
        + "}\n"
        "    }"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--json", type=Path, default=Path("Tools/particle_defs.json"))
    parser.add_argument("--particles", type=Path, default=Path("particles.c"))
    args = parser.parse_args()
    rows = json.loads(args.json.read_text(encoding="utf-8"))
    generated = (
        f"const ParticleTypeDef particleTypeDefs[{len(rows)}] = {{\n"
        + ",\n".join(entry(row) for row in rows)
        + "\n};\n"
        f"const u16 particleTypeDefCount = {len(rows)};\n"
        "const PSysDef* PSysTypeGet(u16 index) {\n"
        "    return index < particleTypeDefCount ? &particleTypeDefs[index].def : 0;\n"
        "}\n"
    )
    text = args.particles.read_text(encoding="utf-8")
    begin = "/* BEGIN GENERATED PARTICLE TYPE TABLE */"
    end = "/* END GENERATED PARTICLE TYPE TABLE */"
    start = text.index(begin) + len(begin)
    finish = text.index(end, start)
    args.particles.write_text(text[:start] + "\n" + generated + text[finish:], encoding="utf-8")


if __name__ == "__main__":
    main()