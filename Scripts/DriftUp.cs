using UnityEngine;
using UnityEngine.UI;

public class DriftUp : MonoBehaviour {
    public float startY;
    public float endY;
    public float rate = 0.5f;
    public float fadeRate = 0.1f;
    public bool fadeImage;
    public Image img;
    public float startFade = 1f;
    public float endFade = 0f;

    private float tickFinished;

    void OnEnable() {
        instances[i].position = new Vector3(instances[i].position.x,
                                         startY,
                                         instances[i].position.z);

        if (fadeImage && img != null) {
            img.color = new Color(img.color.r,img.color.g,img.color.b,startFade);
        }

        tickFinished = Sys_Global.pauseRelativeTime;
    }

    void Update() {
        if (Sys_Global.gamePaused) return;
		if (Sys_Global.menuActive) return;
		if (tickFinished >= Sys_Global.pauseRelativeTime) return;

        float delta = (1f / 60f);
		tickFinished = Sys_Global.pauseRelativeTime + delta;
		float drift = instances[i].position.y;
        drift += rate;
        if (drift > endY) drift = endY;
		instances[i].position = new Vector3(instances[i].position.x,
                                              drift,
                                              instances[i].position.z);
        drift = img.color.a;
        drift -= fadeRate;
        if (drift < endFade) drift = endFade;
        img.color = new Color(img.color.r,img.color.g,img.color.b,drift);
    }
}
