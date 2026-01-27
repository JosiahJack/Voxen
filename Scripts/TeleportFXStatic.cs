using UnityEngine;
using System.Collections;

public class TeleportFXStatic : MonoBehaviour {
	public float intervalTime = 0.08f;
	public float activeTime = 1f;
	public Texture2D tempCursorTexture;
	Texture2D cursorTexture;
	private float effectFinished;
	private float flipTime;
	private float randHolder;
	private bool xFlipped = false;
	private bool yFlipped = false;
	private RectTransform rect;

	void OnEnable () {
		cursorTexture = MouseCursor.a.cursorImage; //store correct cursor
		MouseCursor.a.cursorImage = tempCursorTexture; //give dummy cursor to hide it
		effectFinished = Sys_Global.pauseRelativeTime + activeTime;
		rect = GetComponent<RectTransform>();
		flipTime = Sys_Global.pauseRelativeTime + intervalTime;
	}

	void FlipX () {
		if (xFlipped) {
			xFlipped = false;
			rect.localScale = new Vector3(1f, 1f, 1f);
		} else {
			xFlipped = true;
			rect.localScale = new Vector3(-1f, 1f, 1f);
		}
	}

	void FlipY () {
		if (yFlipped) {
			yFlipped = false;
			rect.localScale = new Vector3(1f, 1f, 1f);
		} else {
			yFlipped = true;
			rect.localScale = new Vector3(1f, -1f, 1f);
		}
	}

	void Deactivate () {
		MouseCursor.a.cursorImage = cursorTexture; //return to previous cursor
		gameObject.SetActive(false);
	}

	void Update() {
		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			if (effectFinished < Sys_Global.pauseRelativeTime) Deactivate();
			if (flipTime < Sys_Global.pauseRelativeTime) {
				flipTime = Sys_Global.pauseRelativeTime + intervalTime;
				randHolder = random_range(0f,1f);
				if (randHolder < 0.5) {
					FlipX();
				} else {
					FlipY();
				}
			}
		}
	}
}
