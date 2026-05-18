#include "intro.h"
#include "raylib.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <string>


static Music introMusic;

static std::vector<Texture2D> videoFrames;
static int currentFrame = 0;
static float frameTimer = 0.0f;
static float frameDelay = 0.1f; 
static bool videoFinished = false;


void InitIntroVideo() {
    
    for (int i = 1; i <= 9; i++) {
        std::string filename = "assets/videos/TransIntro/" + std::to_string(i) + ".png";
        Texture2D tex = LoadTexture(filename.c_str());
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        videoFrames.push_back(tex);
    }
    currentFrame = 0;
    frameTimer = 0.0f;
    videoFinished = false;
}

// --- Update function ---
void UpdateIntroVideo() {
    if (videoFinished) return;

    frameTimer += GetFrameTime();
    if (frameTimer >= frameDelay) {
        currentFrame++;
        frameTimer = 0.0f;
    }

    if (currentFrame >= videoFrames.size()) {
        videoFinished = true;
    }
}

// --- Draw function ---
void DrawIntroVideo() {
     if (!videoFinished && currentFrame < videoFrames.size()) {
        Texture2D frame = videoFrames[currentFrame];

        ClearBackground(BLACK);

        DrawTexturePro(
            frame,
            {0, 0, (float)frame.width, (float)frame.height},   
            {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, 
            {0, 0},                                          
            0.0f,                                            
            WHITE                                           
        );
    }
}

//Flag to check if video has finished
bool IsVideoFinished() {
    return videoFinished;
}

// --- Cleanup function ---
void UnloadIntroVideo() {
    for (auto &tex : videoFrames) {
        UnloadTexture(tex);
    }
    videoFrames.clear();
}


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