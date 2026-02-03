using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class GrenadeTimerSlider : MonoBehaviour {
    Slider slideS;
	public Slider actualSlider;
	public Text valueText;

	void Awake () {
        slideS = GetComponent<Slider>();
	}

	void Update () {
		if (inventoryPlayer1.grenadeCurrent != -1) {
			if (inventoryPlayer1.grenadeCurrent == 5) {
				valueText.text = inventoryPlayer1.nitroTimeSetting.ToString("0.0");
			} else if (inventoryPlayer1.grenadeCurrent == 6) {
				valueText.text = inventoryPlayer1.earthShakerTimeSetting.ToString("0.0");
			}
		}

	}

    public void SetValue() {
		if (inventoryPlayer1.grenadeCurrent != 5 && inventoryPlayer1.grenadeCurrent != 6) return;

		Sys_UI.mouseClickHeldOverGUI = true;
		float val = actualSlider.value;
		if (val >= 60f) val = 60f;
		if (inventoryPlayer1.grenadeCurrent == 5) {
			if (val < 2f) val = 2f;
			inventoryPlayer1.nitroTimeSetting = val;
		} else if (inventoryPlayer1.grenadeCurrent == 6) {
			if (val < 4f) val = 4f;
			inventoryPlayer1.earthShakerTimeSetting = val;
		}

		slideS.value = val;
		valueText.text = slideS.value.ToString("0.0");
		Slider slidLH = Sys_UI.itemTabLH.grenadeTimerSliderSlider.GetComponent<Slider>();
		Slider slidRH = Sys_UI.itemTabRH.grenadeTimerSliderSlider.GetComponent<Slider>();
		if (inventoryPlayer1.grenadeCurrent == 5) {
			if (slidLH != actualSlider) slidLH.value = inventoryPlayer1.nitroTimeSetting;
			if (slidRH != actualSlider) slidRH.value = inventoryPlayer1.nitroTimeSetting;
		} else if (inventoryPlayer1.grenadeCurrent == 6) {
			if (slidLH != actualSlider) slidLH.value = inventoryPlayer1.earthShakerTimeSetting;
			if (slidRH != actualSlider) slidRH.value = inventoryPlayer1.earthShakerTimeSetting;
		}
    }
}
