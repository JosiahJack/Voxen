
// Externally set in inspector per instance
float speed = 0.64f; // save
Vector3 targetPosition;
FuncStates startState; // save
float percentAjar = 0; // save
float percentMoved = 0; // save
FuncStates currentState; // save
int[] chunkIDs; // save
float startTime; // save
Vector3 startPosition; // save
Rigidbody rbody;
bool stopSoundPlayed; // save

void InitializeFromLoad() {
    rbody = GetComponent<Rigidbody>();
    SFXSource = GetComponent<AudioSource>();
    rbody.collisionDetectionMode = CollisionDetectionMode.ContinuousSpeculative;
    Vector3 tempVec = Vector3_A_minus_B(instances[i].position, targetPosition);
    float distTotal = distance_vector3(startPosition, targetPosition);
    tempVec = -tempVec.normalized;
    if (currentState == FuncStates_AjarMovingTarget)     tempVec *= (distTotal * percentAjar);
    else if (currentState == FuncStates_AjarMovingStart) tempVec *= (distTotal * (1f - percentAjar));
    else if (currentState == FuncStates_MovingStart)     tempVec *= (distTotal * (1f - percentMoved));
    else                                                 tempVec *= (distTotal * percentMoved);

    tempVec.x = vclamp(tempVec.x,-10000.0f,10000.0f);
    tempVec.y = vclamp(tempVec.y,-10000.0f,10000.0f);
    tempVec.z = vclamp(tempVec.z,-10000.0f,10000.0f);
    tempVec += instances[i].position;
    instances[i].position = tempVec;
}
