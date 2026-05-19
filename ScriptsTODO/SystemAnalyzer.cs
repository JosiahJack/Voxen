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
		Eng_UI->sysAnalyzerLH.SetActive(false);
		Eng_UI->sysAnalyzerRH.SetActive(false);
		Eng_UI->mouseClickHeldOverGUI = true;
		
	}
    // Start is called before the first frame update
    void Update() {
		descSecurity.text = Eng_Text->stringTable[474];
		security.text = LevelManager.a.levelSecurity[LevelManager.a.currentLevel] + Eng_Text->stringTable[307];
		descLaser.text = Eng_Text->stringTable[475];
		laser.text = Const.a.questData.LaserDestroyed ? Eng_Text->stringTable[486] : Eng_Text->stringTable[485];
		descLifepod.text = Eng_Text->stringTable[476];
		lifepod.text = Const.a.questData.SelfDestructActivated ? Eng_Text->stringTable[488] : Eng_Text->stringTable[487];
		descShield.text = Eng_Text->stringTable[477];
		shield.text = Const.a.questData.ShieldActivated ? Eng_Text->stringTable[490] : Eng_Text->stringTable[489];
		descReactor.text = Eng_Text->stringTable[478];
		reactor.text = Const.a.questData.SelfDestructActivated ? Eng_Text->stringTable[491] : Eng_Text->stringTable[492];
		descProcessor.text = Eng_Text->stringTable[479];
		int nodeCount = 0;
		for (int i=0;i<14;i++) {
			nodeCount += LevelManager.a.levelSmallNodeCount[i];
			nodeCount += LevelManager.a.levelLargeNodeCount[i];
			nodeCount -= LevelManager.a.levelSmallNodeDestroyedCount[i];
			nodeCount -= LevelManager.a.levelLargeNodeDestroyedCount[i];
		}
		processor.text = nodeCount.ToString();
		descProgram.text = Eng_Text->stringTable[480];
		if (!Const.a.questData.LaserDestroyed) {
			program.text = Eng_Text->stringTable[494];
		} else {
			if (!Const.a.questData.BetaGroveJettisoned) {
				program.text = Eng_Text->stringTable[495];
			} else {
				if (!(Const.a.questData.AntennaNorthDestroyed && Const.a.questData.AntennaSouthDestroyed && Const.a.questData.AntennaWestDestroyed && Const.a.questData.AntennaEastDestroyed)) {
					program.text = Eng_Text->stringTable[496];
				} else {
					if (!Const.a.questData.BridgeSeparated) {
						program.text = Eng_Text->stringTable[497];
					} else {
						program.text = Eng_Text->stringTable[498];
					}
				}
			}
		}
		descAlpha.text = Eng_Text->stringTable[481];
		alpha.text = Eng_Text->stringTable[492];
		descBeta.text = Eng_Text->stringTable[482];
		beta.text = Const.a.questData.BetaGroveJettisoned ? Eng_Text->stringTable[493] : Eng_Text->stringTable[492];
		descGamma.text = Eng_Text->stringTable[483];
		gamma.text = Eng_Text->stringTable[493];
		descDelta.text = Eng_Text->stringTable[484];
		delta.text = Eng_Text->stringTable[492];
	}
}
