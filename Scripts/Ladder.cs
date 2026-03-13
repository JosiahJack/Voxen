using UnityEngine;
using System.Collections;

public class Ladder : MonoBehaviour {	
	void  OnTriggerEnter (Collider other){
		if (other.CompareTag("Player")) {
			Eng_Global->instances[PLAYER1].ladderState++;
			if (Eng_Global->instances[PLAYER1].ladderState < 1) Eng_Global->instances[PLAYER1].ladderState = 1;
		}
	}
	
	void  OnTriggerExit (Collider other){
		if (other.CompareTag("Player")) {
			Eng_Global->instances[PLAYER1].ladderState--;
			if (Eng_Global->instances[PLAYER1].ladderState < 0) Eng_Global->instances[PLAYER1].ladderState = 0;
		}
	}
}
