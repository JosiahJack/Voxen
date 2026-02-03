public class ButtonSwitch : MonoBehaviour {
	void ToggleLocked() {
		string was = locked.ToString();
		locked = !locked;
	}

	void ToggleMaterial() {
		if (mRenderer == null) mRenderer = GetComponent<MeshRenderer>();
		if (alternateOn) instances[i].texture = alternateSwitchMaterial;
		else             instances[i].texture = mainSwitchMaterial;
	}

	void SetMaterialToAlternate() {
		if (!blinkWhenActive) return;

		instances[i].texture = alternateSwitchMaterial;
	}

	void SetMaterialToNormal() {
		if (!blinkWhenActive) return;
        
		instances[i].texture = mainSwitchMaterial;
	}
}
