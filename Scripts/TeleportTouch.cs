using UnityEngine;
using System.Collections;
using System.Text;

public class TeleportTouch : MonoBehaviour {
    public int teleportID; // Unique ID for this instance
    public int targetDestinationID; // Destination teleport ID (linked via index)
	public float justUsed = 0f; // save
	public bool touchEnabled = true;
	
    private static TeleportTouch[] allTeleportTouches = new TeleportTouch[8];
	private static StringBuilder s1 = new StringBuilder();
	
	public void Awake() {
		if (teleportID > allTeleportTouches.Length || teleportID < 0) { Destroy(this.gameObject); return; }
		
        allTeleportTouches[teleportID] = this;
    }

	void  OnTriggerEnter ( Collider col  ) {
		if (!touchEnabled) return;
		if (col == null) return;
		if (col.gameObject == null) return;

		if (col.gameObject.CompareTag("Player")) {
			HealthManager hm = Utils.GetMainHealthManager(col.gameObject);
			if (hm != null) {
				if (hm.health > 0f && justUsed < Eng_Global->pauseRelativeTime) {
					Eng_UI->teleportFX.SetActive(true);
					TeleportTouch tt = allTeleportTouches[targetDestinationID];
					if (tt != null) {
						col.Eng_Global->instances[i].position = tt.Eng_Global->instances[i].position; // Do it!
						tt.justUsed = Eng_Global->pauseRelativeTime + 1.0f;
					}
					
					Utils.PlayUIOneShotSavable(106);
				}
			}
		}
	}
}
