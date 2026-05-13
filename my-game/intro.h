#pragma once
#include "raylib.h"

bool UpdateIntro();  // returns true when player wants to start
void DrawIntro(int highScore, Texture2D introTex); 
bool UpdateExit(); // returns true when player wants to exit

