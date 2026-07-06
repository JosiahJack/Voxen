using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class PuzzleWirePuzzle : MonoBehaviour {
	public int securityThreshhold = 100; // if security level is not below this level, this is unusable
	public bool dead = false;
	public bool[] wiresOn;
	public bool[] rowsActive;
	public int[] currentPositionsLeft; // save
	public int[] currentPositionsRight; // save
	public int[] solutionPositionsLeft;
	public int[] solutionPositionsRight;
	public HUDColor theme;
	public HUDColor[] wireColors;
	public string target;
	public bool locked = false; // save
	public int successMessageLingdex = 4;
	public string successMessage = "";
	public int messageOnLockedLingdex = 302;
	public int messageOnBrokenLingdex = 189;
	public bool puzzleSolved; // save
	private static StringBuilder s1 = new StringBuilder();
	private Animator anim;
	
	public bool animate = true;
	public bool inUse = false;

	void Awake() {
		puzzleSolved = false;
		if (animate) {
			anim = GetComponent<Animator>();
			if (anim == null) DualLog("BUG: Puzzle panel has no animator on PuzzleGridPuzzle.cs");
		}
	}

	public void SendDataBackToPanel(PuzzleWire pw, bool stillInUse) {
		currentPositionsLeft[0] = pw.wire1LHPosition;
		currentPositionsLeft[1] = pw.wire2LHPosition;
		currentPositionsLeft[2] = pw.wire3LHPosition;
		currentPositionsLeft[3] = pw.wire4LHPosition;
		currentPositionsLeft[4] = pw.wire5LHPosition;
		currentPositionsLeft[5] = pw.wire6LHPosition;
		currentPositionsLeft[6] = pw.wire7LHPosition;
		currentPositionsRight[0] = pw.wire1RHPosition;
		currentPositionsRight[1] = pw.wire2RHPosition;
		currentPositionsRight[2] = pw.wire3RHPosition;
		currentPositionsRight[3] = pw.wire4RHPosition;
		currentPositionsRight[4] = pw.wire5RHPosition;
		currentPositionsRight[5] = pw.wire6RHPosition;
		currentPositionsRight[6] = pw.wire7RHPosition;
		inUse = stillInUse;
	}

	public void Use (UseData ud) {
		if (dead) {
			CenterStatusPrint(messageOnBrokenLingdex);
			return;
		}

		if (GetCurrentLevelSecurity() > securityThreshhold) {
			Sys_UI.BlockedBySecurity(World.instances[i].position);
			return;
		}

		if (Cheats.superoverride || World.diffMis == 0) {
			// SHODAN can go anywhere!  Full security override!
			locked = false;
		}

		if (locked) {
			CenterStatusPrint(messageOnLockedLingdex);
			return;
		}

		TargetIO tio = GetComponent<TargetIO>();
		if (tio != null) {
			ud.SetBits(tio);
		} else {
			DualLog("BUG: no TargetIO.cs found on an object with a "
					  + "PuzzleGridPuzzle.cs script!  Trying to call Use "
					  + "without parameters!");
		}

		CenterStatusPrint(190); //Puzzle accessed
		inUse = true;
		Sys_UI.SendWirePuzzleToDataTab(wiresOn,rowsActive,
											 currentPositionsLeft,
											 currentPositionsRight,
											 solutionPositionsLeft,
											 solutionPositionsRight,theme,
											 wireColors,target,ud,
											 World.instances[i].position,this);
	}

	public void UseTargets (GameObject owner) {
		UseData ud = new UseData();
		ud.owner = owner;
		UseTargets(gameObject,ud,target);
		CenterStatusPrint(successMessageLingdex);
	}
}
