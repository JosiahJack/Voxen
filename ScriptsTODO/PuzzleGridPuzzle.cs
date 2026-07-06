using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class PuzzleGridPuzzle : MonoBehaviour {
	public int securityThreshhold = 100; // If security level is not below this
										 // level, this is unusable.
	public bool dead = false;
	public bool[] grid; // save
	public PuzzleCellType[] cellType;
	public PuzzleGridType gridType;
	public int sourceIndex;
	public int outputIndex;
	public int width;
	public int height;
	public HUDColor theme;
	public string target;
	public bool locked = false; // save
	public int successMessageLingdex = 4;
	public int messageOnLockedLingdex = 302;
	public int messageOnBrokenLingdex = 189;
	public int alreadyFiredMessageLingdex = 312;
	public bool puzzleSolved; // save
	public bool onlyFireOnce = true;
	public bool animate = true;
	public bool inUse = false;

	bool fired = false; // save
	private Animator anim;
	private bool alreadyOpen = false;
	private static StringBuilder s1 = new StringBuilder();

	void Awake() {
		puzzleSolved = false;
		if (animate) {
			anim = GetComponent<Animator>();
			if (anim == null) {
				DualLog("BUG: Puzzle panel has no animator on "
						  + "PuzzleGridPuzzle.cs");
			}

			alreadyOpen = false;
		}
	}

	public void SendDataBackToPanel(PuzzleGrid pg) {
		grid = pg.grid;
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

		CenterStatusPrint("%s", Text->stringTable[190],ud.owner); // Puzzle accessed
		inUse = true;
		if (animate && anim != null && !alreadyOpen) {
			anim.Play("Open");
			alreadyOpen = true;
		}

		Sys_UI.SendGridPuzzleToDataTab(grid,cellType,gridType,
											 sourceIndex,outputIndex,width,
											 height,theme,target,ud,
											 World.instances[i].position,this);
	}

	public void UseTargets (GameObject owner) {
		if (onlyFireOnce && fired) {
			CenterStatusPrint(alreadyFiredMessageLingdex);
			return;
		}

		if (onlyFireOnce) fired = true;
		UseData ud = new UseData();
		ud.owner = owner;
		UseTargets(gameObject,ud,target);
		CenterStatusPrint(successMessageLingdex);
	}
}
