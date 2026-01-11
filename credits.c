
char creditStats[4096];

float GetScore(float stupid, bool isFinal) {
    float score = 0.0f;
    float victories = (float)(Sys_Global.kills + Sys_Global.cyberkills);
    float secs = 0.0f;
    secs = vfloor((float)Sys_Global.pauseRelativeTime / 3600.0f);
    if (!isFinal) { // Report score if no deaths.
        score = victories * 10000.0f;
        score -= vmin(score * 0.666f,secs * 100.0f);
        score *= ((stupid + 1.0f) / 37.0f);
        if (stupid > 35.0f) score += 2222222.0f; // secret kevin bonus
        return vfloor(score);
    }
    
    // Death is 10 anti-kills, but you always keep at least a third of your kills.
    float deathPenalty = Sys_Global.ressurections * 10.0f;
    score = victories - vmin(deathPenalty,victories * 0.666f);
    score *= 10000.0f;
    score -= vmin(score * 0.666f,secs * 100.0f);
    score *= ((stupid + 1.0f) / 37.0f); // 9 * 4 + 1 is best difficulty factor
    if (stupid > 35.0f) score += 2222222.0f; // secret kevin bonus
    return vfloor(score);
}

void CreditsStats(void) {
    size_t off = 0;
    off += snprintf(creditStats + off, sizeof(creditStats), "================================================================================\nCITADEL\n");
    off += snprintf(creditStats + off, sizeof(creditStats), "================================================================================\nCONGRATULATIONS %s\n", Sys_Global.playerName);    
    uint32_t hours, minutes; double secs;
    double t = Sys_Global.pauseRelativeTime;
    double tb = (vfloor(t/3600.0));
    hours = (uint32_t)tb;
    t = t - (tb * 3600.0);
    tb = vfloor(t / 60.0); 
    minutes = (uint32_t)tb;
    secs = t - (tb * 60.0);
    off += snprintf(creditStats + off, sizeof(creditStats), "Straight Time: %uh %um %.3fs\n", hours, minutes, secs);
    t = Sys_Global.absoluteTime;
    tb = vfloor(t/3600.0);
    hours = (uint32_t)tb;
    t = t - (tb * 3600.0);
    tb = vfloor(t / 60.0); 
    minutes = (uint32_t)tb;
    secs = t - (tb * 60.0);
    off += snprintf(creditStats + off, sizeof(creditStats), "Total Time (with reload from deaths): %uh %um %.3fs\n", hours, minutes, secs);
    float stupid = 0.0f;
    stupid += (float)(Sys_Global.difficultyCombat * Sys_Global.difficultyCombat);
    stupid += (float)(Sys_Global.difficultyPuzzle * Sys_Global.difficultyPuzzle);
    stupid += (float)(Sys_Global.difficultyMission * Sys_Global.difficultyMission);
    stupid += (float)(Sys_Global.difficultyCyber * Sys_Global.difficultyCyber);
    uint32_t finalSubscore = GetScore(stupid, false);
    off += snprintf(creditStats + off, sizeof(creditStats), "Kills: %u\nKills in Cyberspace: %u\nScoreSubtotal: %u\nDeaths: %u\nRessurections: %u\n", Sys_Global.kills, Sys_Global.cyberkills, (uint32_t)finalSubscore, Sys_Global.deaths, Sys_Global.ressurections);
    off += snprintf(creditStats + off, sizeof(creditStats), "Combat: %u | Puzzle: %u | Mission: %u | Cyber: %u\n", Sys_Global.difficultyCombat, Sys_Global.difficultyPuzzle, Sys_Global.difficultyMission, Sys_Global.difficultyCyber);
    uint32_t finalScore = (uint32_t)GetScore(stupid, true);
    off += snprintf(creditStats + off, sizeof(creditStats), "Difficulty Index: %.2f\nFinal Score: %u\n\n", (double)stupid, finalScore);
    off += snprintf(creditStats + off, sizeof(creditStats), "Shots Fired: %u\nGrenades Thrown: %u\n", Sys_Global.shotsFired, Sys_Global.grenadesThrown);
    off += snprintf(creditStats + off, sizeof(creditStats), "Damage Dealt: %f\nDamage Received: %f\nSaves Scummed: %u\n\nClick to continue...\n", (double)Sys_Global.damageDealt, (double)Sys_Global.damageReceived, Sys_Global.savesScummed);
}

void CreditsScroll(void) {
    if (!Sys_Global.creditsActive) return;
    
    if (Sys_Input.mouseButtons[GLFW_MOUSE_BUTTON_1].pressed) {
        ++Sys_Global.creditsPageIndex;
        if (Sys_Global.creditsPageIndex > Sys_Global.creditsLength) {Sys_Global.creditsActive = false; return; }
    }
    
    if (Sys_Global.creditsPageIndex == 1) {
        CreditsStats();
        RenderFormattedText(GetScreenRelativeX(0.219f), GetScreenRelativeY(0.0125f), TEXT_WHITE, FONT_NORMAL, (const char*)&creditStats);
    }
}
