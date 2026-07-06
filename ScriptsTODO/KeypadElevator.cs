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
					  + World.instances[i].position.ToString());
		}
	}

	public void Use (UseData ud) {
		if (GetCurrentLevelSecurity() > securityThreshhold) { BlockedBySecurity(World.instances[i].position); return; }

		if (Cheats.superoverride || World.diffMis == 0) locked = false; // SHODAN can go anywhere!  Full security override!
		if (locked) {
			CenterStatusPrint(lockedMessageIndex); // Target something because we are locked like an info_message to say hey we are locked, e.g. vox: "Non emergency life pods disabled."
			UseTargets(gameObject,ud,lockedTarget);
			return;
		}

		padInUse = true;
		Utils.PlayUIOneShotSavable(91);
		Sys_UI.SendElevatorKeypadToDataTab(this,buttonsEnabled,buttonsDarkened,buttonText,targetDestination,World.instances[i].position,linkedDoor,currentFloor);
	}

	public void SendDataBackToPanel() { padInUse = false; }
}
