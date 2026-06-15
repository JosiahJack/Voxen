import os
import re
from collections import defaultdict

# Configuration
DATA_DIR = './Data'
LEVEL_FILES = [f'level{i}.txt' for i in range(14)]

# Exact public bool fields from TargetIO.cs
VALID_IO_FLAGS = {
    "tripTrigger", "doorOpen", "doorOpenIfUnlocked", "doorClose", "doorLock", "doorUnlock",
    "switchTrigger", "chargeStationRecharge", "enemyAlert", "forceBridgeActivate",
    "forceBridgeDeactivate", "forceBridgeToggle", "gravityLiftToggle", "textureChangeToggle",
    "lightOn", "lightOff", "lightToggle", "funcwallMove", "missionBitOn", "missionBitOff",
    "missionBitToggle", "sendEmail", "switchLockToggle", "lockCodeToScreenMaterialChanger",
    "spawnerActivate", "spawnerActivateAlerted", "cyborgConversionToggle", "GOSetActive",
    "GOSetDeactive", "GOToggleActive", "toggleRadiationTrigger", "disableThisGOOnAwake",
    "toggleRelayEnabled", "togglePuzzlePanelLocked", "testQuestBitIsOn", "testQuestBitIsOff",
    "playSoundOnce", "stopSound", "sendSprintMessage", "radiationTreatment",
    "startFlashingMaterials", "stopFlashingMaterials", "unlockElevatorPad", "unlockKeycodePad",
    "unlockPuzzlePad", "screenShake", "awakeSleepingEnemy", "branchFlip", "branchFlipOnly",
    "doorAccessCardOverrideToggle", "unlockSwitch", "lockElevatorPad", "alreadyDisabledThisGOOnceEver",
    "doorToggle"
}

def parse_entity_line(line):
    entity = {}
    parts = line.strip().split('|')
    for part in parts:
        if not part or ':' not in part:
            continue
        key, value = part.split(':', 1)
        key = key.strip()
        value = value.strip()
        entity[key] = value
    return entity

def is_valid_target_name(name: str) -> bool:
    return bool(name) and re.match(r'^[a-zA-Z0-9_]+$', name)

def main():
    # targetname -> list of (level, line_num)
    all_targetnames = defaultdict(list)
    # List of all targeting connections: (targetter_level, targetter_line, target_value, targetname_level, targetname_line)
    connections = []
    
    io_flags_count = defaultdict(int)
    suspicious_targets = []
    suspicious_targetnames = []

    for level_num in range(14):
        filename = os.path.join(DATA_DIR, f'level{level_num}.txt')
        if not os.path.exists(filename):
            print(f"Warning: {filename} not found. Skipping.")
            continue
        
        print(f"Processing {filename}...")
        with open(filename, 'r', encoding='utf-8') as f:
            for line_num, line in enumerate(f, 1):
                if not line.strip():
                    continue
                
                entity = parse_entity_line(line)
                
                targetname = entity.get('targetname')
                if targetname:
                    all_targetnames[targetname].append((f'level{level_num}', line_num))
                    if not is_valid_target_name(targetname):
                        suspicious_targetnames.append((targetname, f'level{level_num}', line_num))
                
                target = entity.get('target')
                if target:
                    targetter_loc = (f'level{level_num}', line_num)
                    if not is_valid_target_name(target):
                        suspicious_targets.append((target, f'level{level_num}', line_num))
                    
                    # Link to all matching targetnames
                    if target in all_targetnames:
                        for tn_loc in all_targetnames[target]:
                            connections.append((*targetter_loc, target, *tn_loc))
                    else:
                        # Still record the attempt even if orphan
                        connections.append((*targetter_loc, target, None, None))

                # IO Flags
                for key, value in entity.items():
                    if key in VALID_IO_FLAGS and value == '1':
                        io_flags_count[key] += 1

    # === Reports ===

    print("\n" + "="*100)
    print("=== ALL TARGET → TARGETNAME CONNECTIONS ===")
    print("="*100)
    print(f"{'Targetter':<12} {'Line':<6} {'→ Target':<30} {'Targettee':<12} {'Line'}")
    print("-" * 100)
    
    for tgt_level, tgt_line, target_val, tn_level, tn_line in sorted(connections, key=lambda x: (x[0], x[1])):
        if tn_level is None:
            print(f"{tgt_level:<12} {tgt_line:<6} → {target_val:<30} (ORPHAN - no targetname)")
        else:
            print(f"{tgt_level:<12} {tgt_line:<6} → {target_val:<30} {tn_level:<12} {tn_line}")

    print("\n" + "="*80)
    print("=== IO FLAG USAGE COUNTS (only flags set to 1) ===")
    print("="*80)
    for flag, count in sorted(io_flags_count.items(), key=lambda x: -x[1]):
        print(f"{flag:35} : {count:4} times")

    # Suspicious names
    print("\n" + "="*80)
    print("=== SUSPICIOUS TARGETS (invalid chars) ===")
    print("="*80)
    for t, lvl, ln in sorted(suspicious_targets):
        print(f"  '{t}'  @ {lvl} line {ln}")

    print("\n" + "="*80)
    print("=== SUSPICIOUS TARGETNAMES (invalid chars) ===")
    print("="*80)
    for tn, lvl, ln in sorted(suspicious_targetnames):
        print(f"  '{tn}'  @ {lvl} line {ln}")

    # Orphans
    all_targets = {conn[2] for conn in connections if conn[2]}
    orphaned_targets = [t for t in sorted(all_targets) if t not in all_targetnames]
    orphaned_targetnames = [tn for tn in sorted(all_targetnames) if tn not in all_targets]

    print("\n" + "="*80)
    print("=== ORPHANED TARGETS (no matching targetname) ===")
    print("="*80)
    for t in orphaned_targets:
        count = sum(1 for c in connections if c[2] == t)
        print(f"  • {t}  ({count} references)")

    print("\n" + "="*80)
    print("=== ORPHANED TARGETNAMES (never targeted) ===")
    print("="*80)
    for tn in orphaned_targetnames:
        print(f"  • {tn}  (exists in {len(all_targetnames[tn])} places)")

    print(f"\nSummary:")
    print(f"   Total connections   : {len(connections)}")
    print(f"   Unique targets      : {len(all_targets)}")
    print(f"   Unique targetnames  : {len(all_targetnames)}")
    print(f"   Orphaned targets    : {len(orphaned_targets)}")
    print(f"   Orphaned targetnames: {len(orphaned_targetnames)}")

if __name__ == "__main__":
    main()
    
