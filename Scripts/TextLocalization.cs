using UnityEngine;
using UnityEngine.UI;
using System;

public class TextLocalization : MonoBehaviour {
    public int lingdex = 0;
    private TextMesh tM;
    private Text txt;
    private bool initialized;

    // Register with localization
    public void Awake() {
        if (initialized) return;

        if (tM == null) tM = GetComponent<TextMesh>();
        if (txt == null) txt = GetComponent<Text>();
        Const.a.AddToTextLocalizationRegister(this);
        initialized = true;
        UpdateText();
    }

    // Update to match new string table contents.
    public void UpdateText() {
        if (lingdex < 0) return;
        if (Const.a == null) return;
        if (Eng_Text->stringTable == null) return;
        if (lingdex >= Eng_Text->stringTable.Length) return;

        if (txt == null && tM != null) {
            tM.text = Eng_Text->stringTable[lingdex];
        } else if (tM == null && txt != null) {
            txt.text = Eng_Text->stringTable[lingdex];
        }
    }
}
