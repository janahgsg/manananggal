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

// ===== NEW: Info overlay state =====
static bool showInfo = false;
static Texture2D infoTexture;


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

// ===== NEW: Load info image =====
void InitInfoTexture() {
    infoTexture = LoadTexture("assets/images/info.png"); 
}

// ===== NEW: Unload info image =====
void UnloadInfoTexture() {
    UnloadTexture(infoTexture);
}

void UpdateIntroVideo() {
    if (videoFinished) return;

    frameTimer += GetFrameTime();
    if (frameTimer >= frameDelay) {
        currentFrame++;
        frameTimer = 0.0f;
    }

    if (currentFrame >= (int)videoFrames.size()) {
        videoFinished = true;
    }
}

void DrawIntroVideo() {
    if (!videoFinished && currentFrame < (int)videoFrames.size()) {
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

bool IsVideoFinished() {
    return videoFinished;
}

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

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

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

// ===== NEW: Update info button & overlay =====
void UpdateInfo() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float buttonWidth = 300;
    float buttonHeight = 80;

    Rectangle infoButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        screenHeight / 2.0f + buttonHeight * 2 + 50,
        buttonWidth,
        buttonHeight
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, infoButton);

    if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showInfo = true;
    }

    // Close button when info is showing
    if (showInfo) {
        Rectangle closeButton = {
            screenWidth / 2.0f - 75,
            screenHeight - 80.0f,
            150,
            50
        };
        if (CheckCollisionPointRec(mouse, closeButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            showInfo = false;
        }
    }
}

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
        WHITE
    );

    // ===== TITLE =====
    const char* title = "Curse of the Wings";
    int titleSize = 120;

    Font scaryFont = LoadFontEx("assets/font/Nosifer-Regular.ttf", 64, 0, 0);
    Vector2 textSize = MeasureTextEx(scaryFont, title, (float)titleSize, 2);

    float titleX = screenWidth / 2.0f - textSize.x / 2;
    float titleY = screenHeight / 2.0f - 200;
    float fadeAlpha = fminf(GetTime() * 0.5f, 1.0f);

    DrawTextEx(scaryFont, title, {titleX + 3, titleY + 3}, (float)titleSize, 0, BLACK);
    DrawTextEx(scaryFont, title, {titleX, titleY}, (float)titleSize, 0, Fade(Color{139, 0, 0, 255}, fadeAlpha));

    // ===== HIGH SCORE =====
    const char* scoreText = TextFormat("High Score: %d", highScore);
    int scoreFontSize = 40;

    Font scoreFont = LoadFontEx("assets/font/Quantico-Regular.ttf", 64, 0, 0);
    Vector2 scoreSize = MeasureTextEx(scoreFont, scoreText, (float)scoreFontSize, 0);

    float scoreX = screenWidth / 2.0f - scoreSize.x / 2;
    float scoreY = titleY + textSize.y + 40;

    DrawTextEx(scoreFont, scoreText, {scoreX, scoreY}, (float)scoreFontSize, 0, WHITE);

    // ===== PLAY BUTTON =====
    float buttonWidth = 300;
    float buttonHeight = 80;

    Rectangle playButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        scoreY + 50,   
        buttonWidth,
        buttonHeight
    };

    bool playHovered = CheckCollisionPointRec(GetMousePosition(), playButton);
    if (playHovered) {
        DrawRectangleRounded({playButton.x - 5, playButton.y - 5, playButton.width + 10, playButton.height + 10}, 0.3f, 10, Fade(MAROON, 0.5f));
    }
    Color playButtonColor = playHovered ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
    DrawRectangleRounded(playButton, 0.3f, 10, playButtonColor);

    Font gamefont = LoadFontEx("assets/font/Chewy-Regular.ttf", 64, 0, 0);
    const char* playText = "PLAY";
    int playFontSize = 50;
    Vector2 playTextSize = MeasureTextEx(gamefont, playText, (float)playFontSize, 0);
    DrawTextEx(gamefont, playText, {playButton.x + playButton.width / 2 - playTextSize.x / 2, playButton.y + playButton.height / 2 - playTextSize.y / 2}, (float)playFontSize, 0, WHITE);

    // ===== EXIT BUTTON =====
    Rectangle exitButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        screenHeight / 2.0f + buttonHeight + 30,
        buttonWidth,
        buttonHeight
    };

    bool exitHovered = CheckCollisionPointRec(GetMousePosition(), exitButton);
    if (exitHovered) {
        DrawRectangleRounded({exitButton.x - 5, exitButton.y - 5, exitButton.width + 10, exitButton.height + 10}, 0.3f, 10, Fade(MAROON, 0.5f));
    }
    Color exitButtonColor = exitHovered ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
    DrawRectangleRounded(exitButton, 0.3f, 10, exitButtonColor);

    const char* exitText = "EXIT";
    int exitFontSize = 50;
    Vector2 exitTextSize = MeasureTextEx(gamefont, exitText, (float)exitFontSize, 0);
    DrawTextEx(gamefont, exitText, {exitButton.x + exitButton.width / 2 - exitTextSize.x / 2, exitButton.y + exitButton.height / 2 - exitTextSize.y / 2}, (float)exitFontSize, 0, WHITE);

    // ===== NEW: INFO BUTTON =====
    Rectangle infoButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        screenHeight / 2.0f + buttonHeight * 2 + 50,
        buttonWidth,
        buttonHeight
    };

    bool infoHovered = CheckCollisionPointRec(GetMousePosition(), infoButton);
    if (infoHovered) {
        DrawRectangleRounded({infoButton.x - 5, infoButton.y - 5, infoButton.width + 10, infoButton.height + 10}, 0.3f, 10, Fade(MAROON, 0.5f));
    }
    Color infoButtonColor = infoHovered ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
    DrawRectangleRounded(infoButton, 0.3f, 10, infoButtonColor);

    const char* infoText = "INFO";
    int infoFontSize = 50;
    Vector2 infoTextSize = MeasureTextEx(gamefont, infoText, (float)infoFontSize, 0);
    DrawTextEx(gamefont, infoText, {infoButton.x + infoButton.width / 2 - infoTextSize.x / 2, infoButton.y + infoButton.height / 2 - infoTextSize.y / 2}, (float)infoFontSize, 0, WHITE);

    // ===== NEW: INFO OVERLAY =====
    if (showInfo) {
        // Dark background overlay
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));

        // Show the info image centered
        float imgScale = fminf((float)screenWidth / infoTexture.width, (float)(screenHeight - 100) / infoTexture.height);
        float imgW = infoTexture.width * imgScale;
        float imgH = infoTexture.height * imgScale;

        DrawTexturePro(
            infoTexture,
            {0, 0, (float)infoTexture.width, (float)infoTexture.height},
            {screenWidth / 2.0f - imgW / 2, 20, imgW, imgH},
            {0, 0},
            0.0f,
            WHITE
        );

        // Close button
        Rectangle closeButton = {
            screenWidth / 2.0f - 75,
            screenHeight - 80.0f,
            150,
            50
        };
        bool closeHovered = CheckCollisionPointRec(GetMousePosition(), closeButton);
        DrawRectangleRounded(closeButton, 0.3f, 10, closeHovered ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255});

        Vector2 closeTextSize = MeasureTextEx(gamefont, "CLOSE", 35, 0);
        DrawTextEx(gamefont, "CLOSE", {closeButton.x + closeButton.width / 2 - closeTextSize.x / 2, closeButton.y + closeButton.height / 2 - closeTextSize.y / 2}, 35, 0, WHITE);
    }
}