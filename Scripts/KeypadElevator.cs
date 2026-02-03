using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Text;

public class KeypadElevator : MonoBehaviour {
	public Door linkedDoor;
	public GameObject[] targetDestination; // Set by ElevatorKeypad.cs in Use()
										   // which actually gets it from
										   // ElevatorButton.cs.

	public int securityThreshhold = 100; // If security level is not below this
										 // level, this is unusable.
	public bool[] buttonsEnabled;
	public bool[] buttonsDarkened;
	public string[] buttonText;
	public int currentFloor;
	public bool padInUse; // save
	public bool locked = false; // save
	public string lockedTarget;
	public int lockedMessageIndex = -1;
	
	private static StringBuilder s1 = new StringBuilder();

	void Start () {
		padInUse = false;
		if (linkedDoor == null) {
			DualLog("BUG: no linked Door for KeypadElevator at location: "
					  + instances[i].position.ToString());
		}
	}

	public void Use (UseData ud) {
		if (GetCurrentLevelSecurity() > securityThreshhold) {
			Sys_UI.BlockedBySecurity(instances[i].position);
			return;
		}

		if (LevelManager.a.superoverride || Sys_Global.difficultyMission == 0) {
			// SHODAN can go anywhere!  Full security override!
			locked = false;
		}

		if (locked) {
			// Target something because we are locked like an info_message to say
			// hey we are locked, e.g. vox: "Non emergency life pods disabled."
			CenterStatusPrint(lockedMessageIndex);
			UseTargets(gameObject,ud,lockedTarget);
			return;
		}

		padInUse = true;
		Utils.PlayUIOneShotSavable(91);
		Sys_UI.SendElevatorKeypadToDataTab(this,buttonsEnabled,
												 buttonsDarkened,buttonText,
												 targetDestination,
												 instances[i].position,linkedDoor,
												 currentFloor);
	}

	public void SendDataBackToPanel() {
		padInUse = false;
	}

	public static string Save(GameObject go) {
		KeypadElevator ke = go.GetComponent<KeypadElevator>();
		if (ke == null) {
			DualLog("KeypadElevator missing on savetype of KeypadElevator! "
					  + " GameObject.name: " + go.name);

			return "0|0";
		}

		s1.Clear();
		s1.Append(Utils.BoolToString(ke.padInUse,"KeypadElevator.padInUse"));
		s1.Append(Utils.splitChar);
		s1.Append(Utils.BoolToString(ke.locked,"locked"));
		return s1.ToString();
	}

	public static int Load(GameObject go, ref string[] entries, int index) {
		KeypadElevator ke = go.GetComponent<KeypadElevator>();
		if (ke == null) {
			DualLog("KeypadElevator.Load failure, ke == null");
			return index + 2;
		}

		if (index < 0) {
			DualLog("KeypadElevator.Load failure, index < 0");
			return index + 2;
		}

		if (entries == null) {
			DualLog("KeypadElevator.Load failure, entries == null");
			return index + 2;
		}

		ke.padInUse = Utils.GetBoolFromString(entries[index],
											  "KeypadElevator.padInUse");
		index++;

		ke.locked = Utils.GetBoolFromString(entries[index],"locked"); index++;
		return index;
	}
}
