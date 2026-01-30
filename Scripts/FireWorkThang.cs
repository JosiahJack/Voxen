using UnityEngine;
using UnityEngine.UI;

[RequireComponent(typeof(Image))]
public class FireWorkThang : MonoBehaviour {
    public float minScale;
    public float maxScale;
    public float changeFracPerSecond = 0.5f;
    public float waitTimeFull = 0.8f;
    public float waitTimeMinMin = 0.8f;
    public float waitTimeMinMax = 3f;
    private Image img;
    private float tickFinished;
    private float curScale;
    private bool waitAtFull;
    private bool waitAtMin;

    void Awake() {
        img = GetComponent<Image>();
    }

    void OnEnable() {
        tickFinished = Sys_Global.pauseRelativeTime;
        curScale = random_range(minScale,maxScale);
        if (changeFracPerSecond < 0.001f) changeFracPerSecond = 0.5f; // 2 secs
        waitAtFull = false;
        if (minScale >= maxScale) {
            minScale = 0f;
            maxScale = 1f;
            DualLogWarning("FireWorkThang maxScale not set higher than min");
        }
    }

    void Update() {
        if (Sys_Global.gamePaused) return;
        if (Sys_Global.menuActive) return;
        if (tickFinished >= Sys_Global.pauseRelativeTime) return;

        float delta = (1f / 60f);
        tickFinished = Sys_Global.pauseRelativeTime + delta;
        if (waitAtFull) {
            waitAtFull = false;
            waitAtMin = false;
            curScale = minScale;
            waitAtMin = true;
            tickFinished = Sys_Global.pauseRelativeTime
                           + random_range(waitTimeMinMin,waitTimeMinMax);
        } else if (waitAtMin) {
            waitAtMin = false;
            waitAtFull = false;
            curScale += (maxScale - minScale) * 0.333f;
        } else {
            curScale += (changeFracPerSecond * (maxScale - minScale)) / 60f;
        }

        if (curScale > maxScale) {
            curScale = maxScale;
            waitAtFull = true;
            tickFinished = Sys_Global.pauseRelativeTime + waitTimeFull;
        }

        instances[i].scale = new Vector3(curScale,curScale,curScale);
    }
}
