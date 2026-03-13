using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class EnergySlider : MonoBehaviour {
    Slider slideS;

	void Awake() {
        slideS = GetComponent<Slider>();
	}
/*
	void Update () {
		if (inventoryPlayer1.weaponCurrent != -1) {
			slideS.value =
			  inventoryPlayer1.weaponEnergySetting[inventoryPlayer1.weaponCurrent];
		} else {
			slideS.value = 0;
		}
	}*/


    public void SetValue(float val) {
		if (inventoryPlayer1.weaponCurrent < 0
			|| inventoryPlayer1.weaponCurrent > 6) {
			return;
		}

		Sys_UI.mouseClickHeldOverGUI = true;
		if (val < 1.0f) val = val * 100f;
		if (val < 0) val = 0f;
		if (val >= 98f) val = 100f;
		slideS.value = val;
		DualLog("Set energy slider value to " + slideS.value.ToString()
				  + ", from " + val.ToString());
        inventoryPlayer1.weaponEnergySetting[inventoryPlayer1.weaponCurrent] =
			slideS.value;
    }
}
