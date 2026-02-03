using System;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
using System.Collections;
using System.Runtime.InteropServices;
using System.Text;

public class MouseLookScript : MonoBehaviour {
    // External references
	public GameObject player;
	public GameObject canvasContainer;
	public GameObject compassContainer;
	public GameObject automapContainerLH;
	public GameObject automapContainerRH;
	public GameObject compassMidpoints;
	public GameObject compassLargeTicks;
	public GameObject compassSmallTicks;
    [Tooltip("Game object that houses the MFD tabs")] public GameObject tabControl;
	[Tooltip("Text in the data tab in the MFD that displays when searching an object containing no items")] public Text dataTabNoItemsText;
	public LogContentsButtonsManager logContentsManager;
	public GameObject[] hardwareButtons;
	public PuzzleWire puzzleWire;
	public PuzzleGrid puzzleGrid;
	public GameObject shootModeButton;
	public HealthManager hm;
	public GameObject playerRadiationTreatmentFlash;
	public Vector2 lastMousePos;
	
    // Internal references
    bool inventoryMode;
	public bool holdingObject;
    Vector2 cursorHotspot;
    Vector3 cameraFocusPoint;
	GameObject currentButton;
	GameObject currentSearchItem;
    public int heldObjectIndex; // save
	public int heldObjectCustomIndex; // save
	public int heldObjectAmmo; // save
	public int heldObjectAmmo2; // save
	public bool heldObjectLoadedAlternate; // save
	bool firstTimePickup;
	bool firstTimeSearch;
	public bool grenadeActive;
	public bool inCyberSpace;
    public float yRotation;
	Vector3 cyberspaceReturnPoint; // save
	Vector3 cyberspaceReturnCameraLocalRotation; // save
	Vector3 cyberspaceReturnPlayerCapsuleLocalRotation; // save
	int cyberspaceReturnLevel; // save
	Vector3 cyberspaceRecallPoint; // save
	bool vmailActive = false;
	bool geniusActive = false;
	private float keyboardTurnSpeed = 15f; // Speed multiplier for turning the view with the keyboard.
    private float tossOffset = 0.5f; // Distance from player origin to spawn objects when tossing them.
    private float tossForce = 10f; // Force given to spawned objects when tossing them.
	private float[] cameraDistances;
    float xRotation; // save
    private float zRotation;
    private float yRotationV;
    private float xRotationV;
    private float zRotationV;
    private float currentZRotation;
    private string mlookstring1;
    Camera playerCamera;
    private GameObject heldObject;
	private Quaternion tempQuat;
	private Vector3 tempVec;
    private RaycastHit tempHit;
	private Vector3 cameraRecoilLerpPos;
	private float cyberSpinSensitivity = 0.6f;
	private float shakeFinished;
	private float shakeForce;
	private string f9 = "f9";
	private string f6 = "f6";
	private string qsavename = "quicksave";
	private string mouseX = "Mouse X";
	private string mouseY = "Mouse Y";
	private Vector3 cursorPoint;
	private float headBobTimeShift;
	private float headBobX;
	private float headBobY;
	private float headBobZ;
	private float rotSpeedX = 0f;
	private float rotSpeedY = 0f;
	private Transform playerCapsuleTransform;
	float returnFromCyberspaceFinished;
	private float dropFinished;
	float randomShakeFinished;
	float randomKlaxonFinished;
	public Vector2 debugRT;
	public Vector2 debugAng;
	public float joyXStartTime;
	public float joyYStartTime;
	public float joyXSignLast;
	public float joyYSignLast;
	public float headBobShiftFinished;
	private float bobTarget;
	private float headBobXVel;
	private float headBobYVel;
	private static StringBuilder s1 = new StringBuilder();
    
	public static MouseLookScript a;

	void Awake() {
		a = this;
		a.playerCamera = GetComponent<Camera>(); // Needed elsewhere, do early.
	}

    void Start (){
		ResetHeldItem();
		Cursor.lockState = CursorLockMode.None;
		inventoryMode = false; // Start with inventory mode turned off.
		if (Application.platform == RuntimePlatform.Android) {
			ForceInventoryMode();
			shootModeButton.SetActive(true);
		} else {
			shootModeButton.SetActive(false);
		}

		cameraDistances = new float[32];
		SetCameraCullDistances();
		playerCamera.depthTextureMode = DepthTextureMode.Depth;
		grenadeActive = false;
		yRotation = 0;
		xRotation = 0;
		canvasContainer.SetActive(true); // Enable UI.
		firstTimePickup = true;
		firstTimeSearch = true;
		inCyberSpace = false;
		shakeFinished = Sys_Global.pauseRelativeTime;
		returnFromCyberspaceFinished = 0;
		dropFinished = 0;

		// PlayerCapsule
		// -> LeanTransform
        // -> -> MainCamera: MouseLookScript component.
		playerCapsuleTransform = transform.parent.transform.parent.transform;

		randomShakeFinished = Sys_Global.pauseRelativeTime;
		randomKlaxonFinished = Sys_Global.pauseRelativeTime;
		headBobShiftFinished = Sys_Global.pauseRelativeTime;
		bobTarget = 0.3f;
    }
    
    void OnPreCull() {
		DynamicCulling.a.Cull(false); // Update dynamic culling system.
	}

	void Update() {
		// Allow quick load straight from the menu or pause.
		if (Input.GetKeyUp(f9)) {
			if (inCyberSpace) {
				CenterStatusPrint("%s", Sys_Text.stringTable[1023]); // "Cannot load in cyberspace"
				return;
			}

			MainMenuHandler.a.LoadGame(7);
		}

        if (Sys_Global.menuActive) {
			// Ignore mouselook and turn off camera when main menu is up.
			if (!MainMenuHandler.a.fileBrowserOpen) Cursor.visible = false;
			else Cursor.visible = true;

			if (playerCamera.enabled) playerCamera.enabled = false;
			return;
		}

		if (Sys_Global.gamePaused) return;
		if (instances[PLAYER1].ressurectingFinished > Sys_Global.pauseRelativeTime) return;

		Utils.EnableCamera(playerCamera);

		// Unpaused, normal functions::
		// ====================================================================
		if (Input.GetKeyUp(f6)) {
			if (inCyberSpace) {
				CenterStatusPrint("%s", Sys_Text.stringTable[602]); // Cannot save in cyberspace
				return;
			}
			
			if (MissionTimer.a.timesUP) {
				return;
			}

			Const.a.StartSave(7,qsavename);
		}

		if(GetInput.a.ToggleMode()) ToggleInventoryMode(); // Toggle inventory mode<->shoot mode
		if (Const.a.questData.SelfDestructActivated && LevelManager.a.currentLevel != 13 && LevelManager.a.currentLevel != 9) { // Not Cyberspace, Not the bridge, separated
			if (randomShakeFinished < Sys_Global.pauseRelativeTime) {
				randomShakeFinished = Sys_Global.pauseRelativeTime + random_range(5f,20f);
				ScreenShake(3f,2f);
			}
			
			if (randomKlaxonFinished < Sys_Global.pauseRelativeTime) {
				randomKlaxonFinished = Sys_Global.pauseRelativeTime + random_range(10f,20f);
				Utils.PlayUIOneShotSavable(104); // klaxon
			}
		}

		RecoilAndRest(); // Spring Back to Rest from Recoil
		keyboardTurnSpeed = 15f * Const.a.MouseSensitivity;
		KeyboardTurn();
		KeyboardLookUpDn();
		TouchLook();
		if (inCyberSpace) { // Barrel roll!
			if (GetInput.a.LeanLeft()) {
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.instances[i].position,
					playerCapsuleTransform.transform.forward,
					cyberSpinSensitivity * Time.deltaTime * 100f
				);
			}

			if (GetInput.a.LeanRight()) {
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.instances[i].position,
					playerCapsuleTransform.transform.forward,
					cyberSpinSensitivity * Time.deltaTime * -1f * 100f
				);
			}
		} else {
			if (compassContainer.activeInHierarchy) {
				// Update automap player icon orientation.
				compassContainer.instances[i].rotation =
					Quaternion.Euler(0f, -yRotation + 180f, 0f);
			}
		}

		if (!inventoryMode) Mouselook(); // Only do mouselook in Shoot Mode.
		if(GetInput.a.Use()) Frob(); // Frob what is under our cursor.
	}

	public void Frob() {
		if (vmailActive && !inCyberSpace) {
			inventoryPlayer1.DeactivateVMail(); vmailActive = false;
			return;
		}

		if (!GUIState.a.isBlocking && !inCyberSpace) {
			if (dropFinished < Time.time) {
				currentButton = null; // Force this to reset.
				if (holdingObject) {
					if (!FrobWithHeldObject()) DropHeldItem();
				} else FrobEmptyHanded();
			}
		} else {
			//We are holding cursor over the GUI
			if (holdingObject && !inCyberSpace) {
				AddItemToInventory(heldObjectIndex,heldObjectCustomIndex);
				MouseCursor.a.liveGrenade = false;
				ResetHeldItem();
			} else InventoryButtonUse();
		}
	}

	public void Mouselook() {
		if (returnFromCyberspaceFinished >= Time.time) return; // Not yet.

		returnFromCyberspaceFinished = 0;

		// PROCESS INPUT SIGNALS
		// ----------------------------------------------------------------
		float angX = 0f; // Angle change for X.
		float angY = 0f; // Angle change for Y.
		// Handle mouse input from a standard mouse.
		float deltaX = Input.GetAxisRaw(mouseX) * Const.a.MouseSensitivity * Const.a.GraphicsFOV;
		float deltaY = Input.GetAxisRaw(mouseY) * Const.a.MouseSensitivity * Const.a.GraphicsFOV;

		// Handle thumbstick input from a controller.
		Vector2 rightThumbstick = new Vector2(Input.GetAxisRaw("JoyAxis4"), // Horizontal Left < 0, Right > 0
											  Input.GetAxisRaw("JoyAxis5") * -1f); // Vertical Down > 0, Up < 0 Inverted
		// X
		float signX = rightThumbstick.x > 0.0f ? 1.0f
					  : rightThumbstick.x < 0.0f ? -1.0f : 0f;
		if (signX != joyXSignLast) {
			joyXStartTime = Time.time; // Zero crossing.
			rotSpeedX = 0f; // Reset integrator windup.
		}

		joyXSignLast = signX;
		rightThumbstick.x *= Const.a.MouseSensitivity * 20f;
		rotSpeedX += rightThumbstick.x; // Integrate to give fine initial.
		if (rightThumbstick.x == 0f) rotSpeedX = 0f;
		if (rotSpeedX != 0f) deltaX = rotSpeedX;

		// Y
		float signY = rightThumbstick.y > 0.0f ? 1.0f
					  : rightThumbstick.y < 0.0f ? -1.0f : 0f;
		if (signY != joyYSignLast) {
			joyYStartTime = Time.time; // Zero crossing.
			rotSpeedY = 0f; // Reset integrator windup.
		}

		joyYSignLast = signY;
		rightThumbstick.y *= Const.a.MouseSensitivity * 20f;
		rotSpeedY += rightThumbstick.y; // Integrate to give fine initial.
		if (rightThumbstick.y == 0) rotSpeedY = 0f;
		if (rotSpeedY != 0f) deltaY = rotSpeedY;

		// Apply input delta from mouse or controller to angles.
		if (signX != 0f || signY != 0f) {
			// Using controller, use integrated deltas.
			angX = deltaX;
			angY = deltaY;
		} else {
			// Using mouse, map input to deg per screen half / screen.
			angX = deltaX * ((Const.a.GraphicsFOV / 2f) / Screen.width / 2f);
			angY = deltaY * ((Const.a.GraphicsFOV / 2f) / Screen.height / 2f);
		}

		// For my inspector viewing pleasure.
		debugRT = new Vector2(deltaX,deltaY);
		debugAng = new Vector2(angX,angY);

		// High pass filter to prevent jumpy behavior.
		if (angX > Const.a.GraphicsFOV) angX = Const.a.GraphicsFOV;
		if (angY > Const.a.GraphicsFOV) angY = Const.a.GraphicsFOV;

		// APPLY MOUSE LOOK
		// --------------------------------------------------------------------
		if (inCyberSpace) {
			// CYBER MOUSE LOOK
			if (Const.a.InputInvertCyberspaceLook) xRotation = -angY;
			else xRotation = angY;

			xRotation = Clamp0360(xRotation); // Limit up/down to within 360°.
			yRotation = angX;
			playerCapsuleTransform.RotateAround(
				playerCapsuleTransform.instances[i].position,
				playerCapsuleTransform.transform.up,yRotation
			);

			playerCapsuleTransform.RotateAround(
				playerCapsuleTransform.instances[i].position,
				playerCapsuleTransform.transform.right,-xRotation
			);
		} else {
			// NORMAL MOUSE LOOK
			if (Const.a.InputInvertLook) xRotation += angY;
			else xRotation -= angY;

			xRotation = vclamp(xRotation, -90f, 90f); // Limit up/down.
			yRotation += angX;

			// Apply the mouselook. Left/Right component applied to capsule.
			playerCapsuleTransform.localRotation = Quaternion.Euler(0f,
																	yRotation,
																	0f);

			// Up down component only applied to camera.  Must be 0 for others
			// or else movement will go in wrong direction!
			instances[i].rotation = Quaternion.Euler(xRotation,0f,0f);
			float xCenter = (float)Screen.width * 0.5f;
			float yCenter = (float)Screen.height * 0.5f;
			float xOffset = ((float)Input.mousePosition.x - xCenter);
			float yOffset = ((float)Input.mousePosition.y - yCenter);
			if (xOffset > 2f || yOffset > 2f) {
				MouseCursor.SetCursorPosInternal((int)xCenter,(int)yCenter);
			}
		}
	}

	public void EnterCyberspace(Vector3 entryPoint) {
		cyberspaceRecallPoint = entryPoint;
		playerRadiationTreatmentFlash.SetActive(true);
		cyberspaceReturnPoint = instances[PLAYER1].instances[i].position;
		cyberspaceReturnCameraLocalRotation = instances[i].rotation.eulerAngles;
		cyberspaceReturnPlayerCapsuleLocalRotation = playerCapsuleTransform.localRotation.eulerAngles;
		cyberspaceReturnLevel = LevelManager.a.currentLevel;
		Sys_UI.EnterCyberspace();
		LevelManager.a.LoadLevel(13,cyberspaceRecallPoint);
		instances[PLAYER1].inCyberSpace = true;
		instances[PLAYER1].leanCapsuleCollider.enabled = false;
		hm.inCyberSpace = true;
		inCyberSpace = true;
		playerCamera.useOcclusionCulling = false;
		Sys_UI.DrawTicks(true);
		SetCameraCullDistances();
		Utils.PlayUIOneShotSavable(81); // cyber
	}

	public void ExitCyberspace() {
		playerRadiationTreatmentFlash.SetActive(true);
		Sys_UI.ExitCyberspace();
		LevelManager.a.LoadLevel(cyberspaceReturnLevel,cyberspaceReturnPoint);

		// Left/right component applied to capsule.
		playerCapsuleTransform.localRotation = Quaternion.Euler(0f,
			cyberspaceReturnPlayerCapsuleLocalRotation.y,0f);

		instances[i].rotation = // Up down component applied to camera
			Quaternion.Euler(cyberspaceReturnCameraLocalRotation.x,
							 cyberspaceReturnCameraLocalRotation.y,
							 cyberspaceReturnCameraLocalRotation.z);

		xRotation = cyberspaceReturnCameraLocalRotation.x;
		yRotation = cyberspaceReturnPlayerCapsuleLocalRotation.y;

		returnFromCyberspaceFinished = Time.time + 0.1f; // Prevent mouselook
														 // messing it up.
		instances[PLAYER1].inCyberSpace = false;
		instances[PLAYER1].rbody.velocity = (Vector3){0.0f,0.0f,0.0f};
		instances[PLAYER1].leanCapsuleCollider.enabled = true;
		hm.inCyberSpace = false;
		inCyberSpace = false;
		playerCamera.useOcclusionCulling = true;
		Const.a.decoyActive = false;
		Sys_UI.DrawTicks(true);
		Utils.PlayUIOneShotSavable(81); // cyber
		SetCameraCullDistances();
	}

	// Draw line from cursor - used for projectile firing, e.g. magpulse/stugngun/railgun/plasma
	public void SetCameraFocusPoint() {
		cursorPoint = MouseCursor.a.GetCursorScreenPointForRay();
        if (Raycast(playerCamera.ScreenPointToRay(cursorPoint), out tempHit, 71.68f)) cameraFocusPoint = tempHit.point;
	}

	// Clamp cyberspace up/down look rotation to with in +/- 360f.
	float Clamp0360(float val) {
		return (val - (vceil(val*(1f/360f)) * 360f)); // Subtract out 360 times the number of times 360 fits within val.
	}

	public void SetCameraCullDistances() {
		if (cameraDistances == null) cameraDistances = new float[32];
		else if (cameraDistances.Length < 32) cameraDistances = new float[32];
		
		if (inCyberSpace) {
			for (int i=0;i<32;i++) { cameraDistances[i] = 3350f; } // Increased from 2400 to fit the Saturn's rings without overlapping with star sphere.
		} else {
			for (int i=0;i<32;i++) { cameraDistances[i] = 79f; } // Can't see further than this.  31 * 2.56 - player radius 0.48 = 78.88f rounded up to be careful..longest line of sight is the crawlway on level 6
			cameraDistances[0]  = 45.1f; // Default, most static objects and some dynamic.
			cameraDistances[1]  = 16f;   // TransparentFX, for mist and drips
			cameraDistances[4]  = 30f;   // Water, used for effects like steam
										 // and well, spraying water.
			cameraDistances[14] = 30f;   // PhysObjects, patches, carts, barrels
			cameraDistances[15] = 3350f; // Sky is visible, only exception.
		}
		playerCamera.layerCullDistances = cameraDistances; // Cull anything beyond 79f except for sky layer.
	}
	
	void TouchLook() {
	    Vector2 rightTouchstick = GetInput.a.rightTS.Coordinate();
	    if (rightTouchstick.x < 0f) {
			yRotation -= keyboardTurnSpeed * rightTouchstick.x;
			playerCapsuleTransform.localRotation = Quaternion.Euler(0f, yRotation, 0f);
		} else if (rightTouchstick.x > 0f) {
			yRotation += keyboardTurnSpeed * rightTouchstick.x;
			playerCapsuleTransform.localRotation = Quaternion.Euler(0f, yRotation, 0f);
		}
		
		if (rightTouchstick.y < 0f) {
			if ((inCyberSpace && Const.a.InputInvertCyberspaceLook) || (!inCyberSpace && Const.a.InputInvertLook))
				xRotation -= keyboardTurnSpeed;
			else
				xRotation += keyboardTurnSpeed;

			if (!inCyberSpace) xRotation = vclamp(xRotation, -90f, 90f);  // Limit up and down angle.
			instances[i].rotation = Quaternion.Euler(xRotation,0f,
													   instances[i].rotation.z);
		} else if (rightTouchstick.y > 0f) {
			if ((inCyberSpace && Const.a.InputInvertCyberspaceLook) || (!inCyberSpace && Const.a.InputInvertLook))
				xRotation += keyboardTurnSpeed * rightTouchstick.y;
			else
				xRotation -= keyboardTurnSpeed * rightTouchstick.y;

			if (!inCyberSpace) xRotation = vclamp(xRotation, -90f, 90f);  // Limit up and down angle.
			instances[i].rotation = Quaternion.Euler(xRotation, 0f,
													   instances[i].rotation.z);
		}
	}

	void KeyboardTurn() {
		if (inCyberSpace) {
			float angX = 0f;
			if (GetInput.a.TurnLeft()) {
				// Modulate input to deg per screen half / screen.
				angX = -keyboardTurnSpeed * 18f * ((Const.a.GraphicsFOV / 2f) / Screen.width / 2f);
				yRotation = angX;
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.instances[i].position,
					playerCapsuleTransform.transform.up,yRotation
				);
			} else if (GetInput.a.TurnRight()) {
				angX = keyboardTurnSpeed * 18f * ((Const.a.GraphicsFOV / 2f) / Screen.width / 2f);
				yRotation = angX;
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.instances[i].position,
					playerCapsuleTransform.transform.up,yRotation
				);
			}
		} else {
			if (GetInput.a.TurnLeft()) {
				yRotation -= keyboardTurnSpeed;
				playerCapsuleTransform.localRotation = Quaternion.Euler(0f, yRotation, 0f);
			} else if (GetInput.a.TurnRight()) {
				yRotation += keyboardTurnSpeed;
				playerCapsuleTransform.localRotation = Quaternion.Euler(0f, yRotation, 0f);
			}
		}
		
		Vector2 rightTouchstick = GetInput.a.rightTS.Coordinate();
	}

	void KeyboardLookUpDn() {
		if (inCyberSpace) {
			float angY = 0f;
			if (GetInput.a.LookDown()) {
				// Modulate input to deg per screen half / screen.
				angY = -keyboardTurnSpeed * 18f * ((Const.a.GraphicsFOV / 2f) / Screen.height / 2f);
				if (Const.a.InputInvertCyberspaceLook) xRotation = -angY;
				else xRotation = angY;
			
				xRotation = Clamp0360(xRotation); // Limit up/down to within 360°.
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.instances[i].position,
					playerCapsuleTransform.transform.right,-xRotation
				);
			} else if (GetInput.a.LookUp()) {
				angY = keyboardTurnSpeed * 18f * ((Const.a.GraphicsFOV / 2f) / Screen.height / 2f);
				if (Const.a.InputInvertCyberspaceLook) xRotation = -angY;
				else xRotation = angY;
			
				xRotation = Clamp0360(xRotation); // Limit up/down to within 360°.
					playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.instances[i].position,
					playerCapsuleTransform.transform.right,-xRotation
				);
			}
		} else {
			// Cyberspace...more like a plane so giving the option to invert it separately.
			if (GetInput.a.LookDown()) {
				if ((inCyberSpace && Const.a.InputInvertCyberspaceLook) || (!inCyberSpace && Const.a.InputInvertLook))
					xRotation -= keyboardTurnSpeed;
				else
					xRotation += keyboardTurnSpeed;

				if (!inCyberSpace) xRotation = vclamp(xRotation, -90f, 90f);  // Limit up and down angle.
				instances[i].rotation = Quaternion.Euler(xRotation,0f,
														instances[i].rotation.z);
			} else if (GetInput.a.LookUp()) {
				if ((inCyberSpace && Const.a.InputInvertCyberspaceLook) || (!inCyberSpace && Const.a.InputInvertLook))
					xRotation += keyboardTurnSpeed;
				else
					xRotation -= keyboardTurnSpeed;

				if (!inCyberSpace) xRotation = vclamp(xRotation, -90f, 90f);  // Limit up and down angle.
				instances[i].rotation = Quaternion.Euler(xRotation, 0f,
														instances[i].rotation.z);
			}
		}
	}

	bool RayOffset() {
		bool successfulRay = false;
		successfulRay = Raycast(playerCamera.ScreenPointToRay(cursorPoint), out tempHit,Const.frobDistance,Const.a.layerMaskPlayerFrob);
		if (successfulRay) {
			successfulRay = (tempHit.collider != null);
			if (successfulRay) {
				successfulRay = (tempHit.collider.CompareTag("Usable") || tempHit.collider.CompareTag("Searchable") || tempHit.collider.CompareTag("NPC"));
			}
		}
		return successfulRay;
	}

	bool TargetIDFrob(Vector3 cP) {
		if (Application.platform == RuntimePlatform.Android) {
			if (MouseLookScript.a.inCyberSpace) {
				WeaponFire.a.FireCyberWeapon();
				return true;
			}
		}

		if (inCyberSpace) return false;

		float dist = TargetID.GetTargetIDSensingRange(true);
		bool successfulRay = Raycast(playerCamera.ScreenPointToRay(cP),
											 out tempHit,dist,
											 Const.a.layerMaskPlayerTargetIDFrob);

		// Success here means hit a useable something.
		// If a ray hits a wall or other unusable something, that's not success
		if (successfulRay) {
			successfulRay = (tempHit.collider != null);
			if (successfulRay) {
				successfulRay = tempHit.collider.CompareTag("NPC");
			}
		}

		if (!successfulRay) return false;

		// Say we can't use enemy and give enemy name.
		AIController aic = tempHit.collider.gameObject.GetComponent<AIController>();
		if (aic == null) return false;

		HealthManager hm = Utils.GetMainHealthManager(tempHit);
		if (hm != null) {
			if (hm.health <= 0 && aic.searchColliderGO != null) {
				currentSearchItem = aic.searchColliderGO;
				SearchObject(currentSearchItem.GetComponent<SearchableItem>().lookUpIndex);
				return true; // True = do and check nothing further this frob.
			}
		}

		if (inventoryPlayer1.hasHardware[4] && inventoryPlayer1.hardwareVersion[4] > 1) {
			if (!aic.hasTargetIDAttached) {
				WeaponFire.a.CreateTargetIDInstance(-1f,aic.healthManager,-1f);
				if (Application.platform != RuntimePlatform.Android) {
					return true;
				}
			}
		}

		if (Application.platform == RuntimePlatform.Android) {
			// Cyber handled just above, normal fire condition only here.
			int constDex = WeaponCurrent.a.weaponIndex;
			int wepdex = WeaponFire.Get16WeaponIndexFromConstIndex(constDex);
			WeaponFire.a.StartNormalAttack(wepdex);
			return true;
		}

		// "Can't use <enemy>"
		CenterStatusPrint("%s", Sys_Text.stringTable[29] + Const.a.nameForNPC[aic.index],
					 player);

		return true;
	}

	void FrobEmptyHanded() {
		if (holdingObject) return;

		RaycastHit firstHit;
		float offset = Screen.height * 0.02f;
		cursorPoint = MouseCursor.a.GetCursorScreenPointForRay();
		if (TargetIDFrob(cursorPoint)) return;

		Ray castDir = playerCamera.ScreenPointToRay(cursorPoint);
		bool successfulRay = Raycast(castDir, out tempHit, Const.frobDistance, Const.a.layerMaskPlayerFrob);
		firstHit = tempHit;
		// Success here means hit a useable something.
		// If a ray hits a wall or other unusable something,
		// that's not success and print "Can't use <something>".
		if (successfulRay) {
			successfulRay = (tempHit.collider != null);
			if (successfulRay) {
				successfulRay = (tempHit.collider.CompareTag("Usable")
								 || tempHit.collider.CompareTag("Searchable"));
			}
		}

		// Shoot rays in a pattern like this
		// * * *
		// * + *
		// * * *

		// In an order like this:
		// 8 3 6
		// 5 1 4
		// 7 2 9
		// To kind of walk around the center point to hopefully minimize rays
		// we try and tighten our lug nuts properly so the wheels don't fall 
		// off this thing.

		if (!successfulRay) { // Try down
			cursorPoint.y -= offset;
			successfulRay = RayOffset();
			cursorPoint.y += offset;
		}
		if (!successfulRay) { // Try up
			cursorPoint.y += offset;
			successfulRay = RayOffset();
			cursorPoint.y -= offset;
		}
		if (!successfulRay) { // Try to the right
			cursorPoint.x += offset;
			successfulRay = RayOffset();
			cursorPoint.x -= offset;
		}
		if (!successfulRay) { // Try to the left
			cursorPoint.x -= offset;
			successfulRay = RayOffset();
			cursorPoint.x += offset;
		}
		if (!successfulRay) { // Try up and to the right
			cursorPoint.x += offset;
			cursorPoint.y += offset;
			successfulRay = RayOffset();
			cursorPoint.x -= offset;
			cursorPoint.y -= offset;
		}
		if (!successfulRay) { // Try down and to the left
			cursorPoint.x -= offset;
			cursorPoint.y -= offset;
			successfulRay = RayOffset();
			cursorPoint.x += offset;
			cursorPoint.y += offset;
		}
		if (!successfulRay) { // Try up and to the left, cupid shuffle
			cursorPoint.x -= offset;
			cursorPoint.y += offset;
			successfulRay = RayOffset();
			cursorPoint.x += offset;
			cursorPoint.y -= offset;
		}
		if (!successfulRay) { // Try down and to the right
			cursorPoint.x += offset;
			cursorPoint.y -= offset;
			successfulRay = RayOffset();
			cursorPoint.x -= offset;
			cursorPoint.y += offset;
		}

		if (!successfulRay) tempHit = firstHit;

		// Okay we've checked first center, then in a box patter of 8, surely
		// we've hit something the player was reasonably aiming at by now.
		if (successfulRay) {
			if (tempHit.collider.CompareTag("Usable")) { // Use
				UseData ud = new UseData ();
				ud.owner = player;
				UseHandler uh = tempHit.collider.gameObject.GetComponent<UseHandler>();
				if (uh != null) {
					uh.Use(ud);
				} else {
					UseHandlerRelay uhr = tempHit.collider.gameObject.GetComponent<UseHandlerRelay>();
					if (uhr != null) {
						if (uhr.referenceUseHandler != null) {
							uhr.referenceUseHandler.Use(ud);
						}
					} else {
						DualLog("BUG: Attempting to use a useable without a UseHandler or UseHandlerRelay!");
					}
				}
			} else if (tempHit.collider.CompareTag("Searchable")) { // Search
				currentSearchItem = tempHit.collider.gameObject;
				SearchObject(currentSearchItem.GetComponent<SearchableItem>().lookUpIndex);
			} else {
				CenterStatusPrint(29); // "Can't use "
			}
		} else { // Frobbed into empty space, so whatever it is is too far.
			if (tempHit.collider != null) {
				// Can't use <something>
				UseName.UseNameSprint(tempHit.collider.gameObject);
			} else {
				// You are too far away from that
				CenterStatusPrint("%s", Sys_Text.stringTable[30],player);
			}
		}
	}

	bool FrobWithHeldObject() {
		if (heldObjectIndex < 0) {
			DualLog("BUG: Attempting to frob with held object, but "
					  + "heldObjectIndex < 0.");
			return false; // Invalid item will be dropped, wasn't used up.
		}

		bool frobUser = (heldObjectIndex == 54 || heldObjectIndex == 56
						 || heldObjectIndex == 57 || heldObjectIndex == 61
						 || heldObjectIndex == 64 || heldObjectIndex == 92
						 || heldObjectIndex == 93 || heldObjectIndex == 94);

		if (!frobUser) return false;

		cursorPoint = MouseCursor.a.GetCursorScreenPointForRay();
		if (!Raycast(playerCamera.ScreenPointToRay(cursorPoint),
							 out tempHit, Const.frobDistance)) {
			return false; // Can't use it on something, go ahead and drop it.
		}

		// Cannot notify of attempt to frob with different index since this is
		// how we normally drop items.
		GameObject go = tempHit.collider.gameObject;
		if (go == null) return false;
		if (!tempHit.collider.CompareTag("Usable")) return false;

		UseData ud = new UseData();
		ud.owner = player;
		ud.mainIndex = heldObjectIndex;
		ud.customIndex = heldObjectCustomIndex;
		UseHandler uh = go.GetComponent<UseHandler>();
		bool playedSound = false;
		if (uh != null) {
			Utils.PlayUIOneShotSavable(91); // searchsound
			playedSound = true;
			uh.Use(ud);
			return true; // Item can get absorbed, not dropped.
		}

		UseHandlerRelay uhr = go.GetComponent<UseHandlerRelay>();
		if (uhr != null) {
			
			if (!playedSound) Utils.PlayUIOneShotSavable(91); // searchsound
			uhr.referenceUseHandler.Use(ud);
			return true; // Item can get absorbed, not dropped.
		}

		DualLog("BUG: Attempting to frob use a useable " + go.name
				  + " without a UseHandler or UseHandlerRelay!");

		return false;
	}

	void PutObjectInHand(int useableConstdex, int customIndex, int ammo1,
						 int ammo2, bool loadedAlt, bool fromButton) {
		if (useableConstdex < 0) return;

		holdingObject = true;
		heldObjectIndex = useableConstdex;
		heldObjectCustomIndex = customIndex;
		heldObjectAmmo = ammo1;
		heldObjectAmmo2 = ammo2;
		heldObjectLoadedAlternate = loadedAlt;
		if (fromButton) 
		ForceInventoryMode();
	}

	void RemoveWeapon() {
		// Take weapon out of inventory, removing weapon, remove weapon and any
		// other strings I need to CTRL+F my way to this buggy code!
		WeaponButton wepbut = currentButton.GetComponent<WeaponButton>();
		int indexPriorToRemoval = wepbut.useableItemIndex;
		int am1 = WeaponCurrent.a.currentMagazineAmount[wepbut.WepButtonIndex];
		WeaponCurrent.a.currentMagazineAmount[wepbut.WepButtonIndex] = 0;
		int am2 = WeaponCurrent.a.currentMagazineAmount2[wepbut.WepButtonIndex];
		WeaponCurrent.a.currentMagazineAmount2[wepbut.WepButtonIndex] = 0;
		bool loadAlt = false;
		if (am2 > 0) loadAlt = true;
		PutObjectInHand(indexPriorToRemoval,-1,am1,am2,loadAlt,true);
		WeaponCurrent.a.RemoveWeapon(wepbut.WepButtonIndex);
		inventoryPlayer1.RemoveWeapon(wepbut.WepButtonIndex);
		Sys_UI.SetAmmoIcons(-1,false) ; // Clear the ammo icons.
		Sys_UI.HideAmmoAndEnergyItems();
		wepbut.useableItemIndex = -1;
		wepbut = Sys_UI.wepbutMan.wepButtonsScripts[0];
		WeaponCurrent.a.WeaponChange(wepbut.useableItemIndex,
									 wepbut.WepButtonIndex);
	}

	// Because Unity does not see fit for their Button class to support right
	// click behavior...or any other reasonable mouse button interaction.
	void InventoryButtonUse() {
		if (holdingObject) return;
		if (!GUIState.a.overButton) return;
		if (GUIState.a.overButtonType == ButtonType.None) return;
		if (currentButton == null) return;

		int indexPriorToRemoval = -1;
		int customIndexPrior = -1;
		switch(GUIState.a.overButtonType) {
			case ButtonType.Weapon: RemoveWeapon(); break;
			case ButtonType.Grenade:
				GrenadeButton grenbut = currentButton.GetComponent<GrenadeButton>();
				indexPriorToRemoval = grenbut.useableItemIndex;
				inventoryPlayer1.grenAmmo[grenbut.GrenButtonIndex]--;
				inventoryPlayer1.GrenadeCycleDown();
				//inventoryPlayer1.grenadeCurrent = -1; This was up here, and seemed fine.  Might need to revert line 473 add.
				if (inventoryPlayer1.grenAmmo[grenbut.GrenButtonIndex] <= 0) {
					inventoryPlayer1.grenAmmo[grenbut.GrenButtonIndex] = 0;
					inventoryPlayer1.grenadeCurrent = -1;
					for (int i = 0; i < 7; i++) {
						if (inventoryPlayer1.grenAmmo[i] > 0) {
							inventoryPlayer1.grenadeCurrent = i;
						}
					}

					Sys_UI.SendInfoToItemTab(inventoryPlayer1.grenadeCurrent);
					if (inventoryPlayer1.grenadeCurrent < 0) {
						inventoryPlayer1.grenadeCurrent = 0;
					}
				}

				grenadeActive = true;
				PutObjectInHand(indexPriorToRemoval,-1,0,0,false,true);
				break;
			case ButtonType.Patch:
				PatchButton patbut = currentButton.GetComponent<PatchButton>();
				indexPriorToRemoval = patbut.useableItemIndex;
				inventoryPlayer1.patchCounts[patbut.PatchButtonIndex]--;
				if (inventoryPlayer1.patchCounts[patbut.PatchButtonIndex] <= 0) {
					inventoryPlayer1.patchCounts[patbut.PatchButtonIndex] = 0;
					inventoryPlayer1.patchCurrent = -1;
					
					for (int i = 0; i < 7; i++) {
						if (inventoryPlayer1.patchCounts[i] > 0) inventoryPlayer1.patchCurrent = i;
					}
					Sys_UI.SendInfoToItemTab(inventoryPlayer1.patchCurrent);
					if (inventoryPlayer1.patchCurrent < 0) {
						inventoryPlayer1.patchCurrent = 0;
					}
				}
				PutObjectInHand(indexPriorToRemoval,-1,0,0,false,true);
				break;
			case ButtonType.GeneralInv:
				GeneralInvButton genbut = 
					currentButton.GetComponent<GeneralInvButton>();

				// Access Cards button
				if (genbut.GeneralInvButtonIndex == 0) {
					Sys_UI.OpenLastItemSide();
					Sys_UI.SendInfoToItemTab(81);
					return;
				}

				indexPriorToRemoval = genbut.useableItemIndex;
				customIndexPrior = genbut.customIndex;
				inventoryPlayer1.generalInventoryIndexRef[genbut.GeneralInvButtonIndex] = -1;
				inventoryPlayer1.generalInvCurrent = -1;
				for (int i = 0; i < 7; i++) {
					if (inventoryPlayer1.generalInventoryIndexRef[i] >= 0) {
						inventoryPlayer1.generalInvCurrent = i;
					}
				}
				int referenceIndex = -1;
				if (inventoryPlayer1.generalInvCurrent >= 0) {
					referenceIndex = inventoryPlayer1.genButtons[inventoryPlayer1.generalInvCurrent].transform.GetComponent<GeneralInvButton>().useableItemIndex;
				}

				if (referenceIndex < 0 || referenceIndex > 110) {
					Sys_UI.ResetItemTab();
				} else {
					Sys_UI.SendInfoToItemTab(referenceIndex,genbut.customIndex);
				}
				PutObjectInHand(indexPriorToRemoval,customIndexPrior,0,0,false,true);
				break;
			case ButtonType.Search:
				SearchButton sebut = currentButton.GetComponentInParent<SearchButton>();
				int tempButtonindex = currentButton.GetComponent<SearchContainerButton>().refIndex;
				SearchButtonClick(tempButtonindex,sebut);
				break;
			case ButtonType.PGrid:
				PuzzleUIButton puib = currentButton.GetComponent<PuzzleUIButton>();
				if (puib != null) puzzleGrid.OnGridCellClick(puib.buttonIndex);
				break;
			case ButtonType.PWire:
				PuzzleUIButton wpuib = currentButton.GetComponent<PuzzleUIButton>();
				if (wpuib != null) {
					if (wpuib.isRH)
						puzzleWire.ClickRHNode(wpuib.buttonIndex);
					else
						puzzleWire.ClickLHNode(wpuib.buttonIndex);
				}
				break;
			case ButtonType.Vaporize:
				VaporizeButton vapB = currentButton.GetComponent<VaporizeButton>();
				if (vapB != null) {
					vapB.OnVaporizeClick();
				}
				break;
			case ButtonType.ShootMode:
				ForceShootMode();
				
				break;
			case ButtonType.GrenadeTimerSlider:
				Button btn = currentButton.GetComponent<Button>();
				DualLog("GrenadeTimerSlider invoke");
				break;
		}
	}
	
	public void SearchButtonClick(int index, SearchButton sebut) {
		holdingObject = true;
		heldObjectIndex = sebut.contents[index];
		heldObjectCustomIndex = sebut.customIndex[index];
		if (currentSearchItem != null) {
			SearchableItem sitem = currentSearchItem.GetComponent<SearchableItem>();
			sitem.contents[index] = -1;
			sitem.customIndex[index] = -1;
		}
		
		sebut.contents[index] = -1;
		sebut.customIndex[index] = -1;
		Sys_UI.DisableSearchItemImage(index);
		sebut.CheckForEmpty();
		
		if (Const.a.InputQuickItemPickup) {
			AddItemToInventory(heldObjectIndex,heldObjectCustomIndex);
			ResetHeldItem();
		} else {
			CenterStatusPrint("%s", Sys_Text.stringTable[heldObjectIndex + 326] + Sys_Text.stringTable[319],player);
			ForceInventoryMode();
		}	
	}

	void RecoilAndRest() {
		float targetY = Const.a.playerCameraOffsetY * instances[PLAYER1].currentCrouchRatio;
		float targetX = 0f;
		if (instances[PLAYER1].relSideways > 0) targetX += 0.12f;
		if (instances[PLAYER1].relSideways < 0) targetX -= 0.12f;
		if (instances[PLAYER1].relForward != 0) targetY -= 0.08f;
		if (shakeFinished > Sys_Global.pauseRelativeTime) {
			headBobX = instances[PLAYER1].position.x + random_range(shakeForce * -0.17f, shakeForce * 0.17f);
			headBobY = instances[PLAYER1].position.y + random_range(shakeForce * -0.08f, shakeForce * 0.08f);
			headBobZ = instances[PLAYER1].position.z + random_range(shakeForce * -0.17f, shakeForce * 0.17f);
		} else {
			headBobZ = 0f;
			Vector3 vel = instances[PLAYER1].rbody.velocity;
			vel.y = 0f;
			if (instances[PLAYER1].relForward + instances[PLAYER1].relSideways != 0 && Sys_Settings.HeadBob) {
				if (instances[PLAYER1].headBobShiftFinished < Sys_Global.pauseRelativeTime) {
					instances[PLAYER1].headBobShiftFinished = Sys_Global.pauseRelativeTime + 0.2f;
					if (!instances[PLAYER1].isSprinting) instances[PLAYER1].headBobShiftFinished += 0.1f;
					bobTarget = HeadBobAmount * -1f * vsign(bobTarget);
				}

				if (instances[PLAYER1].rbody.velocity.magnitude > 0.1f) headBobY = smooth_damp(headBobY,targetY + bobTarget,ref headBobYVel,Const.HeadBobRate);
				headBobX = smooth_damp(headBobX,targetX,ref headBobXVel,Const.HeadBobRate);
			} else {
				headBobX = smooth_damp(headBobX,0f,ref headBobXVel,Const.HeadBobRate);
				headBobY = smooth_damp(headBobY,Const.a.playerCameraOffsetY * instances[PLAYER1].currentCrouchRatio,ref headBobYVel,Const.HeadBobRate);
			}
		}
		
		if (inCyberSpace) headBobX = headBobY = headBobZ = 0f;
		instances[PLAYER1].position = (Vector3){ headBobX, headBobY, headBobZ };
	}

	void AddItemFail(int index) { // Expects usableItem index
		DropHeldItem();
		CenterStatusPrint("%s", Sys_Text.stringTable[32] + Sys_Text.stringTable[index + 326]
					 + Sys_Text.stringTable[318],player); // Inventory full.
	}

	public void AddItemToInventory(int index, int customIndex) {
		Sys_UI.mouseClickHeldOverGUI = true; // Prevent gun shooting.
		if (index < 0) index = 0; // Good check on paper.
		if (index > 110) index = 94; // Way to get a head.
		if ((index >= 0 && index <= 5)
             || index == 33
             || index == 35
             || (index >= 52 && index < 59)
             || (index >= 61 && index <= 64)
             || (index >= 92 && index <= 101)) {
			if (!inventoryPlayer1.AddGeneralObjectToInventory(index,customIndex)) {
				AddItemFail(index);
			}
		} else if (index == 6) {
			inventoryPlayer1.AddAudioLogToInventory(heldObjectCustomIndex);
		} else if (index >= 36 && index <= 51) {
			if (!inventoryPlayer1.AddWeaponToInventory(index,heldObjectAmmo,
												  heldObjectAmmo2,
												  heldObjectLoadedAlternate)) {
				AddItemFail(index);
			}
		} else if (index == 34 || index == 81 || (index >= 83 && index <= 91) || index == 110) {
			inventoryPlayer1.AddAccessCardToInventory(index);
		} else {
			switch (index) {
				case 7:  inventoryPlayer1.AddGrenadeToInventory(0,index); break; // Frag
				case 8:  inventoryPlayer1.AddGrenadeToInventory(3,index); break; // Concussion
				case 9:  inventoryPlayer1.AddGrenadeToInventory(1,index); break; // EMP
				case 10: inventoryPlayer1.AddGrenadeToInventory(6,index); break; // Earth Shaker
				case 11: inventoryPlayer1.AddGrenadeToInventory(4,index); break; // Land Mine
				case 12: inventoryPlayer1.AddGrenadeToInventory(5,index); break; // Nitropak
				case 13: inventoryPlayer1.AddGrenadeToInventory(2,index); break; // Gas
				case 14: inventoryPlayer1.AddPatchToInventory(2,index); break;
				case 15: inventoryPlayer1.AddPatchToInventory(6,index); break;
				case 16: inventoryPlayer1.AddPatchToInventory(5,index); break;
				case 17: inventoryPlayer1.AddPatchToInventory(3,index); break;
				case 18: inventoryPlayer1.AddPatchToInventory(4,index); break;
				case 19: inventoryPlayer1.AddPatchToInventory(1,index); break;
				case 20: inventoryPlayer1.AddPatchToInventory(0,index); break;
				case 21: inventoryPlayer1.AddHardwareToInventory(0,index,customIndex,true); break;
				case 22: inventoryPlayer1.AddHardwareToInventory(1,index,customIndex,true); break;
				case 23: inventoryPlayer1.AddHardwareToInventory(2,index,customIndex,true); break;
				case 24: inventoryPlayer1.AddHardwareToInventory(3,index,customIndex,true); break;
				case 25: inventoryPlayer1.AddHardwareToInventory(4,index,customIndex,true); break;
				case 26: inventoryPlayer1.AddHardwareToInventory(5,index,customIndex,true); break;
				case 27: inventoryPlayer1.AddHardwareToInventory(6,index,customIndex,true); break;
				case 28: inventoryPlayer1.AddHardwareToInventory(7,index,customIndex,true); break;
				case 29: inventoryPlayer1.AddHardwareToInventory(8,index,customIndex,true); break;
				case 30: inventoryPlayer1.AddHardwareToInventory(9,index,customIndex,true); break;
				case 31: inventoryPlayer1.AddHardwareToInventory(10,index,customIndex,true); break;
				case 32: inventoryPlayer1.AddHardwareToInventory(11,index,customIndex,true); break;
				case 60: inventoryPlayer1.AddAmmoToInventory(12,index, Const.a.magazinePitchCountForWeapon[12], false); break; // rubber slugs
				case 65: inventoryPlayer1.AddAmmoToInventory(8,index, Const.a.magazinePitchCountForWeapon2[8], true); break; // magpulse cartridge super
				case 66: inventoryPlayer1.AddAmmoToInventory(2,index, Const.a.magazinePitchCountForWeapon[2], false); break; // needle darts
				case 67: inventoryPlayer1.AddAmmoToInventory(2,index, Const.a.magazinePitchCountForWeapon2[2], true); break; // tranquilizer darts
				case 68: inventoryPlayer1.AddAmmoToInventory(9,index, Const.a.magazinePitchCountForWeapon[9], false); break; // standard bullets
				case 69: inventoryPlayer1.AddAmmoToInventory(9,index, Const.a.magazinePitchCountForWeapon2[9], true); break; // teflon bullets
				case 70: inventoryPlayer1.AddAmmoToInventory(7,index, Const.a.magazinePitchCountForWeapon[7], false); break; // hollow point rounds
				case 71: inventoryPlayer1.AddAmmoToInventory(7,index, Const.a.magazinePitchCountForWeapon2[7], true); break; // slug rounds
				case 72: inventoryPlayer1.AddAmmoToInventory(0,index, Const.a.magazinePitchCountForWeapon[0], false); break; // magnesium tipped slugs
				case 73: inventoryPlayer1.AddAmmoToInventory(0,index, Const.a.magazinePitchCountForWeapon2[0], true); break; // penetrator slugs
				case 74: inventoryPlayer1.AddAmmoToInventory(3,index, Const.a.magazinePitchCountForWeapon[3], false); break; // hornet clip
				case 75: inventoryPlayer1.AddAmmoToInventory(3,index, Const.a.magazinePitchCountForWeapon2[3], true); break; // splinter clip
				case 76: inventoryPlayer1.AddAmmoToInventory(11,index, Const.a.magazinePitchCountForWeapon[11], false); break; // rail rounds
				case 77: inventoryPlayer1.AddAmmoToInventory(13,index, Const.a.magazinePitchCountForWeapon[13], false); break; // slag magazine
				case 78: inventoryPlayer1.AddAmmoToInventory(13,index, Const.a.magazinePitchCountForWeapon2[13], true); break; // large slag magazine
				case 79: inventoryPlayer1.AddAmmoToInventory(8,index, Const.a.magazinePitchCountForWeapon[8], false); break; // magpulse cartridges
				case 80: inventoryPlayer1.AddAmmoToInventory(8,index, Const.a.magazinePitchCountForWeapon2[8], false); break; // small magpulse cartridges
			}
		}

		Utils.PlayUIOneShotSavable(87); // frob_item
		int numberFoundContents = 0;
		if (currentSearchItem != null) {
			SearchableItem curSearchScript = currentSearchItem.GetComponent<SearchableItem>();
			if (curSearchScript != null) {
				int[] resultContents = {-1,-1,-1,-1};  // create blanked container for search results
				for (int i=3;i>=0;i--) {
					resultContents[i] = curSearchScript.contents[i];
					if (resultContents[i] > -1) numberFoundContents++; // if something was found, add 1 to count
				}
			}
	    	if (numberFoundContents == 0) {
				currentSearchItem = null;
				Sys_UI.ReturnTabsFromSearch();
			}
		}
		firstTimePickup = false;
	}

	public void DropHeldItem() {
		dropFinished = Time.time + 0.2f; // Prevent immediate regrab at high fps
		if (heldObjectIndex < 0 || heldObjectIndex > 110) { 
			DualLog("BUG: Attempted to DropHeldItem with index out of bounds (<0 or >110) and heldObjectIndex = " + heldObjectIndex.ToString(),player);
			ResetHeldItem();
			return;
		}

		if (!grenadeActive) heldObject = Const.a.GetPrefab(heldObjectIndex + 307); // heldObject is set by UseGrenade() so don't override here.
		if (heldObject == null) {
			CenterStatusPrint("BUG: Object "+heldObjectIndex.ToString()+" not assigned, vaporized.",player);
			ResetHeldItem();
			return;
		}

		GameObject tossObject = null;
		bool freeObjectInPoolFound = false;
		GameObject levelDynamicContainer = LevelManager.a.GetCurrentDynamicContainer();

		// Find any free inactive objects within the level's Levelnumber.Dynamic container and activate those before instantiating
		if (!grenadeActive) {
			for (int i=0;i<levelDynamicContainer.transform.childCount;i++) {
				Transform tr = levelDynamicContainer.transform.GetChild(i);
				GameObject go = tr.gameObject;
				UseableObjectUse reference = go.GetComponent<UseableObjectUse>();
				if (reference != null) {
					if (reference.useableItemIndex == heldObjectIndex && go.activeSelf == false) {
						reference.customIndex = heldObjectCustomIndex;
						tossObject = go;
						freeObjectInPoolFound = true;
						break;
					}
				}
			}

			if (freeObjectInPoolFound) {
				if (tossObject == null) {
					CenterStatusPrint("BUG: Failed to get freeObjectInPool for object being dropped!",player);
					ResetHeldItem();
					return;
				} else {
					tossObject.instances[i].position = (instances[i].position + (transform.forward * tossOffset));
				}
			} else {
				// DualLog("WARNING: Failed to get freeObjectInPool for object " + heldObject.ToString() + "being dropped! MouseLookScript DropHeldItem.",player);
				tossObject = Instantiate(heldObject,(instances[i].position + (transform.forward * tossOffset)),Const.a.quaternionIdentity) as GameObject;  //effect
				if (tossObject == null) {
					CenterStatusPrint("BUG: Failed to instantiate object being dropped!",player);
					ResetHeldItem();
					return;
				}
			}
			if (tossObject.activeSelf != true) tossObject.SetActive(true);
			if (levelDynamicContainer != null) {
				tossObject.transform.SetParent(levelDynamicContainer.transform,true);
			}

			Vector3 tossDir = MouseCursor.a.GetCursorScreenPointForRay();
			tossDir = playerCamera.ScreenPointToRay(tossDir).direction;
			Rigidbody rbody = tossObject.GetComponent<Rigidbody>();
			if (rbody != null) {
				rbody.isKinematic = false;
				rbody.useGravity = true;
				rbody.velocity = tossDir * tossForce;
			}

			UseableObjectUse uou = tossObject.GetComponent<UseableObjectUse>();
			uou.customIndex = heldObjectCustomIndex;
			uou.ammo = heldObjectAmmo;
			uou.ammo2 = heldObjectAmmo2;
			uou.heldObjectLoadedAlternate = heldObjectLoadedAlternate;
		} else {
			// Throw an active grenade
			grenadeActive = false;
			Sys_UI.mouseClickHeldOverGUI = true; // Prevent shooting it.
			tossObject = Instantiate(heldObject,(instances[i].position + (transform.forward * tossOffset)),Const.a.quaternionIdentity) as GameObject;  //effect
			if (tossObject == null) {
				CenterStatusPrint("BUG: Failed to instantiate object being dropped!",player);
				ResetHeldItem();
				return;
			}

            Const.a.grenadesThrown++;
			if (levelDynamicContainer != null){
				tossObject.transform.SetParent(levelDynamicContainer.transform,true);
			}
			tossObject.layer = 11; // Set to player bullets layer to prevent collision and still be visible.
			Vector3 tossDir = MouseCursor.a.GetCursorScreenPointForRay();
			tossDir = playerCamera.ScreenPointToRay(tossDir).direction;
			Rigidbody rbody = tossObject.GetComponent<Rigidbody>();
			if (rbody != null) {
				rbody.isKinematic = false;
				rbody.useGravity = true;
				rbody.velocity = tossDir * tossForce;
			}
			GrenadeActivate ga = tossObject.GetComponent<GrenadeActivate>();
			if (ga != null) ga.Activate(); // Time to boom!
			MouseCursor.a.liveGrenade = false;
		}
		ResetHeldItem();
	}

	public void ResetHeldItem() {
		heldObjectIndex = -1;
		heldObjectCustomIndex = -1;
		heldObjectAmmo = 0;
		heldObjectAmmo2 = 0;
		heldObjectLoadedAlternate = false;
		holdingObject = false;
		grenadeActive = false;
		MouseCursor.a.justDroppedItemInHelper = true;
	}

	public void ToggleInventoryMode() {
		if (inventoryMode)	ForceShootMode();
		else				ForceInventoryMode();
	}

	public void ForceShootMode() {
		if (Const.a.NoShootMode) return; // We are being like the original now!

		
		Sys_UI.mouseClickHeldOverGUI = false;
		Automap.a.CloseFullmap();
		Cursor.lockState = CursorLockMode.Locked;
		Cursor.visible = false;
		inventoryMode = false;
		if (Application.platform == RuntimePlatform.Android) {
			shootModeButton.SetActive(true);
		} else {
			shootModeButton.SetActive(false);
		}

		if (vmailActive) {
			inventoryPlayer1.DeactivateVMail();
			vmailActive = false;
		}
	}

	public void ForceInventoryMode() {
		if (inventoryMode) return;

		
		if (Sys_Global.menuActive || Sys_Global.gamePaused) {
			Cursor.lockState = CursorLockMode.None;
		} else {
			#if UNITY_EDITOR
				Cursor.lockState = CursorLockMode.None;
			#else	
				Cursor.lockState = CursorLockMode.Confined;
			#endif
		}
		MouseCursor.SetCursorPosInternal((int)(Screen.width * 0.5f),(int)(Screen.height * 0.5f));
		Cursor.visible = false;
		MouseCursor.a.deltaX = 0;
		MouseCursor.a.deltaY = 0;
		MouseCursor.a.cursorPosition.x = (Screen.width / 2);
		MouseCursor.a.cursorPosition.y = (Screen.height / 2);
		inventoryMode = true;
		if (!Const.a.noHUD) shootModeButton.SetActive(true);
		else shootModeButton.SetActive(false);
	}

	void SearchObject (int index){
		if (currentSearchItem == null) { DualLog("BUG: Early exit from SearchObject, currentSearchItem was null!"); return;}

		bool useFX = true;
		SearchableItem curSearchScript = currentSearchItem.GetComponent<SearchableItem>();
		if (curSearchScript.searchableInUse) {
			for (int i=0;i<4;i++) {
				if (curSearchScript.contents[i] >= 0) {
					MouseCursor.a.GetComponent<MouseCursor>().cursorImage = Const.a.useableItemsFrobIcons[curSearchScript.contents[i]];
					heldObjectIndex = curSearchScript.contents[i];
					heldObjectCustomIndex = curSearchScript.customIndex[i];
					curSearchScript.contents[i] = -1;
					curSearchScript.customIndex[i] = -1;
					if (heldObjectIndex != -1) holdingObject = true;
					CenterStatusPrint("%s", Sys_Text.stringTable[heldObjectIndex + 326]
								 + Sys_Text.stringTable[319],player); // picked up

					Sys_UI.DisableSearchItemImage(i);
					useFX = false;
					break;
				}
			}
		} else {
			Utils.PlayUIOneShotSavable(91); // searchsound
		}

		curSearchScript.searchableInUse = true;

		// Search through array to see if any items are in the container
		int numberFoundContents = 0;
		int[] resultContents = {-1,-1,-1,-1};  // create blanked container for search results
		int[] resultCustomIndex = {-1,-1,-1,-1};  // create blanked container for search results custom indices
		for (int i=3;i>=0;i--) {
			resultContents[i] = curSearchScript.contents[i];
			resultCustomIndex[i] = curSearchScript.customIndex[i];
			// If something was found, add 1 to count.
			if (resultContents[i] > -1) numberFoundContents++;
		}

		if (firstTimeSearch) {
			firstTimeSearch = false;
			Sys_UI.OpenTab (4, true, TabMSG.Search, -1,Handedness.LH);
		}
		Sys_UI.SendSearchToDataTab(curSearchScript.objectName,
										 numberFoundContents,resultContents,
										 resultCustomIndex,
										 currentSearchItem.instances[i].position,
										 curSearchScript, useFX);
		ForceInventoryMode();
	}

	public void UseGrenade (int index) {
		if (holdingObject) { CenterStatusPrint("%s", Sys_Text.stringTable[311],player); return; } // Can't use grenade, hands full
		if (index < 7 || index > 13) { DualLog("BUG: index outside of 7 to 13 passed to UseGrenade() in MouseLookScript.cs"); return; }

		ForceInventoryMode();  // Inventory mode is turned on when picking something up.
		ResetHeldItem();
		MouseCursor.a.liveGrenade = true;
		grenadeActive = true;
		CenterStatusPrint("%s", Sys_Text.stringTable[index + 326]
					 + Sys_Text.stringTable[320],player); // activated, grenade is LIVE!

		switch(index) { // Subtract one from the correct grenade inventory
			case 7:  heldObject = Const.a.GetPrefab(370); inventoryPlayer1.RemoveGrenade(0); break; // Frag
			case 8:  heldObject = Const.a.GetPrefab(372); inventoryPlayer1.RemoveGrenade(3); break; // Concussion
			case 9:  heldObject = Const.a.GetPrefab(387); inventoryPlayer1.RemoveGrenade(1); break; // EMP
			case 10: heldObject = Const.a.GetPrefab(389); inventoryPlayer1.RemoveGrenade(6); break; // Earth Shaker
			case 11: heldObject = Const.a.GetPrefab(402); inventoryPlayer1.RemoveGrenade(4); break; // Land Mine
			case 12: heldObject = Const.a.GetPrefab(403); inventoryPlayer1.RemoveGrenade(5); break; // Nitropak
			case 13: heldObject = Const.a.GetPrefab(404); inventoryPlayer1.RemoveGrenade(2); break; // Gas
		}
		Sys_UI.ResetItemTab();
		PutObjectInHand(index,-1,0,0,false,true);
	}
