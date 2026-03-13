public class ButtonSwitch : MonoBehaviour {
	void ToggleLocked() {
		string was = locked.ToString();
		locked = !locked;
	}

	void ToggleMaterial() {
		if (mRenderer == null) mRenderer = GetComponent<MeshRenderer>();
		if (alternateOn) Eng_Global->instances[i].texture = alternateSwitchMaterial;
		else             Eng_Global->instances[i].texture = mainSwitchMaterial;
	}

	void SetMaterialToAlternate() {
		if (!blinkWhenActive) return;

		Eng_Global->instances[i].texture = alternateSwitchMaterial;
	}

	void SetMaterialToNormal() {
		if (!blinkWhenActive) return;
        
		Eng_Global->instances[i].texture = mainSwitchMaterial;
	}
}
