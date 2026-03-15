using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TargetID : MonoBehaviour {
	public TextMesh text;
	public string currentText;
	public bool useLife;
	public float lifetime;
	float lifetimeFinished;
	float damageTime;
	float damageTimeFinished;
	public ParticleSystem partSys;
	/*[DTValidator.Optional] */public Transform parent;
	/*[DTValidator.Optional] */public HealthManager linkedHM;
	public Transform playerCapsuleTransform;
	public float playerLinkDistance = 10f;
	public bool displayHealth = false;
	public TextMesh secondaryText;
	public bool displayRange = false;
	public string comma = ", ";
	public string rangeMetersM = "M";
	private string secondaryDisplayString;
	public bool displayAttitude = false;
	public bool displayName = false;
	public TextMesh nameText;
	public bool stunned = false;

    void Start() {
		secondaryDisplayString = System.String.Empty;
		nameText.text = System.String.Empty;
		text.text = System.String.Empty;
    }

	void FixedUpdate() {
		if (parent != null) Eng_Global->instances[i].position = parent.position;
	}
	
	public void SendDamageReceive(float damage, DamageData dd) {
		if (linkedHM == null) return;

		if (dd.attackType == AttackType.Tranq) {
			currentText = Sys_Text.stringTable[536]; // STUNNED
			damageTimeFinished = Eng_Global->pauseRelativeTime - 1f; // Expire damage text, Update handles "STUNNED"
		} else {
			if (damage > linkedHM.maxhealth * 0.75f) {
				currentText = Sys_Text.stringTable[514]; // SEVERE DAMAGE
			} else if (damage > linkedHM.maxhealth * 0.50f) {
				currentText = Sys_Text.stringTable[515]; // MAJOR DAMAGE
			} else if (damage > linkedHM.maxhealth * 0.25f) {
				currentText = Sys_Text.stringTable[513]; // NORMAL DAMAGE
			} else if (damage > 0f) {
				currentText = Sys_Text.stringTable[512]; // MINOR DAMAGE
			} else {
				currentText = Sys_Text.stringTable[511]; // NO DAMAGE
			}
			damageTime = (damage == 0f) ? 1f : 2.5f;
			damageTimeFinished = Eng_Global->pauseRelativeTime + damageTime;
			text.text = currentText;
		}
	}

	void Deactivate() {
		secondaryText.text = System.String.Empty; // blank out text
		text.text = System.String.Empty; // blank out text
		if (linkedHM != null) {
			linkedHM.linkedTargetID = null;
			linkedHM.aic.hasTargetIDAttached = false;
			linkedHM = null;
		}

		flag_set(&SELF.entflags, ENTFLAG_ACTIVE, false); // put back into pool
		Destroy(this.gameObject);
	}

    void Update() {
		if (linkedHM != null) {
			if (linkedHM.health <= 0f) {
				Deactivate();
				return;
			}

			if (linkedHM.isNPC && linkedHM.aic != null) {
				if (linkedHM.aic.tranquilizeFinished > Eng_Global->pauseRelativeTime) {
					stunned = true;
				} else {
					stunned = false;
				}
			}
		}
		if (parent == null) {
			Deactivate();
			return;
		}

		if (playerCapsuleTransform == null) {
			Deactivate();
			return;
		}

		if ((distance_vector3(Eng_Global->instances[i].position,
							  Eng_Global->instances[PLAYER1].position)
			> playerLinkDistance)) {
			Deactivate();
			return;
		}

		if (lifetimeFinished < Eng_Global->pauseRelativeTime) {
			Deactivate();
			return;
		}

		if (displayName) {
			if (nameText != null && linkedHM != null) {
				if (linkedHM.aic != null) nameText.text = linkedHM.aic.targetID; 
			}
		}

		if (displayHealth && linkedHM != null) {
			secondaryDisplayString = vfloor(linkedHM.health).ToString();
		} else {
			secondaryDisplayString = System.String.Empty;
		}

		if (displayRange && linkedHM != null) {
			float range = distance_vector3(Eng_Global->instances[PLAYER1].position,
										   linkedHM.Eng_Global->instances[i].position);

			if (displayHealth) secondaryDisplayString += comma;
			secondaryDisplayString += (range.ToString("0.0") + rangeMetersM);
		}

		if (displayAttitude && linkedHM != null) {
			if (displayRange || displayHealth) secondaryDisplayString += comma;
			if (linkedHM.aic.asleep) {
				secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[519]); // Asleep
			} else {
				switch (linkedHM.aic.currentState) {
					case AIState_Walk: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[517]); break; // Cautious
					case AIState_Inspect: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[517]); break; // Cautious
					case AIState_Interacting: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[517]); break; // Cautious
					case AIState_Run: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[518]); break; // Hostile
					case AIState_Attack1: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[518]); break; // Hostile
					case AIState_Attack2: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[518]); break; // Hostile
					case AIState_Attack3: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[518]); break; // Hostile
					case AIState_Pain: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[518]); break; // Hostile
					default: secondaryDisplayString = (secondaryDisplayString + Sys_Text.stringTable[516]); break; // Idle
				}
			}
		}
		secondaryText.text = secondaryDisplayString;
		if (currentText != System.String.Empty) {
			if (linkedHM != null) {
				if (linkedHM.aic != null) {
					if (linkedHM.aic.tranquilizeFinished > Eng_Global->pauseRelativeTime
						&& damageTimeFinished < Eng_Global->pauseRelativeTime) {
						currentText = Sys_Text.stringTable[536]; // STUNNED
					} else {
						if (damageTimeFinished < Eng_Global->pauseRelativeTime) {
							currentText = "";
							if (!Eng_Global->inventoryPlayer1.hasHardware[4]
								&& (currentText != Sys_Text.stringTable[511])) {

								Deactivate();
								return;
							}
						}
					}
				}
			}
			text.text = currentText;
		}
    }

	public static float GetTargetIDSensingRange(bool manual) {
		float sensingRange = 12f;
		if (manual) {
			// Get manual lockon distance for frob raytrace.  Less than tether.
			switch (Eng_Global->inventoryPlayer1.hardwareVersion[4]) {
				case 1: sensingRange = 13f; break;
				case 2: sensingRange = 13f; break;
				case 3: sensingRange = 13f; break;
				case 4: sensingRange = 18f; break;
			}
		} else {
			// Get auto-lock distance.  Less than tether.
			switch (Eng_Global->inventoryPlayer1.hardwareVersion[4]) {
				case 1: sensingRange = 0f; break; // No auto-lock on v1
				case 2: sensingRange = 0f; break; // No auto-lock on v2
				case 3: sensingRange = 13f; break;
				case 4: sensingRange = 20f; break;
			}
		}
		return sensingRange;
	}

	// Set to higher than the auto-lock distances above.
	public static float GetTargetIDTetherRange() {
		float dist = 15f;
		switch (Eng_Global->inventoryPlayer1.hardwareVersion[4]) {
			case 1: dist = 15f; break; // Set higher than manual lockons.
			case 2: dist = 15f; break; // Set higher than manual lockons.
			case 3: dist = 15f; break;
			case 4: dist = 22f; break;
		}

		return dist;
	}
}
