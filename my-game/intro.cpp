#include "intro.h"
#include "raylib.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>


static Music introMusic;


void InitIntroMusic() {
    introMusic = LoadMusicStream("assets/audio/intro.mp3");
    PlayMusicStream(introMusic);
    SetMusicVolume(introMusic, 0.5f); 
}


void UpdateIntroMusic() {
    UpdateMusicStream(introMusic);
}


void StopIntroMusic() {
    StopMusicStream(introMusic);
    UnloadMusicStream(introMusic);
}
    

// ================= UPDATE INTRO =================
bool UpdateIntro() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float buttonWidth = 300;
    float buttonHeight = 80;

    
    float scoreY = screenHeight / 2.0f - 200 + 120 + 40; 
    Rectangle playButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        scoreY + 50,   
        buttonWidth,
        buttonHeight
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, playButton);

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);};

// ================= UPDATE EXIT =================
bool UpdateExit() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float buttonWidth = 300;
    float buttonHeight = 80;

    
    Rectangle exitButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        screenHeight / 2.0f + buttonHeight + 30, 
        buttonWidth,
        buttonHeight
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, exitButton);

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}



// ================= DRAW =================
void DrawIntro(int highScore, Texture2D introTex) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

     // ===== BACKGROUND =====
     DrawTexturePro(
     introTex,
     (Rectangle){ 0, 0, (float)introTex.width, (float)introTex.height },   
     (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight },         
     (Vector2){ 0, 0 },                                                    
     0.0f,                                                                 
     WHITE );

    
      // ===== TITLE =====
     const char* title = "Curse of the Wings";
     int titleSize = 120;

     Font scaryFont = LoadFontEx("assets/font/Nosifer-Regular.ttf", 64, 0, 0);

     Vector2 textSize = MeasureTextEx(scaryFont, title, (float)titleSize, 2);

     float titleX = screenWidth / 2.0f - textSize.x / 2;
     float titleY = screenHeight / 2.0f - 200;

     float fadeAlpha = fminf(GetTime() * 0.5f, 1.0f);

     // Shadow
     DrawTextEx(scaryFont,
         title,
         {titleX + 3, titleY + 3}, // offset for shadow
         (float)titleSize,
          0,
          BLACK);

    // Main title with fade
     DrawTextEx(scaryFont,
         title,
         {titleX, titleY},
         (float)titleSize,
         0,
         Fade(Color{139, 0, 0, 255}, fadeAlpha));

     
     // ===== HIGH SCORE =====
     const char* scoreText = TextFormat("High Score: %d", highScore);
     int scoreFontSize = 40;
     
     Font scoreFont = LoadFontEx("assets/font/Quantico-Regular.ttf", 64, 0, 0);
     Vector2 scoreSize = MeasureTextEx(scoreFont, scoreText, (float)scoreFontSize, 0);

     float scoreX = screenWidth / 2.0f - scoreSize.x / 2;
     float scoreY = titleY + textSize.y + 40;

    DrawTextEx(scoreFont,
         scoreText,
         {scoreX, scoreY},
         (float)scoreFontSize,
         0,
          WHITE);

    
    // ===== PLAY BUTTON =====

    float buttonWidth = 300;
    float buttonHeight = 80;


     Rectangle playButton = {
     screenWidth / 2.0f - buttonWidth / 2,
     scoreY + 50,   
     buttonWidth,
     buttonHeight
    };
    
    Vector2 mouse = GetMousePosition();
    bool playHovered = CheckCollisionPointRec(GetMousePosition(), playButton);

     if (playHovered) {
    Rectangle glowRect = {
        playButton.x - 5,
        playButton.y - 5,
        playButton.width + 10,
        playButton.height + 10
     };
    DrawRectangleRounded(glowRect, 0.3f, 10, Fade(MAROON, 0.5f));
     }

     Color playButtonColor = playHovered ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
     DrawRectangleRounded(playButton, 0.3f, 10, playButtonColor);

      Font gamefont = LoadFontEx("assets/font/Chewy-Regular.ttf", 64, 0, 0);

     const char* playText = "PLAY";
     int playFontSize = 50;
     Vector2 playTextSize = MeasureTextEx(gamefont, playText, (float)playFontSize, 0);

    DrawTextEx(gamefont,
    playText,
    {
        playButton.x + playButton.width / 2 - playTextSize.x / 2,
        playButton.y + playButton.height / 2 - playTextSize.y / 2
    },
    (float)playFontSize,
    0,
    WHITE);


     // ===== EXIT BUTTON =====
     float exitButtonWidth = 300;
     float exitButtonHeight = 80;

     Rectangle exitButton = {
          screenWidth / 2.0f - exitButtonWidth / 2,
          screenHeight / 2.0f + exitButtonHeight + 30, // below Play
          exitButtonWidth,
          exitButtonHeight
     };

     bool exitHovered = CheckCollisionPointRec(GetMousePosition(), exitButton);

    if (exitHovered) {
    Rectangle glowRect = {
        exitButton.x - 5,
        exitButton.y - 5,
        exitButton.width + 10,
        exitButton.height + 10
     };
    DrawRectangleRounded(glowRect, 0.3f, 10, Fade(MAROON, 0.5f));
     }

     Color exitButtonColor = exitHovered ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
     DrawRectangleRounded(exitButton, 0.3f, 10, exitButtonColor);

     const char* exitText = "EXIT";
     int exitFontSize = 50;
     Vector2 exitTextSize = MeasureTextEx(gamefont, exitText, (float)exitFontSize, 0);

     DrawTextEx(gamefont,
         exitText,
         {
        exitButton.x + exitButton.width / 2 - exitTextSize.x / 2,
        exitButton.y + exitButton.height / 2 - exitTextSize.y / 2
         },
         (float)exitFontSize,
         0,
         WHITE);
     }