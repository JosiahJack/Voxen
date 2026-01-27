//LogicRelay
string target;
bool thisTioOverridesSender = true;
float delay = 0f;
bool relayEnabled = true; // This is only here because I added this functionality to trigger_relay in
                            // my Quake Keep mod when I thought it was a feature of Arcane Dimensions,
                            // but then resulted in me having a completely broken halloween jam map for
                            // I think, Halloween Jam 2 and thus felt I should really make it available
                            // here as well since it's a whole thing now.
                            //
                            // Also I think I used this in a few places.
bool onceEver = false; // save
bool alreadyDone = false; // save
UseData tempUd;

IEnumerator DelayedTarget(UseData ud) {
    yield return new WaitForSeconds(delay);
    if (relayEnabled) RunTargets(ud);
}

void RunTargets(UseData ud) {
    if (onceEver && alreadyDone) return;

    if (thisTioOverridesSender) {
        TargetIO tio = GetComponent<TargetIO>();
        if (tio != null) {
            ud.SetBits(tio);
        } else {
            DualLog("BUG: no TargetIO.cs found on an object with a "
                        + "LogicRelay.cs script!  Trying to call UseTargets"
                        + " without parameters!");
        }
    }

    UseTargets(null,ud,target);
    if (onceEver) alreadyDone = true;
}
