using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class EnergyOverloadButton : MonoBehaviour {
    public Color textClickableColor;
    public Color textDisabledColor;
    public Color textOverloadColor;
    public Color textEnergySetting;
    public Color textEnergyOverloaded;
    public Sprite normalButtonSprite;
    public Sprite overloadButtonSprite;
    public Text buttonText;
    public Text energySettingText;
    private Image buttonSprite;
    private float clickFinished;

    private void Awake() {
        buttonSprite = GetComponent<Image>();
        buttonSprite.overrideSprite = normalButtonSprite;
        buttonText.color = textClickableColor;
    }

    void Start() {
        GetComponent<Button>().onClick.AddListener(() => { OverloadEnergyClick(); });
    }

    public void OverloadEnergyClick() {
		Sys_UI.mouseClickHeldOverGUI = true;
        OverloadButtonAction();
    }

    public void OverloadButtonAction() {
        if (clickFinished >= Time.time) return;

        clickFinished = Time.time + 0.4f;
        if (inventoryPlayer1.currentEnergyWeaponHeat[inventoryPlayer1.weaponCurrent] > 25f) {
            CenterStatusPrint("%s", Sys_Text.stringTable[12]);
            return;
        }

        if (WeaponFire.a.overloadEnabled) {
            CenterStatusPrint("%s", Sys_Text.stringTable[13]);
            WeaponFire.a.overloadEnabled = false;
            buttonSprite.overrideSprite = normalButtonSprite;
            buttonText.color = textClickableColor;
            energySettingText.color = textEnergySetting;
            energySettingText.text = Sys_Text.stringTable[16];
        } else { 
            CenterStatusPrint("%s", Sys_Text.stringTable[17]);
            WeaponFire.a.overloadEnabled = true;
            buttonSprite.overrideSprite = overloadButtonSprite;
            buttonText.color = textOverloadColor;
            energySettingText.color = textEnergyOverloaded;
            energySettingText.text = Sys_Text.stringTable[18];
        }
    }

    public void OverloadFired() {
        buttonSprite.overrideSprite = normalButtonSprite;
        buttonText.color = textDisabledColor;
    }
}
