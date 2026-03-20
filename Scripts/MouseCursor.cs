using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;

public class MouseCursor : MonoBehaviour {
    public GameObject playerCamera;
	public GameObject uiCamera;
	private Camera uiCameraCam;
	public bool liveGrenade = false;
	public string toolTip = "";
	public bool toolTipHasText = false;
	public Camera mainCamera;
	public RectTransform centerMFDPanel;
	public GameObject inventoryAddHelper;
    public float cursorSize = 24f;
    public Texture2D cursorImage;
	public RawImage cursorUIImage;
	public Canvas canvas;
	public Canvas cursorCanvas;
	private RectTransform canvasRectTransform;
	private RectTransform cursorCanvasRectTransform;
	private RectTransform cursorRectTransform;
	public CanvasScaler canvasScaler;
    private float offsetX;
    private float offsetY;
	public bool justDroppedItemInHelper = false;
	public Rect drawTexture;
	public List<RectTransform> uiRaycastRects;
	public List<GameObject> uiRaycastRectGOs;
	public Vector2 cursorPosition;
	public GUIStyle liveGrenadeStyle;
	public GUIStyle toolTipStyle;
	public GUIStyle toolTipStyleLH;
	public GUIStyle toolTipStyleRH;
	public GameObject tooltipCenter;
	public GameObject tooltipLeft;
	public GameObject tooltipRight;
	public GameObject tooltipLiveGrenade;
	public Text tooltipCenterText;
	public Text tooltipLeftText;
	public Text tooltipRightText;
	public Text tooltipLiveGrenadeText;
	public Handedness toolTipType;
	public Texture2D cursorDefaultTexture;
	public Texture2D cyberspaceCursor;
	public Texture2D cursorLHTexture;
	public Texture2D cursorRHTexture;
	public Texture2D cursorDNTexture;
	private Texture2D tooltipTexture;
	public Texture2D cursorGUI;
	public RectTransform energySliderRect;
	public float cursorScreenPercentage = 0.02f;
	private float halfFactor = 0.5f;
	public float deltaX;
	public float deltaY;
	public Vector2 lastMousePos;
	public GraphicRaycaster raycaster;
	private List<RaycastResult> graphicCastResults;
	private PointerEventData pev;
	
	public static MouseCursor a;

	void Awake() {
		a = this;
		a.uiCameraCam = uiCamera.GetComponent<Camera>();
		cursorSize = Screen.width * cursorScreenPercentage;
		a.drawTexture = new Rect((Screen.width*halfFactor) - offsetX, (Screen.height * halfFactor) - cursorSize, cursorSize, cursorSize);
		deltaX = deltaY = 0;
		lastMousePos = cursorPosition = Input.mousePosition;
		pev = new PointerEventData(EventSystem.current);
		graphicCastResults = new List<RaycastResult>();
		canvasRectTransform = canvas.gameObject.GetComponent<RectTransform>();
		if (canvasRectTransform == null) DualLogError("Can't access RectTransform on canvas!");
		
		cursorCanvasRectTransform = cursorCanvas.gameObject.GetComponent<RectTransform>();
		if (cursorCanvasRectTransform == null) DualLogError("Can't access RectTransform on cursorCanvas!");
		
		cursorRectTransform = cursorUIImage.GetComponent<RectTransform>();
		if (cursorRectTransform == null) DualLogError("Can't access RectTransform on Cursor!");
	}

	#if UNITY_STANDALONE_LINUX
		[DllImport("libX11")]
		static extern IntPtr XOpenDisplay(string display);

		[DllImport("libX11")]
		static extern int XCloseDisplay(IntPtr display);

		[DllImport("libX11")]
		static extern int XWarpPointer(IntPtr display, IntPtr src_w, 
									   IntPtr dest_w, int src_x, int src_y,
									   uint src_width, uint src_height, 
									   int dest_x, int dest_y);
	#elif UNITY_STANDALONE_WIN
		[DllImport("user32.dll")]
		public static extern bool SetCursorPos(int X, int Y);
	#endif

	public static void SetCursorPosInternal(int x, int y) {
		// TODO: Still experiencing issues, best to live with this bug a while
		// yet as it is better to have consistent behavior rather than what
		// this does.

		//#if UNITY_STANDALONE_LINUX
		//	IntPtr display = XOpenDisplay(null);
		//	if (display == IntPtr.Zero) {
		//		throw new Exception("Failed to open display");
		//	}

		//	DualLog("warping pointer to " + x.ToString() + ", " + y.ToString());
		//	XWarpPointer(display, IntPtr.Zero, IntPtr.Zero, 0, 0, 0, 0, x, y);
		//	XCloseDisplay(display);
		//#elif UNITY_STANDALONE_WIN
		//	SetCursorPos((int)(Screen.width * 0.5f),(int)(Screen.height * 0.5f));
		//#endif
	}

	public void RegisterRaycastRect(GameObject go, RectTransform rectToAdd) {
		uiRaycastRects.Add(rectToAdd);
		uiRaycastRectGOs.Add(go);
	}
	
	private void SetCursorPositionMovable() {
		drawTexture.Set(Input.mousePosition.x - offsetX,Screen.height - Input.mousePosition.y - offsetY,cursorSize,cursorSize);
		cursorUIImage.rectTransform.anchoredPosition = new Vector2((Input.mousePosition.x / Screen.width) * canvasRectTransform.sizeDelta.x,(((-1f * (Screen.height - (Input.mousePosition.y))) / Screen.height) * canvasRectTransform.sizeDelta.y) + canvasRectTransform.sizeDelta.y);
		cursorUIImage.rectTransform.sizeDelta = new Vector2(cursorSize * halfFactor,cursorSize * halfFactor); // Pivot is 0.5,0.5
	}
	
	private void SetCursorPositionAtCenter() {
		drawTexture.Set((Screen.width * halfFactor) - offsetX, (Screen.height * halfFactor) - cursorSize - offsetY, cursorSize, cursorSize);
		cursorUIImage.rectTransform.anchoredPosition = new Vector2((halfFactor * canvasRectTransform.sizeDelta.x),(halfFactor * canvasRectTransform.sizeDelta.y));
		cursorUIImage.rectTransform.sizeDelta = new Vector2(cursorSize * halfFactor,cursorSize * halfFactor); // Pivot is 0.5,0.5
	}
	
	private void EnableTooltips() {
		if (toolTipHasText && !Eng_Global->gamePaused && !Eng_Global->menuActive && (MouseLookScript.a.inventoryMode || liveGrenade)) {
			switch(toolTipType) {
				case Handedness.LH:
					tooltipLeft.SetActive(true);
					tooltipLeftText.text = toolTip;
					tooltipTexture = cursorLHTexture;
					break;
				case Handedness.RH:
					tooltipRight.SetActive(true);
					tooltipRightText.text = toolTip;
					tooltipTexture = cursorRHTexture;
					break;
				default: // Handedness.Center
					tooltipCenter.SetActive(true);
					tooltipCenterText.text = toolTip;
					tooltipTexture = cursorDNTexture;
					break;
			}
		} else {
			DisableTooltips();
		}
	}
	
	private void DisableTooltips() {
		tooltipCenter.SetActive(false);
		tooltipLeft.SetActive(false);
		tooltipRight.SetActive(false);
	}
	
	private void DisableLiveGrenadeTooltip() {
		tooltipLiveGrenade.SetActive(false);
	}
	
	private void EnableLiveGrenadeTooltip() {
		tooltipLiveGrenade.SetActive(true); // Display "live" next to cursor
		tooltipLiveGrenadeText.text = Eng_Text->stringTable[586];
	}

	void Update() {
		if (Const.a.noHUD) {
			cursorSize = Screen.width * cursorScreenPercentage * 0.1f;// 1 pixel "beauty" cursor.
		} else {
			cursorSize = Screen.width * cursorScreenPercentage;
		}
		
		cursorCanvasRectTransform.sizeDelta = canvasRectTransform.sizeDelta;
		offsetX = cursorSize * halfFactor;
		offsetY = offsetX;
		cursorPosition = new Vector2(Input.mousePosition.x,Input.mousePosition.y);
		cursorPosition.x = vclamp(cursorPosition.x,0,Screen.width);
		cursorPosition.y = vclamp(cursorPosition.y,0,Screen.height);
		lastMousePos = Input.mousePosition;
		UpdateSafeZone();
		UpdateEventSystemPointerStatus();
		CheckIfOutOfScreenBounds();
		UpdateInventoryAddHelper();

		// Maintain cursor mode.
		if (Eng_Global->gamePaused || Eng_Global->menuActive) {
			Cursor.lockState = CursorLockMode.None;
		} else if (MouseLookScript.a.inventoryMode) {
// 			#if UNITY_EDITOR
				Cursor.lockState = CursorLockMode.None;
// 			#else	
// 				Cursor.lockState = CursorLockMode.Confined;
// 			#endif

			if (GUIState.a.overButton || GUIState.a.overButtonType != ButtonType.None) {
				Eng_Global->uiIsBlocking = true;
			}
		} else {
			Cursor.lockState = CursorLockMode.Locked;
			Eng_Global->uiIsBlocking = false;
		}
		
		bool hideCursorForMinigame = false;
		if (MinigameCursor.a != null) {
			if (MinigameCursor.a.mouseOverPanel) hideCursorForMinigame = true;
		}

		if (Eng_Global->gamePaused || Eng_Global->menuActive) {
            // Pause / Menu Cursor
 			SetCursorPositionMovable();
			DisableTooltips();
			DisableLiveGrenadeTooltip();
			cursorImage = cursorGUI;
			if (!cursorUIImage.gameObject.activeSelf) cursorUIImage.gameObject.SetActive(true);
			if (cursorUIImage.texture != cursorImage) cursorUIImage.texture = cursorImage;
			return;
		}

		if (hideCursorForMinigame) {
			if (cursorUIImage.gameObject.activeSelf) cursorUIImage.flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false);
        } else {
			if (!cursorUIImage.gameObject.activeSelf) cursorUIImage.gameObject.SetActive(true);
		}
		
		if (MouseLookScript.a.inventoryMode) {
            // Inventory Mode Cursor
 			SetCursorPositionMovable();
			if (toolTipHasText && Eng_Global->uiIsBlocking) EnableTooltips();
			else                                         DisableTooltips();
			
			if (liveGrenade) EnableLiveGrenadeTooltip();
			else             DisableLiveGrenadeTooltip();
			
			if (MouseLookScript.a.inCyberSpace) {
				if (Eng_Global->uiIsBlocking) {
					if (toolTipHasText) {
						cursorImage = tooltipTexture;
					} else {						
						cursorImage = cursorGUI;
					}
				} else {
					cursorImage = cyberspaceCursor;
				}
				
				DisableLiveGrenadeTooltip();
			} else {
				if (MouseLookScript.a.vmailActive) {
					cursorImage = Const.a.useableItemsFrobIcons[108]; // vmail
				} else if (Eng_Global->uiIsBlocking && !Eng_Global->inventoryPlayer1.holdingObject) {
					if (toolTipHasText) {
						cursorImage = tooltipTexture;
					} else {						
						cursorImage = cursorGUI;
					}
				} else if (Eng_Global->inventoryPlayer1.holdingObject && Eng_Global->inventoryPlayer1.holdingObjectIndex >= 0) {
					cursorImage = Const.a.useableItemsFrobIcons[Eng_Global->inventoryPlayer1.holdingObjectIndex];
				} else {
					cursorImage = GetWeaponCursor();
				}
			}
        } else {
			// Shoot Mode Cursor
			SetCursorPositionAtCenter();
			DisableTooltips();
			if (liveGrenade) EnableLiveGrenadeTooltip();
			else             DisableLiveGrenadeTooltip();
			
			if (MouseLookScript.a.inCyberSpace) {				
				cursorImage = cyberspaceCursor;
				DisableLiveGrenadeTooltip();
			} else {
				if (Eng_Global->inventoryPlayer1.holdingObject && Eng_Global->inventoryPlayer1.holdingObjectIndex >= 0) {
					cursorImage = Const.a.useableItemsFrobIcons[Eng_Global->inventoryPlayer1.holdingObjectIndex];
				} else {
					cursorImage = GetWeaponCursor();
				}
			}
        }
		
		// Actually set the cursor texure now:
		if (cursorUIImage.texture != cursorImage) cursorUIImage.texture = cursorImage;
	}
	
	private Texture2D GetWeaponCursor() {
		switch(Eng_Global->inventoryPlayer1.weaponIndex) {
			case 36: return Const.a.useableItemsFrobIcons[102]; // red
			case 37: return Const.a.useableItemsFrobIcons[107]; // blue
			case 38: return Const.a.useableItemsFrobIcons[102]; // red
			case 39: return Const.a.useableItemsFrobIcons[105]; // green
			case 40: return Const.a.useableItemsFrobIcons[107]; // blue
			case 41: return Const.a.useableItemsFrobIcons[103]; // orange
			case 42: return Const.a.useableItemsFrobIcons[103]; // orange
			case 43: return Const.a.useableItemsFrobIcons[102]; // red
			case 44: return Const.a.useableItemsFrobIcons[104]; // yellow
			case 45: return Const.a.useableItemsFrobIcons[102]; // red
			case 46: return Const.a.useableItemsFrobIcons[106]; // teal
			case 47: return Const.a.useableItemsFrobIcons[104]; // yellow
			case 48: return Const.a.useableItemsFrobIcons[102]; // red
			case 49: return Const.a.useableItemsFrobIcons[105]; // green
			case 50: return Const.a.useableItemsFrobIcons[107]; // blue
			case 51: return Const.a.useableItemsFrobIcons[106]; // teal
			default: return Const.a.useableItemsFrobIcons[105]; // green
		}	
	}

	void UpdateSafeZone() {
		if (Eng_Global->gamePaused) return;
		if (Eng_Global->menuActive) return;

		if (cursorPosition.x < (0.96925f * Screen.width) && cursorPosition.x > (0.029282f * Screen.width)
			&& cursorPosition.y > (0.13541f * Screen.height) && cursorPosition.y < (0.70703f * Screen.height)) {
			Eng_Global->uiIsBlocking = false; // in the safe zone!
		}
	}

	void UpdateEventSystemPointerStatus() {
		if (Eng_Global->gamePaused) return;
		if (Eng_Global->menuActive) return;

		pev.position = cursorPosition;
		graphicCastResults.Clear();
		if (!MouseLookScript.a.inventoryMode) return;
		
		raycaster.Raycast(pev, graphicCastResults);
		if (graphicCastResults.Count > 0) {
			Eng_Global->uiIsBlocking = true;
			EventSystem.current.SetSelectedGameObject(graphicCastResults[0].gameObject);
			EventTrigger evt = graphicCastResults[0].gameObject.GetComponent<EventTrigger>();
			if (evt != null) {
				RectTransform recTr = graphicCastResults[0].gameObject.GetComponent<RectTransform>();
				if (recTr != null) {
					if (RectTransformUtility.RectangleContainsScreenPoint(recTr,cursorPosition,uiCameraCam)) {
						evt.OnPointerEnter(pev);
					} else {
						evt.OnPointerExit(pev);
					}
				}
			}
			if (Input.GetMouseButtonDown(0)) {
				ExecuteEvents.Execute(graphicCastResults[0].gameObject, pev, ExecuteEvents.submitHandler);
			}
		} else {
			Eng_Global->uiIsBlocking = false;
			EventSystem.current.SetSelectedGameObject(null);
		}
	}

	void CheckIfOutOfScreenBounds() {
		if (Eng_Global->menuActive) return;
		if (Eng_Global->gamePaused) return;

		if (cursorPosition.y > Screen.height || cursorPosition.y < 0
			|| cursorPosition.x < 0 || cursorPosition.x > Screen.width) {
			Eng_Global->uiIsBlocking = true; // outside the screen, don't shoot we're innocent!
		}
	}

	void UpdateInventoryAddHelper() {
		if (Eng_Global->gamePaused) return;
		if (Eng_Global->menuActive) return;

		if (cursorPosition.y > (0.13541f*Screen.height)
			&& cursorPosition.y < (0.70703f*Screen.height)
			&& cursorPosition.x < (0.96925f*Screen.width)
			&& cursorPosition.x > (0.029282f*Screen.width)) {
			Eng_Global->uiIsBlocking = false; // in the safe zone!
		}

		if (MouseLookScript.a.inventoryMode && Eng_Global->inventoryPlayer1.holdingObject) {
			// Be sure to pass the camera to the 3rd parameter if using
			// "Screen Space - Camera" on the Canvas, otherwise use "null"
			if (RectTransformUtility.RectangleContainsScreenPoint(centerMFDPanel,cursorPosition,uiCameraCam)) {
				if (!inventoryAddHelper.activeInHierarchy) inventoryAddHelper.SetActive(true);
				Eng_Global->uiIsBlocking = true;
			} else {
				if (inventoryAddHelper.activeInHierarchy) inventoryAddHelper.SetActive(false);
				if (justDroppedItemInHelper) {
					
					justDroppedItemInHelper = false; // only disable blocking state once, not constantly
				}
			}
		} else {
			if (justDroppedItemInHelper) {
				justDroppedItemInHelper = false; // only disable blocking state once, not constantly
				inventoryAddHelper.SetActive(false);
				
			}
		}
	}

	public Vector3 GetCursorScreenPointForRay() {
		Vector3 retval = cursorRectTransform.anchoredPosition3D; // retval = new Vector2(426.6665,240);
		retval.x = (retval.x / canvasRectTransform.sizeDelta.x) * Screen.width; // retval.x = (426.6665 / 853.7501) * 1366
		retval.y = (retval.y / canvasRectTransform.sizeDelta.y) * Screen.height; // retval.y = ((240 - (1366 * 0.05)) / 480) * 1
		if (retval.x > Screen.width) retval.x = Screen.width;
		if (retval.x < 0) retval.x = 0;
		if (retval.y > Screen.height) retval.y = Screen.height;
		if (retval.y < 0) retval.y = 0;
		return retval;
	}
}
