#include "mod.h"

MOD_TO_ENGINE float GetBasePlayerSpeed(bool running) {
    bool isSprinting = Sprint();
    if (Eng_Cheats->noclip && isSprinting) return PLAYER_MAX_CYBER_SPEED * 2.5f;
    if (Eng_Cheats->noclip) return PLAYER_MAX_CYBER_SPEED * 1.5f;
    if (Eng_Global->currentLevel == LEVEL_CYBERSPACE) return PLAYER_MAX_CYBER_SPEED; //Cyber space speed

    float retval = PLAYER_MAX_WALK_SPEED;
    float bonus = 0.0f;
    if (Eng_Global->boosterActive) bonus = PLAYER_BOOSTER_SPEED_BOOST;
    BodyState bodyState = Eng_Global->instances[PLAYER1].bodyState;
    switch (bodyState) {
        case BodyState_Standing:      retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Crouch:        retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_CrouchingDown: retval = PLAYER_MAX_CROUCH_SPEED; break;
        case BodyState_StandingUp:    retval = PLAYER_MAX_WALK_SPEED;   break;
        case BodyState_Prone:         retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningDown:   retval = PLAYER_MAX_PRONE_SPEED;  break;
        case BodyState_ProningUp:     retval = PLAYER_MAX_PRONE_SPEED;  break;
    }

    if ((isSprinting || Eng_Global->boosterActive) && running) {
        if (Eng_Global->instances[PLAYER1].fatigue > 80.0f && Eng_Global->boosterActive) retval = PLAYER_MAX_SPRINT_SPEED_FATIGUED;
        else                                                                retval = PLAYER_MAX_SPRINT_SPEED;

        if (bodyState == BodyState_Standing || bodyState == BodyState_Crouch || bodyState == BodyState_CrouchingDown) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_CROUCH_SPEED) * 1.5f); // Subtract off the difference in speed between walking and crouching from the sprint speed
        } else if (bodyState == BodyState_Prone || bodyState == BodyState_ProningDown || bodyState == BodyState_ProningUp) {
            retval -= ((PLAYER_MAX_WALK_SPEED - PLAYER_MAX_PRONE_SPEED) * 2.0f); // Subtract off the difference in speed between walking and proning from the sprint speed.
        }
    }

    return retval + bonus;
}
