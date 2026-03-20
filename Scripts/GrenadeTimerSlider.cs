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
		if (Eng_Global->inventoryPlayer1.grenadeCurrent != -1) {
			if (Eng_Global->inventoryPlayer1.grenadeCurrent == 5) {
				valueText.text = Eng_Global->inventoryPlayer1.nitroTimeSetting.ToString("0.0");
			} else if (Eng_Global->inventoryPlayer1.grenadeCurrent == 6) {
				valueText.text = Eng_Global->inventoryPlayer1.earthShakerTimeSetting.ToString("0.0");
			}
		}

	}

    public void SetValue() {
		if (Eng_Global->inventoryPlayer1.grenadeCurrent != 5 && Eng_Global->inventoryPlayer1.grenadeCurrent != 6) return;

		Eng_UI->mouseClickHeldOverGUI = true;
		float val = actualSlider.value;
		if (val >= 60f) val = 60f;
		if (Eng_Global->inventoryPlayer1.grenadeCurrent == 5) {
			if (val < 2f) val = 2f;
			Eng_Global->inventoryPlayer1.nitroTimeSetting = val;
		} else if (Eng_Global->inventoryPlayer1.grenadeCurrent == 6) {
			if (val < 4f) val = 4f;
			Eng_Global->inventoryPlayer1.earthShakerTimeSetting = val;
		}

		slideS.value = val;
		valueText.text = slideS.value.ToString("0.0");
		Slider slidLH = Eng_UI->itemTabLH.grenadeTimerSliderSlider.GetComponent<Slider>();
		Slider slidRH = Eng_UI->itemTabRH.grenadeTimerSliderSlider.GetComponent<Slider>();
		if (Eng_Global->inventoryPlayer1.grenadeCurrent == 5) {
			if (slidLH != actualSlider) slidLH.value = Eng_Global->inventoryPlayer1.nitroTimeSetting;
			if (slidRH != actualSlider) slidRH.value = Eng_Global->inventoryPlayer1.nitroTimeSetting;
		} else if (Eng_Global->inventoryPlayer1.grenadeCurrent == 6) {
			if (slidLH != actualSlider) slidLH.value = Eng_Global->inventoryPlayer1.earthShakerTimeSetting;
			if (slidRH != actualSlider) slidRH.value = Eng_Global->inventoryPlayer1.earthShakerTimeSetting;
		}
    }
}
