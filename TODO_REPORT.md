# Voxen TODO Report — Unity (Citadel) Cross-Reference

Date: 2026-09-16. Scope: all `TODO`/`FIXME` markers in Voxen `*.c`/`*.h`,
`ScriptsTODO/` backlog, and every Unity C# script cross-checked for a Voxen
counterpart. UI hotspot analysis driven by `CitadelScene.unity` Canvas
hierarchy (`Canvas`, `CenterMFDPanel`, `TabsLH`/`TabsRH`, `AutomapCanvas`,
`CompassContainer`, `BioMonitorContainer`, Vmail/Keypad/Elevator/Puzzle nodes).
Each Unity C# script maps to a handful of functions in `citadel.c` (845 dense
lines) plus `ui.c` (`CenterMFD`/`SideMFD`/`RenderMenu`/`RenderPausedUI`).

## 1. In-code TODO inventory (verbatim, with assessment)

### `voxen.c`
| Line | Marker | Assessment |
|---|---|---|
| 105 | `} // TODO` (end of per-level starting-inventory switch) | Ambiguous anchor. Switch cases 6–9 carry item lists; treat as "verify starting inventory per level". P2. |
| 108 | `ScreenShake` — `// TODO actually shake` | Timer recorded, `(void)shakeForce`: no camera transform applied. Pairs with missing `EffectScreenShake.cs`. P1 (juice, cheap). |
| 617 | `//TESTING TODO REMOVE!` full-hardware grant | Debug cheat granting all hardware v4 at boot. **Must remove before ship.** P0. |
| 661 | `InitAudio(); //synth_set_room(0.66f,0.8f); TODO` | Reverb/room config not applied. Audio polish. P3. |

### `entity.c`
| Line | Marker | Assessment |
|---|---|---|
| 375 | L8 cams `/*TODO ADD THE REST!*/` | Only `SecScreen1_Camera` registered; remaining Security-level cams missing. Data-only fix. P1. |
| 578 | `// TODO in LoadLevelMod directly ...` (generateContents) | Design note on commented code; harmless. P3. |
| 589 | constIndex 555 `/*prop_cyber_switch CyberSwitchInitAfterLoad(i); TODO*/` | Cyber-switch init skipped at load. Gameplay wiring. P1. |

### `textures.c` / `text.c` / `models.c`
| Loc | Marker | Assessment |
|---|---|---|
| `textures.c:135` | `/*TODO just use scratch!*/` | Arena free should use scratch arena. Perf/health. P3. |
| `textures.c:162` | `/*prop_console02,need combined tex TODO*/` + early return | Animated texture path dead for console02 screens. Visual. P2. |
| `text.c:425` | `// TODO Just use scratch!` | Font-data free path. P3. |
| `models.c:454` | `// TODO Add cyber exit and item anims` | `CyberExit`/`CyberItem` anim clips missing. P2. |

### `citadel.c` (game logic — highest density of real gaps)
| Line | Marker | Assessment |
|---|---|---|
| 20 | Reflex patch `/*TODO ... at loading savegame*/` | `timeScale`/absolute-time offset not restored on load. Save/load correctness. P1. |
| 51 | `PlayLog` `/*World.invP1.vmailLogIndex=(i16)logIndex; TODO*/` | Vmail index never stored; viewer works (`PlayVmail` + `RenderUI` vmail block) but resume/reopen position lost. P2. |
| 199 | `// CreditsScroll, TODO video text phases` | `CreditsUpdate` does text pages only; phases 0–3 (video/text sequencing) unimplemented. P2. |
| 262 | `GeneralInvClick` `/*TODO actual actions ...*/` | Single-click on general inventory is a no-op (double-click/apply via `GeneralInvApply` works). P1 — half the input path dead. |
| 540 | `// TODO Handle hopper and zerog texture changes` | `dyingTexture` swap commented out (`NPC_Hopper_Death`, `NPC_ZeroG_MutantAnims`). Visual. P2. |
| 577 | `// TODO validate in arg without double calling SpawnDynamicObject` | Code health in `ProjectileLaunched`. P3. |
| 578 | `// TODO: store damage data into projectile entity fields` | Deferred-impact payload missing; same gap echoed at `weapons.c:181/205/237`. P1 (hitscan works, projectiles lose damage context). |
| 737 | `TriggerTargetted` `/*TODO run this trigger entity's targets*/` | **Trigger entities never fire targets** (`UseTargets` exists, wiring missing). Gameplay-critical. P0. |
| 771 | `SearchObject` `/*TODO re-frob should pull first found item out*/` | Search UX incomplete. P2. |

### `ui.c` (all UI stubs live here as comments inside real functions)
| Line | Marker | Assessment |
|---|---|---|
| 292 | `/*SensaroundCenter Plane*/ } TODO` | Center sensaround overlay missing. P2. |
| 317 | `/*TODO Sensaround Plane*/` (HW v2+) | Side sensaround planes missing. P2. |
| 319 | `// TODO REMOVE Test BG ... TODO gate by search active` | Debug BG + search-active gating for side MFD. Cleanup. P1. |

### `weapons.c`
| Line | Marker | Assessment |
|---|---|---|
| 55 | `HudHeatBleed` commented out | Heat-bleed HUD feedback missing. P2. |
| 181/205/237 | `CreateTargetIDInstance` / damage payload, commented out | Same deferred-impact gap as `citadel.c:578`. Note: `CreateTargetIDInstance` itself IS ported — only call sites missing. P1. |
| 197 | `uou.HitForce(dd)` knockback commented out | No physics knockback on hits. P2. |
| 202 | `ApplyImpactForce` commented out | Impact force commented at call site (`ApplyImpactForce` exists in `citadel.c:267`). Wire-up. P1. |
| 272 | `/*TODO ... spamming physobjects ...*/` | Edit-mode dev tool submode. P3. |
| 286/293 | `PlayUIOneShotSavable(238)` no-ammo, commented | Verify `play_wav` covers it; else add cue. P2. |
| 326 | Fire-sound `PlayUIOneShotSavable`, commented | Same as above. P2. |

### `biomonitor.c` + `ScriptsTODO/automap.c`
| Loc | Marker | Assessment |
|---|---|---|
| `biomonitor.c:82/85/88/89/92` | `SetPixel` TODOs + `// TODO actually render texture` | Graph state machine exists (`BioMonitorInit`, `IncrementERG/CHI/ECG` are index-bump stubs); nothing uploads to a UI texture, and `RenderUI`'s `showBioMonitor` block is commented out. P1 (hardware button + `HW_BIO` toggle already wired in `HardwareButtons`). |
| `automap.c:511` | `Utils.Activate(automapFull); TODO` (ported Unity comment) | 578-line partial C-ification still shaped around `GameObject` refs; side/full map, FoW tiles, hazard overlays, player icons all unrendered; `RenderUI` automap block commented out. P1. |

## 2. C# → Voxen cross-reference

### Ported (counterpart functions exist)
Inventory, WeaponFire, WeaponCurrent, PlayerEnergy, PlayerHealth
(`HealthManager*`), PlayerMovement (`physics.c`/`winput.c`), PlayerPatch,
TargetID, Door, FuncWall, ForceBridge, Trigger (except §1 `TriggerTargetted`
wiring), LogicTimer, TriggerCounter, TextureChanger, ButtonSwitch, HealingBed,
ChargeStation, CyborgConversionToggle, DelayedSpawn, TeleportTouch,
ProjectileEffectImpact, DamageData, TargetIO (`UseTargets`), UseName,
VaporizeButton (`VaporizeClick`), GrenadeActivate, ElevatorButton,
Email (`EmailTargetted`), Radiation, QuestBits/QuestBitRelay, CyberDoor,
CyberTimer, CyberWall, CyberPush, CyberItem, CyberMine, CyberIce, CyberDecoy,
AIController, AIAnimationController, SaveLoad (`SaveGame`/`LoadGame` in
`entity.c` — **verify full-state coverage**: missionBits, inventory, logs,
patches, ressurection levels), LevelManager (`LoadAllLevels`/`LoadLevel`),
MainMenuHandler (`RenderMenu`), Config + video tabs (`ui.c`/`winput.c`),
GetInput/MouseLook (`winput.c`), ConsoleEmulator, SkyRotate, LightAnimation,
MouseCursor (`GetCursorTexture`), StatusBarTextDecay, TextWarningsManager
(`AppendTextWarning` + render), NewGame page (`RenderMenu` `Mpg_NewGame` —
verify against `NewGameGraphSystem.cs`, still in `ScriptsTODO`), CreditsScroll
(partial: text pages done, video phases TODO), Grayscale (`ModRequestsGrayscale`
exists — verify application),-effect adjacent: `ScreenShake` stubbed (see §1).

### Partial (logic exists, UI/call-site missing)
`BioMonitor` (state, no texture/UI), `TargetID` (no weapon-hit call sites),
weapons impact/knockback (functions exist, call sites commented out),
`ElevatorKeypad` (button logic, no keypad UI), `PaperLog` (13-line script,
`SendPaperLogToDataTab` missing), `BerserkEffect` (patch timing in
`PatchUpdate`, no FX), `CyberExit` (exit logic, no anims), `Vmail`
(viewer renders; log-index TODO), `HardwareButton` (toggles render; tab
contents missing).

### Missing (no Voxen counterpart) — the UI hotspot
- **MFD core**: `MFDManager.cs` (1311) — `OpenTab`, `SendInfoToItemTab`,
  `SendSearchToDataTab`, `SendGrid/WirePuzzleToDataTab`,
  `SendPaperLog/AudioLog/KeypadKeycodeToDataTab`, center-tab notify,
  multimedia table contents (`OpenLog/Email/Data/NotesTableContents`),
  `ReturnTabsFromSearch`, `DisableAllCenterTabs`. `TabMSG`
  (`None,Search,AudioLog,Keypad,Elevator,GridPuzzle,WirePuzzle,EReader,Weapon,SystemAnalyzer`)
  has no Voxen enum.
- **Side tabs**: `ItemTabManager.cs` (128), automap (§1), Target tab, Data tab
  (`SystemAnalyzer.cs` (84) in `ScriptsTODO`; `ui.c` tab 5 is an empty stub).
- **Center tabs**: Hardware / General / Software contents (all empty stubs in
  `CenterMFD`); `HardwareInvButton`, `SoftwareButtonText`, `SoftwareInvButton`,
  `GeneralInvButton`, `WeaponButton`/`WeaponButtonsManager`,
  `GrenadeButton`/`GrenadeButtonsManager`/`GrenadeTimerSlider`/`GrenadeProximity`,
  `PatchButton`, `EnergyHeatTickManager`, `EnergyOverloadButton`
  (`OverloadButtonAction` exists in `citadel.c:223` — wire to UI),
  `AmmoIconManager`, `ItemIconManager`, `LeftMFDTabs`, `TabButtons`.
- **Reader/multimedia**: `LogTextReaderManager`, `EmailContentsButtonsManager`,
  `LogContentsButtonsManager`, `LogCountsText`, `LogDataTabContainerManager`,
  `LogBackButton`, `LogMoreButton`, `MultiMediaLogButton`,
  `EReaderSectionsButtons` (+highlight), `QuestLogNotesManager.cs` (269).
- **Puzzles**: `PuzzleGrid.cs` (975) + `PuzzleGridPuzzle.cs` (106),
  `PuzzleWire.cs` (664) + `PuzzleWirePuzzle.cs` (103).
- **Minigames**: `Minigame15.cs` (287), `MinigamePing.cs` (290),
  `MinigameTriopToe.cs` (216), `MinigameBotBounce.cs` (187),
  `MinigameCursor.cs` (53, `ScriptsTODO`); `MinigameCnc.cs` is in NEITHER
  `ScriptsTODO` NOR Voxen — confirm whether Cnc shipped in Unity or was cut.
- **Codes/elevator**: `KeypadKeycode.cs` (94) + `KeypadKeycodeButtons.cs` (139)
  + `KeycodeButton.cs` (68) + `KeycodeDigitImage.cs` (24),
  `KeypadElevator.cs` (49), `CodeScreen.cs` (in `Assets`, no Voxen refs —
  triage).
- **Overlays/FX UI**: `CompassContainer` (no Voxen refs — compass unported),
  crosshair (`GetCrosshairTexture` exists — verify it renders),
  health/energy indicator blocks + BioMonitor texts (commented out in
  `RenderUI`), Teleport/Radiation/Healing/Shield-activation FX blocks
  (commented out), `PainStaticFX.cs`, `MissionTimer.cs` (97, stubbed in
  `RenderUI`), CyberTimer block (stubbed), `AutomapFull` block (stubbed).
- **Small/misc**: `PlaySoundTriggered.cs` (85, trivial),
  `SpawnManager.cs` (81, verify vs `DelayedSpawn`),
  `SecurityCameraRotate.cs` (52, rotate behavior; `AddCamView` poses exist),
  `MaterialChanger.cs` (30)/`MaterialFlash.cs` (69) (likely fold into
  `texIndex` writes — verify, then delete from backlog),
  `ImageSequenceTextureArrayUI.cs` (85, generic sequencer; vmail hardcodes
  frame ranges instead), `TickIndicatorAnimation.cs` (61, `TickBar` is static),
  `TouchEnergyDrain.cs` (31, verify trigger wiring vs `TakeEnergy`),
  `UseableAttachment.cs` (70), `UseableObjectUse.cs`, `UseHandler.cs`
  (`UseEntity` covers part — diff carefully), `SearchButton.cs`,
  `SearchableItem.cs` (vs `SearchObject`), `InteractablePanel.cs` (79),
  `ObjectImpact.cs` (35), `TeleportFXStatic.cs` (63), `LogicRelay.cs`,
  `LogicBranch.cs`, `LogicCounter.cs` (vs ported `TriggerCounter`),
  `CyberAccess.cs`, `MusicTrigger.cs`, `ReverbZone.cs` (entity mention —
  verify), `AmbientRegistration.cs`, `Billboard.cs`, `DriftUp.cs`,
  `MarkAsFloor.cs`, `RandomYRotation.cs`, `RandomStartOff.cs`,
  `VideoRTReset.cs`, `ConfigurationMenuVideo.cs` (verify vs `RenderMenu`
  config), `MobileInputController.cs` (likely out of scope — confirm),
  `StartMenuDifficultyController.cs` (verify vs `Mpg_NewGame`),
  `GameEnd.cs` (entity mention — verify end-of-game flow),
  keybind-remap UI (`ConfigKeybindButton.cs`, `ConfigInputLabels.cs`,
  `InputTab`, `KeycodeButton*` — no Voxen refs; remapping missing).
- `ScriptsTODO/EntityNeedsCanvas.txt` (8 lines) is itself a mini-backlog:
  `ActivateButton` (done marker `-`) vs `AIAnimationController` anim-state
  fields — diff against `AIAnimationControllerUpdate`/`ChangeAnim`.

## 3. UI hotspot status (`ui.c` today)

| Canvas area | Voxen status |
|---|---|
| Main menu / Pause / Load / Save / NewGame / Options-video-audio / Credits-text | DONE (`RenderMenu`, `RenderPausedUI`, `SaveGame`/`LoadGame`, F6/F9 quick) |
| Options-Input/keybind remap | MISSING |
| Center MFD Weapons tab | DONE (list + ammo + select) |
| Center MFD Hardware / General / Software / EReader tabs | EMPTY STUBS |
| Side Weapon tab | PARTIAL (name + icon; no ClipBox/heat-ticks/reload/energy-slider) |
| Side Item / Automap / Target / Data tabs | EMPTY STUBS |
| Health/Energy tick bars, hardware edge buttons, text warnings, cursor, vmail viewer, shoot-mode, add-to-inventory helper | DONE |
| BioMonitor graphs + texts, energy/health indicator detail blocks | MISSING (state exists) |
| Compass, crosshair-verify, sensaround planes, shield/sight FX overlays, full automap, mission/cyber timers | MISSING / STUBBED |
| Search, keypad, elevator-code, grid/wire puzzles, system analyzer, audio-log reader, paper log, EReader sections, notes | MISSING (all route through unported `MFDManager.OpenTab`) |

## 4. Prioritized remaining work

**P0 (ship blockers / gameplay-critical)**
1. Remove `voxen.c:617` testing hardware grant.
2. Implement `TriggerTargetted` target firing (`citadel.c:737`).
3. Verify save/load full-state coverage (reflex time offset, `citadel.c:20`).

**P1 (UI completion, biggest mass)**
4. `MFDManager` port: `TabMSG` enum + `OpenTab` dispatch + `Send*ToDataTab`
   family + center-tab notify + multimedia tables. Unlocks every Data-tab client.
5. Center tabs Hardware/General/Software; side tabs Item/Target; side Weapon
   detail (ClipBox, heat ticks, reload, energy slider).
6. Single-click `GeneralInvClick` actions (`citadel.c:262`).
7. Automap: finish `automap.c` port (side + full + FoW + hazards + icons).
8. Puzzles: Grid (975+106) then Wire (664+103); keypad/elevator-code UI.
9. BioMonitor texture upload + `showBioMonitor` UI block.
10. Deferred-impact wiring (projectile payload, `CreateTargetIDInstance` call
    sites, `ApplyImpactForce` call site).
11. L8 remaining sec cameras; cyber-switch init; real `ScreenShake`; vmail
    index; search-active gating + test-BG removal (`ui.c:319`).

**P2 (visuals/audio correctness)**
12. Compass, sensaround planes, shield/sight/teleport/radiation/healing FX
    blocks, mission/cyber timer blocks, heat-bleed + overheat ticks, knockback,
    no-ammo/fire UI cues (or confirm `play_wav` covers), hopper/zerog death
    textures, console02 combined texture, cyber exit/item anims, credits video
    phases, search re-frob pull-out, keybind-remap UI, `MinigameCnc` triage,
    `CodeScreen` triage.

**P3 (health/polish, do last)**
13. Scratch-arena cleanups (`textures.c:135`, `text.c:425`),
    `SpawnDynamicObject` double-call validation, `generateContents` note,
    reverb `synth_set_room`, edit-mode physobject submode, `MaterialChanger` /
    `MaterialFlash` verify-and-drop, mobile-input scope decision.

## 5. Size estimate

Unported gameplay/UI C# totals ~6.5k lines (`MFDManager` 1311, Grid 1081,
Wire 767, automap 578, `NewGameGraphSystem` 377, minigames ~740, notes 269,
keypad/elevator ~330, managers/tabs/buttons ~1000). At Voxen density
(`citadel.c` packs each script into a handful of functions) expect roughly
1.5–2.5k new dense lines, mostly in `ui.c` + `citadel.c`, plus UI-frame
texture budgets for automap/bio/vmail (frames already reserved for vmail).
