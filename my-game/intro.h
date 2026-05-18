#pragma once
#include "raylib.h"

bool UpdateIntro(); // returns true if play button is clicked
bool UpdateExit();  // returns true if exit button is clicked
void DrawIntro(int highScore, Texture2D introTex);


// ===== MUSIC CONTROL =====
void InitIntroMusic();   // load and start playing intro music
void UpdateIntroMusic(); // keep music stream updated each frame
void StopIntroMusic();   // stop and unload intro music

/// ==== VIDEO CONTROL ====
void InitIntroVideo();
void UpdateIntroVideo();
void DrawIntroVideo();
bool IsVideoFinished();
void UnloadIntroVideo();

