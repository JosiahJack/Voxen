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
		Sys_UI.sysAnalyzerLH.SetActive(false);
		Sys_UI.sysAnalyzerRH.SetActive(false);
		Sys_UI.mouseClickHeldOverGUI = true;
		
	}
    // Start is called before the first frame update
    void Update() {
		descSecurity.text = Text->stringTable[474];
		security.text = LevelManager.a.levelSecurity[LevelManager.a.currentLevel] + Text->stringTable[307];
		descLaser.text = Text->stringTable[475];
		laser.text = Const.a.questData.LaserDestroyed ? Text->stringTable[486] : Text->stringTable[485];
		descLifepod.text = Text->stringTable[476];
		lifepod.text = Const.a.questData.SelfDestructActivated ? Text->stringTable[488] : Text->stringTable[487];
		descShield.text = Text->stringTable[477];
		shield.text = Const.a.questData.ShieldActivated ? Text->stringTable[490] : Text->stringTable[489];
		descReactor.text = Text->stringTable[478];
		reactor.text = Const.a.questData.SelfDestructActivated ? Text->stringTable[491] : Text->stringTable[492];
		descProcessor.text = Text->stringTable[479];
		int nodeCount = 0;
		for (int i=0;i<14;i++) {
			nodeCount += LevelManager.a.levelSmallNodeCount[i];
			nodeCount += LevelManager.a.levelLargeNodeCount[i];
			nodeCount -= LevelManager.a.levelSmallNodeDestroyedCount[i];
			nodeCount -= LevelManager.a.levelLargeNodeDestroyedCount[i];
		}
		processor.text = nodeCount.ToString();
		descProgram.text = Text->stringTable[480];
		if (!Const.a.questData.LaserDestroyed) {
			program.text = Text->stringTable[494];
		} else {
			if (!Const.a.questData.BetaGroveJettisoned) {
				program.text = Text->stringTable[495];
			} else {
				if (!(Const.a.questData.AntennaNorthDestroyed && Const.a.questData.AntennaSouthDestroyed && Const.a.questData.AntennaWestDestroyed && Const.a.questData.AntennaEastDestroyed)) {
					program.text = Text->stringTable[496];
				} else {
					if (!Const.a.questData.BridgeSeparated) {
						program.text = Text->stringTable[497];
					} else {
						program.text = Text->stringTable[498];
					}
				}
			}
		}
		descAlpha.text = Text->stringTable[481];
		alpha.text = Text->stringTable[492];
		descBeta.text = Text->stringTable[482];
		beta.text = Const.a.questData.BetaGroveJettisoned ? Text->stringTable[493] : Text->stringTable[492];
		descGamma.text = Text->stringTable[483];
		gamma.text = Text->stringTable[493];
		descDelta.text = Text->stringTable[484];
		delta.text = Text->stringTable[492];
	}
}
