#pragma once
#include "raylib.h"

bool UpdateIntro();  // returns true when player wants to start
void DrawIntro(int highScore, Texture2D introTex); 
bool UpdateExit(); // returns true when player wants to exit


// ===== MUSIC CONTROL =====
void InitIntroMusic();   // load and start playing intro music
void UpdateIntroMusic(); // keep music stream updated each frame
void StopIntroMusic();   // stop and unload intro music
