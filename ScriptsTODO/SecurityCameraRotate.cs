// Rotates a security camera back and forth between two angle values, pausing
// at each angle value for an instance-set amount of time.  This assumes that
// the current transform is either the camera directly or a wrapper GameObject
// that contains a child whose drooped angle will be preserved while this
// parent transform rotates along true up/down axis.
float startYAngle = 0f;
float endYAngle = 180f;
float waitTime = 0.8f;
bool active;
bool rotatePositive; // save
float degreesYPerSecond = 4f;
float waitingFinished = 0f;
void Start () {
    waitingFinished = World.pauseRelativeTime;
    rotatePositive = true;
    active = (this.enabled);
    if (mR == null) mR = gameObject.GetComponentInChildren<MeshRenderer>(true);
}

void Update() {
    if (!World.paused && !World.menuActive) {
        if (mR != null) {
            if (!mR.isVisible || !mR.enabled) return;
        } else { mR = gameObject.GetComponentInChildren<MeshRenderer>(true); return; }

        if (waitingFinished < World.pauseRelativeTime) {
            if (rotatePositive) RotatePositive();
            else                RotateNegative();
        }
    }
}

void RotatePositive () {
    if (((World.instances[i].rotation.eulerAngles.y + 1f) >= endYAngle)
        && ((World.instances[i].rotation.eulerAngles.y - 1f) <= endYAngle)) {
        rotatePositive = false;
        waitingFinished = World.pauseRelativeTime + waitTime;
        return;
    }
    
    transform.Rotate((V3){0,degreesYPerSecond * 0.1f,0),Space.World);
}

void RotateNegative () {
    if (((World.instances[i].rotation.eulerAngles.y + 1f) >= startYAngle) && ((World.instances[i].rotation.eulerAngles.y - 1f) <= startYAngle)) {
        rotatePositive = true;
        waitingFinished = World.pauseRelativeTime + waitTime;
        return;
    }
    
    transform.Rotate((V3){0,degreesYPerSecond * 0.1f * -1,0),Space.World);
}
