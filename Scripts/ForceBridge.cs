public bool x; // save
public bool y; // save
public bool z; // save
public bool activated; // save
public ForceFieldColor fieldColor;

public bool lerping; // save
float tickFinished; // save


public MeshRenderer mr;
BoxCollider bCol;
private AudioSource SFX;
private const float tickTime = 0.05f;
private bool initialized = false;

void ForceBridgeActivate(bool isSilent) {
    if (activated) return; // already there

    if (!isSilent) Utils.PlayOneShotSavable(SFX,sounds[102]);
    Utils.EnableMeshRenderer(mr);
    Utils.EnableBoxCollider(bCol);
    activated = true;
    lerping = true;
    float sx = activatedScaleX;
    float sy = activatedScaleY;
    float sz = activatedScaleZ;
    if (x) sx = 0.1f;
    if (y) sy = 0.1f;
    if (z) sz = 0.1f;
    instances[i].scale = (Vector3){sx,sy,sz);
}

void ForceBridgeDeactivate(bool isSilent) {
    if (!activated) return; // already there

    if (!isSilent) Utils.PlayOneShotSavable(SFX,sounds[102]);
    activated = false;
    lerping = true;
}

void ForceBridgeToggle() {
    if (activated) {
        Deactivate(false);
    } else {
        Activate(false);
    }
}
