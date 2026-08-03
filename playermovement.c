// playermovement.c - Player Movement, Crouch, Prone, Leaning
float GetBasePlayerSpeed(u16 p,bool running){
    bool sprint=Sprint(); if(Cheats.noclip)return PLAYER_MAX_CYBER_SPEED*(sprint?2.5f:1.5f); if(World.curLev==LEVEL_CYBERSPACE)return PLAYER_MAX_CYBER_SPEED;
    BodyState b=World.instances[p].bodyState; float v=WALK_SPEED;
    switch(b){ case BodyState_CrouchingDown: case BodyState_Crouch:v=CROUCH_SPEED; break; case BodyState_Prone: case BodyState_ProningDown: case BodyState_ProningUp:v=PLAYER_MAX_PRONE_SPEED; break; default:break; }
    if ((sprint||World.boosterActive) && running) { v = World.invP1.fatigue > 80.0f && World.boosterActive ? SPRINT_SPEED_FATIGUED : SPRINT_SPEED;
    if (b==BodyState_Standing||b==BodyState_Crouch||b==BodyState_CrouchingDown)  v -= (WALK_SPEED-CROUCH_SPEED)*1.5f;
    else if(b==BodyState_Prone||b==BodyState_ProningDown||b==BodyState_ProningUp)v -= (WALK_SPEED-PLAYER_MAX_PRONE_SPEED)*2.f;}
    return v + (World.boosterActive ? PLAYER_BOOSTER_SPEED_BOOST : 0.0f);
}

INLINE float smooth_damp(float cur, float targ, float* vel, float tm, float dt) { float o=2.0f / vmax(tm,0.0001f); float x=o * dt; float exp=1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x); float d=cur - targ; float t=(*vel + o * d) * dt; *vel=(*vel - o * t) * exp; return targ + (d + t) * exp; }
Overlap CapMsh(ShapeCapsule,u16,const float*);
bool CantStand(u16 playerIdx, float targetHeight) { // I can't stand it.
    float oldHeight = World.colliderSize[playerIdx].y; V3 oldPos = World.position[playerIdx];
    World.colliderSize[playerIdx].y = targetHeight; World.position[playerIdx].y += (targetHeight - oldHeight); // Temporarily morph player into the standing capsule
    bool blocked = false; i32 cx=PosGetCellCoordX(World.position[playerIdx].x), cz=PosGetCellCoordZ(World.position[playerIdx].z); u32 mask=GetCollisionMask(World.layer[playerIdx]);
    for (i32 dx = -1; dx <= 1 && !blocked; ++dx) {
        for (i32 dz = -1; dz <= 1 && !blocked; ++dz) {
            u32 cell = PosGetCellCoordsP(cx + dx, cz + dz);
            for (u16 k = 0; k < cellCounts[cell]; ++k) {
                u16 b = cellLists[cell][k]; if (b == playerIdx || !(mask & World.layer[b]) || World.col[b] == COLTYPE_NONE) continue;
                if (World.col[b] == COLTYPE_MSH) { Overlap r = CapMsh(Entity_GetCap(playerIdx),World.instances[b].modelIndex,&modelMatrices[b*16]); if (r.hit && r.pen > 0.08f) { blocked = true; break; } }
            }
        }
    }
    World.colliderSize[playerIdx].y = oldHeight; World.position[playerIdx] = oldPos; return blocked;
}

KeyState* GetCodeMapping(int settingIndex);
void ApplyPlayerMovements(float dt) {
    Entity *p = &World.instances[PLAYER1]; Quaternion r = World.rotation[PLAYER1]; float leanSpeed = 70.0f, leanMaxAngle = 35.0f; float leanInput = (float)LeanLeft() - (float)LeanRight(); bool doubleTapLean = DoubleTapLeanLeft() || DoubleTapLeanRight();
    bool movingForward = Forward() > 0.1f, leanRight = leanInput < 0.0f, leanLeft = leanInput > 0.0f;
    if (doubleTapLean) { World.invP1.leanResetting = true; World.invP1.leanVelocity = 0.0f; KeyState *kL = GetCodeMapping(7), *kR = GetCodeMapping(8); kL->pressed = kR->pressed = false; } // Double-tap lean: initiate smooth reset to upright over 0.2 seconds
    if (World.invP1.leanResetting) { 
        World.invP1.leanTarget = smooth_damp(World.invP1.leanTarget,0.0f,&World.invP1.leanVelocity,0.2f,dt); 
        if(vabs(World.invP1.leanTarget) < 0.5f){World.invP1.leanTarget=World.invP1.leanVelocity=0.0f; World.invP1.leanResetting=false;} 
    } else {
        if (leanLeft || leanRight) { if(leanLeft){World.invP1.leanRightTapFinished =0;} if(leanRight){World.invP1.leanLeftTapFinished=0;} World.invP1.leanTarget=vclamp(World.invP1.leanTarget + (leanInput * leanSpeed * dt),-leanMaxAngle,leanMaxAngle); }
        else if (movingForward) { if (vabs(World.invP1.leanTarget) < 0.5f) { World.invP1.leanTarget = 0.0f; } else { World.invP1.leanTarget -= (World.invP1.leanTarget > 0.0f ? 1.0f : -1.0f) * leanSpeed * dt; } }
    }
    World.cam_roll = World.invP1.leanTarget;
    float targetRatio=1.0f, transitionSec=0.2f; float currentRatio=World.invP1.currentCrouchRatio;
    if (Crouch()) { // Crouch key always targets crouch ratio from any state
        if (p->bodyState == BodyState_Crouch) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState = BodyState_StandingUp;}} // Already at crouch → toggle up to standing
        else if (currentRatio > PLAYER_CROUCH_RATIO) { p->bodyState = BodyState_CrouchingDown;} // Above crouch → go down to crouch (handles "if standing up will go back to crouched")
        else {p->bodyState=BodyState_ProningUp;} // Below crouch → go up to crouch (handles "if proning down will go back to crouched")
    } else if (Prone()) {
        if (p->bodyState == BodyState_Standing) { p->bodyState = BodyState_ProningDown; } // Standing → go to prone
        else if (currentRatio > PLAYER_CROUCH_RATIO) { if (!CantStand(PLAYER1,PLAYER_HEIGHT)){p->bodyState=BodyState_StandingUp;}else{p->bodyState = BodyState_ProningDown;} } // Between crouch and standing → up to standing
        else if (p->bodyState == BodyState_Crouch) { p->bodyState = BodyState_ProningDown; } // Crouch → go to prone
        else { p->bodyState = BodyState_ProningUp; } // Between prone and crouch, or prone → up to crouch
    }
    switch (p->bodyState) {
        case BodyState_CrouchingDown:targetRatio=-0.01f; break; case BodyState_StandingUp:targetRatio=1.01f; break; case BodyState_ProningDown:targetRatio=-0.01f; break;
        case BodyState_ProningUp:targetRatio=1.01f; transitionSec+=0.1f; break; case BodyState_Crouch:targetRatio=PLAYER_CROUCH_RATIO; break; case BodyState_Prone:targetRatio=PLAYER_PRONE_RATIO; break; default:targetRatio=1.0f; break;
    }
    float lastRatio = World.invP1.currentCrouchRatio;
    World.invP1.currentCrouchRatio = smooth_damp(lastRatio,targetRatio,&World.invP1.crouchingVelocity,transitionSec,dt);
    if (World.invP1.currentCrouchRatio >= 1.0f) { World.invP1.currentCrouchRatio = 1.0f; if(p->bodyState == BodyState_StandingUp){p->bodyState=BodyState_Standing;} }
    else if (p->bodyState == BodyState_CrouchingDown && World.invP1.currentCrouchRatio <= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningUp && World.invP1.currentCrouchRatio >= PLAYER_CROUCH_RATIO) { World.invP1.currentCrouchRatio = PLAYER_CROUCH_RATIO; p->bodyState = BodyState_Crouch; }
    else if (p->bodyState == BodyState_ProningDown && World.invP1.currentCrouchRatio <= PLAYER_PRONE_RATIO) { World.invP1.currentCrouchRatio = PLAYER_PRONE_RATIO; p->bodyState = BodyState_Prone; }
    World.colliderSize[PLAYER1].y = PLAYER_HEIGHT * World.invP1.currentCrouchRatio; // Split capsule shape in the middle, camera is thus 0.16 away from top of the capsule ((2 / 2 = 1) - 0.84 which is PLAYER_CAM_OFFSET_Y)
    float h=(float)Forward() - (float)Backpedal(), s=(float)StrafeRight() - (float)StrafeLeft(), vertInput=(float)SwimUp() - (float)SwimDn();
    float y2=r.y*r.y, xz=r.x*r.z, wy=r.w*r.y;
    p->forward=V3_Normalize((V3){ 2.0f*(xz + wy),2.0f*(r.y*r.z - r.w*r.x),1.0f - 2.0f*(r.x*r.x + y2) }); p->right=V3_Normalize((V3){ 1.0f - 2.0f*(y2 + r.z*r.z),2.0f*(r.x*r.y + r.w*r.z),2.0f*(xz - wy) });
    V3 inputDir={ p->forward.x*h + p->right.x*s,vertInput,p->forward.z*h + p->right.z*s}; 
    float inputLenSq = V3_dot(inputDir,inputDir); V3 w = (inputLenSq > 0.0001f) ? V3_ScaleByF(inputDir, 1.0f / vsqrtf(inputLenSq)) : (V3){0, 0, 0};
    bool isRunning = (inputLenSq > 0.01f); float speed = GetBasePlayerSpeed(PLAYER1,isRunning) * 1.75f, accel=World.boosterActive ? 1.0f : 3.0f; V3 targetVel = V3_ScaleByF(w,speed); 
    if (World.invP1.ladderState > 0) { float climbSpeed = (Sprint() && isRunning) ? 1.2f : 0.4f; targetVel = (V3){p->right.x * s * speed * 0.3f, h * climbSpeed * 25.0f, p->right.z * s * speed * 0.3f}; accel = 5.0f; }
    else { if (vabs(vertInput) < 0.001f) { targetVel.y = World.velocity[PLAYER1].y; } }
    V3 dv = V3_AsubB(targetVel, World.velocity[PLAYER1]); 
    dv = (V3){ vclamp(dv.x, -10.0f, 10.0f), vclamp(dv.y, -10.0f, 10.0f), vclamp(dv.z, -10.0f, 10.0f) };
    World.velocity[PLAYER1] = V3_AplusB(World.velocity[PLAYER1], V3_ScaleByF(dv, accel * vclamp(dt, 0.0005f, 0.1f)));
}
