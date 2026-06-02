#include "intro.h"
#include "raylib.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <string>
#include <iostream>
#define BG2_TOTAL_FRAMES 41

static float introAlpha = 0.0f; // starts fully transparent
static bool fadeIn = true;
static int bg2FrameIndex = 0;
static bool bg2Finished = false;
static float bg2Alpha = 0.0f; // starts transparent
static Texture2D bg2Frames[BG2_TOTAL_FRAMES];
static float bg2FrameTimer = 0.0f;
static float bg2FrameDelay = 0.08f;

void InitBg2TransitionVideo()
{

    bg2FrameIndex = 0;
    bg2Finished = false;
    for (int i = 0; i < BG2_TOTAL_FRAMES; i++)
    {
        bg2Frames[i] = LoadTexture(TextFormat("assets/videos/nighttrans/%d.png", i + 1));
    }
}

void UpdateBg2TransitionVideo()
{
    if (bg2Finished)
        return;

    bg2FrameTimer += GetFrameTime(); // time since last frame
    if (bg2FrameTimer >= bg2FrameDelay)
    {
        bg2FrameTimer = 0.0f;
        bg2FrameIndex++;
        if (bg2FrameIndex >= BG2_TOTAL_FRAMES)
        {
            bg2Finished = true;
        }
    }
}

void DrawBg2TransitionVideo()
{
    if (!bg2Finished)
    {
        float alpha = GetBg2TransitionAlpha();
        Color fadeColor = WHITE;
        fadeColor.a = (unsigned char)(alpha * 255);
        DrawTexture(bg2Frames[bg2FrameIndex], 0, 0, fadeColor);
    }
}

float GetBg2TransitionProgress()
{
    return (float)bg2FrameIndex / (float)BG2_TOTAL_FRAMES;
}

float GetBg2TransitionAlpha()
{
    float progress = GetBg2TransitionProgress();

    // Fade in during first 30% of video
    if (progress < 0.2f)
    {
        return progress / 0.2f; // 0 → 1
    }
    // Stay fully visible until 90%
    else if (progress < 0.9f)
    {
        return 1.0f;
    }
    // Fade out during last 10%
    else
    {
        return (1.0f - progress) / 0.1f; // 1 → 0
    }
}

bool IsBg2TransitionFinished()
{
    return bg2Finished;
}

void UnloadBg2TransitionVideo()
{
    for (int i = 0; i < BG2_TOTAL_FRAMES; i++)
    {
        UnloadTexture(bg2Frames[i]);
    }
}

static std::vector<Texture2D> bg1Frames;
static int bg1FrameIndex = 0;
static float bg1FrameTimer = 0.0f;
static float bg1FrameDelay = 0.08f; // ~12 fps
static bool bg1Finished = false;

void InitBg1TransitionVideo()
{
    bg1Frames.clear();
    for (int i = 1; i <= 23; i++)
    {
        std::string filename = "assets/videos/daytrans/" + std::to_string(i) + ".png";
        std::cout << "Loading: " << filename << std::endl; // debug print
        Texture2D tex = LoadTexture(filename.c_str());
        if (tex.id == 0)
        {
            std::cout << "Failed to load: " << filename << std::endl;
        }
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        bg1Frames.push_back(tex);
    }
    bg1FrameIndex = 0;
    bg1FrameTimer = 0.0f;
    bg1Finished = false;
}

void UpdateBg1TransitionVideo()
{
    if (bg1Finished)
        return;

    bg1FrameTimer += GetFrameTime();
    if (bg1FrameTimer >= bg1FrameDelay)
    {
        bg1FrameIndex++;
        bg1FrameTimer = 0.0f;
    }

    if (bg1FrameIndex >= (int)bg1Frames.size())
    {
        bg1Finished = true;
    }
}

float GetBg1TransitionProgress()
{
    if (bg1Frames.empty())
        return 0.0f;
    return (float)bg1FrameIndex / (float)(bg1Frames.size() - 1);
}

void DrawBg1TransitionVideo()
{
    if (!bg1Finished && bg1FrameIndex < (int)bg1Frames.size())
    {
        Texture2D frame = bg1Frames[bg1FrameIndex];
        DrawTexturePro(
            frame,
            {0, 0, (float)frame.width, (float)frame.height},
            {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
            {0, 0},
            0.0f,
            WHITE);
    }
}

float GetBg1TransitionAlpha()
{
    float progress = GetBg1TransitionProgress();

    // Fade in during first 30% of video
    if (progress < 0.2f)
    {
        return progress / 0.2f; // 0 → 1
    }
    // Stay fully visible until 90%
    else if (progress < 0.9f)
    {
        return 1.0f;
    }
    // Fade out during last 10%
    else
    {
        return (1.0f - progress) / 0.1f; // 1 → 0
    }
}

bool IsBg1TransitionFinished()
{
    return bg1Finished;
}

void UnloadBg1TransitionVideo()
{
    for (auto &tex : bg1Frames)
    {
        UnloadTexture(tex);
    }
    bg1Frames.clear();
}

static Music introMusic;

static std::vector<Texture2D> videoFrames;
static int currentFrame = 0;
static float frameTimer = 0.0f;
static float frameDelay = 0.1f;
static bool videoFinished = false;


void InitIntroVideo()
{
    for (int i = 1; i <= 9; i++)
    {
        std::string filename = "assets/videos/TransIntro/" + std::to_string(i) + ".png";
        Texture2D tex = LoadTexture(filename.c_str());
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        videoFrames.push_back(tex);
    }
    currentFrame = 0;
    frameTimer = 0.0f;
    videoFinished = false;
}

void UpdateIntroVideo()
{
    if (videoFinished)
        return;

    frameTimer += GetFrameTime();
    if (frameTimer >= frameDelay)
    {
        currentFrame++;
        frameTimer = 0.0f;
    }

    if (currentFrame >= (int)videoFrames.size())
    {
        videoFinished = true;
    }
}

void DrawIntroVideo()
{
    if (!videoFinished && currentFrame < (int)videoFrames.size())
    {
        Texture2D frame = videoFrames[currentFrame];
        ClearBackground(BLACK);
        DrawTexturePro(
            frame,
            {0, 0, (float)frame.width, (float)frame.height},
            {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
            {0, 0},
            0.0f,
            WHITE);
    }
}

bool IsVideoFinished()
{
    return videoFinished;
}

void UnloadIntroVideo()
{
    for (auto &tex : videoFrames)
    {
        UnloadTexture(tex);
    }
    videoFrames.clear();
}

void InitIntroMusic()
{
    introMusic = LoadMusicStream("assets/audio/intro.mp3");
    PlayMusicStream(introMusic);
    SetMusicVolume(introMusic, 0.5f);
}

void UpdateIntroMusic()
{
    UpdateMusicStream(introMusic);
}

void StopIntroMusic()
{
    StopMusicStream(introMusic);
    UnloadMusicStream(introMusic);
}

// =====================================================
// GLOBALS
// =====================================================
static bool showInfo = false;
static int infoPage = 1;
static const int maxPages = 2;

static Texture2D infoTextures[2];
static Texture2D playButtonTex;
static Texture2D exitButtonTex;
static Texture2D infoButtonTex;
static Texture2D closeButtonTex;
static Texture2D nextButtonTex;
static Texture2D prevButtonTex;
static Font scoreFont;

// This is the instance of your struct
static IntroButtons introButtons;

// =====================================================
// BUTTON DRAW HELPER
// =====================================================
static void DrawButton(Texture2D tex, Rectangle rect, bool hovered)
{
    float scaleX = rect.width / (float)tex.width;
    float scaleY = rect.height / (float)tex.height;
    float scale  = (scaleX < scaleY) ? scaleX : scaleY;

    float drawWidth  = tex.width * scale;
    float drawHeight = tex.height * scale;

    Rectangle dest = {
        rect.x + (rect.width - drawWidth) / 2.0f,
        rect.y + (rect.height - drawHeight) / 2.0f,
        drawWidth,
        drawHeight
    };

   
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), dest);

    DrawTexturePro(tex,
                   {0,0,(float)tex.width,(float)tex.height},
                   dest,
                   {0,0},
                   0.0f,
                   isHovered ? WHITE : Fade(WHITE, 0.7f));
}


// =====================================================
// INIT / UNLOAD
// =====================================================
void InitIntro()
{
    playButtonTex  = LoadTexture("assets/buttons/play.png");
    exitButtonTex  = LoadTexture("assets/buttons/exit.png");
    infoButtonTex  = LoadTexture("assets/buttons/Binfo.png");
    closeButtonTex = LoadTexture("assets/buttons/close.png");
    nextButtonTex  = LoadTexture("assets/buttons/next.png");
    prevButtonTex  = LoadTexture("assets/buttons/prev.png");

    infoTextures[0] = LoadTexture("assets/images/info.png");
    infoTextures[1] = LoadTexture("assets/images/info2.png");

    scoreFont = LoadFontEx("assets/font/Quantico-Regular.ttf", 64, 0, 0);
}




void UnloadIntro()
{
    UnloadTexture(playButtonTex);
    UnloadTexture(exitButtonTex);
    UnloadTexture(infoButtonTex);
    UnloadTexture(closeButtonTex);
    UnloadTexture(nextButtonTex);
    UnloadTexture(prevButtonTex);

    for (int i = 0; i < maxPages; i++) UnloadTexture(infoTextures[i]);

    UnloadFont(scoreFont);
}


// =====================================================
// UPDATE INTRO
// =====================================================
int UpdateIntro()
{
    Vector2 mouse = GetMousePosition();

    if (CheckCollisionPointRec(mouse, introButtons.play) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        return 1; // Play
    }

    if (CheckCollisionPointRec(mouse, introButtons.exit) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        return 2; // Exit
    }

    if (!showInfo &&
        CheckCollisionPointRec(mouse, introButtons.info) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        showInfo = true;
        infoPage = 1; // reset to first page
        return 3; // Info opened
    }

    if (showInfo)
    {
        if (CheckCollisionPointRec(mouse, introButtons.close) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            showInfo = false;
            return 3; // Info closed
        }

        if (infoPage < maxPages &&
            CheckCollisionPointRec(mouse, introButtons.next) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            infoPage++;
        }

        if (infoPage > 1 &&
            CheckCollisionPointRec(mouse, introButtons.prev) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            infoPage--;
        }
    }

    return 0;
}


// =====================================================
// DRAW INTRO
// =====================================================
void SetupButtons(float scoreY, float scoreHeight)
{
    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    
    float btnWidth   = 600.0f;   
    float btnHeight  = 150.0f;  
    float gap        = 0.0f;   
    float centerX    = screenWidth/2.0f - btnWidth/2.0f;

    
    introButtons.play = { centerX, scoreY + scoreHeight + gap, btnWidth, btnHeight };

    introButtons.info = { centerX, introButtons.play.y + btnHeight + gap, btnWidth, btnHeight };

    introButtons.exit = { centerX, introButtons.info.y + btnHeight + gap, btnWidth, btnHeight };


    float smallW = 170.0f;
    float smallH = 75.0f;

    introButtons.close = { screenWidth/2.0f - smallW/2.0f, screenHeight - smallH - 40.0f, smallW, smallH };
    introButtons.next  = { screenWidth/2.0f + 200.0f, screenHeight - smallH - 40.0f, smallW, smallH };
    introButtons.prev  = { screenWidth/2.0f - 200.0f - smallW, screenHeight - smallH - 40.0f, smallW, smallH };
}



void DrawIntro(int highScore, Texture2D introTex, Texture2D titleTex)
{
    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    InitIntro();

    // Background
    DrawTexturePro(introTex,
                   {0,0,(float)introTex.width,(float)introTex.height},
                   {0,0,(float)screenWidth,(float)screenHeight},
                   {0,0}, 0.0f, WHITE);


    // Title
    float titleW = screenWidth * 0.50f;
    float titleH = screenHeight * 0.20f;
    float startY = screenHeight * 0.15f;
    Rectangle titleRect = {screenWidth/2.0f - titleW/2.0f, startY, titleW, titleH};

    
    DrawTexturePro(titleTex, {0,0,(float)titleTex.width,(float)titleTex.height},
                   {titleRect.x+4, titleRect.y+4, titleRect.width, titleRect.height},
                   {0,0}, 0.0f, BLACK);


    DrawTexturePro(titleTex, {0,0,(float)titleTex.width,(float)titleTex.height},
                   titleRect, {0,0}, 0.0f, WHITE);



    // High Score
    const char* scoreText = TextFormat("High Score: %d", highScore);
    int scoreFontSize = 40;

    Vector2 scoreSize = MeasureTextEx(scoreFont, scoreText, (float)scoreFontSize, 0);

    float scoreX = screenWidth/2.0f - scoreSize.x/2.0f;
    float scoreY = titleRect.y + titleRect.height + 20.0f;
    DrawTextEx(scoreFont, scoreText, {scoreX, scoreY}, (float)scoreFontSize, 0, WHITE);

    SetupButtons(scoreY, (float)scoreFontSize);


    // Draw buttons
    DrawButton(playButtonTex, introButtons.play, CheckCollisionPointRec(GetMousePosition(), introButtons.play));
    DrawButton(infoButtonTex, introButtons.info, CheckCollisionPointRec(GetMousePosition(), introButtons.info));
    DrawButton(exitButtonTex, introButtons.exit, CheckCollisionPointRec(GetMousePosition(), introButtons.exit));


    // Info overlay
    if (showInfo)
    {
        DrawRectangle(0,0,screenWidth,screenHeight, Fade(BLACK,0.85f));

        float overlayW = screenWidth * 0.8f;
        float overlayH = screenHeight * 0.75f;
        Rectangle infoRect = { screenWidth/2.0f - overlayW/2.0f, screenHeight/2.0f - overlayH/2.0f, overlayW, overlayH };

        DrawTexturePro(infoTextures[infoPage-1],
                       {0,0,(float)infoTextures[infoPage-1].width,(float)infoTextures[infoPage-1].height},
                       infoRect, {0,0}, 0.0f, WHITE);

        DrawButton(closeButtonTex, introButtons.close, CheckCollisionPointRec(GetMousePosition(), introButtons.close));
        if (infoPage < maxPages)
            DrawButton(nextButtonTex, introButtons.next, CheckCollisionPointRec(GetMousePosition(), introButtons.next));
        if (infoPage > 1)
            DrawButton(prevButtonTex, introButtons.prev, CheckCollisionPointRec(GetMousePosition(), introButtons.prev));
    }
}
