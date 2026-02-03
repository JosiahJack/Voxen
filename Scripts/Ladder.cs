using UnityEngine;
using System.Collections;

public class Ladder : MonoBehaviour {	
	void  OnTriggerEnter (Collider other){
		if (other.CompareTag("Player")) {
			instances[PLAYER1].ladderState++;
			if (instances[PLAYER1].ladderState < 1) instances[PLAYER1].ladderState = 1;
		}
	}
	
	void  OnTriggerExit (Collider other){
		if (other.CompareTag("Player")) {
			instances[PLAYER1].ladderState--;
			if (instances[PLAYER1].ladderState < 0) instances[PLAYER1].ladderState = 0;
		}
	}
}
