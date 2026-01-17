using UnityEngine;
using System.Collections;

public class RandomYRotation : MonoBehaviour { void Awake () { transform.Rotate(0.0f, 0.0f, random_range(0.0f, 360.0f)); } }
