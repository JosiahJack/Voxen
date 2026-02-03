using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlaySoundTriggered : MonoBehaviour {
	public int SFXClip = -1;
	public bool loopingAmbient = false;
	public bool playEverywhere = false;
	public bool playSoundOnParticleEmit = false;
	
	private AudioSource SFX;
	bool currentlyPlaying = false;
	int numparticles = 0;
	int burstemittcnt1 = 15;
	int burstemittcnt2 = 30;
	private bool justPaused;
	private ParticleSystem psys;

    void Start() {
		if (SFXClip > 0) SFX.clip = sounds[SFXClip];
		else DualLog("Unassigned clip index on PlaySoundTriggered at " + instances[i].position.ToString() + " for " + gameObject.name);
		if (playEverywhere) {
			SFX.spatialBlend = 0.0f;
		} else {
			SFX.spatialBlend = 1.0f;
		}
		
		if (loopingAmbient) {
			if (SFX != null) SFX.loop = true;
			currentlyPlaying = true;
			if (SFX != null) SFX.Play();
		}

		if (playSoundOnParticleEmit) {
			psys = GetComponent<ParticleSystem>();
			if (psys == null) DualLog("ERROR: missing ParticleSystem for PlaySoundTriggered");
			loopingAmbient = false; //only play when triggered by the psys emission
			numparticles = 0;
		}
    }

	// For ambient noises
	void OnEnable() {
		if (SFX == null) SFX = GetComponent<AudioSource>();
		if (loopingAmbient) {
			if (SFX != null) SFX.loop = true;
			if (SFX != null) SFX.clip = sounds[SFXClip];
			if (SFX != null) SFX.Play();
		}
	}

	void Update() {
		if (currentlyPlaying) {
			if (Sys_Global.gamePaused || Sys_Global.menuActive) {
				if (SFX != null) SFX.Pause();
				justPaused = true;
			} else {
				if (justPaused) {
					if (SFX != null) SFX.UnPause();
					justPaused = false;
				}
			}
		}

		if (!Sys_Global.gamePaused && !Sys_Global.menuActive) {
			if (playSoundOnParticleEmit){
				int count = psys.particleCount;
				if (count > numparticles && (count == burstemittcnt1 || count == burstemittcnt2)) {
					Utils.PlayOneShotSavable(SFX,sounds[SFXClip]);
				}
				numparticles = count;
			}
		}
	}

    public void PlaySoundEffect() {
		if (SFX != null) SFX.loop = false;
		Utils.PlayOneShotSavable(SFX,sounds[SFXClip]);
	}
	
	public void StopSoundEffect() {
		if (SFX != null) SFX.Stop();
		currentlyPlaying = false;
	}
}
