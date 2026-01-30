using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

// Should only exist on the Item Tab.  When clicked, deletes one useless item
// from the general inventory, namely the currently highlighted one.
public class VaporizeButton : MonoBehaviour {
	public Image ico;
	public Text ict;
	private EventTrigger evenT;
	private bool pointerEntered;

	void Awake() {
		pointerEntered = false;
		evenT = GetComponent<EventTrigger>();
		if (evenT == null) evenT = gameObject.AddComponent<EventTrigger>();
		if (evenT != null) {
			// Create a new entry for the PointerEnter event
            EventTrigger.Entry pointerEnter = new EventTrigger.Entry();
            pointerEnter.eventID = EventTriggerType.PointerEnter;
            pointerEnter.callback.AddListener((data) => {
				OnPointerEnterDelegate((PointerEventData)data);
			});

            evenT.triggers.Add(pointerEnter);

            // Create a new entry for the PointerExit event
            EventTrigger.Entry pointerExit = new EventTrigger.Entry();
            pointerExit.eventID = EventTriggerType.PointerExit;
            pointerExit.callback.AddListener((data) => {
				OnPointerExitDelegate((PointerEventData)data);
			});

            evenT.triggers.Add(pointerExit);
		} else DualLog("Failed to add EventTrigger to " + gameObject.name);
	}

	void OnEnable() {
		pointerEntered = false;
	}

	// Handle OnPointerEnter event, replaces OnMouseEnter
    public void OnPointerEnterDelegate(PointerEventData data) { PtrEnter(); }

	// Handle OnPointerExit event, replaces OnMouseExit
    public void OnPointerExitDelegate(PointerEventData data) { PtrExit(); }

	public void PtrEnter () {
		if (pointerEntered) return;

		GUIState.a.PtrHandler(true,true,ButtonType.Generic,gameObject);
		MouseLookScript.a.currentButton = gameObject;
		pointerEntered = true;
	}

	public void PtrExit () {
		if (!pointerEntered) return;

		GUIState.a.ClearOverButton();
		pointerEntered = false;
	}

	public void OnVaporizeClick() {
		MFDManager.a.mouseClickHeldOverGUI = true;
		if (Inventory.a == null) return;
		if (inventoryPlayer1.generalInvCurrent == 0) return; // Access Cards index.

		int cur = inventoryPlayer1.generalInvCurrent;
		inventoryPlayer1.generalInventoryIndexRef[cur] = -1; // Remove item
		inventoryPlayer1.generalInvCurrent -= 1;
		if (inventoryPlayer1.generalInvCurrent < 0) {
			inventoryPlayer1.generalInvCurrent = 0; // Bound to lowest, but only
		}									   // since it is Access Cards.


		cur = inventoryPlayer1.generalInvCurrent;
		if (inventoryPlayer1.generalInventoryIndexRef[cur] < 0) {
			for (int i=13; i >= 0; i--) {
				if (inventoryPlayer1.generalInventoryIndexRef[i] >= 0) {
					inventoryPlayer1.generalInvCurrent = i;
					break; // Found last item in inventory.
				}
			}
		}

		cur = inventoryPlayer1.generalInvCurrent;
		int indexRef = inventoryPlayer1.generalInventoryIndexRef[cur];
		if (inventoryPlayer1.generalInvCurrent == 0) {
			if (inventoryPlayer1.HasAnyAccessCards()) {
				MFDManager.a.SendInfoToItemTab(indexRef);
			} else {
				// If no access cards, reset item tab to show nothing.
				MFDManager.a.SendInfoToItemTab(-1);
				PtrExit();
			}
		} else {
			GeneralInvButton genbut = inventoryPlayer1.genButtons[cur].GetComponent<GeneralInvButton>();
			MFDManager.a.SendInfoToItemTab(indexRef,genbut.customIndex);
		}
	}
}
