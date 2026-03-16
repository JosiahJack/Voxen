using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.SceneManagement;
using System.Linq;
using System.IO;

public class PauseScript : MonoBehaviour {
	public GameObject pauseText;
	public GameObject[] disableUIOnPause;
	public GameObject saltTheFries;
	public GameObject[] enableUIOnPause;
	public GameObject mainMenu;
	public GameObject saveDialog;
	public GameObject hardSaveDialog;

	bool paused = false;
	bool previousInvMode = false;
	bool onSaveDialog = false;
	public float relativeTime;
	public float absoluteTime;
	List<AmbientRegistration> ambientRegistry;
	private bool menuActive = true; // Store the state of the main menu
	                                // gameobject active state so that we don't
									// have to do a gameobject engine call more
									// than once on every Update all over the
									// code.

	public static PauseScript a;

	public void SetA() {
		if (a == null) a = this;		
	}
	
	void Awake() {
		SetA();
		a.ambientRegistry = new List<AmbientRegistration>();
		a.previousInvMode = true;
	}

	// The whole point right here:
	public bool Paused() { return paused || Const.a.loading; }
	public bool MenuActive() { return menuActive; }

	void Update() {
	    if (relativeTime > 0f) absoluteTime += Time.deltaTime;
		if (Input.GetKeyDown(KeyCode.F12)) TakeScreenshot();

		menuActive = mainMenu.activeSelf;
		if (!menuActive) {
			if (!MouseLookScript.a.playerCamera.enabled) MouseLookScript.a.playerCamera.enabled = true;
			if (GetInput.a.Menu()) {
				if (onSaveDialog)
					ExitSaveDialog();
				else
					PauseToggle();
			}

			if (Input.GetKeyDown(KeyCode.Home)
			    || Input.GetKeyDown(KeyCode.Menu)) {

			    PauseEnable();
			}

			CheckForSuperWinCmdKey();
			//if (!Paused()) RaycastAudioOcclusion(); TODO setting, 2.8ms cpu cost!!!!
		}

		if (!Paused()) relativeTime += Time.deltaTime;
	}

	public void ConsoleEntryEnterDelegate() {
		ConsoleEmulator.ConsoleEntryEnter();
	}

	public void RaycastAudioOcclusion() {
		// Raytraced Audio Occlusion with no bounce ;)
		int hitCount = 0;
		float newVolume = 1.0f;
		RaycastHit[] results = new RaycastHit[6];
		for (int i=0;i<ambientRegistry.Count;i++) {
			if (ambientRegistry[i] == null) continue;

			hitCount = RaycastNonAlloc(
						MouseLookScript.a.Eng_Global->instances[i].position,
						ambientRegistry[i].Eng_Global->instances[i].position
						- MouseLookScript.a.Eng_Global->instances[i].position,
						results,32f,Const.a.layerMaskPlayerFrob,
						QueryTriggerInteraction.UseGlobal);

			ambientRegistry[i].SFX.volume =
				ambientRegistry[i].normalVolume;

			if (hitCount > 0) {
				if (hitCount > 5) {
					newVolume = ambientRegistry[i].normalVolume * 0.40f;
				} else if (hitCount == 5) {
					newVolume = ambientRegistry[i].normalVolume * 0.50f;
				} else if (hitCount == 4) {
					newVolume = ambientRegistry[i].normalVolume * 0.60f;
				} else if (hitCount == 3) {
					newVolume = ambientRegistry[i].normalVolume * 0.70f;
				} else if (hitCount == 2) {
					newVolume = ambientRegistry[i].normalVolume * 0.80f;
				} else {
					newVolume = ambientRegistry[i].normalVolume * 0.90f;
				}

				ambientRegistry[i].SFX.volume = newVolume;
			}
		}

		Const.a.NPCAudioOcclusion();
	}

	private bool PhysObjAffectedByFloor(Vector3 objpos, Vector3 floorpos) {
		if (objpos.x - floorpos.x > 1.28f && objpos.z - floorpos.z > 1.28f) {
			return true;
		}

		return false;
	}

	void CheckForSuperWinCmdKey() {
		if (   Input.GetKeyDown(KeyCode.LeftCommand)     // Linux
			|| Input.GetKeyDown(KeyCode.RightCommand)    // Linux
			|| Input.GetKeyDown(KeyCode.LeftWindows)     // Windows
			|| Input.GetKeyDown(KeyCode.RightWindows)) { // Windows
			PauseEnable();
		}
	}

	public void PauseToggle() {
		if (Paused())	PauseDisable();
		else			PauseEnable();
	}

	public void PauseEnable() {
		AudioListener.pause = true;
		PauseSystems();
		previousInvMode = MouseLookScript.a.inventoryMode;
		if (MouseLookScript.a.inventoryMode == false) {
			MouseLookScript.a.ToggleInventoryMode();
		}
		
		if (Eng_Global->inventoryPlayer1.vmailbetajet.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailbetajetVideo.Pause();
		if (Eng_Global->inventoryPlayer1.vmailbridgesep.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailbridgesepVideo.Pause();
		if (Eng_Global->inventoryPlayer1.vmailcitadestruct.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailcitadestructVideo.Pause();
		if (Eng_Global->inventoryPlayer1.vmailgenstatus.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailgenstatusVideo.Pause();
		if (Eng_Global->inventoryPlayer1.vmaillaserdest.activeInHierarchy) Eng_Global->inventoryPlayer1.vmaillaserdestVideo.Pause();
		if (Eng_Global->inventoryPlayer1.vmailshieldsup.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailshieldsupVideo.Pause();
		EnablePauseUI();
		pauseText.SetActive(true);
	}

	public void PauseDisable() {
		AudioListener.pause = false;
		UnpauseSystems();
		if (previousInvMode != MouseLookScript.a.inventoryMode) {
			MouseLookScript.a.ToggleInventoryMode();
			MouseLookScript.a.SetCameraCullDistances();
		}
		DisablePauseUI();
		if (Eng_Global->inventoryPlayer1.vmailbetajet.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailbetajetVideo.Play();
		if (Eng_Global->inventoryPlayer1.vmailbridgesep.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailbridgesepVideo.Play();
		if (Eng_Global->inventoryPlayer1.vmailcitadestruct.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailcitadestructVideo.Play();
		if (Eng_Global->inventoryPlayer1.vmailgenstatus.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailgenstatusVideo.Play();
		if (Eng_Global->inventoryPlayer1.vmaillaserdest.activeInHierarchy) Eng_Global->inventoryPlayer1.vmaillaserdestVideo.Play();
		if (Eng_Global->inventoryPlayer1.vmailshieldsup.activeInHierarchy) Eng_Global->inventoryPlayer1.vmailshieldsupVideo.Play();
		pauseText.SetActive(false);
	}

	public void PauseSystems() {
		paused = true;
		for (int i=0;i<disableUIOnPause.Length;i++) {
			disableUIOnPause[i].SetActive(false);
		}

		for (int k=0;k<Const.a.prb.Count;k++) Const.a.prb[k].Pause();
		for (int k=0;k<Const.a.psys.Count;k++) Const.a.psys[k].Pause();
		for (int k=0;k<Const.a.panimsList.Count;k++) {
			Const.a.panimsList[k].Pause();
		}

		PauseAmbients();
	}

	public void PauseAmbients() {
		for (int u=0;u<ambientRegistry.Count;u++) {
			if (ambientRegistry[u].SFX != null) ambientRegistry[u].SFX.Pause();
		}
	}

	public void UnpauseAmbients() {
		for (int u=0;u<ambientRegistry.Count;u++) {
			if (ambientRegistry[u].SFX != null) ambientRegistry[u].SFX.UnPause();
		}
	}

	public void UnpauseSystems() {
		paused = false;
		for (int i=0;i<disableUIOnPause.Length;i++) {
			disableUIOnPause[i].SetActive(true);
		}

		for (int k=0;k<Const.a.prb.Count;k++) {
			if (Const.a.prb[k] == null) continue;
			
			Const.a.prb[k].UnPause();
		}
		
		for (int k=0;k<Const.a.psys.Count;k++) {
			if (Const.a.psys[k] == null) continue;
			
			Const.a.psys[k].UnPause();
		}
		
		for (int k=0;k<Const.a.panimsList.Count;k++) {
			if (Const.a.panimsList[k] == null) continue;
			
			Const.a.panimsList[k].UnPause();
		}

		UnpauseAmbients();
		Eng_Global->instances[PLAYER1].ConsoleDisable();
	}

	public void OpenSaveDialog() {
		if (onSaveDialog) return;

		if (Eng_Global->instances[PLAYER1].inCyberSpace) {
			CenterStatusPrint("%s", Sys_Text.stringTable[602]); // Cannot save in cyberspace
			OpenSaveDialogHard();
			return;
		}

		DisablePauseUI();
		if (Eng_Global->justSavedTimeStamp < Time.time) {
			onSaveDialog = true;
			saveDialog.SetActive(true);
		}
	}

	public void OpenSaveDialogHard() {
		if (onSaveDialog) return;

		DisablePauseUI();
		if (Eng_Global->justSavedTimeStamp < Time.time) {
			onSaveDialog = true;
			hardSaveDialog.SetActive(true);
		}
	}

	public void ExitSaveDialog() {
		EnablePauseUI();
		saveDialog.SetActive(false);
		hardSaveDialog.SetActive(false);
		onSaveDialog = false;
	}

	public void SavePause() {
		if (Eng_Global->instances[PLAYER1].inCyberSpace) {
			CenterStatusPrint("%s", Sys_Text.stringTable[602]); // Cannot save in cyberspace
			return;
		}
		if (onSaveDialog) return;

		DisablePauseUI();
		saveDialog.SetActive(false); // turn off dialog
		mainMenu.SetActive(true);
		MainMenuHandler.a.GoToSaveGameSubmenu(true);
	}

	public void LoadPause() {
		if (onSaveDialog) return;

		DisablePauseUI();
		saveDialog.SetActive(false); // turn off dialog
		mainMenu.SetActive(true);
		MainMenuHandler.a.GoToLoadGameSubmenu(true);
	}

	public void SavePauseQuit() {
		DisablePauseUI();
		saveDialog.SetActive(false); // turn off dialog
		mainMenu.SetActive(true);
		MainMenuHandler.a.InitialDisplay.SetActive(false);
		GameObject newGameIndicator = GameObject.Find("NewGameIndicator");
		GameObject loadGameIndicator = GameObject.Find("LoadGameIndicator");
		GameObject freshGame = GameObject.Find("GameNotYetStarted");
		if (newGameIndicator != null) Utils.SafeDestroy(newGameIndicator);
		if (loadGameIndicator != null) Utils.SafeDestroy(loadGameIndicator);
		if (freshGame != null) Utils.SafeDestroy(freshGame);
		MainMenuHandler.a.GoToSaveGameSubmenu(true);
	}

	public void NoSavePauseQuit() {
		DisablePauseUI();
		saveDialog.SetActive(false); // turn off dialog
		mainMenu.SetActive(true);
		GameObject newGameIndicator = GameObject.Find("NewGameIndicator");
		GameObject loadGameIndicator = GameObject.Find("LoadGameIndicator");
		GameObject freshGame = GameObject.Find("GameNotYetStarted");
		if (newGameIndicator != null) Utils.SafeDestroy(newGameIndicator);
		if (loadGameIndicator != null) Utils.SafeDestroy(loadGameIndicator);
		if (freshGame != null) Utils.SafeDestroy(freshGame);
		MainMenuHandler.a.GoToFrontPage();
	}

	public void PauseQuitHard() {
		mainMenu.SetActive(true);
		MainMenuHandler.a.Quit();
	}

	public void EnablePauseUI() {
		for (int i=0;i<enableUIOnPause.Length;i++) {
			enableUIOnPause[i].SetActive(true);
			StartMenuButtonHighlight smbh = 
				enableUIOnPause[i].GetComponent<StartMenuButtonHighlight>();

			if (smbh != null) {
				smbh.DeHighlight(); // Prevent persisted states.
				if (i == 3 && Eng_Global->instances[PLAYER1].inCyberSpace) { // Save button
					smbh.enabled = false;
				} else {
					smbh.enabled = true;
				}
			}
		}
	}

	public void DisablePauseUI() {
		for (int i=0;i<enableUIOnPause.Length;i++) {
			enableUIOnPause[i].SetActive(false);
		}
	}

	public void PauseOptions () {
		if (onSaveDialog) return;

		DisablePauseUI();
		mainMenu.SetActive(true);
		MainMenuHandler.a.GoToOptionsSubmenu(true);
	}


	public void TakeScreenshot() {
		string sname = System.DateTime.UtcNow.ToString("ddMMMyyyy_HH_mm_ss")
					   + "_" + Const.a.versionString + ".png";
		string spath = Utils.SafePathCombine(Application.streamingAssetsPath,
											 "Screenshots");

		// Check and recreate Screenshots folder if it was deleted.
        if (!Directory.Exists(spath)) Directory.CreateDirectory(spath);
		spath = Utils.SafePathCombine(spath,sname);
		ScreenCapture.CaptureScreenshot(spath);
		StartCoroutine(ScreenshotSprint(sname));
	}

	// Let screenshot save without putting text in it.
	public IEnumerator ScreenshotSprint(string sname) {
		yield return new WaitForSeconds(0.1f);
		CenterStatusPrint("%s", Sys_Text.stringTable[1024] + sname); // "Wrote screenshot "

	}

	// No need to clear, these are all unsaved and static.
	public void AddAmbientToRegistry(AmbientRegistration ar) {
		ambientRegistry.Add(ar);
	}
}
