# Voxen UI TODO — Canvas render + interaction audit

Source review:
- `ui.c` (1825 lines, full read) — comments + `RenderUIImage` / `RenderTextL` / `RenderTextC` / `RenderTextR` calls.
- `Tools/ui_layout.txt` (7842 lines; Canvas section `11–2143`, `2115` Canvas nodes parsed) — used as element inventory.
- `~/ai-workspaces/Citadel/Assets/Scenes/CitadelScene.unity` (47 MB, 1588685 lines, 21382 `m_Name:`) — spot-confirmed hierarchy names and `m_MethodName:` onClick targets (e.g. `TeleportFX:907174`, `WepNameTextLH:895363`, `ClipBox:911584/933367`, `CloseFullmapButton:959369`, `OverloadButton:902384/9319251`, `AutomapFull:1090480`, `CyberSPrint:1088831`, `EMPStatic:1117628`, `VmailPlayer:1151708`, `MissionTimerT:1159355`, `BioMonitorContainer:1301887`, `StatusBar:1350628`, plus `ElevButtonClick`, `CloseElevatorPad:1070528`, `CloseKeycodePad:985612`, `AutomapZoomIn:1043390`, `ToggleSideTop:972081`, `OverloadEnergyClick:885532`, `LoadSecondaryAmmoType:1025654`, `OnVaporizeClick:892425`, `ApplyButtonClicked:837669`, `OnActivateClick:848333`, `MinigameStart_*`, `TabReset:437044`, `OpenMinigames:77135`, `TabButtonClick:857036`, etc.).

Conventions applied:
- `Image` element → needs `RenderUIImage`. `Text` → needs `RenderTextL/C/R` (Voxen uses almost exclusively `RenderTextL`; only `RenderTextC` is `ui.c:1809` status text; `RenderTextR` is never used).
- `Button` element:
  - If the Button node itself carries an `Image(...)` with a real sprite (`tex=908/899/950/951/1020-1024/1086/1087/keypad_*/puzzle_*/wire_node`, etc.) → needs **both** `RenderUIImage` (background) + `RenderText*` (child `Text`, if any) + interaction.
  - If the Button has no `Image` (e.g. `PatchInventory/Button`, `ui_layout.txt:604`) or only a hitbox `Image(...,builtin-white)` / `Image(...,none)` with no distinct art (e.g. center weapon/grenade/general list rows, `SearchContentLH1-4`, `MinigameClose/MinigameBack` with `none`) → **must NOT** get a `RenderUIImage` per task rule; needs only child-`Text` render (if any) + interaction. These are marked “correctly omitted” below, not as missing.
  - `RawImage` (automap, vmail video, biomonitor graph) has no 1:1 Voxen primitive; Voxen uses `RenderUIImage` placeholders (`tex 0` / `QUAD:none`). Treated as “needs a render call” and flagged where absent.
- Shared code paths cover both sides: `SideMFD(isRH)` / `SideMFDHeader(isRH)` with `dx=isRH?1059:0` render LH+RH from one block; `CenterMFD`, `TickBar`, `TextWarnings` loop, `RenderConsumables`, etc. RH duplicates are therefore **not** double-counted — one entry notes “×2 (LH+RH)”.
- `UNMAPPED:[...]` / `QUAD:builtin-white|builtin-knob|none` with `tex 0` in `ui.c` (e.g. elevator/keycode/puzzle/wire/minigame/EReader blocks) counts as a render call present, even though the texture id is still a placeholder.
- Containers with no `Image`/`Text`/`Button`/`Slider`/`RawImage` (e.g. `MFDManager`, `TabsLH`, `TargetTabLH/RH`, `ButtonBank`, `Fill Area`, `Viewport`) need no render call and are omitted.

Voxen interaction primitives searched: `HwBtnClick`, `UI_Button`, `UI_MenuButton`, `UI_MenuInteractable`, `UI_Slider`, `UI_Checkbox`, plus bespoke handlers (`WeaponSelectSlot`, `ConsumableSelect/Use`, `GeneralInvClick/Apply/Take`, `VaporizeClick`, `SearchTakeSlot/CloseSearch`, `MFD_SelectTab`, `ForceShootMode`).

---

## A — Missing render calls (`RenderUIImage` / `RenderTextL/C/R`)

### A1. Top-level Canvas FX / overlays / full-screen (all confirmed in Unity, none rendered in `ui.c`)

| ui_layout.txt | Unity | Type | Missing |
|---|---|---|---|
| `12 TeleportFX [OFF]` | `m_Name: TeleportFX` | Image `teleportstatic_red` | `RenderUIImage` |
| `13 RadiationFX [OFF]` | present | Image `teleportstatic_white` | `RenderUIImage` |
| `14 HealingFX [OFF]` | `m_Name: HealingFX` | Image (color, `none`) | `RenderUIImage` |
| `15 ShieldFX [OFF]` | `m_Name: ShieldFX` | Image (color, `none`) | `RenderUIImage` |
| `16 ShieldActivation [OFF]` + `17 waveup` + `18 wavedn` | present | 2× Image `tex 1076` | 2× `RenderUIImage` |
| `19 ShieldDeactivation [OFF]` + `20 waveup` + `21 wavedn` | present | 2× Image `tex 1076` | 2× `RenderUIImage` |
| `22 Loading [OFF]` (Image black) | present | Image | `RenderUIImage` |
| `23 LoadingText` | `m_Name: LoadingText` | Text `LOADING...` | `RenderTextL/C/R` |
| `24 LoadingPercentText` | present | Text `0.00%` | `RenderTextL/C/R` |
| `25 DeathRessurectionFX [OFF]` + `26 spawndelaycontainer (11)` + `28/30/32/34/36/38/40/42/44/46/48` (11 spawners, no visual — N/A) + `27 blackground` (Image black) | present | Image | `RenderUIImage` for `blackground` |
| `29 Text 1 (1)`, `31 Text 1 (6)`, `33 Text 1 (2)`, `35 Text 1 (6)`, `37 Text 1 (7)`, `39 Text 1 (3)`, `41 Text 1 (6)`, `43 Text 1 (7)`, `45 Text 1 (4)`, `47 Text 1 (6)`, `49 Text 1 (5)` | present | 11× Text (resurrection sequence) | 11× `RenderTextL/C/R` |
| `72 AutomapFull [OFF]` container N/A; `73 AutomapFullRawImage` | `m_Name: AutomapFullRawImage` | RawImage | `RenderUIImage` (no `SideMFD`/automap-full path renders it) |
| `74 PlayerIconFull [OFF]` | `m_Name: PlayerIconFull` | Image `player_180` | `RenderUIImage` |
| `75 CloseFullmapButton` + `76 Text CLOSE MAP` | `m_Name: CloseFullmapButton` | Button Image `908` (distinct) + Text | `RenderUIImage` + `RenderTextL/C/R` |
| `1439 EnergyTickPanel`, `1440 HealthTickPanel` | `m_Name: EnergyTickPanel` | 2× Image `healthticks24` (panel bg) | 2× `RenderUIImage` panel bg. Note `TickBar()` (`ui.c:232-233`) renders ticks (`964/963/962`) + indicators (`939/956`) but never the `healthticks24` panel background itself |
| `1443 EnergySurge [OFF]` | present | Image `944` | `RenderUIImage` |
| `1444 HealthIndicatorCyber [OFF]` | present | Image `954` | `RenderUIImage` (cyberspace variant; only base `939/956` rendered) |
| `1445 EnergyDrainText`, `1446 EnergyJPMText` | present | 2× Text (empty) | 2× `RenderTextL/C/R` |
| `1514 EMPStatic`, `1515 PainStatic` | `m_Name: EMPStatic` | 2× Image `painstatic_*` | 2× `RenderUIImage` |
| `1516 SightDimming` | present | Image `sightDimming` | `RenderUIImage` |
| `1517 DeathFXContainer` N/A; `1518 DeathFX [OFF]` | `m_Name: DeathFXContainer` | Image (`none`) | `RenderUIImage` |
| `2122 QuitBlank [OFF]` | present | Image black | `RenderUIImage` |
| `2135 TouchEnterButton [OFF]` (under `2130 Console`) | present | Button Image `builtin-white` (distinct hitbox) + `Button->Pause.ConsoleEntryEnterDelegate()` | `RenderUIImage` (console text itself is rendered `ui.c:1808`; button bg is not) |

Covered here (do not re-add): `ShootModeButton:59` (`RenderUIImage 1020`, `ui.c:1745`); `TextWarnings:60` + `WarningText (0-9):61-70` (loop `ui.c:1746-1748`); `MissionTimerT:77` + `MissionTimer:78` (placeholders `ui.c:1751-1752`, values static `"0"` — live timer still TODO); `CyberSPrint:1428` (`ui.c:1755`, static string); `BioMonitorContainer:1429` subtree (`Graph` placeholder `ui.c:1761` + 8 texts `ui.c:1762-1770`, values static); `EnergyIndicator:1441` + `HealthIndicator:1442` (`ui.c:232`); `VmailPlayer:1447` + 6 `Vmail*Video` RawImages (single `RenderUIImage vmailFrame`, `ui.c:1798` — shared path, covered); `SearchFXContainer:1454` + `SearchFXLH:1455`/`SearchFXRH:1456` (`RenderSearchFX`, `ui.c:614-634`); `StatusBar:2121` (`RenderTextC statusText`, `ui.c:1809`).

### A2. `WeaponTabLH/RH` — only name + icon rendered, rest missing (×2 LH+RH; layout `81-132` + RH mirror `1014-1065`; Unity `WepNameTextLH:895363`, `ClipBox:911584`, `EnergyHeatTicks:893739`, `EnergySlider:894318/1023523`, `OverloadButton:902384`, `ReloadNormalButton:1229770`)

`SideMFDHeader tab==1` (`ui.c:288-291`) renders only `WepNameTextLH/RH` (`RenderTextL`) + `WepIconLH/RH` (`RenderUIImage wepIconTexIndices`). Everything else in the comment header (`ClipBox, EnergyHeatTicks, ReloadButtons, EnergySlider`) has no call:

- `ClipBox:84` (Image `0038_0031`) → `RenderUIImage`. Same RH.
- `AmmoIconLH:85` (Image `blank`) → `RenderUIImage`. Same RH.
- `CurrentMagazineAmount10s:86`, `100s:87`, `1s:88` (Images `nullsprite`) → 3× `RenderUIImage` per side.
- `EnergyHeatTicks:89 [OFF]` (Image `945`) + `OverheatTick1:90` … `OverheatTick10:99` (Images `947/948/949`) → 11× `RenderUIImage` per side.
- `EnergySettingText:100`, `LOWText:101`, `HIGHText:102`, `HeatText:103` → 4× `RenderTextL/C/R` per side.
- `OverloadButton:104 [OFF]` (distinct `950`) + `105 Text OVERLOAD` → `RenderUIImage` + `RenderTextL/C/R` per side.
- `UnloadButton:106 [OFF]` (distinct `950`) + `107 Text UNLOAD` → `RenderUIImage` + `RenderTextL/C/R` per side.
- `ReloadNormalButton:108 [OFF]` (distinct `951`) + `109 Text LOAD NORMAL` → `RenderUIImage` + `RenderTextL/C/R` per side.
- `ReloadAlternateButton:110 [OFF]` (distinct `950`) + `111 Text LOAD ALT` → `RenderUIImage` + `RenderTextL/C/R` per side.
- `EnergySlider:112 [OFF]` subtree per side: `Background:113` (`952`) → `RenderUIImage`; `Fill:115` (`1079`) → `RenderUIImage`; `Handle:117` (`953`) → `RenderUIImage`. `Fill Area:114` / `Handle Slide Area:116` are layout-only (no call needed). `Button:118` … `Button (14):132` (15× `Image none` hitboxes) correctly need **no** `RenderUIImage` per sprite rule — but see §B for missing `EnergySlider.SetValue()` interaction.

### A3. `ItemTabLH/RH` — delegated coverage, two slider gaps (layout `133-164` + RH mirror)

Covered via shared paths (not per-element comments, but calls exist): `ItemIcon:134` + `ItemText:135` + `VaporizeButton:136/Text:137` + `ApplyButton:138/Text:139` + `UseButton:140/Text:141` are rendered through `RenderGeneralItem` / `RenderConsumableItem` (`ui.c:548-572,454-465`); `EMAILButton:143/Text:144` + `LOGSButton:145/Text:146` + `DATAButton:147/Text:148` + `NOTESButton:149/Text:150` are rendered in `SideMFDHeader tab==2 reader` (`ui.c:292-299`); `AccessCardsList:151` is rendered in the access-card loop (`ui.c:561-567`). Do not re-add those as missing (optionally add explicit `// ItemIcon` etc. comments for 1:1 traceability).

Still missing:

- `GrenadeTimerSliderLH:152 [OFF]` subtree per side: `Background:153` (`952`) → `RenderUIImage`; `Fill:155` (`1079`) → `RenderUIImage`; `Handle:157` (`953`) → `RenderUIImage`; `TimeNumberText:158` → `RenderTextL/C/R`. `RenderConsumableItem` (`ui.c:460-464`) renders a generic timer bar (`1087/1086` + value text) for nitro/earthshaker only — it does not map these four element names, and uses different art. Flag as partial/missing.
- `Slider:159` (second generic slider) + `Background:160` (`952`) → `RenderUIImage`; `Fill:162` (`builtin-white`) → `RenderUIImage` (or confirm deprecated/duplicated and delete from Unity); `Handle:164` (`953`) → `RenderUIImage`. No Voxen call references this subtree.

### A4. `AutomapTabLH/RH` — entirely unrendered (×2; layout `165-183` + RH mirror; `SideMFDHeader tab==3` is an empty stub `ui.c:301`)

All need calls; `AutomapMask:167` / `AutomapContainerLH:168` are `Image(...,none)` near-transparent but still `Image` nodes — include for strict 1:1, or explicitly waive as mask-only:

- `AutomapMask:167` (Image `none` + Mask) → `RenderUIImage` (or waive with comment if mask-only is intentional).
- `AutomapContainerLH` (Image `none`) → `RenderUIImage` (or waive).
- `AutomapImageLH_CamLinked:169` (RawImage `AutomapRender`) → render call (Voxen has no automap texture path at all).
- `AutomapImageLH:170 [OFF]` (RawImage `L1`) → render call.
- `PlayerIconLH:172` (Image `player_180`) → `RenderUIImage`.
- `CircleLg:173` (Image `circle_lg`) → `RenderUIImage`.
- `CircleSm:174` (Image `circle_sm`) → `RenderUIImage`.
- `AutomapSideImageLH:175 [OFF]` (Image `automap_side_LG1`) → `RenderUIImage`.
- `ZoomInButton:176` (distinct `908`) + `177 Text +` → `RenderUIImage` + `RenderTextL/C/R`.
- `ZoomOutButton:178` (distinct `908`) + `179 Text -` → `RenderUIImage` + `RenderTextL/C/R`.
- `GoFullButton:180` (distinct `908`) + `181 Text FULL` → `RenderUIImage` + `RenderTextL/C/R`.
- `GoSideButton:182` (distinct `908`) + `183 Text SIDE` → `RenderUIImage` + `RenderTextL/C/R`.
- Same 12 entries for RH (`AutomapContainerRH`, `PlayerIconRH`, `AutomapSideImageRH`, `ZoomIn/Out/GoFull/GoSide` RH + texts). `IndividualOverlaysContainerL1:171 [OFF]` is layout-only (no call needed).

### A5. `DataTabLH/RH` generic header (layout `185-189`; covered blocks are elevator/keycode/search/puzzle/wire/sysanalyzer/minigames/audiolog in `SideMFD data==1/2/5/3/4/7/9/6`)

- `DataHeaderTextLH:188 [OFF]` (+ RH mirror) → `RenderTextL/C/R`. No `data==` branch renders it.
- `DataNoItemsTextLH:189` (+ RH mirror) → `RenderTextL/C/R`. `RenderSearch` has its own “No Items” (`ui.c:654`) for search `data==5` only; the generic data-tab empty text is not rendered.
- `BlockedBySecurityLH:186 [OFF]` + `BlockedBySecurityText:187` → covered (`ui.c:663-666`, placeholder tex `1110`). Not TODO.
- `SearchContentsContainerLH:263` (+ RH `1194`) — `Image builtin-knob` container bg → `RenderUIImage` per side. `RenderSearch` (`ui.c:637-656`) renders header/item icons/`No Items`/close-X but never the container background. `SearchContentLH1-4:266-269` / `RH1-4` are `Image none` hitboxes → correctly no `RenderUIImage` (see §B for interaction, which is present).
- Elevator (`190-219`), keycode (`220-260`), grid (`275-393`), wire (`395-456`), sysanalyzer (`458-482`), minigames (`485-512`), audiolog (`data==6`, `ui.c:809-816`) render blocks are present in `SideMFD` with per-element `RenderUIImage`/`RenderTextL` + `// BTN`/`// C#` comments — do not re-add as missing renders. Texture ids there are still `UNMAPPED` placeholders (`2132-2146`) and `QUAD:none/builtin-white` (`tex 0`); remaining work is art mapping, not missing calls.

### A6. Center MFD (layout `529-1010`)

- `MainTab:530` containers `WeaponInventory:531`, `WeaponShotsInventory:547`, `GrenadeInventory:556`, `GrenadeCountsInventory:586`, `PatchInventory:602`, `PatchCountsInventory:639` — all `Image builtin-knob` backgrounds → 6× `RenderUIImage` missing. Voxen renders only headers + rows (`CenterMFDHeader` `ui.c:260-271` + `RenderConsumables` `ui.c:442-453`).
- Headers `TextWeaponsHeader:532`, `TextShotsHeader:548`, grenade/patch header Texts (`557 GRENADES`, `587 #`, `603 PATCHES`, `640 #`) → covered (`ui.c:261,444`). Row texts/counts/use-boxes (`1086/1079`) → covered (`ui.c:445-451`, `UI_Consumables`). Center weapon `Button:533-546` (7× `builtin-white` alpha 0 hitboxes) correctly need no `RenderUIImage`. `GrenadeCounts1-7:588-601` (`builtin-white` bgs) + `Text #1-#7` placeholders are replaced by live counts (`ui.c:449`) — container bgs are the only gap; placeholder texts are superseded, not missing.
- `HardwareTab:641`: `Label:642 HARDWARE` → covered (`ui.c:1259`). `HarwareInventory:643` (`builtin-knob`) → `RenderUIImage` missing. Row `Button:644-670` bgs are `builtin-white` hitboxes → correctly no `RenderUIImage`; row texts are rendered (`ui.c:1260-1265`).
- `GeneralTab:672`: `Label:673 GENERAL` → covered (`ui.c:540`). `GeneralInventory:674` (`builtin-knob`) → `RenderUIImage` missing. Row `Button/AccessCardsButton:675-727` bgs are `builtin-white` hitboxes → correctly no `RenderUIImage`; row texts + `ApplySubButton (1-13)` (`1086`) + `Image` (`1079`) are rendered (`ui.c:539-546`).
- `SoftwareTab:729`: `Label:730 SOFTS` → covered as `SOFTWARE` (`ui.c:1271`; rename noted). Row buttons (`ICEDrill:732`, `Pulser:735`, `CyberShield:738`, `Turbo:741`, `Decoy:746`, `Recall:751`, `Games:756`, all `builtin-white` hitboxes) correctly need no `RenderUIImage`; labels/versions/counts are rendered (`ui.c:1268-1276`). Missing: `Turbo:744 UseButton` (`1086`) + `745 Image` (`1079`); `Decoy:749 UseButton` + `750 Image`; `Recall:754 UseButton` + `755 Image` → 3× `RenderUIImage` + 3× `RenderUIImage` missing. No software use-box is rendered in `CenterMFD` software block.
- `MultiMediaDataReader:758` EReader center → fully rendered (`MultiMediaHeaderLabel` `ui.c:1279`; log table `ui.c:1281-1354`; folder `ui.c:1355-1432`; text reader `ui.c:1433-1447`; email `ui.c:1449-1582`; data `ui.c:1583-1651`; notes toggles/backgrounds/labels `ui.c:1652-1725`). No missing calls; `NoteObject*` wrappers are layout-only.
- `ButtonMain:1002` (distinct `1024`) + `Text:1003 Button`; `ButtonHardware:1004` (`1021`) + `Text:1005`; `ButtonGeneral:1006` (`1021`) + `Text:1007`; `ButtonSoftware:1008` (`1021`) + `Text:1009` — backgrounds covered (`CenterMFDHeader`, `ui.c:253`); 4× child `Text Button` → `RenderTextL/C/R` missing. Side siblings `ButtonWeapon:517` / `ButtonItem:519` / `ButtonAutomap:521` / `ButtonTarget:523` / `ButtonData:525` (+ RH `1418-1426`) have no child `Text` in Unity, so backgrounds alone (`SideMFDHeader`, `ui.c:277-283`) are sufficient there.
- `SensaroundCenter:1000` (+ `Plane [no-rect]`) and `SensaroundLH:514` / `SensaroundRH:1415` are `TODO Sensaround Plane` stubs (`ui.c:258,285`) — no render call by design so far; keep as explicit TODO, not a silent gap.

### A7. Pause / MainMenu / cursor / debug (architectural divergence — strict 1:1 intentionally not followed)

Voxen replaces these Unity hierarchies with its own `RenderPausedUI` (`ui.c:185-210`) / `RenderMenu` (`ui.c:43-183`) / cursor (`ui.c:1823`) / FPS-location-console-status (`ui.c:1801-1821,1808-1809`). Do not add 1:1 per-element calls without first deciding to abandon the replacement. Functional render gaps inside the replacement:

- Pause `SaveQuestion:1493` + `SaveHardQuestion:1504` dialogs (`ClickBlocker`, `Image 922`, `Border 1086`, `SaveQuestionText`, `Yes/No/CancelButton` + texts) have no Voxen render. Unity `PauseText:1475`, `PauseBackground:1476`, `Resume/Load/Save/Options/PauseQuit/PauseQuitHard` + texts are functionally replaced but not element-mapped.
- MainMenu `MainMenuBackgroundImage:1520`, `TitleText/TitleTextLaser`, all `FrontPage/SingleplayerPage/MultiplayerPage/OptionsPage` tab/label/slider/dropdown art, `InputTab` labels/keybind rows (`LabelKeybind*`, `Keybind (0-39)`), `AudioTab` volume rows beyond Master/Music, `NewGamePage` insets/labels, `LoadGamePage/SaveGamePage` slot rows (`LoadButtonPad/Text`, `SaveButtonPad/Text/NameInput`), `CreditsPage/Scroller/textCredits`, `CheckingEnvironment` art — Voxen menu renders only its own backgrounds/table/inset/tab art + headers (`ui.c:44-49,77-78,138,147-148`) and per-page text rows it implements. `Mpg_Options/Input` (`ui.c:122-123`) sets `menuItemCount=49` but renders nothing; `Mpg_Audio` renders only Master/Music sliders (`ui.c:127-128`), not `VolumeAudioLogs/Effects/Message` or language rows.
- Cursor: cursor shell is rendered (`GetCursorTexture`, `ui.c:1823`); `CursorContainer:2137` children `TooltipLeft:2139` / `TooltipRight:2140` / `TooltipCenter:2141` + `LiveGrenadeText:2142` → 4× `RenderTextL/C/R` missing.
- `FPS:2124` children (`msTween:2125`, `fpsText:2126`, `fpsTrailing:2127`, `versionText:2128`) and `LocationIndicator:2129` are functionally replaced by `Cheats.showFPS/showLocation` texts (`ui.c:1801-1807,1819-1821`) — not element-mapped by design.
- `Touchables:1457 [OFF]` subtree (`MainMenuTouchButton:1458/Text:1459`, `ConsoleButton:1460/Text:1461`, `LeftTouchstick:1462/Knob:1463`, `RightTouchstick:1464/Knob:1465`, `TouchSpace:1466/Text:1467`, `TouchLMB:1468/Text:1469`, `TouchSwimUp:1470/Text:1471`, `TouchSwimDown:1472/Text:1473`) — zero Voxen renders. If desktop-only is intentional, waive explicitly; otherwise each `Image` → `RenderUIImage` and each `Text` → `RenderTextL/C/R`.
- `AutomapCanvas` (`ui_layout.txt:2144-7801`, ~5650 FoW tiles + `playerOverlayFull`) and `UICamera/CompassContainer` (`7802-7841`) are world-space, not Canvas HUD — no `ui.c` immediate-mode calls reference them. Confirm scope waiver before adding.

---

## B — Missing button interaction logic (Voxen `HwBtnClick` / `UI_Button` / `UI_MenuButton` / `UI_Slider` / bespoke handlers)

Covered (do not re-add): 8 hardware buttons (`Hardware.ShieldClick/LanternClick/SensaroundClick/BioClick/InfraredClick/EReaderClick/BoosterClick/JumpJetsClick` → `HardwareButtons` + EReader branch `ui.c:239-248,586-590`); `ShootModeButton->MainCamera.ForceShootMode()` (`ui.c:585`); 4 center tabs (`MFDManager.CenterTabButtonClick[0-3]` → `ui.c:591-592`); 8 side tabs (`TabButtonsPanelLH/RH.TabButtonClick[0-4]` → `ui.c:593-594`); center weapon rows (`WeaponButton.cs` → `WeaponSelectSlot`, `ui.c:269`); center grenade/patch rows + use-boxes + nitro/earthshaker timer drag (`GrenadeButton.cs/PatchButton.cs/DoubleClick` → `UI_Consumables`, `ui.c:414-441`); center general rows + apply sub-boxes (`GeneralInvButton.cs/GeneralInvApply` → `UI_GeneralInventory`, `ui.c:509-538`); side general/consume use-vaporize boxes (`VaporizeButton.OnVaporizeClick`, `MFDManager.ApplyButtonClicked`, `UseButton.OnActivateClick` → `ui.c:528-534,429-438`); side EReader sections (`EReaderSectionsLH/RH.OnClick[0-3]` → `ui.c:605-610`); search close + 4 slots per side (`MFDManager.CloseSearch`, `SearchButtonClick/SearchContainerButton` → `CloseSearch/SearchTakeSlot`, `ui.c:595-602`).

Missing — each needs a Voxen click/hover handler (or an explicit wont-implement note):

1. `CloseFullmapButton:75 -> MFDManager.CloseFullmap()` (×1). No automap-full handler exists.
2. Weapon-tab side controls (×2 LH+RH): `OverloadButton:104 -> OverloadButton.OverloadEnergyClick()`; `UnloadButton:106 -> Inventory.Unload()`; `ReloadNormalButton:108 -> Inventory.LoadPrimaryAmmoType()`; `ReloadAlternateButton:110 -> Inventory.LoadSecondaryAmmoType()`; `EnergySlider:112 Button:118-(14):132 (15×) -> EnergySlider.SetValue()`. Unity `EnergySlider.cs` / `energyOverloadButton` refs confirmed; `ui.c` has no branch for any of them.
3. Automap-tab side controls (×2): `ZoomInButton:176 -> MFDManager.AutomapZoomIn()`; `ZoomOutButton:178 -> MFDManager.AutomapZoomOut()`; `GoFullButton:180 -> MFDManager.AutomapGoFull()`; `GoSideButton:182 -> MFDManager.ToggleSideTop()`. `ui.c:301` stub has no interaction.
4. Elevator pad (×2): inner `Keypad.Button (1-8):194/197/200/203/207/210/213/216 -> *.ElevButtonClick()` (8 per side, 16 total); outer `ElevButton1-8:193/196/199/202/206/209/212/215` are non-Button frames (no handler needed); `CloseButton:218 -> MFDManager.CloseElevatorPad()`. Renders exist (`ui.c:670-735`); handlers do not.
5. Keycode pad (×2): inner `Button (1-9,-,0,C):223/226/229/232/235/238/241/244/247/250/253/256 -> KeycodeButton.cs[index]` (12 per side, 24 total; `ui.c:739-795` marks all `// BTN ...: ?` — Unity target is index dispatch, still unimplemented); `CloseButton:260 -> MFDManager.CloseKeycodePad()`. Renders exist; handlers do not.
6. Grid puzzle (×2 LH/RH): `Button:281-(34):383` (35 per side, `PuzzleGridLH/RH.OnGridCellClick([0-34])`) + `CloseButton:393 -> MFDManager.ClosePuzzleGrid()`. Renders exist (`ui.c:819-1043` + RH mirror); no `HwBtnClick` dispatches them.
7. Wire puzzle (×2): `NodeBase (+1-6) LH:406-424` (7, `PuzzleWireLH.ClickLHNode([0-6])`) + `NodeBase (+1-6) RH:428-446` (7, `ClickRHNode([0-6])`) + `CloseButton:456 -> MFDManager.ClosePuzzleWire()`. Same for RH panel mirror. Renders exist (`ui.c:1044-1140`); handlers do not.
8. `SystemAnalyzerDisplayLH:458 / RH -> SystemAnalyzerDisplayLH.Close()` via `CloseButton:482/1387` (×2). Rendered (`ui.c:1141-1192`); close handler missing.
9. Minigames (×2): `MiniGameButton0_Ping:487 -> MinigameStart_Ping()`; `1_15:489 -> MinigameStart_15()`; `2_Wing0:491 -> MinigameStart_Wing0()`; `3_Botbounce:493 -> MinigameStart_Botbounce()`; `4_EelZapper:495 -> MinigameStart_EelZapper()`; `5_Road:497 -> MinigameStart_Road()`; `6_TriopToe:499 -> MinigameStart_TriopToe()`; `7_CorporateConquer:501 -> MinigameStart_CorporateConquer()`; `8_Chess:503 -> MinigameStart_Chess()`; `MinigameClose:506 -> TabReset()`; `PingGameOver:509 -> Ping.ResetOnGameOver()|Fifteen.Reset()`; `MinigameBack:512 -> OpenMinigames()`. Rendered (`ui.c:1193-1248`); none dispatched.
10. Center EReader browser (all `// BTN ...: ?` in `ui.c`, no dispatcher): log-table `Button:533-(9):...` (10, `MultiMediaLogTableButton.cs`); folder `Button:1359-(14)` (15, `MultiMediaLogButton.cs`); `MoreButton:1437` (`LogMoreButton.cs`); `BackButton:1442` (`LogBackButton.cs`); email `Button:1453-(25)` (26); data `Button:1587-(12)` (13). `NotesTab NoteToggle*` intentionally have no `Button->` (hover-only) — no handler needed. `UI_ProcessNavigation` handles only side EReader section tabs, never `MFD_CenterTab==5` row clicks.
11. Center hardware/software lists: `HardwareTab Button:644-(13)` (`HardwareInvButton.cs`, 14 rows) and `SoftwareTab ICEDrill:732/Pulser:735/CyberShield:738/Turbo:741/Decoy:746/Recall:751/Games:756 (SoftInvClick)` + `Turbo/Decoy/Recall UseButton:744/749/754 (SoftInvClick(7))` have no `HwBtnClick` path (center software/hardware blocks are render-only, `ui.c:1258-1277`).
12. Menus/pause/touch (divergent system — map deliberately or waive): Unity `Pause.PauseDisable/LoadPause/SavePause/PauseOptions/OpenSaveDialog(Hard)/SavePauseQuit/NoSavePauseQuit/ExitSaveDialog/PauseQuitHard` (`Resume:1479`, `Load:1481`, `Save:1483`, `Options:1485`, `PauseQuit:1487/1489`, `Yes/No/Cancel:1498-1502/1510-1512`) are replaced by Voxen `RenderPausedUI` `UI_Button` indices `0-5` (`ui.c:191-209`) with different semantics (no save-confirm dialogs); `Touchables` buttons (`MainMenuTouchButton->Pause.PauseEnable()`, `ConsoleButton`, `TouchSpace/LMB/SwimUp/Down`) + `Console/TouchEnterButton->Pause.ConsoleEntryEnterDelegate()` have no handlers; `MainMenu` option/credits/checking-environment buttons (`VideoResolutionApply:1562`, `Keybind:1710-1788 (40)`, `SetPresetDefault/Legacy:1815/1817`, `TabButtonGraphics/Input/Audio:1909-1913`, `Yes/No:1922-1924`, difficulty `Combat/Mission/Puzzle/CyberDifficultyButton (×4 each):1939-1981`, `LoadButton (0-7):1991-2012`, `SaveButton (0-7):2021-2063`, `PathSearch/CopyFromPath/MenuCheckForRESClose:2111-2115`, `MainMenu/SingleplayerMenuButton*`, `BackButton*`) are replaced by `RenderMenu` with partial parity only (new-game difficulties + start `ui.c:153-174` covered; options-input empty, options-audio partial, load/save slots absent — see §A7).

---

## C — Missing button interaction callbacks (Unity method + `.cs` file)

Review basis: `citadel.c` (225 function defs scanned) + `ui.c` (80 function defs scanned) for existing Voxen equivalents. Backend in `weapons.c` (`Unload`, `LoadPrimaryAmmoType`, `LoadSecondaryAmmoType`) and `winput.c` (`ForceShootMode`) noted where the logic exists but the MFD/menu click dispatch does not. `ScriptsTODO/*.cs` (`MFDManager.cs`, `PuzzleGrid.cs`, `KeycodeButton.cs`, `PuzzleWirePuzzle.cs`, `automap.c`, `SystemAnalyzer.cs`, `MinigamePing.cs`, etc.) are **not** counted as existing — they are uncompiled reference ports. `Button->?` rows have no Unity onClick method; the listed `.cs` script itself is the callback equivalent.

Reviewed but **NOT missing** (Voxen equivalent exists under same or renamed function — do not re-add):
- `Hardware.ShieldClick/LanternClick/SensaroundClick/BioClick/InfraredClick/EReaderClick/BoosterClick/JumpJetsClick` — `UIButtonMask.cs` → `HardwareButtons()` (`ui.c:239-248`) + EReader branch (`ui.c:586-590`).
- `MainCamera.ForceShootMode()` — `UIButtonMask.cs` → `ForceShootMode()` (`winput.c:394`) via `HwBtnClick(667,699,0,32)` (`ui.c:586`).
- `MainCamera.UseGrenade(7/8/9/10/11/12/13)` — `UIPointerMask.cs` → `UseGrenade()` (`citadel.c:89`) via `ConsumableSelect/Use` (`ui.c:394-441`).
- `Button.DoubleClick()` / `Button (1-6).DoubleClick()` (patch quick-use) — `UIPointerMask.cs` → `PatchUse()` (`citadel.c`) via double-click/use-box path (`ui.c:416-424`).
- `Button (1-13).GeneralInvApply(13)` + `GeneralInvButton.cs` (`?` rows) + `MFDManager.ApplyButtonClicked()` + `UseButton.OnActivateClick()` (`ActivateButton.cs`) + `VaporizeButton.OnVaporizeClick()` (`VaporizeButton.cs`) → `GeneralInvClick/Apply`, `GeneralInvDoubleClick`, `GeneralInventoryActivate`, `VaporizeClick` (`citadel.c:204,288-303`) via `UI_GeneralInventory` (`ui.c:509-538`) and side boxes (`ui.c:528-534`).
- `WeaponButton.cs` (`?`, 7 center rows) → `WeaponSelectSlot()` (`ui.c:20`) via `CenterMFDHeader` row clicks (`ui.c:269`).
- `GrenadeButton.cs` / `PatchButton.cs` (`?`) → `ConsumableSelect()` (`ui.c:394-398`).
- `TabButtonsPanelLH/RH.TabButtonClick([0-4])` + `MFDManager.CenterTabButtonClick([0-3])` — `UIButtonMask.cs` → `MFD_SelectTab()` (`ui.c:14-17`) via `HwBtnClick` center/side tab paths (`ui.c:591-594`).
- `EReaderSectionsLH/RH.OnClick([0-3])` — `UIButtonMask.cs` + `EReaderSectionsButtonHighlight.cs` → EReader section branch (`ui.c:605-610`).
- `SearchContentsContainerLH/RH.SearchButtonClick([0-3])` — `UIButtonMask.cs` + `SearchContainerButton.cs` → `SearchTakeSlot()` (`citadel.c:829`) via `UI_ProcessNavigation` slots (`ui.c:599-602`); `MFDManager.CloseSearch()` → `CloseSearch()` (`citadel.c:814`) via close-X (`ui.c:599`).
- `MainMenu.GoToSingleplayerSubmenu/GoToMultiplayerSubmenu/GoToNewGameSubmenu/GoToOptionsSubmenu/GoToLoadGameSubmenu` — `MainMenuButtonHighlight.cs` → `ChangeMenuPage()` (`ui.c:42`).
- `MainMenu.GoBack()` — `MainMenuButtonHighlight.cs` / `StartMenuButtonHighlight.cs` → `MenuGoBack()` (`ui.c:40`).
- `MainMenu.PlayIntro()/PlayCredits()` → `ChangeMenuPage(Mpg_IntroVideo/CreditsVideo)` (`ui.c:59-60`).
- `MainMenu.Quit()` → `OS_Exit(0)` (`ui.c:53`).
- `MainMenu.StartGame()` — `StartMenuButtonHighlight.cs` → `GoIntoGame()` (`voxen.c:653`) via START (`ui.c:168`).
- `Combat/Mission/Puzzle/CyberDifficultyButton.OnDiffButtonClick()` — `StartMenuButtonHighlight.cs` + `StartMenuDifficultyButton.cs` → inline `World.diffCbt/Mis/Puz/Cyb` cycling (`ui.c:153-166`; no separate named function, functionally covered).
- `MainMenu.SetOptionsTabGraphics/Input/Audio()` — `MainMenuButtonHighlight.cs` → `currentMenuTab=0/1/2` via `UI_Button` tabs (`ui.c:80,83,86`).

Missing — each needs a Voxen `HwBtnClick`/`UI_Button`/dispatcher target (or explicit wont-implement). Indexed variants collapsed to one row with counts:

### C1. Weapon / energy tab
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status (`citadel.c` / `ui.c`) |
|---|---|---|---|
| `OverloadButton.OverloadEnergyClick()` | `EnergyOverloadButton.cs`, `UIButtonMask.cs` | `OverloadButton:104,1037` (×2 LH+RH) | Backend PRESENT (`citadel.c:236 OverloadButtonAction()`, `citadel.c:235 VisualState`) — UI dispatch missing in `ui.c` |
| `Inventory.Unload()` | `UIButtonMask.cs` (no dedicated logic `.cs` on node) | `UnloadButton:106,1039` (×2) | Backend PRESENT (`weapons.c:296 Unload()`) — dispatch missing |
| `Inventory.LoadPrimaryAmmoType()` | `UIButtonMask.cs` | `ReloadNormalButton:108,1041` (×2) | Backend PRESENT (`weapons.c:304 LoadPrimaryAmmoType()`) — dispatch missing |
| `Inventory.LoadSecondaryAmmoType()` | `UIButtonMask.cs` | `ReloadAlternateButton:110,1043` (×2) | Backend PRESENT (`weapons.c:317 LoadSecondaryAmmoType()`) — dispatch missing |
| `EnergySlider.SetValue()` | `UIButtonMask.cs` (parent `EnergySlider.cs`) | `Button:118-(14):132` (15× LH) + `1051-1065` (15× RH) = 30× | MISSING — no Voxen equivalent in `citadel.c`/`ui.c` |

### C2. Automap
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `MFDManager.CloseFullmap()` | `MFDManager.cs` (`ScriptsTODO/MFDManager.cs:28`), `UIButtonMask.cs` | `CloseFullmapButton:75` (×1) | STUB ONLY (`citadel.c:772 CloseFullmap(){}` empty, never dispatched) |
| `MFDManager.AutomapZoomIn()` | `MFDManager.cs`, `UIButtonMask.cs` | `ZoomInButton:176,1107` (×2) | MISSING (only `ScriptsTODO/automap.c` commented stubs) |
| `MFDManager.AutomapZoomOut()` | `MFDManager.cs`, `UIButtonMask.cs` | `ZoomOutButton:178,1109` (×2) | MISSING |
| `MFDManager.AutomapGoFull()` | `MFDManager.cs`, `UIButtonMask.cs` | `GoFullButton:180,1111` (×2) | MISSING |
| `MFDManager.ToggleSideTop()` | `MFDManager.cs`, `UIButtonMask.cs` | `GoSideButton:182,1113` (×2) | MISSING |

### C3. Elevator / keycode pads
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `Keypad.Button (1-8).ElevButtonClick()` (8 distinct names, ×2 panels = 16 dispatches) | `ElevatorButton.cs`, `UIButtonMask.cs` (pad `ElevatorKeypad.cs`) | `Keypad.Button (1):194,1125` … `(8):216,1147` | PARTIAL: world-entity backend EXISTS (`citadel.c:228 ElevatorButtonClick(u16 self)` for 3D elevator) — MFD pad floor-select dispatch missing in `ui.c` |
| `MFDManager.CloseElevatorPad()` | `MFDManager.cs` (`ScriptsTODO/MFDManager.cs:144`), `UIButtonMask.cs` | `CloseButton:218,1149` (×2) | MISSING |
| `(no onClick; `KeycodeButton.cs` dispatch)` — Unity `Button->?` + `KeycodeButton.cs[index]` (covers `KeycodeButtonClick()/Keypress(index)` in `ScriptsTODO/KeycodeButton.cs:56-62`) | `KeycodeButton.cs` (pad `KeypadKeycodeButtons.cs`) | `Button (1):223,1154` … `(9):247,1178`, `(-):250,1181`, `(0):253,1184`, `(C):256,1187` (12× LH + 12× RH = 24) | MISSING — no equivalent in `citadel.c`/`ui.c` |
| `MFDManager.CloseKeycodePad()` | `MFDManager.cs` (`ScriptsTODO/MFDManager.cs:154`), `UIButtonMask.cs` | `CloseButton:260,1192` (×2) | MISSING |

### C4. Grid / wire puzzles, system analyzer, minigames
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `PuzzleGridLH.OnGridCellClick([0-34])` (35) | `PuzzleUIButton.cs`, `UIButtonMask.cs` (panel `PuzzleGrid.cs`) | `Button:281` … `Button (34):383` | MISSING (only `ScriptsTODO/PuzzleGrid.cs`) |
| `PuzzleGridRH.OnGridCellClick([0-34])` (35) | `PuzzleUIButton.cs`, `UIButtonMask.cs` (panel `PuzzleGrid.cs`) | `Button:1212` … `Button (34):1314` | MISSING |
| `MFDManager.ClosePuzzleGrid()` | `MFDManager.cs` (`ScriptsTODO/MFDManager.cs:120`), `UIButtonMask.cs` | `CloseButton:393,1324` (×2) | MISSING |
| `PuzzleWireLH.ClickLHNode([0-6])` (7) | `PuzzleUIButton.cs`, `UIButtonMask.cs` (panel `PuzzleWire.cs`) | `NodeBase:406` … `(6):424` | MISSING (only `ScriptsTODO/PuzzleWirePuzzle.cs`) |
| `PuzzleWireLH.ClickRHNode([0-6])` (7) | `PuzzleUIButton.cs`, `UIButtonMask.cs` (panel `PuzzleWire.cs`) | `NodeBase:428` … `(6):446` | MISSING |
| `PuzzleWireRH.ClickLHNode([0-6])` (7) | `PuzzleUIButton.cs`, `UIButtonMask.cs` (panel `PuzzleWire.cs`) | `NodeBase:1337` … `(6):1355` | MISSING |
| `PuzzleWireRH.ClickRHNode([0-6])` (7) | `PuzzleUIButton.cs`, `UIButtonMask.cs` (panel `PuzzleWire.cs`) | `NodeBase:1359` … `(6):1377` | MISSING |
| `MFDManager.ClosePuzzleWire()` | `MFDManager.cs` (`ScriptsTODO/MFDManager.cs:132`), `UIButtonMask.cs` | `CloseButton:456,1387` (×2) | MISSING |
| `SystemAnalyzerDisplayLH.Close()` | `UIButtonMask.cs` (panel `SystemAnalyzer.cs`) | `CloseButton:482` (×1) | MISSING (only `ScriptsTODO/SystemAnalyzer.cs`) |
| `SystemAnalyzerDisplayRH.Close()` | `UIButtonMask.cs` (panel `SystemAnalyzer.cs`) | `CloseButton:1413` (×1) | MISSING |
| `MFDManager.MinigameStart_Ping/15/Wing0/Botbounce/EelZapper/Road/TriopToe/CorporateConquer/Chess` (9) | `MFDManager.cs`, `UIButtonMask.cs` | `MiniGameButton0_Ping:487` … `8_Chess:503` | MISSING |
| `MFDManager.TabReset()` | `MFDManager.cs`, `UIButtonMask.cs` | `MinigameClose:506` | MISSING |
| `MFDManager.OpenMinigames()` | `MFDManager.cs`, `UIButtonMask.cs` | `MinigameBack:512` | MISSING |
| `Ping.ResetOnGameOver()` + `Fifteen.Reset()` (dual target `Button->Ping.ResetOnGameOver()\|Fifteen.Reset()`) | (no `.cs` on node; logic `ScriptsTODO/MinigamePing.cs:287`, `ScriptsTODO/MinigameBotBounce.cs:184`) | `PingGameOver:509` | MISSING |

### C5. Center EReader browser + center inventory lists
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `(no onClick; `MultiMediaLogTableButton.cs` dispatch)` (10 level rows) | `MultiMediaLogTableButton.cs`, `UIButtonMask.cs` | Log-table `Button:761?-` series per `ui.c:1289-1352` comments (10) | MISSING — `PlayLog/PlayLastAddedLog` backend exists (`citadel.c:52-53`) but no browser-row dispatch in `ui.c` |
| `(no onClick; `MultiMediaLogButton.cs` dispatch)` — folder rows (15) | `MultiMediaLogButton.cs`, `UIButtonMask.cs` | `ui.c:1362-1432` block (15) | MISSING |
| `(no onClick; `MultiMediaLogButton.cs` dispatch)` — email rows (26) | `MultiMediaLogButton.cs`, `UIButtonMask.cs` | `ui.c:1456-1581` block (26) | MISSING |
| `(no onClick; `MultiMediaLogButton.cs` dispatch)` — data rows (13) | `MultiMediaLogButton.cs`, `UIButtonMask.cs` | `ui.c:1590-1650` block (13) | MISSING |
| `(no onClick; `LogMoreButton.cs`)` | `LogMoreButton.cs`, `UIButtonMask.cs` | `MoreButton:824` (`ui.c:1441`) | MISSING |
| `(no onClick; `LogBackButton.cs`)` | `LogBackButton.cs`, `UIButtonMask.cs` | `BackButton:826` (`ui.c:1446`) | MISSING |
| `ICEDrill/Pulser/CyberShield/Turbo/Decoy/Recall/Games.SoftInvClick()` (7) + `Turbo/Decoy/Recall.SoftInvClick(7)` use-boxes (3) | `SoftwareInvButton.cs`, `UIButtonMask.cs` / use-box `UIPointerMask.cs` | `ICEDrill:732`, `Pulser:735`, `CyberShield:738`, `Turbo:741+744`, `Decoy:746+749`, `Recall:751+754`, `Games:756` | MISSING — `UI_SoftwareInventory()` is a stub (`ui.c:351 return false`); backend `UseCyberspaceItem/UseTurbo/UseDecoy/UseRecall` exists (`citadel.c:67-70`) but no UI dispatch |
| `(no onClick; `HardwareInvButton.cs` dispatch)` (14 rows) | `HardwareInvButton.cs`, `UIButtonMask.cs` | `HardwareTab Button:644-(13)` | MISSING — `UI_HardwareInventory()` only declared (`ui.c:353`), never defined/dispatched |

### C6. Touch / console buttons
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `Pause.PauseEnable()` | (no `.cs` on node) | `MainMenuTouchButton:1458` | MISSING |
| `(no onClick; touch stick/button)` | `UIButtonMask.cs` (+ `Builtin`) | `ConsoleButton:1460`, `TouchSpace:1466`, `TouchLMB:1468`, `TouchSwimUp:1470`, `TouchSwimDown:1472` | MISSING — no touch handling in `citadel.c`/`ui.c` |
| `Pause.ConsoleEntryEnterDelegate()` | (no `.cs` on node) | `TouchEnterButton:2135` (under `Console:2130`) | MISSING |

### C7. Pause menu (Voxen `RenderPausedUI` covers shell only; dialogs unmapped)
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `Pause.LoadPause()` | `StartMenuButtonHighlight.cs` | `Load:1481` | MISSING (Voxen LOAD goes to `Mpg_Load` menu page, not `Pause.LoadPause` semantics) |
| `Pause.SavePause()` | `StartMenuButtonHighlight.cs` | `Save:1483` | MISSING (same; no save-confirm dialog path) |
| `Pause.PauseOptions()` | `StartMenuButtonHighlight.cs` | `Options:1485` | MISSING as named callback (Voxen goes to `Mpg_Options` via its own pause-options branch) |
| `Pause.OpenSaveDialog()` | `StartMenuButtonHighlight.cs` | `PauseQuit:1487` | MISSING — no `SaveQuestion` dialog in `ui.c` |
| `Pause.OpenSaveDialogHard()` | `StartMenuButtonHighlight.cs` | `PauseQuitHard:1489` | MISSING — no `SaveHardQuestion` dialog |
| `Pause.SavePauseQuit()` | `StartMenuButtonHighlight.cs` | `YesButton:1498` (`SaveQuestion`) | MISSING |
| `Pause.NoSavePauseQuit()` | `StartMenuButtonHighlight.cs` | `NoButton:1500` | MISSING |
| `Pause.ExitSaveDialog()` | `StartMenuButtonHighlight.cs` | `CancelButton:1502`, `NoButton:1512` | MISSING |
| `Pause.PauseQuitHard()` | `StartMenuButtonHighlight.cs` | `YesButton:1510` (`SaveHardQuestion`) | MISSING |
| Note: `Pause.PauseDisable()` (`Resume:1479`, `StartMenuButtonHighlight.cs`) is NOT missing — covered by `World.paused=false` (`ui.c:191`). | | | |

### C8. Main menu / options / load-save (only genuinely absent dispatches; renamed navigation excluded per list above)
| Unity callback (missing) | `.cs` file(s) | Unity buttons (layout) | Voxen status |
|---|---|---|---|
| `MainMenu.LoadGame()` + `LoadGame(1-7)` (8 slots) | `MainMenuButtonHighlight.cs` | `LoadButton:1991` … `(7):2012` | MISSING UI dispatch — backend `LoadGame()` exists (`entity.c:704`; quick-load `F9` in `winput.c:413`) but `Mpg_Load` renders only background+BACK (`ui.c:135-143`), no slot rows |
| `MainMenu.SaveGameEntry()` + `SaveGameEntry(1-6)` + `SaveQuickSaveButton(7)` (8 slots) | `MainMenuButtonHighlight.cs` | `SaveButton:2021` … `(7):2063` | MISSING UI dispatch — backend `SaveGame()` exists (`entity.c:698`; quick-save `F6` in `winput.c:412`) but `Mpg_Save` has no slot rows |
| `VideoResolutionApply.OnApplyClick()` | `ConfigurationMenuVideoApply.cs` | `VideoResolutionApply:1562` | MISSING |
| `Keybind.KeybindButtonClick()` + `(1-39)` (40) | `ConfigKeybindButton.cs` | `Keybind:1710` … `(39):1788` | MISSING — `Mpg_Options/Input` renders nothing (`ui.c:122-123`) |
| `MainMenu.SetConfigPreset()` / `SetConfigPreset(1)` | (no `.cs` on node) | `SetPresetDefault:1815`, `SetPresetLegacy:1817` | MISSING |
| `MainMenu.ApplyPreset()` / `CancelPresetSet()` | (no `.cs` on node) | `YesButton:1922`, `NoButton:1924` (`SetPresetQuestion`) | MISSING |
| `MainMenu.PathSearch()` | (no `.cs` on node) | `PathSearchButton:2111` | MISSING |
| `MainMenu.CopyFromPath()` | (no `.cs` on node) | `CopyFromPathButton:2115` | MISSING |
| `MainMenu.CloseDataFileNotification()` | (no `.cs` on node) | `MenuCheckForRESCloseButton:2113` | MISSING |

---

## D — `ScriptsTODO/` vs loose `.c` (particularly `ui.c`): fully implemented → delete local copy

Review basis: all 35 `ScriptsTODO/` entries read against loose top-level `.c` (especially `ui.c:1826` + `citadel.c:936` + `biomonitor.c:93`, with `weapons.c`/`winput.c`/`entity.c` backend noted). Criterion for DELETE is strict: **every** behavior in the TODO script has a working equivalent in compiled loose `.c` (same or renamed function, verified by body comparison, not name match alone). Reference-only ports in `ScriptsTODO/` are never counted as existing. `EntityNeedsCanvas.txt` is a doc note, not a script — excluded from delete list.

### D1. Fully implemented → safe to delete local repo copy (1 file)

- `ScriptsTODO/ItemTabManager.cs` (128 lines: `Reset()`, `EReaderSectionSContainerOpen()`, `SendItemDataToItemTab(index,custIdx)`) → DELETE (`rm ScriptsTODO/ItemTabManager.cs`).
  - `Reset()` ≡ `MFD_ResetGeneral()` + `MFD_GeneralChanged()` (`ui.c:10,55`) combined with `RenderGeneralItem`/`RenderConsumableItem` guards (`ui.c:454-455,549-551`: render nothing when no item/consumable selected).
  - `EReaderSectionSContainerOpen()` (hide apply/vaporize/use/timers, show reader + icon 23 + string 349) ≡ side-reader path: EReader branch in `UI_ProcessNavigation` (`ui.c:586-590`: `MFD_ResetGeneral`, `MFD_CenterTab=5`, `LefTab=2`, `mfdItemReader[0]=true`) + section buttons render (`ui.c:292-299`) + `SideMFD` reader dispatch (`ui.c:604-610`).
  - `SendItemDataToItemTab()`: head `92/93/94` + 19-entry `custIdx` table → identical `heads[19]={37,11,32,1,7,9,10,12,13,14,15,17,25,27,28,31,33,35,36}` + `tex=1272+heads` (`ui.c:556-559`); icon `useableItemsIcons[constIndex]` + `stringTable[constIndex+326]` ≡ `GetItemFrobTexture(item+307)` + `GeneralInvLabel()` (`ui.c:423,553-555`, `citadel.c` item tables); access-card list for `34/81/110/83-91` ≡ access-card loop for slot 0 (`ui.c:561-567` + `citadel.c:27-29`); apply for `14-21/52/53/55` ≡ split across `GeneralInvCanUse(52/53/55)` (`citadel.c:279`) + consumable `ConsumableSelect/Use` (`ui.c:441-448`); vaporize for `<6/33/35/58/62` ≡ exact `GeneralInvCanVaporize()` (`citadel.c:280`); nitro/earthshaker timer for `12/10` ≡ `RenderConsumableItem` timer for `row>=5` (`ui.c:460-464`, `grenadeItems` maps rows 5/6 to `12/10`) + `ConsumableSetTimer()` (`ui.c:456-459`).
  - No remaining behavior in the file lacks a loose-`.c` counterpart. `citadel.c:29 AccessCardCodeForType` even carries a `// Called by ItemTabManager` comment from the port.

### D2. NOT fully implemented → KEEP (do not delete)

| `ScriptsTODO/` file | Why it stays (what is missing in loose `.c`, esp. `ui.c`) |
|---|---|
| `automap.c` (578 lines, body commented out) | `ui.c:301` automap stub renders nothing; §A4 + §C2 fully missing (zoom/full/side/close, FoW, overlays). No compiled automap system. |
| `MFDManager.cs` (1311 lines) | Only a subset ported (`MFD_SelectTab`, `MFD_Open/CloseSearch`+`CloseSearch`, `GeneralInvApply` for `ApplyButtonClicked`, search render/tether). Missing per §C: `AutomapGoFull/CloseFullmap`, `Enter/ExitCyberspace`, `ClosePuzzleGrid/Wire/Elevator/Keycode`, `OpenTab`, `SendGrid/Wire/Search/PaperLog/AudioLog/KeypadKeycode`, `BlockedBySecurity` tab, `ShowAmmo/EnergyItems`, `SetWepInfo` (partial name+icon only), `Open*MultiMedia`, `OpenMinigames/MinigameStart_*`, ammo-count/highlight paths. |
| `PuzzleGrid.cs` (975) + `PuzzleGridPuzzle.cs` (106) | Render shell only in `ui.c:819-1043`; `OnGridCellClick/EvaluatePuzzle/SendGrid/Reset/UseTargets` (§C4) absent in `citadel.c`/`ui.c`. |
| `PuzzleWire.cs` (664) + `PuzzleWirePuzzle.cs` (103) | Same: render shell only (`ui.c:1044-1140`); `ClickLH/RHNode/SendWirePuzzleData/Reset` (§C4) absent. |
| `KeypadKeycodeButtons.cs` (139) + `KeycodeButton.cs` (68) + `KeypadKeycode.cs` (94) + `KeypadElevator.cs` (49) + `KeycodeDigitImage.cs` (24) | Pad renders exist as `UNMAPPED` placeholders (`ui.c:670-806`) but zero entry/dispatch logic: no `Keypress/KeypressAction/SetDigit`, no numpad polling, no quest-code decode, no `SendElevatorKeypad/KeypadKeycodeToDataTab`, no digit-image update, no `SendDataBackToPanel` MFD wiring (§C3). `citadel.c:228 ElevatorButtonClick` is the 3D-world elevator, not the MFD pad. |
| `SystemAnalyzer.cs` (84) | `ui.c:1141-1192` renders hardcoded placeholders (`100%%/Charging/Disabled/...`), not live `levelSecurity/questData/nodeCount/program` `Update()`; `Close()` (§C4) undispatched. |
| `Minigame15.cs` (287) + `MinigameBotBounce.cs` (187) + `MinigamePing.cs` (290) + `MinigameTriopToe.cs` (216) + `MinigameCursor.cs` (53) | Only menu shell rendered (`ui.c:1193-1248`); game logic (`Reset/Slide`, ball/paddle/score, minimax, panel cursor mapping, `ResetOnGameOver`) absent (§C4). |
| `MissionTimer.cs` (97) | Logic PORTED to `citadel.c` (`MissionTimerInit/UpdateToNextMission/Update`, new `World.misTimerTimesUP/misTimerMission` in `common.h`, ticked from `ModUpdate`, init from `NewGame`) and render WIRED in `ui.c` (MissionTimerT←`misTimerMission` label, MissionTimer←`MM:SS` from `misTimerT` / `869` when `timesUP`); only the `QuestLogNotesManager.UpdateToNextMission` cross-notify still absent — KEEP file until that notify has an equivalent. |
| `QuestLogNotesManager.cs` (269) | Only quest-bit→note side effects ported (`citadel.c:722 QuestBitNoteSideEffects`); `LogAdded/NodesDestroyed/NotifyLevelChange/UpdateToNextMission` dynamic label/sec-code updates absent (`AddAudioLogToInventory`, `ui.c:1653-1725` static labels do not replicate them). |
| `NewGameGraphSystem.cs` (377) | Fake-data menu graph (`erg 0.35/chi sine/5 s beat`, `620×36`) not ported; `biomonitor.c:18-92` ports the *live* biomonitor graph (fatigue/energy/FPS-driven, `0.0211/0.05/0.0104/0.02` ticks), which is the sibling `BiomonitorGraphSystem`, not this redesigned fake variant. |
| `TickIndicatorAnimation.cs` (61) | `TickBar()` (`ui.c:277-283`) replicates the `88/176` thresholds and base indicators (`939/956`) but not the `thinkTime 0.5 s` 7-sprite low-health/energy blink cycle (`indicatorImages[0-6]`). Partial only. |
| `ImageSequenceTextureArrayUI.cs` (85) | `BlockedBySecurityLH` rendered once as static `1110` (`ui.c:663`); frame-sequence player (`resourceFolder/frameDelay/stopAtEnd/replayOnEnable/deactivateAtEnd`) absent. |
| `TeleportFXStatic.cs` (63) | `TeleportFX/RadiationFX` (§A1) have no `RenderUIImage`, no flip/deactivate/cursor-hide cycle. |
| `TouchEnergyDrain.cs` (31) | Backend `TakeEnergy()` (`citadel.c:263`) exists, but collision-gated `drainage/tick` drainer component absent. |
| `InteractablePanel.cs` (79) | Generic `ButtonSwitchUse/DoorUse/UseTargets` exist, but `requiredIndex` (incl. Abe-head `92/cust 1`), `installationItem/effects`, open/install messaging flow absent. |
| `UseableAttachment.cs` (70) | `UseTargets/SpawnExplosionEffect/GrenadeExplode` exist, but attachment `56/57/61/64` dispatch + plastique timer/destructables flow absent. |
| `ObjectImpact.cs` (35) | No `OnCollisionEnter` min/max-speed impact-SFX component in `physics.c`/`audio.c`. |
| `PlaySoundTriggered.cs` (85) | `play_wav` backend exists, but `SFXClip/loopingAmbient/playEverywhere/particle-emit` trigger component absent. |
| `SpawnManager.cs` (81) | `SpawnDynamicObject()` (`citadel.c:773`) exists, but difficulty-scaled repopulating spawner (`numberToSpawn/numberActive/min/max/allSpawnedResetDelay/AreaClear/AreaHidden`) absent. |
| `SecurityCameraRotate.cs` (52) | No back-and-forth `startY/endY/waitTime/degreesYPerSecond` camera component in loose `.c`. |
| `MaterialChanger.cs` (30) + `MaterialFlash.cs` (69) | `TextureChangerToggle()` (`citadel.c:178`) is the texture path, not the mesh-material code/sec-code (`MaterialChanger`) or self-destruct flashing light (`MaterialFlash`) flows. |
| `PaperLog.cs` (13) | `PlayLog/AddAudioLogToInventory` backend exists, but `Use()` → `CenterTabButtonClickSilent(4)` + `SendPaperLogToDataTab` + `ForceInventoryMode` dispatch absent (§C5-adjacent). |
| `KeypadKeycode.cs` covered above; `EntityNeedsCanvas.txt` (8, doc note) | Not code; keep as reference, not a delete candidate. |

---

## E — `?` fields in `ui.c` comments: what the prior model meant, Citadel ground truth, and what was written into `ui.c` (APPLIED — see E1/E2 for the exact replacements now in `ui.c`)

`ui.c` was NOT edited for this section. All line numbers below refer to current `ui.c` (1362 lines). Prior-model conventions were recovered from `Tools/ui_layout.txt` (scene-hierarchy dump, `Button->` + `tex=?[...]` fields) and `Tools/ui_texmap.txt` (texture cross-ref, `?` = index not yet assigned).

### E1. `BTN <name>: ?` (78×) — means “Unity scene has NO onClick”, NOT “no image”

The guess “`?` = no image, text-only button” is incorrect for these 78. Every one of them HAS an image in Unity (`keypad_inner_on.png`, or `builtin-white` hitbox quads rendered as tex `0` in `ui.c`).

Ground truth:
- `Tools/ui_layout.txt` shows the Unity Button component’s persistent onClick call as `Button->?` (e.g. line 223: `Button (1) … Button->? C#:KeycodeButton.cs[index=1]`). `?` is the layout dumper’s faithful copy of “no persistent listener in `CitadelScene.unity`”.
- The click handling is registered at runtime in C# `Start()` via `GetComponent<Button>().onClick.AddListener(...)`, so it is invisible to the scene-hierarchy dump. Verified in `~/ai-workspaces/Citadel/Assets/Scripts/`:
  - `KeycodeButton.cs`: `Start()` → `KeycodeButtonClick()` → `KeycodeButtonUse()` → `KeypadKeycodeButtons.Keypress(index)`. Per-button `index` params (`1-9`, `10`=`-`/backspace, `0`, `11`=`C`/clear) are already in `ui_layout.txt` (`C#:KeycodeButton.cs[index=N]`, lines 223-256, ×2 LH+RH panels but `ui.c` shares one `dx` block so 12 comment lines: `ui.c:747-780`).
  - `MultiMediaLogTableButton.cs`: `Start()` → `LogTableButtonClick()` → `OpenLogsLevelFolder(logTableButtonIndex)`. `logTableButtonIndex 0-9` already in layout (lines 761-788); 10 comment lines `ui.c:1060-1087`.
  - `MultiMediaLogButton.cs`: `Start()` → `LogButtonClick()` → reader open + `PlayLog(logReferenceIndex)` + `SendLogData`. `logButtonIndex/logReferenceIndex/isEmailButton` params already in layout; 15 folder rows (`ui.c:1092-1120`), 26 email rows (`ui.c:1133-1183`), 13 data rows (`ui.c:1188-1212`).
  - `LogMoreButton.cs`: `Start()` → `LogMoreButtonClick()` — pages the reader text in 568-char chunks, else closes the reader (`ResetMultiMediaTabs/ClearDataTab/ReturnToLastTab`). 1 line `ui.c:1125`.
  - `LogBackButton.cs`: `Start()` → `LogBackButtonClick()` — restores full text from `audioLogSpeech2Text[refIndex]`. 1 line `ui.c:1127`.
- No prefab lookup needed: these buttons are scene objects (confirmed via `m_Name` in `CitadelScene.unity`), and all per-button params are already captured in `ui_layout.txt`. No further data needs obtaining from the Citadel repo for the `?` itself; the remaining work is Voxen dispatch (§C3-keycode, §C5-browser), not comment research.
- Applied `ui.c` replacements (comment-only, matching the file's existing `BTN <name>: <Callback>() <scripts>` convention — e.g. `BTN CloseButton: MFDManager.CloseElevatorPad() UIButtonMask.cs`):
  - `BTN Button (1): ? KeycodeButton.cs` → `BTN Button (1): KeycodeButtonClick() KeycodeButton.cs` (same pattern for `(2-9)`, `(-)`, `(0)`, `(C)` — 12 lines `ui.c:745-778`).
  - `BTN Button: ? UIButtonMask.cs,MultiMediaLogTableButton.cs` → `BTN Button: LogTableButtonClick() UIButtonMask.cs,MultiMediaLogTableButton.cs` (same for `(1-9)` — 10 lines `ui.c:1054-1081`).
  - `BTN Button: ? UIButtonMask.cs,MultiMediaLogButton.cs` → `BTN Button: LogButtonClick() UIButtonMask.cs,MultiMediaLogButton.cs` (same for `(1-14)/(1-25)/(1-12)` folder+email+data — 54 lines `ui.c:1086-1206`).
  - `BTN MoreButton: ? UIButtonMask.cs,LogMoreButton.cs` → `BTN MoreButton: LogMoreButtonClick() UIButtonMask.cs,LogMoreButton.cs` (`ui.c:1119`).
  - `BTN BackButton: ? UIButtonMask.cs,LogBackButton.cs` → `BTN BackButton: LogBackButtonClick() UIButtonMask.cs,LogBackButton.cs` (`ui.c:1121`).

### E2. Truncated elevator `BTN` (8×) — prior model dropped the method name; recoverable from layout+scene, no new Citadel data needed

`ui.c:719-741` read `BTN Keypad.Button (N): Keypad.Button ElevatorButton.cs,UIButtonMask.cs`. `Tools/ui_layout.txt:194-216` has the full persistent call `Button->Keypad.Button (N).ElevButtonClick()` with `C#:ElevatorButton.cs[floorAccessible,doorOpen]`, confirmed by `m_MethodName: ElevButtonClick` in `CitadelScene.unity` and `ElevatorButton.cs:ElevButtonClick()` (`linkedElevatorDoor`/distance/`floorAccessible` → `LevelManager.LoadLevel`). The floor-letter/digit mapping comes from the child `Text (N)` (`R/1/2/3/6/7/8/9`, `ui_layout.txt:195-217`) via `ElevatorButton.cs:OnEnable` (`levR/lev1…levC` → `levelIndex`), already rendered as `RenderTextL` in `ui.c`.
- Applied fix (per N=1..8, 8 lines): `BTN Keypad.Button (N): Keypad.Button` → `BTN Keypad.Button (N): ElevButtonClick()` (script suffix `ElevatorButton.cs,UIButtonMask.cs` kept).

### E3. `UNMAPPED:` / `QUAD:` / texmap `?` — texture-mapping flags, distinct from BTN `?`

- `UNMAPPED:[Unity/asset/path]` (138× in `ui.c`, e.g. `/*BlockedBySecurityLH UNMAPPED:[Resources/BlockedBySecurity/blocked_00…`) mirrors `tex=?[path]` in `ui_layout.txt` and `? | <path> | UNMAPPED` rows in `Tools/ui_texmap.txt`. It means “Unity references this asset; Voxen texture index was unassigned when the comment was written.”
- Stale `UNMAPPED` (do NOT re-obtain; just refresh the comment once editing resumes): `keypad_end/mid/inner_on.png` (2133/2135/2134), `elnum_null.png` (2132), all `puzzle/*` + `wire_*` (2136-2146). These now have indices in `Tools/ui_texmap.txt:65,69-71,90-100` AND entries in `Data/textures.txt:4550-4577`, but `ui.c:718-786,796-963` comments still say `UNMAPPED`. Action: drop the `UNMAPPED:` prefix (keep the path for traceability) after confirming the rendered tex id matches.
- Genuinely unmapped (`?` first column in `Tools/ui_texmap.txt:55,57-64,66-89,101-104`, ~30 assets) — Citadel ground truth verified (representative paths exist under `~/ai-workspaces/Citadel/Assets/`): `healthticks24.png` (§A1 panel bgs), `hud_hwico_shield.png` (ShieldButton), `hudbuttons/0038_0031.png` (ClipBox), `nullsprite.png`/`blank.png` (ammo/icon placeholders), `itemicons/paperico.png` (ItemIcon), automap set (`AutomapRender.renderTexture`, `hudmaps/L1.png`, `L*_hazards.png`, `automap_side_LG1/LG2.png`, `circle_lg/sm.png`, `player_180.png`), `painstatic_blue1/red4.png`, `sightDimming.png`, `teleportstatic_red/white.png`, `shieldwave.png` is mapped (1076) but its waveup/down renders are missing (§A1), render textures (`ConfigCameraView`, `MiniGamesRT`, `VideoDisplayRT`), touch sticks, `centertab.png`, `checkmarkbloom/ssao.png`. Action for a later pass: obtain each (already present in Citadel repo — no prefab/scene search needed beyond the asset file itself), append to `Data/textures.txt`, fill the `?` in `Tools/ui_texmap.txt`, then replace the corresponding `UNMAPPED`/missing-render comments in `ui.c` (see §A1/A2/A4).
- `QUAD:builtin-white|builtin-knob|none` + tex `0` (134×, e.g. `/*ButtonBankLH QUAD:builtin-knob*/`, `/*MinigameClose QUAD:none …*/`) is CORRECT as-is: Unity’s Image is a built-in/no sprite, so Voxen renders an untextured quad. No Citadel asset to obtain and no `?` to resolve. Do not “fix” these into `UNMAPPED`.
- `tex=?[path]` lives only in `Tools/ui_layout.txt`, never in `ui.c`; resolve it through `Tools/ui_texmap.txt`, not by editing `ui.c` directly.

### E4. `?` strings that must NOT be touched in a future `?`-replacement edit

- Puzzle-cell labels `RenderTextL(…,0.6,"?")` (`ui.c:804-905`, 35×) render the literal Unity child text `"?"` on unsolved grid cells — content, not an unknown field.
- Ternary `cond ? a : b` operators throughout `ui.c` — code, not comments. A mechanical find/replace of `?` would corrupt logic; restrict any future edit to the `BTN …: ? <Script>.cs` pattern inside `/*…*/` comments (78 lines listed in E1) plus the 8 elevator truncations in E2.


