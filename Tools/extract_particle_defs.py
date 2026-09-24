#!/usr/bin/env python3
"""Extract Unity ParticleSystem prefab data and cross-reference Voxen texture indices.

This intentionally uses only the Python standard library. Unity YAML is not a
general YAML stream in practice: the component documents and their indentation
are stable, while the Unity-specific tags make generic YAML loaders a poor fit.

Example:
    python3 Tools/extract_particle_defs.py \
        --citadel ../Citadel \
        --voxen . \
        --csv Tools/particle_defs.csv \
        --json Tools/particle_defs.json \
        --markdown Tools/particle_migration_plan.md
"""

from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path
from typing import Iterable


DOC_RE = re.compile(r"(?m)^--- !u!(\d+) &([-\d]+)\n")
NUMBER = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?"


def docs(text: str) -> list[tuple[str, str, str]]:
    matches = list(DOC_RE.finditer(text))
    out = []
    for i, match in enumerate(matches):
        end = matches[i + 1].start() if i + 1 < len(matches) else len(text)
        out.append((match.group(1), match.group(2), text[match.end():end]))
    return out


def direct_value(block: str, key: str, indent: int = 2) -> str:
    pattern = rf"(?m)^[ ]{{{indent}}}{re.escape(key)}:\s*(.*)$"
    match = re.search(pattern, block)
    if not match:
        # Child blocks retain their source indentation. Callers should prefer
        # an exact indent, but falling back makes scalar extraction resilient
        # when a nested Unity version changes one indentation level.
        match = re.search(rf"(?m)^[ ]+{re.escape(key)}:\s*(.*)$", block)
    return match.group(1).strip() if match else ""


def child_block(block: str, key: str, indent: int = 2) -> str:
    """Return the indented contents of a direct mapping key."""
    pattern = rf"(?m)^([ ]{{{indent}}}){re.escape(key)}:[^\n]*\n?"
    match = re.search(pattern, block)
    actual_indent = indent
    if not match:
        pattern = rf"(?m)^([ ]+){re.escape(key)}:[^\n]*\n?"
        match = re.search(pattern, block)
        if match:
            actual_indent = len(match.group(1))
    if not match:
        return ""
    start = match.end()
    lines = block[start:].splitlines(True)
    kept: list[str] = []
    for line in lines:
        line_indent = len(line) - len(line.lstrip(" "))
        if line.strip() and line_indent <= actual_indent and not line.lstrip().startswith("- "):
            break
        kept.append(line)
    return "".join(kept)


def all_numbers(value: str) -> list[float]:
    return [float(x) for x in re.findall(NUMBER, value)]


def first_number(value: str, default: float = 0.0) -> float:
    numbers = all_numbers(value)
    return numbers[0] if numbers else default


def file_id(value: str) -> str:
    match = re.search(r"fileID:\s*([-\d]+)", value)
    return match.group(1) if match else value


def inline_color(value: str) -> str:
    components = color_components(value)
    return "(" + ",".join(components) + ")" if components else ""


def color_components(value: str) -> tuple[str, str, str, str] | None:
    match = re.search(
        rf"\{{r:\s*({NUMBER}),\s*g:\s*({NUMBER}),\s*b:\s*({NUMBER}),\s*a:\s*({NUMBER})\}}",
        value,
    )
    if not match:
        return None
    return match.groups()


def fmt_number(value: float) -> str:
    if abs(value - round(value)) < 1e-6:
        return str(int(round(value)))
    return f"{value:.5g}"


def fmt_range(values: tuple[float, float] | None) -> str:
    if values is None:
        return ""
    return f"{fmt_number(values[0])}..{fmt_number(values[1])}"


def curve_keys(block: str) -> list[tuple[float, float]]:
    curve = child_block(block, "m_Curve", 6)
    keys = []
    for match in re.finditer(
        rf"(?m)^[ ]+- serializedVersion:.*?^[ ]+time:\s*({NUMBER}).*?^[ ]+value:\s*({NUMBER})",
        curve,
        re.S,
    ):
        keys.append((float(match.group(1)), float(match.group(2))))
    return keys


def curve_summary(block: str, enabled: bool = True) -> str:
    if not enabled or not block:
        return "off"
    state = int(first_number(direct_value(block, "minMaxState", 4), 0))
    scalar = first_number(direct_value(block, "scalar", 4), 0)
    minimum = first_number(direct_value(block, "minScalar", 4), 0)
    if state == 0:
        return fmt_number(scalar)
    if state == 3:
        return fmt_range((minimum, scalar))
    keys = curve_keys(child_block(block, "maxCurve", 4))
    if state == 2:
        low = curve_keys(child_block(block, "minCurve", 4))
        return "max[" + ";".join(f"{fmt_number(t)}:{fmt_number(v)}" for t, v in keys) + \
            "] min[" + ";".join(f"{fmt_number(t)}:{fmt_number(v)}" for t, v in low) + "]"
    return "curve[" + ";".join(
        f"{fmt_number(t)}:{fmt_number(v * scalar)}" for t, v in keys
    ) + "]"


def direct_enabled(block: str, indent: int = 2) -> bool:
    return first_number(direct_value(block, "enabled", indent), 0) != 0


def module(psys: str, name: str) -> str:
    return child_block(psys, name, 2)


def module_property(psys: str, module_name: str, property_name: str) -> str:
    return child_block(module(psys, module_name), property_name, 4)


def extract_modules(psys: str) -> dict[str, str]:
    initial = module(psys, "InitialModule")
    emission = module(psys, "EmissionModule")
    shape = module(psys, "ShapeModule")
    color = module(psys, "ColorModule")
    size = module(psys, "SizeModule")
    rotation = module(psys, "RotationModule")
    velocity = module(psys, "VelocityModule")
    uv = module(psys, "UVModule")
    trail = module(psys, "TrailModule")

    start_lifetime = child_block(initial, "startLifetime", 4)
    start_speed = child_block(initial, "startSpeed", 4)
    start_size = child_block(initial, "startSize", 4) or child_block(initial, "startSize3D", 4)
    gravity = child_block(initial, "gravityModifier", 4)
    rate = child_block(emission, "rateOverTime", 4)

    size_curve = child_block(size, "curve", 4)
    rotation_curve = child_block(rotation, "z", 4) or child_block(rotation, "x", 4)
    velocity_x = child_block(velocity, "x", 4)
    velocity_y = child_block(velocity, "y", 4)
    velocity_z = child_block(velocity, "z", 4)
    trail_lifetime = child_block(trail, "lifetime", 4)
    trail_width = child_block(trail, "widthOverTrail", 4)
    unity_shape_type = int(first_number(direct_value(shape, "type", 4), -1))
    shape_radius = first_number(direct_value(shape, "radius", 4), 0.0)
    shape_angle = first_number(direct_value(shape, "angle", 4), 0.0)
    if not (enabled_shape := direct_enabled(shape)):
        shape_type, shape_name = 0, "point"
    elif unity_shape_type in (0, 1, 2, 3):
        shape_type, shape_name = 1, "sphere"
    elif unity_shape_type == 4:
        shape_type, shape_name = 2, "cone"
    else:
        shape_type, shape_name = 0, f"point(unity:{unity_shape_type})"

    enabled = {
        "color": direct_enabled(color),
        "size": direct_enabled(size),
        "rotation": direct_enabled(rotation),
        "velocity": direct_enabled(velocity),
        "trail": direct_enabled(trail),
        "uv": direct_enabled(uv),
        "shape": enabled_shape,
    }
    color_gradient = child_block(color, "gradient", 4)
    gradient = child_block(color_gradient, "maxGradient", 6)
    colors = []
    color_count = int(first_number(direct_value(gradient, "m_NumColorKeys", 8), 0))
    alpha_count = int(first_number(direct_value(gradient, "m_NumAlphaKeys", 8), 0))
    for i in range(max(color_count, alpha_count)):
        color = inline_color(direct_value(gradient, f"key{i}", 8))
        ctime = first_number(direct_value(gradient, f"ctime{i}", 8), 0) / 65535.0
        atime = first_number(direct_value(gradient, f"atime{i}", 8), 0) / 65535.0
        if i < color_count:
            colors.append(f"c{fmt_number(ctime)}:{color}")
        if i < alpha_count:
            components = color_components(direct_value(gradient, f"key{i}", 8))
            colors.append(f"a{fmt_number(atime)}:{components[3] if components else ''}")

    flags = []
    if enabled["shape"]:
        flags.append(f"shape:{shape_name}")
    if enabled["uv"]:
        tiles_x = int(first_number(direct_value(uv, "tilesX", 4), 1))
        tiles_y = int(first_number(direct_value(uv, "tilesY", 4), 1))
        flags.append(f"flipbook:{tiles_x}x{tiles_y}")
    if enabled["trail"]:
        flags.append("trail")

    return {
        "duration": direct_value(psys, "lengthInSec"),
        "looping": direct_value(psys, "looping"),
        "lifetime": curve_summary(start_lifetime),
        "speed": curve_summary(start_speed),
        "size": curve_summary(start_size),
        "gravity": curve_summary(gravity),
        "shape_type": str(shape_type),
        "shape": shape_name,
        "shape_radius": fmt_number(shape_radius),
        "shape_angle": fmt_number(shape_angle),
        "emit_rate": curve_summary(rate, direct_enabled(emission)),
        "color_over_lifetime": ";".join(colors) if enabled["color"] else "off",
        "size_over_lifetime": curve_summary(size_curve, enabled["size"]),
        "rotation_over_lifetime": curve_summary(rotation_curve, enabled["rotation"]),
        "velocity_over_lifetime": ";".join(
            f"{axis}:{curve_summary(curve, enabled['velocity'])}"
            for axis, curve in (("x", velocity_x), ("y", velocity_y), ("z", velocity_z))
        ) if enabled["velocity"] else "off",
        "texture_sheet": (
            f"tiles={direct_value(uv, 'tilesX', 4)}x{direct_value(uv, 'tilesY', 4)} "
            f"frame={curve_summary(child_block(uv, 'frameOverTime', 4), enabled['uv'])}"
        ) if enabled["uv"] else "off",
        "trail_lifetime": curve_summary(trail_lifetime, enabled["trail"]),
        "trail_width": curve_summary(trail_width, enabled["trail"]),
        "features": ",".join(flags),
        "unsupported": "; ".join(
            x for x in (
                "shape distribution/rotation" if enabled["shape"] else "",
                f"unsupported Unity shape type {unity_shape_type}" if enabled["shape"] and shape_type == 0 else "",
                "startSize3D axes" if "startSize3D" in initial and "startSize:" not in initial else "",
                "separate color/alpha key timing" if enabled["color"] and alpha_count != color_count else "",
                "Unity sub-emitters" if direct_enabled(module(psys, "SubModule")) else "",
            ) if x
        ),
    }


def read_guid(path: Path) -> str:
    match = re.search(r"(?m)^guid:\s*([0-9a-f]+)", path.read_text(errors="replace"))
    return match.group(1) if match else ""


def guid_maps(citadel: Path) -> tuple[dict[str, str], dict[str, str]]:
    textures: dict[str, str] = {}
    materials: dict[str, str] = {}
    for meta in (citadel / "Assets").rglob("*.meta"):
        guid = read_guid(meta)
        if not guid:
            continue
        asset = meta.with_suffix("")
        rel = asset.relative_to(citadel).as_posix()
        if asset.suffix.lower() == ".mat":
            materials[guid] = rel
        elif asset.suffix.lower() in {".png", ".jpg", ".jpeg", ".tga", ".psd"}:
            textures[guid] = rel
    return textures, materials


def material_textures(citadel: Path, materials: dict[str, str]) -> dict[str, str]:
    out: dict[str, str] = {}
    for guid, rel in materials.items():
        text = (citadel / rel).read_text(errors="replace")
        match = re.search(
            r"_MainTex:\n\s+m_Texture:\s*\{fileID:\s*2800000,\s*guid:\s*([0-9a-f]+)",
            text,
        )
        if match:
            out[guid] = match.group(1)
        elif re.search(r"_MainTex:\n\s+m_Texture:\s*\{fileID:\s*0", text):
            out[guid] = ""
    return out


def voxen_texture_indices(voxen: Path) -> dict[str, str]:
    out: dict[str, str] = {}
    current = ""
    for line in (voxen / "Data/textures.txt").read_text(errors="replace").splitlines():
        if line.startswith("#") and not line.startswith("//"):
            current = line[1:]
        elif line.startswith("index:") and current:
            out[current] = line.split(":", 1)[1].strip()
            current = ""
    return out


def renderer_materials(renderer: str) -> list[str]:
    block = child_block(renderer, "m_Materials", 2)
    return re.findall(r"guid:\s*([0-9a-f]+)", block)


def voxen_texture_key(asset_path: str) -> str:
    if not asset_path.startswith("Assets/"):
        return ""
    relative = asset_path[len("Assets/"):]
    if relative.startswith("Textures/"):
        return relative
    return "Textures/" + relative


def voxen_indices_for_asset(asset_path: str, vox_indices: dict[str, str]) -> list[str]:
    """Return existing catalog indices, including intentional migration aliases."""
    if asset_path.endswith("ParticleCloudWhiteReduced.png"):
        index = vox_indices.get("Textures/ParticleCloudWhite.png")
        return [index] if index else ["?"]
    if asset_path.endswith("Sprites/explosion1/explosion1.psd"):
        # Unity's PSD is represented by the already-indexed exported frames.
        return [vox_indices.get(f"Textures/Sprites/explosion1/546_133{i}.png", "?")
                for i in range(5, 9)]
    key = voxen_texture_key(asset_path)
    return [vox_indices.get(key, "?")] if key else ["?"]


def parse_prefab(path: Path, citadel: Path, texture_guid: dict[str, str],
                 material_guid: dict[str, str], material_texture: dict[str, str],
                 vox_indices: dict[str, str]) -> list[dict[str, str]]:
    parsed = docs(path.read_text(errors="replace"))
    names: dict[str, str] = {}
    particle_docs: list[tuple[str, str]] = []
    renderer_by_go: dict[str, str] = {}
    for kind, ident, body in parsed:
        if kind == "1":
            name = direct_value(body, "m_Name")
            names[ident] = name
        elif kind == "198" and body.startswith("ParticleSystem:"):
            particle_docs.append((ident, body))
        elif kind == "199" and body.startswith("ParticleSystemRenderer:"):
            go = file_id(direct_value(body, "m_GameObject"))
            renderer_by_go[go] = body

    rows = []
    for ident, psys in particle_docs:
        go = file_id(direct_value(psys, "m_GameObject"))
        renderer = renderer_by_go.get(go, "")
        material_guids = renderer_materials(renderer)
        material_names = []
        texture_names = []
        texture_indices = []
        for material in material_guids:
            mat_rel = material_guid.get(material, "")
            mat_name = Path(mat_rel).stem if mat_rel else f"guid:{material}"
            material_names.append(mat_name)
            tex = material_texture.get(material, "")
            tex_rel = texture_guid.get(tex, "") if tex else ""
            tex_name = Path(tex_rel).name if tex_rel else ("<none>" if not tex else f"guid:{tex}")
            texture_names.append(tex_name)
            if tex_rel.startswith("Assets/"):
                texture_indices.extend(voxen_indices_for_asset(tex_rel, vox_indices))
            else:
                texture_indices.append("?")
        data = extract_modules(psys)
        data.update({
            "prefab": str(path.relative_to(citadel)).replace("\\", "/"),
            "game_object": names.get(go, f"go:{go}"),
            "psys_id": ident,
            "materials": "|".join(material_names),
            "textures": "|".join(texture_names),
            "voxen_texture_indices": "|".join(texture_indices),
            "renderer_mode": direct_value(renderer, "m_RenderMode") or "?",
            "renderer_length_scale": direct_value(renderer, "m_LengthScale") or "?",
        })
        rows.append(data)
    return rows


FIELD_ROWS = [
    ("PSysDef.textures[16]", "ParticleSystemRenderer.m_Materials -> Material._MainTex", "material GUID -> texture .meta GUID -> Voxen Data/textures.txt index", "manual review for missing/non-PNG assets; animated sheets become consecutive texture slots"),
    ("emitRate", "EmissionModule.rateOverTime", "Constant -> value; TwoConstants -> min/max currently needs emitter support; curves -> sampled 32-key emissionCurve only when duration is finite", "rateOverDistance and bursts need separate handling"),
    ("duration", "ParticleSystem.lengthInSec", "copy seconds; looping systems use a long-running duration or require restart semantics", "Unity looping/prewarm/stopAction are not represented"),
    ("lifetimeMin/lifetimeMax", "InitialModule.startLifetime", "Constant/TwoConstants map directly; curves need approximation or spawn-time sampling", "PSysDef has no lifetime curve"),
    ("speedMin/speedMax", "InitialModule.startSpeed", "Constant/TwoConstants map directly", "shape direction/distribution and 3D velocity are not represented"),
    ("shapeType/shapeRadius/shapeAngle", "ShapeModule.type/radius/angle", "Unity sphere types -> sphere volume; cone -> directional cone in +Y; unsupported types fall back to point", "local shape rotation and exact cone placement are intentionally deferred"),
    ("sizeMin/sizeMax", "InitialModule.startSize", "Constant/TwoConstants map directly", "startSize3D axes are not represented"),
    ("gravity", "InitialModule.gravityModifier", "map scalar to downward Y acceleration", "Unity gravity source and simulation-space behavior need verification"),
    ("rampColors/rampTimes", "ColorModule.gradient", "convert color and alpha keys to a unified sorted ramp; cap at 16 keys", "Unity stores color and alpha key timing separately"),
    ("scaleKeys/scaleTimes", "SizeModule.curve", "map the active curve to the 16-key PSysDef curve", "Unity supports separate XYZ curves"),
    ("velKeys/velTimes", "VelocityModule.x/y/z and speedModifier", "map the usable scalar curve to velocityCurve", "Voxen currently applies one scalar velocity curve, not vector acceleration"),
    ("rotKeys/rotTimes", "RotationModule", "map scalar rotation curve; radians conversion required if the Unity value is degrees", "Voxen currently uses rotKeys[0] as angular velocity, not a full rotation curve"),
    ("emissKeys/emissTimes", "EmissionModule curve", "sample finite-duration emission modulation", "not a general Unity burst/sub-emitter representation"),
    ("animWindow", "UVModule.frameOverTime + tilesX/tilesY", "flatten sheet frames into texture slots; map normalized frame timing to animWindow", "frame selection/order and atlas addressing need a renderer decision"),
    ("softness", "Material shader / Renderer soft particles", "derive from material soft-particle settings where applicable", "Unity material softness is not a direct PSysDef field"),
    ("trail*", "TrailModule", "trail enabled/lifetime/width/colors/texture map where a trail material has a Voxen texture", "Unity ribbon topology, min vertex distance, and trail gradient are richer than PSysDef"),
]


def write_csv(path: Path, rows: list[dict[str, str]]) -> None:
    columns = [
        "prefab", "game_object", "psys_id", "duration", "looping", "emit_rate",
        "lifetime", "speed", "size", "gravity", "shape", "shape_type",
        "shape_radius", "shape_angle", "color_over_lifetime",
        "size_over_lifetime", "velocity_over_lifetime", "rotation_over_lifetime",
        "texture_sheet", "materials", "textures", "voxen_texture_indices",
        "renderer_mode", "renderer_length_scale", "trail_lifetime", "trail_width",
        "features", "unsupported",
    ]
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=columns)
        writer.writeheader()
        writer.writerows({column: row.get(column, "") for column in columns} for row in rows)


def write_markdown(path: Path, rows: list[dict[str, str]], texture_guid: dict[str, str],
                   material_guid: dict[str, str], material_texture: dict[str, str],
                   vox_indices: dict[str, str]) -> None:
    prefabs = len({row["prefab"] for row in rows})
    missing_rows = sum("?" in row["voxen_texture_indices"] for row in rows)
    unresolved_texture_guids = sum("guid:" in row["textures"] for row in rows)
    active = {
        "ColorModule": sum(row["color_over_lifetime"] != "off" for row in rows),
        "SizeModule": sum(row["size_over_lifetime"] != "off" for row in rows),
        "VelocityModule": sum(row["velocity_over_lifetime"] != "off" for row in rows),
        "RotationModule": sum(row["rotation_over_lifetime"] != "off" for row in rows),
        "UVModule": sum(row["texture_sheet"] != "off" for row in rows),
        "TrailModule": sum("trail" in row["features"].split(",") for row in rows),
        "ShapeModule": sum(any(feature.startswith("shape:") for feature in row["features"].split(",")) for row in rows),
    }
    lines = [
        "# Unity ParticleSystem -> Voxen Psys migration plan",
        "",
        f"Generated from `{prefabs}` Unity prefabs and `{len(rows)}` ParticleSystem components.",
        "The CSV beside this document is the machine-readable per-component extraction.",
        "",
        "## Scope and order",
        "",
        "1. Import the 51 referenced particle materials/textures first and preserve their existing Voxen texture indices.",
        "2. Convert the standalone `ef_*` effect prefabs into named `PSysDef` records.",
        "3. Convert particle systems embedded in NPC/projectile prefabs, retaining one row per Unity GameObject.",
        "4. Add a data-driven effect lookup so entity/projectile code requests an effect name rather than embedding `PSysDef` literals.",
        "5. Validate representative effects in-engine: muzzle flash, sparks/trail, blood, smoke/steam, fireball, water spray, and an animated sheet.",
        "",
        "## Voxen field cross-reference",
        "",
        "| Voxen field | Unity source | Conversion | Review/gap |",
        "|---|---|---|---|",
    ]
    for row in FIELD_ROWS:
        lines.append("| " + " | ".join(row) + " |")
    lines += [
        "",
        "## Texture index rule",
        "",
        "Material `_MainTex` is resolved through the Unity material and texture meta GUIDs, then looked up in Voxen `Data/textures.txt`. `ParticleCloudWhiteReduced.png` intentionally aliases the existing `ParticleCloudWhite.png` entry. Unity's `explosion1.psd` intentionally expands to the already-indexed `546_1335.png` through `546_1338.png` frames. `?` means the source asset is missing from Voxen's texture catalog or is not a direct texture file.",
        "",
        "| Voxen index | Texture |",
        "|---:|---|",
    ]
    seen: set[str] = set()
    for row in rows:
        for tex, index in zip(row["textures"].split("|"), row["voxen_texture_indices"].split("|")):
            key = f"{index}|{tex}"
            if tex and key not in seen:
                seen.add(key)
                lines.append(f"| {index} | {tex} |")
    lines += [
        "",
        "## Review gates",
        "",
        "- Do not collapse Unity `ColorModule` alpha keys into color keys without preserving alpha timing.",
        "- Keep `TwoConstants`, finite curves, bursts, and sub-emitters marked until Voxen has equivalent runtime behavior.",
        "- Confirm Unity-to-Voxen world scale and degrees-to-radians behavior against one visual reference effect before bulk conversion.",
        "- Treat material blend mode as authoritative for additive/multiply/alpha behavior; the current Voxen runtime derives blend mode from the texture catalog.",
        "",
        "## Extraction validation",
        "",
        "- Active module coverage: " + ", ".join(
            f"{name}={count}" for name, count in active.items()
        ) + ".",
        f"- Rows with at least one unresolved Voxen texture slot: `{missing_rows}`.",
        f"- Rows still carrying an unresolved texture GUID: `{unresolved_texture_guids}`.",
        f"- Material GUIDs resolved: `{len(material_guid)}`; Unity texture GUIDs resolved: `{len(texture_guid)}`; Voxen catalog entries: `{len(vox_indices)}`.",
    ]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--citadel", type=Path, required=True)
    parser.add_argument("--voxen", type=Path, required=True)
    parser.add_argument("--csv", type=Path, required=True)
    parser.add_argument("--json", type=Path, required=True)
    parser.add_argument("--markdown", type=Path, required=True)
    args = parser.parse_args()

    texture_guid, material_guid = guid_maps(args.citadel)
    material_texture = material_textures(args.citadel, material_guid)
    vox_indices = voxen_texture_indices(args.voxen)
    rows: list[dict[str, str]] = []
    for prefab in sorted((args.citadel / "Assets/Resources/Prefabs").glob("*.prefab")):
        rows.extend(parse_prefab(
            prefab, args.citadel, texture_guid, material_guid, material_texture, vox_indices
        ))
    args.csv.parent.mkdir(parents=True, exist_ok=True)
    args.json.parent.mkdir(parents=True, exist_ok=True)
    args.markdown.parent.mkdir(parents=True, exist_ok=True)
    write_csv(args.csv, rows)
    args.json.write_text(json.dumps(rows, indent=2) + "\n", encoding="utf-8")
    write_markdown(args.markdown, rows, texture_guid, material_guid, material_texture, vox_indices)
    print(f"extracted {len(rows)} ParticleSystem components from {len({r['prefab'] for r in rows})} prefabs")
    print(f"wrote {args.csv}")
    print(f"wrote {args.json}")
    print(f"wrote {args.markdown}")


if __name__ == "__main__":
    main()