using UnityEngine;
using System.Collections;

public class TextureStaticNoise : MonoBehaviour {
	public int resolution = 64;
	public float interval = 0.15f;
	private float updateTime;
	private Texture2D texture;
	private bool initialized = false;

	void Awake () { Initialize(); }
	void OnEnable () { Initialize(); }

	void Initialize() {
		if (initialized) return;
		initialized = true;
		texture = new Texture2D(resolution, resolution, TextureFormat.RGB24, true);
		texture.name = "ProceduralStatic";
		GetComponent<MeshRenderer>().material.mainTexture = texture;
		FillTexture();
		updateTime = Sys_Global.pauseRelativeTime + interval;
	}

	void FillTexture () {
		if (texture.width != resolution) texture.Resize(resolution, resolution);
		for (int y=0; y<resolution; y++) {
			for (int x=0; x<resolution; x++) {
				texture.SetPixel(x, y, Color.white * random_range(0.0f,1.0f));
			}
		}
		texture.filterMode = FilterMode.Point;
		texture.Apply();
	}

	void Update() {
		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			if (updateTime < Sys_Global.pauseRelativeTime) {
				updateTime = (Sys_Global.pauseRelativeTime + interval);
				FillTexture();
			}
		}
	}
}
