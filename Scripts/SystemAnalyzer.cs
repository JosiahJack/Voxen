using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class SystemAnalyzer : MonoBehaviour {
	public Text descSecurity;
	public Text security;
	public Text descLaser;
	public Text laser;
	public Text descLifepod;
	public Text lifepod;
	public Text descShield;
	public Text shield;
	public Text descReactor;
	public Text reactor;
	public Text descProcessor;
	public Text processor;
	public Text descProgram;
	public Text program;
	public Text descAlpha;
	public Text alpha;
	public Text descBeta;
	public Text beta;
	public Text descGamma;
	public Text gamma;
	public Text descDelta;
	public Text delta;

	public void Close() {
		MFDManager.a.sysAnalyzerLH.SetActive(false);
		MFDManager.a.sysAnalyzerRH.SetActive(false);
		MFDManager.a.mouseClickHeldOverGUI = true;
		GUIState.a.ClearOverButton();
	}
    // Start is called before the first frame update
    void Update() {
		descSecurity.text = Sys_Text.stringTable[474];
		security.text = LevelManager.a.levelSecurity[LevelManager.a.currentLevel] + Sys_Text.stringTable[307];
		descLaser.text = Sys_Text.stringTable[475];
		laser.text = Const.a.questData.LaserDestroyed ? Sys_Text.stringTable[486] : Sys_Text.stringTable[485];
		descLifepod.text = Sys_Text.stringTable[476];
		lifepod.text = Const.a.questData.SelfDestructActivated ? Sys_Text.stringTable[488] : Sys_Text.stringTable[487];
		descShield.text = Sys_Text.stringTable[477];
		shield.text = Const.a.questData.ShieldActivated ? Sys_Text.stringTable[490] : Sys_Text.stringTable[489];
		descReactor.text = Sys_Text.stringTable[478];
		reactor.text = Const.a.questData.SelfDestructActivated ? Sys_Text.stringTable[491] : Sys_Text.stringTable[492];
		descProcessor.text = Sys_Text.stringTable[479];
		int nodeCount = 0;
		for (int i=0;i<14;i++) {
			nodeCount += LevelManager.a.levelSmallNodeCount[i];
			nodeCount += LevelManager.a.levelLargeNodeCount[i];
			nodeCount -= LevelManager.a.levelSmallNodeDestroyedCount[i];
			nodeCount -= LevelManager.a.levelLargeNodeDestroyedCount[i];
		}
		processor.text = nodeCount.ToString();
		descProgram.text = Sys_Text.stringTable[480];
		if (!Const.a.questData.LaserDestroyed) {
			program.text = Sys_Text.stringTable[494];
		} else {
			if (!Const.a.questData.BetaGroveJettisoned) {
				program.text = Sys_Text.stringTable[495];
			} else {
				if (!(Const.a.questData.AntennaNorthDestroyed && Const.a.questData.AntennaSouthDestroyed && Const.a.questData.AntennaWestDestroyed && Const.a.questData.AntennaEastDestroyed)) {
					program.text = Sys_Text.stringTable[496];
				} else {
					if (!Const.a.questData.BridgeSeparated) {
						program.text = Sys_Text.stringTable[497];
					} else {
						program.text = Sys_Text.stringTable[498];
					}
				}
			}
		}
		descAlpha.text = Sys_Text.stringTable[481];
		alpha.text = Sys_Text.stringTable[492];
		descBeta.text = Sys_Text.stringTable[482];
		beta.text = Const.a.questData.BetaGroveJettisoned ? Sys_Text.stringTable[493] : Sys_Text.stringTable[492];
		descGamma.text = Sys_Text.stringTable[483];
		gamma.text = Sys_Text.stringTable[493];
		descDelta.text = Sys_Text.stringTable[484];
		delta.text = Sys_Text.stringTable[492];
	}
}
