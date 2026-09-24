# Unity ParticleSystem -> Voxen Psys migration plan

Generated from `56` Unity prefabs and `275` ParticleSystem components.
The CSV beside this document is the machine-readable per-component extraction.

## Scope and order

1. Import the 51 referenced particle materials/textures first and preserve their existing Voxen texture indices.
2. Convert the standalone `ef_*` effect prefabs into named `PSysDef` records.
3. Convert particle systems embedded in NPC/projectile prefabs, retaining one row per Unity GameObject.
4. Add a data-driven effect lookup so entity/projectile code requests an effect name rather than embedding `PSysDef` literals.
5. Validate representative effects in-engine: muzzle flash, sparks/trail, blood, smoke/steam, fireball, water spray, and an animated sheet.

## Voxen field cross-reference

| Voxen field | Unity source | Conversion | Review/gap |
|---|---|---|---|
| PSysDef.textures[16] | ParticleSystemRenderer.m_Materials -> Material._MainTex | material GUID -> texture .meta GUID -> Voxen Data/textures.txt index | manual review for missing/non-PNG assets; animated sheets become consecutive texture slots |
| emitRate | EmissionModule.rateOverTime | Constant -> value; TwoConstants -> min/max currently needs emitter support; curves -> sampled 32-key emissionCurve only when duration is finite | rateOverDistance and bursts need separate handling |
| duration | ParticleSystem.lengthInSec | copy seconds; looping systems use a long-running duration or require restart semantics | Unity looping/prewarm/stopAction are not represented |
| lifetimeMin/lifetimeMax | InitialModule.startLifetime | Constant/TwoConstants map directly; curves need approximation or spawn-time sampling | PSysDef has no lifetime curve |
| speedMin/speedMax | InitialModule.startSpeed | Constant/TwoConstants map directly | shape direction/distribution and 3D velocity are not represented |
| shapeType/shapeRadius/shapeAngle | ShapeModule.type/radius/angle | Unity sphere types -> sphere volume; cone -> directional cone in +Y; unsupported types fall back to point | local shape rotation and exact cone placement are intentionally deferred |
| sizeMin/sizeMax | InitialModule.startSize | Constant/TwoConstants map directly | startSize3D axes are not represented |
| gravity | InitialModule.gravityModifier | map scalar to downward Y acceleration | Unity gravity source and simulation-space behavior need verification |
| rampColors/rampTimes | ColorModule.gradient | convert color and alpha keys to a unified sorted ramp; cap at 16 keys | Unity stores color and alpha key timing separately |
| scaleKeys/scaleTimes | SizeModule.curve | map the active curve to the 16-key PSysDef curve | Unity supports separate XYZ curves |
| velKeys/velTimes | VelocityModule.x/y/z and speedModifier | map the usable scalar curve to velocityCurve | Voxen currently applies one scalar velocity curve, not vector acceleration |
| rotKeys/rotTimes | RotationModule | map scalar rotation curve; radians conversion required if the Unity value is degrees | Voxen currently uses rotKeys[0] as angular velocity, not a full rotation curve |
| emissKeys/emissTimes | EmissionModule curve | sample finite-duration emission modulation | not a general Unity burst/sub-emitter representation |
| animWindow | UVModule.frameOverTime + tilesX/tilesY | flatten sheet frames into texture slots; map normalized frame timing to animWindow | frame selection/order and atlas addressing need a renderer decision |
| softness | Material shader / Renderer soft particles | derive from material soft-particle settings where applicable | Unity material softness is not a direct PSysDef field |
| trail* | TrailModule | trail enabled/lifetime/width/colors/texture map where a trail material has a Voxen texture | Unity ribbon topology, min vertex distance, and trail gradient are richer than PSysDef |

## Texture index rule

Material `_MainTex` is resolved through the Unity material and texture meta GUIDs, then looked up in Voxen `Data/textures.txt`. `ParticleCloudWhiteReduced.png` intentionally aliases the existing `ParticleCloudWhite.png` entry. Unity's `explosion1.psd` intentionally expands to the already-indexed `546_1335.png` through `546_1338.png` frames. `?` means the source asset is missing from Voxen's texture catalog or is not a direct texture file.

| Voxen index | Texture |
|---:|---|
| 1051 | nodamage.png |
| 583 | ParticleCloudWhiteReduced.png |
| 579 | onepixel.png |
| 69 | BurstTiny.png |
| 811 | sheet_screendestroyed.png |
| 2046 | explosion1.psd |
| 2050 | explosion1_firespit.png |
| 583 | ParticleCloudWhite.png |
| 820 | Sparqwave.png |
| ? | SparqSpatter.png |
| 67 | Burst.png |
| 817 | Spark.png |
| 158 | darthit.png |
| 794 | Sec2MuzzleBurst3.png |
| 792 | Sec2MuzzleBurst1.png |
| 793 | Sec2MuzzleBurst2.png |
| 795 | Sec2MuzzleBurst4.png |
| 796 | Sec2RotaryMuzzleBurst.png |
| 797 | Sec2RotaryMuzzleSquint.png |
| 821 | SplashesFineParticle.png |
| 582 | ParticleCloudBlack.png |
| 589 | ParticleFlare.png |
| 881 | white.png |
| ? | <none> |
| 566 | npc_maintbotmuzzburst.png |
| 557 | npc_flierbot_muzzleburst2.png |
| 556 | npc_flierbot_muzzleburst1.png |
| 538 | MessyStreakTrail.png |
| 588 | ParticleFlamesSheet.png |
| 571 | npc_repairbot_deathburst.png |
| 387 | gunhit.png |
| 252 | EnemShot2.png |
| 805 | sheet_enemshot4.png |
| 2043 | 546_0164.png |
| 2044 | 546_0167.png |
| ? | magshot01.png |
| 429 | magshot02.png |
| 809 | sheet_plasmashot.png |
| 663 | railshot.png |
| 835 | stunshot.png |

## Review gates

- Do not collapse Unity `ColorModule` alpha keys into color keys without preserving alpha timing.
- Keep `TwoConstants`, finite curves, bursts, and sub-emitters marked until Voxen has equivalent runtime behavior.
- Confirm Unity-to-Voxen world scale and degrees-to-radians behavior against one visual reference effect before bulk conversion.
- Treat material blend mode as authoritative for additive/multiply/alpha behavior; the current Voxen runtime derives blend mode from the texture catalog.

## Extraction validation

- Active module coverage: ColorModule=182, SizeModule=154, VelocityModule=4, RotationModule=8, UVModule=11, TrailModule=17, ShapeModule=114.
- Rows with at least one unresolved Voxen texture slot: `13`.
- Rows still carrying an unresolved texture GUID: `0`.
- Material GUIDs resolved: `708`; Unity texture GUIDs resolved: `2215`; Voxen catalog entries: `2035`.
