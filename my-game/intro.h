#pragma once
#include "raylib.h"

bool UpdateIntro(); // returns true if play button is clicked
bool UpdateExit();  // returns true if exit button is clicked
void DrawIntro(int highScore, Texture2D introTex);

//info
void InitInfoTexture();
void UnloadInfoTexture();
void UpdateInfo();


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

// Functions for BG1 transition
void InitBg1TransitionVideo();
void UpdateBg1TransitionVideo();
void DrawBg1TransitionVideo();
bool IsBg1TransitionFinished();
float GetBg1TransitionAlpha();
void UnloadBg1TransitionVideo();

// Functions for BG2 transition
void InitBg2TransitionVideo();
void UpdateBg2TransitionVideo();
void DrawBg2TransitionVideo();
bool IsBg2TransitionFinished();
float GetBg2TransitionAlpha();
bool IsBg2TransitionFinished();
void UnloadBg2TransitionVideo();

