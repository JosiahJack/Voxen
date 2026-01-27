using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

public class InteractablePanel : MonoBehaviour {
	public int requiredIndex;
	public GameObject installationItem; // item that will be activated to make it look like the player installed something, e.g. an isotope or plastique
	public GameObject[] effects; // any effect items to turn on
	public bool open = false;
	public bool installed = false;
	public int wrongItemMessageLingdex;
	public int SFXInstallationIndex;
	public int SFXOpenIndex;
	public int installedMessageLingdex;
	public int alreadyInstalledMessageLingdex;
	public int SFXAlreadyInstalledIndex;
	public int openMessageLingdex;
	public string target;
	
	private AudioSource SFX;
	private Animator anim;
	private static StringBuilder s1 = new StringBuilder();

	void Start() {
		anim = GetComponent<Animator>();
		SFX = GetComponent<AudioSource>();
	}

	public void Use(UseData ud) {
		if (open) {
			if (installed && ud.mainIndex == -1) {
				Const.sprint(alreadyInstalledMessageLingdex);
				return; // do nothing already done here
			}

			// Was player holding correct item in their hand when they used us?
			if (ud.mainIndex == requiredIndex && (requiredIndex != 92
												  || (requiredIndex == 92
												  && ud.customIndex == 1))) {
												     // Abe Ghiran's head.
				if (installed) { 					 // ... is big
					Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXAlreadyInstalledIndex]);
					return; // do nothing already done here
				}
				installed = true;
				if (installationItem != null) installationItem.SetActive(true);
				Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXInstallationIndex]);
				Const.sprint(installedMessageLingdex);
				// any extra effect objects?  activate them here...good for sparks or turning on any extra bits and bobs
				if (effects.Length > 0) {
					for(int i=0;i<effects.Length;i++) {
						if (effects[i] != null) {
							effects[i].SetActive(true);
							Rigidbody efrb = effects[i].GetComponent<Rigidbody>();
							if (efrb != null) {
								efrb.WakeUp();
								efrb.useGravity = true;
							}
						}
					}
				}

				MouseLookScript.a.ResetHeldItem();

				// use the target now that we are active
				Const.a.UseTargets(gameObject,ud,target);
			} else {
				Utils.PlayOneShotSavable(SFX,Const.a.sounds[43]); // button_deny, aaaahhh!! Try again
				Const.sprint(wrongItemMessageLingdex);
			}
		} else {
			open = true;
			anim.Play("Open");
			Utils.PlayOneShotSavable(SFX,Const.a.sounds[SFXOpenIndex]);
			Const.sprint(openMessageLingdex);
		}
	}
}
