// automap.c - Automap System
#define mapWorldMaxN 85.83999f
#define mapWorldMaxS = -78.00001f
#define mapWorldMaxE -70.44f
#define mapWorldMaxW 93.4f
#define mapTileMinX 8 // top left corner
#define mapTileMinY -1016 // bottom right corner
static const float camMaxAmount = 0.2548032f;

// public class Automap : MonoBehaviour {
// 	public Camera automapCamera;
// 	public GameObject automapCanvasGO;
// 	public GameObject automapContainerLH;
// 	public GameObject automapContainerRH;
// 	public GameObject automapTabLH;
// 	public GameObject automapTabRH;
// 	public GameObject automapFull;
// 	public Image[] automapFoWTiles;
// 	public Image automapBaseImage;
// 	public Image automapInnerCircleLH;
// 	public Image automapInnerCircleRH;
// 	public Image automapOuterCircleLH;
// 	public Image automapOuterCircleRH;
// 	public GameObject automapTopLH;
// 	public GameObject automapTopRH;
// 	public GameObject automapSideLH;
// 	public GameObject automapSideRH;
// 	public Image automapSideLHImage;
// 	public Image automapSideRHImage;
// 	public Sprite[] automapsBaseImages;
// 	public Sprite[] automapsSideImages;
// 	public Image[] automapsHazardOverlays;
// 	public Image automapFullPlayerIcon;
// 	public Transform automapNormalPlayerIconLH;
// 	public GameObject automapNormalPlayerIconGOLH;
// 	public Transform automapNormalPlayerIconRH;
// 	public GameObject automapNormalPlayerIconGORH;
// 	public GameObject[] levelOverlayContainer;
// 	public GameObject levelOverlayContainerR;
// 	public GameObject levelOverlayContainer1;
// 	public GameObject levelOverlayContainer2;
// 	public GameObject levelOverlayContainer3;
// 	public GameObject levelOverlayContainer4;
// 	public GameObject levelOverlayContainer5;
// 	public GameObject levelOverlayContainer6;
// 	public GameObject levelOverlayContainer7;
// 	public GameObject levelOverlayContainer8;
// 	public GameObject levelOverlayContainer9;
// 	public GameObject levelOverlayContainerG1;
// 	public GameObject levelOverlayContainerG2;
// 	public GameObject levelOverlayContainerG4;
// 	public GameObject poolContainerAutomapBotOverlays;
// 	public GameObject poolContainerAutomapMutantOverlays;
// 	public GameObject poolContainerAutomapCyborgOverlays;
// 	public bool[] automapExploredR; // save
// 	public bool[] automapExplored1; // save
// 	public bool[] automapExplored2; // save
// 	public bool[] automapExplored3; // save
// 	public bool[] automapExplored4; // save
// 	public bool[] automapExplored5; // save
// 	public bool[] automapExplored6; // save
// 	public bool[] automapExplored7; // save
// 	public bool[] automapExplored8; // save
// 	public bool[] automapExplored9; // save
// 	public bool[] automapExploredG1; // save
// 	public bool[] automapExploredG2; // save
// 	public bool[] automapExploredG4; // save
// 	public Text automapSideButtonTextLH;
// 	public Text automapSideButtonTextRH;
// 	public bool inSideView; // save
// 
// 	bool inFullMap;
// 	int currentAutomapZoomLevel = 0;
// 	float automapUpdateFinished; // save
// 	private bool[] automapExplored;
// 	private const float automapZoom0 = 1.2f;
// 	private const float automapZoom1 = 0.75f;
// 	private const float automapZoom2 = 0.55f;
// 	// private float circleInnerRangev1 = 7.679999f; //(2.5f * CELLSZ) + CELLXHALF;
// 	// private float circleOuterRangev1 = 11.52f; //(4f * CELLSZ) + CELLXHALF;
// 	// private float circleInnerRangev2 = 8.96f; //(3f * CELLSZ) + CELLXHALF;
// 	// private float circleOuterRangev2 = 12.8f; //(4.5f * CELLSZ) + CELLXHALF;
// 	// private float circleInnerRangev3 = 14.08f; //(5f * CELLSZ) + CELLXHALF;
// 	// private float circleOuterRangev3 = 20.48f; //(7.5f * CELLSZ) + CELLXHALF;
// 	// private float automapFactorx = 1.25f;
// 	// private float automapFactory = 1.135f;
// 	private const float automapCorrectionX = -0.008f;
// 	private const float automapCorrectionY = 0.099f;
// 	private const float automapTileCorrectionX = -516f;
// 	private const float automapTileCorrectionY = -516f;
// 	private const float automapFoWRadius = 30f;
// 	public float automapTileBCorrectionX = 0f;
// 	public float automapTileBCorrectionY = 0f;
// 	private float icoZAdj;
// 	private float updateTime;
// 	private V3 tempVec;
// 	private V3 tempVec2;
// 	private V2 tempVec2b;
// 	private Transform automapCameraTransform;
// 	private RectTransform[] automapFoWTilesRects;
// 	private V2[] automapFoWTilesRectsPos;
// 	private bool initialized = false;
// 	private static StringBuilder s1 = new StringBuilder();
// 
// 	public static Automap a;
// 
// 	public void Awake() {
// 		a = this;
// 		a.initialized = false;
// 		a.inSideView = false;
// 	}
// 
// 	void Start() {
// 		automapExplored = new bool[4096];
// 		automapUpdateFinished = World.pauseRelativeTime;
// 		AutomapZoomAdjust();
// 		icoZAdj = 0f;
// 		automapCameraTransform = automapCamera.transform;
// 		automapExploredR = new bool[4096];
// 		automapExplored1 = new bool[4096];
// 		automapExplored2 = new bool[4096];
// 		automapExplored3 = new bool[4096];
// 		automapExplored4 = new bool[4096];
// 		automapExplored5 = new bool[4096];
// 		automapExplored6 = new bool[4096];
// 		automapExplored7 = new bool[4096];
// 		automapExplored8 = new bool[4096];
// 		automapExplored9 = new bool[4096];
// 		automapExploredG1 = new bool[4096];
// 		automapExploredG2 = new bool[4096];
// 		automapExploredG4 = new bool[4096];
// 		automapFoWTilesRects = new RectTransform[automapFoWTiles.Length];
// 		automapFoWTilesRectsPos = new V2[automapFoWTiles.Length];
// 		for (int i=0;i<automapFoWTiles.Length;i++) {
// 			automapFoWTilesRects[i] = automapFoWTiles[i].rectTransform;
// 			automapFoWTilesRectsPos[i].x = automapFoWTilesRects[i].localPosition.x;
// 			automapFoWTilesRectsPos[i].y = automapFoWTilesRects[i].localPosition.y;
// 			automapExploredR[i] = false;
// 			automapExplored1[i] = false;
// 			automapExplored2[i] = false;
// 			automapExplored3[i] = false;
// 			automapExplored4[i] = false;
// 			automapExplored5[i] = false;
// 			automapExplored6[i] = false;
// 			automapExplored7[i] = false;
// 			automapExplored8[i] = false;
// 			automapExplored9[i] = false;
// 			automapExploredG1[i] = false;
// 			automapExploredG2[i] = false;
// 			automapExploredG4[i] = false;
// 		}
// 
// 		//automapLevelHomePositions[0] =  new V2(  43.97f,  85.66f); // R
// 		//automapLevelHomePositions[1] =  new V2(  -8.53f,  85.99f);
// 		//automapLevelHomePositions[2] =  new V2(  10.20f,  44.80f);
// 		//automapLevelHomePositions[3] =  new V2(   9.40f,  63.83f);
// 		//automapLevelHomePositions[4] =  new V2( -55.65f, 116.80f);
// 		//automapLevelHomePositions[5] =  new V2(  -9.40f,  71.80f);
// 		//automapLevelHomePositions[6] =  new V2(  29.70f,  85.50f);
// 		//automapLevelHomePositions[7] =  new V2(   5.00f,  76.55f);
// 		//automapLevelHomePositions[8] =  new V2(  25.10f,  84.40f);
// 		//automapLevelHomePositions[9] =  new V2(  39.80f,  72.60f);
// 		//automapLevelHomePositions[10] = new V2( 440.80f, 200.60f); // G1
// 		//automapLevelHomePositions[11] = new V2(  80.16f,-196.68f); // G2
// 		//automapLevelHomePositions[12] = new V2(  99.50f, 416.90f); // G4
// 		//automapLevelHomePositions[13] = new V2(   0.00f,   0.00f);
// 		initialized = true;
// 
// 		if (LevelManager.a != null)
// 			SetAutomapExploredReference(LevelManager.a.currentLevel);
// 		else
// 			SetAutomapExploredReference(1);
// 	}
// 
// 	public void UpdateAutomap(V3 playerPosition) {
// 		if (PlayerMovement.a.inCyberSpace) return;
// 		if (!initialized) Start();
// 		if (inSideView) {
// 			Utils.AssignImageOverride(automapSideLHImage,automapsSideImages[LevelManager.a.currentLevel]);
// 			Utils.AssignImageOverride(automapSideRHImage,automapsSideImages[LevelManager.a.currentLevel]);
// 			return;
// 		} 
// 		if (AutoMapDisplayActive()) {
// 			Utils.Activate(automapCamera.gameObject);
// 			Utils.EnableCamera(automapCamera);
// 		} else {
// 			Utils.Deactivate(automapCamera.gameObject);
// 			Utils.DisableCamera(automapCamera);
// 		}
// 		if (World.invP1.hardwareVersion[HW_NAV_IDX] < 2) { Utils.Deactivate(poolContainerAutomapBotOverlays); }
// 		else { Utils.Activate(poolContainerAutomapBotOverlays); }
// 		if (World.invP1.hardwareVersion[HW_NAV_IDX] < 3) {
// 			Utils.Deactivate(poolContainerAutomapCyborgOverlays);
// 			Utils.Deactivate(poolContainerAutomapMutantOverlays);
// 		} else {
// 			Utils.Activate(poolContainerAutomapCyborgOverlays);
// 			Utils.Activate(poolContainerAutomapMutantOverlays);
// 		}
// 
// // 		if (automapUpdateFinished < World.pauseRelativeTime) {
// 			Utils.EnableImage(automapBaseImage);
// 			if (LevelManager.a.currentLevel >= 0) {
// 				Utils.AssignImageOverride(automapBaseImage,
// 					automapsBaseImages[LevelManager.a.currentLevel]);
// 			}
// 
// 			float mapWidth = (Const.mapWorldMaxW - Const.mapWorldMaxE);
// 			float mapHeight = (Const.mapWorldMaxN - Const.mapWorldMaxS);
// 			float ewOffset = (playerPosition.z - Const.mapWorldMaxE);
// 			float nsOffset = (playerPosition.x - Const.mapWorldMaxS);
// 
// 			tempVec.x = ((ewOffset / mapWidth) * (camMaxAmount * 2f))
// 						+ (camMaxAmount * -1f);
// 
// 			tempVec.y = ((nsOffset / mapHeight) * (camMaxAmount * 2f))
// 						+ (camMaxAmount * -1f);
// 
// 			tempVec.z = automapCameraTransform.localPosition.z;
// 			tempVec.x = (tempVec.x * -1f) + automapCorrectionX;
// 			tempVec.y += automapCorrectionY;
// 
// 			// private float mapTileMinX = 8; // top left corner
// 			// private float mapTileMaxY = -8; // top left corner
// 			// private float mapTileMinY = -1016; // bottom right corner
// 			// private float mapTileMaxX = 1016; // bottom right corner
// 			tempVec2b.x = ((ewOffset/mapWidth) * 1008f)
// 						  + Const.mapTileMinX + automapTileBCorrectionX;
// 
// 			tempVec2b.y = (((nsOffset/mapHeight) * 1008f)
// 						  + Const.mapTileMinY + automapTileBCorrectionY);
// 
// 			if (inFullMap) {
// 				Utils.EnableImage(automapFullPlayerIcon);
// 				SetLinkedOverlayPos(automapFullPlayerIcon,1f,PlayerMovement.a.gameObject);
// 				automapFullPlayerIcon.rectTransform.anchoredPosition = (V3){1024f - automapFullPlayerIcon.rectTransform.anchoredPosition.x + 512f - 2048f + 57f - 10.0839f,
// 																				   1024f - automapFullPlayerIcon.rectTransform.anchoredPosition.y - 512f - 168.8f + 68.404f,
// 																				   -0.03544822f);
// 				// Move the map to center.
// 				tempVec.x = 0;
// 				tempVec.y = 0;
// 				automapCameraTransform.localPosition = tempVec; 
// 			} else {
// 				Utils.DisableImage(automapFullPlayerIcon);
// 				// Move the map to reflect player movement.
// 				automapCameraTransform.localPosition = tempVec;
// 			}
// 
// 			// Update player icon rotation.
// 			// Rotation is adjusted for player view and direction vs UI space.
// 			icoZAdj = (PlayerMovement.a.transform.eulerAngles.y * (-1) + 90);
// 
// 			float zLH = automapNormalPlayerIconLH.localRotation.z;
// 			float zRH = automapNormalPlayerIconRH.localRotation.z;
// 			float zFull = automapFullPlayerIcon.rectTransform.localRotation.z;
// 			Quaternion icoQ = Quaternion.Euler(0,0,icoZAdj);
// 			if (vabs(zLH - icoZAdj) > 0.5f) {
// 				automapNormalPlayerIconLH.localRotation = icoQ;
// 			}
// 
// 			if (vabs(zRH - icoZAdj) > 0.5f) {
// 				automapNormalPlayerIconRH.localRotation = icoQ;
// 			}
// 
// 			if (vabs(zFull - icoZAdj) > 0.5f) {
// 				automapFullPlayerIcon.rectTransform.localRotation = icoQ;
// 			}
// 
// 			updateTime = 0.2f;
// 			if (World.invP1.NavUnitVersion() > 1) updateTime = 0.1f;
// 			if (World.invP1.NavUnitVersion() > 2) {
// 				updateTime = 0.05f;
// 
// 				// Display hazards
// 				for (int j=0;j<13;j++) {
// 					if (j != LevelManager.a.currentLevel) {
// 						Utils.DisableImage(automapsHazardOverlays[j]);
// 						Utils.Deactivate(automapsHazardOverlays[j].gameObject);
// 					} else {
// 						Utils.EnableImage(automapsHazardOverlays[j]);
// 						Utils.Activate(automapsHazardOverlays[j].gameObject);
// 					}
// 				}
// 
// 				// Display cyborg and mutant overlays - Handled by AIController
// 				// since it updates it anyways.
// 			}
// 
// 			if (automapUpdateFinished < World.pauseRelativeTime) {
// 				float radiusSquared = automapFoWRadius * automapFoWRadius;
// 				V2 plyrPos = tempVec2b;
// 				// Update explored tiles
// 				for (int i=0;i<4096;i++) {
// 					// if (automapExplored[i]) {
// 					// 	Utils.DisableImage(automapFoWTiles[i]);
// 					// 	Utils.Deactivate(automapFoWTiles[i].gameObject);
// 					// } else {
// 						tempVec2b.x = automapFoWTilesRectsPos[i].x * -1f
// 									- automapTileCorrectionX;
// 
// 						tempVec2b.y = automapFoWTilesRectsPos[i].y
// 									+ automapTileCorrectionY;
// 
// 						tempVec2b.x -= plyrPos.x;
// 						tempVec2b.x *= tempVec2b.x;
// 						tempVec2b.y -= plyrPos.y;
// 						tempVec2b.y *= tempVec2b.y;
// 						if (   tempVec2b.x < radiusSquared
// 							&& tempVec2b.y < radiusSquared
// 							&& (tempVec2b.x + tempVec2b.y) < radiusSquared) {
// 							automapExplored[i] = true;
// 							SetAutomapTileExplored(LevelManager.a.currentLevel,i);
// 							Utils.DisableImage(automapFoWTiles[i]);
// 							Utils.Deactivate(automapFoWTiles[i].gameObject);
// 						}
// 					//}
// 				}
// 				automapUpdateFinished = World.pauseRelativeTime + updateTime;
// 			}
// // 			automapUpdateFinished = World.pauseRelativeTime + updateTime;
// // 		}
// 
// 		SetAutomapActiveState();
// 	}
// 
// 	public void ActivateAutomapUI() {
// 		Utils.EnableCamera(automapCamera);
// 		Utils.Activate(automapCanvasGO);
// 		ActivateLevelOverlayContainer(LevelManager.a.currentLevel);
// 	}
// 
// 	public void DeactivateAutomapUI() {
// 		Utils.DisableCamera(automapCamera);
// 		Utils.Deactivate(automapCanvasGO);
// 		ActivateLevelOverlayContainer(-1);
// 	}
// 
// 	void SetAutomapActiveState() {
// 		if (World.invP1.hasHardware[1]) {
// 			if (AutoMapDisplayActive()) {
// 				ActivateAutomapUI();
// 			} else {
// 				DeactivateAutomapUI();
// 			}
// 		} else {
// 			DeactivateAutomapUI();
// 		}
// 	}
// 
// 	// True if, full map is displayed or either left or right automap.
// 	bool AutoMapDisplayActive() {
// 		if (automapTabLH.activeInHierarchy) return true;
// 		if (automapTabRH.activeInHierarchy) return true;
// 		if (automapFull.activeInHierarchy) return true;
// 		return false;
// 	}
// 
// 	void ActivateLevelOverlayContainer(int current) {
// 		if (!initialized) Start();
// 		for (int i=0;i<levelOverlayContainer.Length;i++) {
// 			if (i == current) continue;
// 
// 			Utils.Deactivate(levelOverlayContainer[i]);
// 		}
// 
// 		if (current < 0 || current >= levelOverlayContainer.Length) return;
// 
// 		Utils.Activate(levelOverlayContainer[current]);
// 	}
// 
// 	public void SetAutomapExploredReference(int currentLevel) {
// 		if (!initialized) Start();
// 		switch(currentLevel) {
// 			case 0: for (int i=0;i<4096;i++) { automapExplored[i] = automapExploredR[i]; } break;
// 			case 1: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored1[i]; } break;
// 			case 2: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored2[i]; } break;
// 			case 3: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored3[i]; } break;
// 			case 4: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored4[i]; } break;
// 			case 5: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored5[i]; } break;
// 			case 6: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored6[i]; } break;
// 			case 7: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored7[i]; } break;
// 			case 8: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored8[i]; } break;
// 			case 9: for (int i=0;i<4096;i++) { automapExplored[i] = automapExplored9[i]; } break;
// 			case 10:for (int i=0;i<4096;i++) { automapExplored[i] = automapExploredG1[i];} break;
// 			case 11:for (int i=0;i<4096;i++) { automapExplored[i] = automapExploredG2[i];} break;
// 			case 12:for (int i=0;i<4096;i++) { automapExplored[i] = automapExploredG4[i];} break;
// 		}
// 
// 		for (int i=0;i<4096;i++) {
// 			if (automapExplored[i]) {
// 				Utils.DisableImage(automapFoWTiles[i]);
// 				Utils.Deactivate(automapFoWTiles[i].gameObject);
// 			} else {
// 				automapFoWTiles[i].enabled = true;
// 				automapFoWTiles[i].gameObject.SetActive(true);
// 			}
// 		}
// 	}
// 
// 	void SetAutomapTileExplored(int currentLevel, int index) {
// 		if (!initialized) Start();
// 		switch(currentLevel) {
// 			case 0: automapExploredR[index] = true; break;
// 			case 1: automapExplored1[index] = true; break;
// 			case 2: automapExplored2[index] = true; break;
// 			case 3: automapExplored3[index] = true; break;
// 			case 4: automapExplored4[index] = true; break;
// 			case 5: automapExplored5[index] = true; break;
// 			case 6: automapExplored6[index] = true; break;
// 			case 7: automapExplored7[index] = true; break;
// 			case 8: automapExplored8[index] = true; break;
// 			case 9: automapExplored9[index] = true; break;
// 			case 10:automapExploredG1[index]= true; break;
// 			case 11:automapExploredG2[index]= true; break;
// 			case 12:automapExploredG4[index]= true; break;
// 		}
// 	}
// 
// 	void AutomapZoomOut() {
// 		if (World.invP1.NavUnitVersion() < 2) {
// 			// Map hardware version doesn't support zoom.
// 			CenterStatusPrint("%s", Text->stringTable[465]);
// 			return;
// 		}
// 
// 		currentAutomapZoomLevel++;
// 		if (currentAutomapZoomLevel > 2) {
// 			currentAutomapZoomLevel = 2;
// 
// 			// zoom at max
// 			CenterStatusPrint("%s", Text->stringTable[316],Const.a.player1);
// 			return;
// 		}
// 		AutomapZoomAdjust();
// 	}
// 
// 	public void AutomapZoomIn() {
// 		if (World.invP1.NavUnitVersion() < 2) {
// 			// Map hardware version doesn't support zoom.
// 			CenterStatusPrint("%s", Text->stringTable[465],Const.a.player1);
// 			return;
// 		}
// 
// 		currentAutomapZoomLevel--;
// 		if (currentAutomapZoomLevel < 0) {
// 			currentAutomapZoomLevel = 0;
// 
// 			// zoom at min
// 			CenterStatusPrint("%s", Text->stringTable[317],Const.a.player1);
// 			return;
// 		}
// 		AutomapZoomAdjust();
// 	}
// 
// 	void AutomapZoomAdjust() {
// 		float zoom = automapZoom0;
// 		switch (currentAutomapZoomLevel) {
// 			case 0:  zoom = automapZoom0; break;
// 			case 1:  zoom = automapZoom1; break;
// 			case 2:  zoom = automapZoom2; break;
// 			default: zoom = automapZoom0; break;
// 		}
// 
// 		V3 scaleVec = (V3){zoom,zoom,zoom);
// 		automapContainerLH.World.instances[i].scale = scaleVec;
// 		automapContainerRH.World.instances[i].scale = scaleVec;
// 	}
// 
// 	public void ToggleSideTop() {
// 		inSideView = !inSideView;
// 		if (inSideView) AutomapGoSide();
// 		else AutomapGoTop();
// 	}
// 
// 	public void AutomapGoSide() {
// 		automapSideButtonTextLH.text = Text->stringTable[887];
// 		automapSideButtonTextRH.text = Text->stringTable[887];
// 		automapInnerCircleLH.flag_set(&SELF.entflags, EF_ACTIVE, false);
// 		automapInnerCircleRH.flag_set(&SELF.entflags, EF_ACTIVE, false);
// 		automapOuterCircleLH.flag_set(&SELF.entflags, EF_ACTIVE, false);
// 		automapOuterCircleRH.flag_set(&SELF.entflags, EF_ACTIVE, false);
// 		automapTopLH.SetActive(false);
// 		automapTopRH.SetActive(false);
// 		automapNormalPlayerIconGOLH.SetActive(false);
// 		automapNormalPlayerIconGORH.SetActive(false);
// 		automapSideLH.SetActive(true);
// 		automapSideRH.SetActive(true);
// 		Utils.AssignImageOverride(automapSideLHImage,automapsSideImages[LevelManager.a.currentLevel]);
// 		Utils.AssignImageOverride(automapSideRHImage,automapsSideImages[LevelManager.a.currentLevel]);
// 	}
// 
// 	public void AutomapGoTop() {
// 		automapSideButtonTextLH.text = Text->stringTable[888];
// 		automapSideButtonTextRH.text = Text->stringTable[888];
// 		automapInnerCircleLH.gameObject.SetActive(true);
// 		automapInnerCircleRH.gameObject.SetActive(true);
// 		automapOuterCircleLH.gameObject.SetActive(true);
// 		automapOuterCircleRH.gameObject.SetActive(true);
// 		automapTopLH.SetActive(true);
// 		automapTopRH.SetActive(true);
// 		automapNormalPlayerIconGOLH.SetActive(true);
// 		automapNormalPlayerIconGORH.SetActive(true);
// 		automapSideLH.SetActive(false);
// 		automapSideRH.SetActive(false);
// 	}
// 
// 	public void AutomapGoFull(u16 playerIdx, SystemUI* ui) {
// 		if (ui->inSideView) {
// 			AutomapGoTop();
// 			UpdateAutomap(World.instances[playerIdx].position);
// 		}
// 		Utils.Activate(automapFull); TODO
// 		ui->inFullMap = true;
// 		AutomapGoFull(ui);
// 	}
// 
// 	public void CloseFullmap() {
// 		if (automapFull.activeInHierarchy) World.Sys_UI.CloseFullmap();
// 		Utils.Deactivate(automapFull);
// 		inFullMap = false;
// 		if (inSideView) AutomapGoSide();
// 		else AutomapGoTop();
// 	}
// 
// 	// Convert from Worldspace into Automapspace
// 	public static V3 GetMapPos(V3 worldPos) {
// 		float mapWidth = (Const.mapWorldMaxW - Const.mapWorldMaxE);
// 		float mapHeight = (Const.mapWorldMaxN - Const.mapWorldMaxS);
// 		float ewOffset = (worldPos.z - Const.mapWorldMaxE);
// 		float nsOffset = (worldPos.x - Const.mapWorldMaxS);
// 		// private float camMaxAmount = 0.2548032f;
// 		// private float mapWorldMaxN = 85.83999f;
// 		// private float mapWorldMaxS = -78.00001f;
// 		// private float mapWorldMaxE = -70.44f;
// 		// private float mapWorldMaxW = 93.4f;
// 
// 		// 34.16488, -45.08, 0.4855735
// 		// x = ((0.6384575295) * 1008f) + 8;
// 		// x = 651
// 		V3 retval = (V3){0f,0f,0f);
// 		retval.y = ((ewOffset/mapWidth) * 1008f) + Const.mapTileMinX;
// 		retval.x = ((nsOffset/mapHeight) * 1008f) + Const.mapTileMinY;
// 		retval.z = -0.03f; // Always moved to be behind the fog of war tiles.
// 		return retval;
// 	}
// 
// 	public static void TurnOnLinkedOverlay(Image over, float health, GameObject go,
// 									bool isNPC) {
// 		if (over == null) return;
// 
// 		bool navVersionFine = Inventory.a != null ? World.invP1.NavUnitVersion() > 1 : false;
// 		if (health > 0 && ((isNPC && navVersionFine) || !isNPC)) {
// 			Utils.EnableImage(over); // Enable on automap.
// 			Utils.Activate(over.gameObject);
// 		} else {
// 			Utils.Deactivate(over.gameObject);
// 			Utils.DisableImage(over);
// 		}
// 	}
// 
// 	public static void SetLinkedOverlayPos(Image over, float health, GameObject go) {
// 		if (over == null) return;
// 		if (go == null) return;
// 		if (!go.activeInHierarchy) return;
// 		if (health <= 0) return;
// 
// 		Transform tr = go.transform;
// 		V3 worldPos = tr.position;
// 		SecurityCameraRotate scr = null;
// 		Transform par = tr.parent;
// 		if (par != null) {
// 			GameObject parGo = par.gameObject;
// 			if (parGo != null) scr = parGo.GetComponent<SecurityCameraRotate>();
// 		}
// 
// 		if (scr != null) worldPos = scr.World.instances[i].position;
// 		over.rectTransform.anchoredPosition = GetMapPos(worldPos);
// 	}
// }
