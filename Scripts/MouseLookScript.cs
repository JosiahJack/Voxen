
	public Vector2 lastMousePos;
    Vector2 cursorHotspot;
    Vector3 cameraFocusPoint;
	GameObject currentButton;
	GameObject currentSearchItem;
    public int heldObjectIndex; // save
	public int heldObjectCustomIndex; // save
	public int heldObjectAmmo; // save
	public int heldObjectAmmo2; // save
	public bool heldObjectLoadedAlternate; // save
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
        shootModeButton.SetActive(false);
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
		shakeFinished = Eng_Global->pauseRelativeTime;
		returnFromCyberspaceFinished = 0;
		dropFinished = 0;

		// PlayerCapsule
		// -> LeanTransform
        // -> -> MainCamera: MouseLookScript component.
		playerCapsuleTransform = transform.parent.transform.parent.transform;

		randomShakeFinished = Eng_Global->pauseRelativeTime;
		randomKlaxonFinished = Eng_Global->pauseRelativeTime;
		headBobShiftFinished = Eng_Global->pauseRelativeTime;
		bobTarget = 0.3f;
    }
    
    void OnPreCull() {
		DynamicCulling.a.Cull(false); // Update dynamic culling system.
	}

	void Update() {
		// Allow quick load straight from the menu or pause.
		if (Input.GetKeyUp(f9)) {
			if (inCyberSpace) {
				CenterStatusPrint("%s", Eng_Text->stringTable[1023]); // "Cannot load in cyberspace"
				return;
			}

			MainMenuHandler.a.LoadGame(7);
		}

        if (Eng_Global->menuActive) {
			// Ignore mouselook and turn off camera when main menu is up.
			if (!MainMenuHandler.a.fileBrowserOpen) Cursor.visible = false;
			else Cursor.visible = true;

			if (playerCamera.enabled) playerCamera.enabled = false;
			return;
		}

		if (Eng_Global->gamePaused) return;
		if (Eng_Global->instances[PLAYER1].ressurectingFinished > Eng_Global->pauseRelativeTime) return;

		Utils.EnableCamera(playerCamera);

		// Unpaused, normal functions::
		// ====================================================================
		if (Input.GetKeyUp(f6)) {
			if (inCyberSpace) {
				CenterStatusPrint("%s", Eng_Text->stringTable[602]); // Cannot save in cyberspace
				return;
			}
			
			if (MissionTimer.a.timesUP) {
				return;
			}

			Const.a.StartSave(7,qsavename);
		}

		if(GetInput.a.ToggleMode()) ToggleInventoryMode(); // Toggle inventory mode<->shoot mode
		if (Eng_Global->SelfDestructActivated && Eng_Global->currentLevel != 13 && Eng_Global->currentLevel != 9) { // Not Cyberspace, Not the bridge, separated
			if (randomShakeFinished < Eng_Global->pauseRelativeTime) {
				randomShakeFinished = Eng_Global->pauseRelativeTime + random_range(5f,20f);
				ScreenShake(3.0f,2.0f);
			}
			
			if (randomKlaxonFinished < Eng_Global->pauseRelativeTime) {
				randomKlaxonFinished = Eng_Global->pauseRelativeTime + random_range(10f,20f);
				Utils.PlayUIOneShotSavable(104); // klaxon
			}
		}

		RecoilAndRest(); // Spring Back to Rest from Recoil
		keyboardTurnSpeed = 15.0f * Const.a.MouseSensitivity;
		KeyboardTurn();
		KeyboardLookUpDn();
		TouchLook();
		if (inCyberSpace) { // Barrel roll!
			if (GetInput.a.LeanLeft()) {
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.Eng_Global->instances[i].position,
					playerCapsuleTransform.transform.forward,
					cyberSpinSensitivity * Time.deltaTime * 100f
				);
			}

			if (GetInput.a.LeanRight()) {
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.Eng_Global->instances[i].position,
					playerCapsuleTransform.transform.forward,
					cyberSpinSensitivity * Time.deltaTime * -1f * 100f
				);
			}
		} else {
			if (compassContainer.activeInHierarchy) {
				// Update automap player icon orientation.
				compassContainer.Eng_Global->instances[i].rotation =
					Quaternion.Euler(0f, -yRotation + 180f, 0f);
			}
		}

		if (!inventoryMode) Mouselook(); // Only do mouselook in Shoot Mode.
		if(GetInput.a.Use()) Frob(); // Frob what is under our cursor.
	}

	public void Frob() {
		if (!Eng_Global->uiIsBlocking && !inCyberSpace) {
			if (dropFinished < Time.time) {
				currentButton = null; // Force this to reset.
				if (Eng_Global->inventoryPlayer1.holdingObject) {
					if (!FrobWithHeldObject()) DropHeldItem();
				} else FrobEmptyHanded();
			}
		} else {
			//We are holding cursor over the GUI
			if ( && !inCyberSpace) {
				AddItemToInventory(Eng_Global->inventoryPlayer1.heldObjectIndex,heldObjectCustomIndex);
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
				playerCapsuleTransform.Eng_Global->instances[i].position,
				playerCapsuleTransform.transform.up,yRotation
			);

			playerCapsuleTransform.RotateAround(
				playerCapsuleTransform.Eng_Global->instances[i].position,
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
			Eng_Global->instances[i].rotation = Quaternion.Euler(xRotation,0f,0f);
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
		cyberspaceReturnPoint = Eng_Global->instances[PLAYER1].Eng_Global->instances[i].position;
		cyberspaceReturnCameraLocalRotation = Eng_Global->instances[i].rotation.eulerAngles;
		cyberspaceReturnPlayerCapsuleLocalRotation = playerCapsuleTransform.localRotation.eulerAngles;
		cyberspaceReturnLevel = LevelManager.a.currentLevel;
		Eng_UI->EnterCyberspace();
		LevelManager.a.LoadLevel(13,cyberspaceRecallPoint);
		Eng_Global->instances[PLAYER1].inCyberSpace = true;
		Eng_Global->instances[PLAYER1].leanCapsuleCollider.enabled = false;
		hm.inCyberSpace = true;
		inCyberSpace = true;
		playerCamera.useOcclusionCulling = false;
		Eng_UI->DrawTicks(true);
		SetCameraCullDistances();
		Utils.PlayUIOneShotSavable(81); // cyber
	}

	public void ExitCyberspace() {
		playerRadiationTreatmentFlash.SetActive(true);
		Eng_UI->ExitCyberspace();
		LevelManager.a.LoadLevel(cyberspaceReturnLevel,cyberspaceReturnPoint);

		// Left/right component applied to capsule.
		playerCapsuleTransform.localRotation = Quaternion.Euler(0f,
			cyberspaceReturnPlayerCapsuleLocalRotation.y,0f);

		Eng_Global->instances[i].rotation = // Up down component applied to camera
			Quaternion.Euler(cyberspaceReturnCameraLocalRotation.x,
							 cyberspaceReturnCameraLocalRotation.y,
							 cyberspaceReturnCameraLocalRotation.z);

		xRotation = cyberspaceReturnCameraLocalRotation.x;
		yRotation = cyberspaceReturnPlayerCapsuleLocalRotation.y;

		returnFromCyberspaceFinished = Time.time + 0.1f; // Prevent mouselook
														 // messing it up.
		Eng_Global->instances[PLAYER1].inCyberSpace = false;
		Eng_Global->instances[PLAYER1].rbody.velocity = (Vector3){0.0f,0.0f,0.0f};
		Eng_Global->instances[PLAYER1].leanCapsuleCollider.enabled = true;
		hm.inCyberSpace = false;
		inCyberSpace = false;
		playerCamera.useOcclusionCulling = true;
		Const.a.decoyActive = false;
		Eng_UI->DrawTicks(true);
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
			Eng_Global->instances[i].rotation = Quaternion.Euler(xRotation,0f,
													   Eng_Global->instances[i].rotation.z);
		} else if (rightTouchstick.y > 0f) {
			if ((inCyberSpace && Const.a.InputInvertCyberspaceLook) || (!inCyberSpace && Const.a.InputInvertLook))
				xRotation += keyboardTurnSpeed * rightTouchstick.y;
			else
				xRotation -= keyboardTurnSpeed * rightTouchstick.y;

			if (!inCyberSpace) xRotation = vclamp(xRotation, -90f, 90f);  // Limit up and down angle.
			Eng_Global->instances[i].rotation = Quaternion.Euler(xRotation, 0f,
													   Eng_Global->instances[i].rotation.z);
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
					playerCapsuleTransform.Eng_Global->instances[i].position,
					playerCapsuleTransform.transform.up,yRotation
				);
			} else if (GetInput.a.TurnRight()) {
				angX = keyboardTurnSpeed * 18f * ((Const.a.GraphicsFOV / 2f) / Screen.width / 2f);
				yRotation = angX;
				playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.Eng_Global->instances[i].position,
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
					playerCapsuleTransform.Eng_Global->instances[i].position,
					playerCapsuleTransform.transform.right,-xRotation
				);
			} else if (GetInput.a.LookUp()) {
				angY = keyboardTurnSpeed * 18f * ((Const.a.GraphicsFOV / 2f) / Screen.height / 2f);
				if (Const.a.InputInvertCyberspaceLook) xRotation = -angY;
				else xRotation = angY;
			
				xRotation = Clamp0360(xRotation); // Limit up/down to within 360°.
					playerCapsuleTransform.RotateAround(
					playerCapsuleTransform.Eng_Global->instances[i].position,
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
				Eng_Global->instances[i].rotation = Quaternion.Euler(xRotation,0f,
														Eng_Global->instances[i].rotation.z);
			} else if (GetInput.a.LookUp()) {
				if ((inCyberSpace && Const.a.InputInvertCyberspaceLook) || (!inCyberSpace && Const.a.InputInvertLook))
					xRotation += keyboardTurnSpeed;
				else
					xRotation -= keyboardTurnSpeed;

				if (!inCyberSpace) xRotation = vclamp(xRotation, -90f, 90f);  // Limit up and down angle.
				Eng_Global->instances[i].rotation = Quaternion.Euler(xRotation, 0f,
														Eng_Global->instances[i].rotation.z);
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

		if (Eng_Global->inventoryPlayer1.hasHardware[4] && Eng_Global->inventoryPlayer1.hardwareVersion[4] > 1) {
			if (!aic.hasTargetIDAttached) { WeaponFire.a.CreateTargetIDInstance(-1f,aic.healthManager,-1f); return true; }
		}

		CenterStatusPrint("%s",Eng_Text->stringTable[29] + Const.a.nameForNPC[aic.index],player); // "Can't use <enemy>"
		return true;
	}

	bool FrobWithHeldObject() {
		if (Eng_Global->inventoryPlayer1.heldObjectIndex < 0) {
			DualLog("BUG: Attempting to frob with held object, but "
					  + "Eng_Global->inventoryPlayer1.heldObjectIndex < 0.");
			return false; // Invalid item will be dropped, wasn't used up.
		}

		bool frobUser = (Eng_Global->inventoryPlayer1.heldObjectIndex == 54 || Eng_Global->inventoryPlayer1.heldObjectIndex == 56
						 || Eng_Global->inventoryPlayer1.heldObjectIndex == 57 || Eng_Global->inventoryPlayer1.heldObjectIndex == 61
						 || Eng_Global->inventoryPlayer1.heldObjectIndex == 64 || Eng_Global->inventoryPlayer1.heldObjectIndex == 92
						 || Eng_Global->inventoryPlayer1.heldObjectIndex == 93 || Eng_Global->inventoryPlayer1.heldObjectIndex == 94);

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
		ud.mainIndex = Eng_Global->inventoryPlayer1.heldObjectIndex;
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

		Eng_Global->inventoryPlayer1.holdingObject = true;
		Eng_Global->inventoryPlayer1.heldObjectIndex = useableConstdex;
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
		int am1 = Eng_Global->inventoryPlayer1.currentMagazineAmount[wepbut.WepButtonIndex];
		Eng_Global->inventoryPlayer1.currentMagazineAmount[wepbut.WepButtonIndex] = 0;
		int am2 = Eng_Global->inventoryPlayer1.currentMagazineAmount2[wepbut.WepButtonIndex];
		Eng_Global->inventoryPlayer1.currentMagazineAmount2[wepbut.WepButtonIndex] = 0;
		bool loadAlt = false;
		if (am2 > 0) loadAlt = true;
		PutObjectInHand(indexPriorToRemoval,-1,am1,am2,loadAlt,true);
		Eng_Global->inventoryPlayer1.RemoveWeapon(wepbut.WepButtonIndex);
		Eng_Global->inventoryPlayer1.RemoveWeapon(wepbut.WepButtonIndex);
		Eng_UI->SetAmmoIcons(-1,false) ; // Clear the ammo icons.
		Eng_UI->HideAmmoAndEnergyItems();
		wepbut.useableItemIndex = -1;
		wepbut = Eng_UI->wepbutMan.wepButtonsScripts[0];
		Eng_Global->inventoryPlayer1.WeaponChange(wepbut.useableItemIndex,
									 wepbut.WepButtonIndex);
	}

	// Because Unity does not see fit for their Button class to support right
	// click behavior...or any other reasonable mouse button interaction.
	void InventoryButtonUse() {
		if (Eng_Global->inventoryPlayer1.holdingObject) return;
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
				Eng_Global->inventoryPlayer1.grenAmmo[grenbut.GrenButtonIndex]--;
				Eng_Global->inventoryPlayer1.GrenadeCycleDown();
				//Eng_Global->inventoryPlayer1.grenadeCurrent = -1; This was up here, and seemed fine.  Might need to revert line 473 add.
				if (Eng_Global->inventoryPlayer1.grenAmmo[grenbut.GrenButtonIndex] <= 0) {
					Eng_Global->inventoryPlayer1.grenAmmo[grenbut.GrenButtonIndex] = 0;
					Eng_Global->inventoryPlayer1.grenadeCurrent = -1;
					for (int i = 0; i < 7; i++) {
						if (Eng_Global->inventoryPlayer1.grenAmmo[i] > 0) {
							Eng_Global->inventoryPlayer1.grenadeCurrent = i;
						}
					}

					Eng_UI->SendInfoToItemTab(Eng_Global->inventoryPlayer1.grenadeCurrent);
					if (Eng_Global->inventoryPlayer1.grenadeCurrent < 0) {
						Eng_Global->inventoryPlayer1.grenadeCurrent = 0;
					}
				}

				grenadeActive = true;
				PutObjectInHand(indexPriorToRemoval,-1,0,0,false,true);
				break;
			case ButtonType.Patch:
				PatchButton patbut = currentButton.GetComponent<PatchButton>();
				indexPriorToRemoval = patbut.useableItemIndex;
				Eng_Global->inventoryPlayer1.patchCounts[patbut.PatchButtonIndex]--;
				if (Eng_Global->inventoryPlayer1.patchCounts[patbut.PatchButtonIndex] <= 0) {
					Eng_Global->inventoryPlayer1.patchCounts[patbut.PatchButtonIndex] = 0;
					Eng_Global->inventoryPlayer1.patchCurrent = -1;
					
					for (int i = 0; i < 7; i++) {
						if (Eng_Global->inventoryPlayer1.patchCounts[i] > 0) Eng_Global->inventoryPlayer1.patchCurrent = i;
					}
					Eng_UI->SendInfoToItemTab(Eng_Global->inventoryPlayer1.patchCurrent);
					if (Eng_Global->inventoryPlayer1.patchCurrent < 0) {
						Eng_Global->inventoryPlayer1.patchCurrent = 0;
					}
				}
				PutObjectInHand(indexPriorToRemoval,-1,0,0,false,true);
				break;
			case ButtonType.GeneralInv:
				GeneralInvButton genbut = 
					currentButton.GetComponent<GeneralInvButton>();

				// Access Cards button
				if (genbut.GeneralInvButtonIndex == 0) {
					Eng_UI->OpenLastItemSide();
					Eng_UI->SendInfoToItemTab(81);
					return;
				}

				indexPriorToRemoval = genbut.useableItemIndex;
				customIndexPrior = genbut.customIndex;
				Eng_Global->inventoryPlayer1.generalInventoryIndexRef[genbut.GeneralInvButtonIndex] = -1;
				Eng_Global->inventoryPlayer1.generalInvCurrent = -1;
				for (int i = 0; i < 7; i++) {
					if (Eng_Global->inventoryPlayer1.generalInventoryIndexRef[i] >= 0) {
						Eng_Global->inventoryPlayer1.generalInvCurrent = i;
					}
				}
				int referenceIndex = -1;
				if (Eng_Global->inventoryPlayer1.generalInvCurrent >= 0) {
					referenceIndex = Eng_Global->inventoryPlayer1.genButtons[Eng_Global->inventoryPlayer1.generalInvCurrent].transform.GetComponent<GeneralInvButton>().useableItemIndex;
				}

				if (referenceIndex < 0 || referenceIndex > 110) {
					Eng_UI->ResetItemTab();
				} else {
					Eng_UI->SendInfoToItemTab(referenceIndex,genbut.customIndex);
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
		Eng_Global->inventoryPlayer1.holdingObject = true;
		Eng_Global->inventoryPlayer1.heldObjectIndex = sebut.contEng_Global->instances[index];
		heldObjectCustomIndex = sebut.customIndex[index];
		if (currentSearchItem != null) {
			SearchableItem sitem = currentSearchItem.GetComponent<SearchableItem>();
			sitem.contEng_Global->instances[index] = -1;
			sitem.customIndex[index] = -1;
		}
		
		sebut.contEng_Global->instances[index] = -1;
		sebut.customIndex[index] = -1;
		Eng_UI->DisableSearchItemImage(index);
		sebut.CheckForEmpty();
		
		if (Const.a.InputQuickItemPickup) {
			AddItemToInventory(Eng_Global->inventoryPlayer1.heldObjectIndex,heldObjectCustomIndex);
			ResetHeldItem();
		} else {
			CenterStatusPrint("%s", Eng_Text->stringTable[Eng_Global->inventoryPlayer1.heldObjectIndex + 326] + Eng_Text->stringTable[319],player);
			ForceInventoryMode();
		}	
	}

	void RecoilAndRest() {
		float targetY = Const.a.playerCameraOffsetY * Eng_Global->instances[PLAYER1].currentCrouchRatio;
		float targetX = 0f;
		if (Eng_Global->instances[PLAYER1].relSideways > 0) targetX += 0.12f;
		if (Eng_Global->instances[PLAYER1].relSideways < 0) targetX -= 0.12f;
		if (Eng_Global->instances[PLAYER1].relForward != 0) targetY -= 0.08f;
		if (shakeFinished > Eng_Global->pauseRelativeTime) {
			headBobX = Eng_Global->instances[PLAYER1].position.x + random_range(shakeForce * -0.17f, shakeForce * 0.17f);
			headBobY = Eng_Global->instances[PLAYER1].position.y + random_range(shakeForce * -0.08f, shakeForce * 0.08f);
			headBobZ = Eng_Global->instances[PLAYER1].position.z + random_range(shakeForce * -0.17f, shakeForce * 0.17f);
		} else {
			headBobZ = 0f;
			Vector3 vel = Eng_Global->instances[PLAYER1].rbody.velocity;
			vel.y = 0f;
			if (Eng_Global->instances[PLAYER1].relForward + Eng_Global->instances[PLAYER1].relSideways != 0 && Eng_Settings->HeadBob) {
				if (Eng_Global->instances[PLAYER1].headBobShiftFinished < Eng_Global->pauseRelativeTime) {
					Eng_Global->instances[PLAYER1].headBobShiftFinished = Eng_Global->pauseRelativeTime + 0.2f;
					if (!Eng_Global->instances[PLAYER1].isSprinting) Eng_Global->instances[PLAYER1].headBobShiftFinished += 0.1f;
					bobTarget = HeadBobAmount * -1f * vsign(bobTarget);
				}

				if (Eng_Global->instances[PLAYER1].rbody.velocity.magnitude > 0.1f) headBobY = smooth_damp(headBobY,targetY + bobTarget,ref headBobYVel,Const.HeadBobRate);
				headBobX = smooth_damp(headBobX,targetX,ref headBobXVel,Const.HeadBobRate);
			} else {
				headBobX = smooth_damp(headBobX,0f,ref headBobXVel,Const.HeadBobRate);
				headBobY = smooth_damp(headBobY,Const.a.playerCameraOffsetY * Eng_Global->instances[PLAYER1].currentCrouchRatio,ref headBobYVel,Const.HeadBobRate);
			}
		}
		
		if (inCyberSpace) headBobX = headBobY = headBobZ = 0f;
		Eng_Global->instances[PLAYER1].position = (Vector3){ headBobX, headBobY, headBobZ };
	}

	public void DropHeldItem() {
		dropFinished = Time.time + 0.2f; // Prevent immediate regrab at high fps
		if (Eng_Global->inventoryPlayer1.heldObjectIndex < 0 || Eng_Global->inventoryPlayer1.heldObjectIndex > 110) { 
			DualLog("BUG: Attempted to DropHeldItem with index out of bounds (<0 or >110) and Eng_Global->inventoryPlayer1.heldObjectIndex = " + Eng_Global->inventoryPlayer1.heldObjectIndex.ToString(),player);
			ResetHeldItem();
			return;
		}

		if (!grenadeActive) heldObject = Const.a.GetPrefab(Eng_Global->inventoryPlayer1.heldObjectIndex + 307); // heldObject is set by UseGrenade() so don't override here.
		if (heldObject == null) {
			CenterStatusPrint("BUG: Object "+Eng_Global->inventoryPlayer1.heldObjectIndex.ToString()+" not assigned, vaporized.",player);
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
					if (reference.useableItemIndex == Eng_Global->inventoryPlayer1.heldObjectIndex && go.activeSelf == false) {
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
					tossObject.Eng_Global->instances[i].position = (Eng_Global->instances[i].position + (transform.forward * tossOffset));
				}
			} else {
				// DualLog("WARNING: Failed to get freeObjectInPool for object " + heldObject.ToString() + "being dropped! MouseLookScript DropHeldItem.",player);
				tossObject = Instantiate(heldObject,(Eng_Global->instances[i].position + (transform.forward * tossOffset)),Const.a.quaternionIdentity) as GameObject;  //effect
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
			Eng_UI->mouseClickHeldOverGUI = true; // Prevent shooting it.
			tossObject = Instantiate(heldObject,(Eng_Global->instances[i].position + (transform.forward * tossOffset)),Const.a.quaternionIdentity) as GameObject;  //effect
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
