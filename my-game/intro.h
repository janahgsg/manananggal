#pragma once
#include "raylib.h"

typedef struct {
    Rectangle play;
    Rectangle exit;
    Rectangle info;
    Rectangle close;
    Rectangle next;
    Rectangle prev;
} IntroButtons;

void InitIntro();
void SetupButtons();
void UnloadIntro();
int UpdateIntro();
void DrawIntro(int highScore, Texture2D introTex, Texture2D titleTex);

// ===== MUSIC CONTROL =====
void InitIntroMusic();   
void UpdateIntroMusic(); 
void StopIntroMusic();   

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

