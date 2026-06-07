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

static Font scoreFont = { 0 };
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
static Sound buttonClickSound;
static Sound hoverSound;
static Texture2D groupLogoTex;
static float logoTimer = 0.0f;
static float logoFadeIn = 1.5f;   // seconds to fade in
static float logoHold = 2.0f;     // seconds to hold full opacity
static float logoFadeOut = 1.5f;  // seconds to fade out
static bool logoDone = false;

void InitGroupLogo() {
    groupLogoTex = LoadTexture("assets/images/group_logo.png"); 
    logoTimer = -0.5f;  
    logoDone = false;
}

bool UpdateGroupLogo() {
    if (logoDone) return true;
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;  
    logoTimer += dt;
    if (logoTimer < 0) return false;  
    float total = logoFadeIn + logoHold + logoFadeOut;
    if (logoTimer >= total) {
        logoDone = true;
        return true;
    }
    return false;
}

void DrawGroupLogo() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float alpha = 0.0f;
    if (logoTimer >= 0) {
        if (logoTimer < logoFadeIn) {
            alpha = logoTimer / logoFadeIn;
        } else if (logoTimer < logoFadeIn + logoHold) {
            alpha = 1.0f;
        } else {
            float t = logoTimer - logoFadeIn - logoHold;
            alpha = 1.0f - (t / logoFadeOut);
        }
    }

    ClearBackground(BLACK);
    DrawTexturePro(
        groupLogoTex,
        {0, 0, (float)groupLogoTex.width, (float)groupLogoTex.height},
        {0, 0, (float)sw, (float)sh},
        {0, 0}, 0.0f,
        Fade(WHITE, alpha)
    );
}

void UnloadGroupLogo() {
    UnloadTexture(groupLogoTex);
}

static Texture2D titleLogoTex;
static float titleLogoTimer = -0.5f;
static float titleLogoFadeIn = 1.5f;
static float titleLogoHold = 2.0f;
static float titleLogoFadeOut = 1.5f;
static bool titleLogoDone = false;

void InitTitleLogo() {
    titleLogoTex = LoadTexture("assets/images/title_logo.png"); 
    titleLogoTimer = -0.5f;
    titleLogoDone = false;
}

bool UpdateTitleLogo() {
    if (titleLogoDone) return true;
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;
    titleLogoTimer += dt;
    if (titleLogoTimer < 0) return false;
    float total = titleLogoFadeIn + titleLogoHold + titleLogoFadeOut;
    if (titleLogoTimer >= total) {
        titleLogoDone = true;
        return true;
    }
    return false;
}

void DrawTitleLogo() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float alpha = 0.0f;
    if (titleLogoTimer >= 0) {
        if (titleLogoTimer < titleLogoFadeIn) {
            alpha = titleLogoTimer / titleLogoFadeIn;
        } else if (titleLogoTimer < titleLogoFadeIn + titleLogoHold) {
            alpha = 1.0f;
        } else {
            float t = titleLogoTimer - titleLogoFadeIn - titleLogoHold;
            alpha = 1.0f - (t / titleLogoFadeOut);
        }
    }

    ClearBackground(BLACK);
    DrawTexturePro(
        titleLogoTex,
        {0, 0, (float)titleLogoTex.width, (float)titleLogoTex.height},
        {0, 0, (float)sw, (float)sh},
        {0, 0}, 0.0f,
        Fade(WHITE, alpha)
    );
}

void UnloadTitleLogo() {
    UnloadTexture(titleLogoTex);
}

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

// STORYLINE IMPLEMENTATION
     static std::vector<Texture2D> storylineFrames;
     static int currentStoryFrame = 0;
     static float storyFrameTimer = 0.0f;
     static float storyFrameDuration = 5.0f; // 5 seconds per image
     static float storyFadeDuration = 1.0f;  // 1 second fade in/out
     static bool storylineFinished = false;

     void InitScoreFont()
    {
        if (scoreFont.texture.id == 0)
            scoreFont = LoadFontEx("assets/font/Quantico-Regular.ttf", 64, 0, 0);
    }
     
     void InitStoryline()
     {
         storylineFrames.clear();
         for (int i = 1; i <= 20; i++)
         {
             std::string filename = "assets/videos/storyline/" + std::to_string(i) + ".png";
             Texture2D tex = LoadTexture(filename.c_str());
             if (tex.id != 0)
             {
                 SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
                 storylineFrames.push_back(tex);
             }
         }
         currentStoryFrame = 0;
         storyFrameTimer = 0.0f;
         storylineFinished = false;
     }
     
     bool UpdateStoryline()
     {
         if (storylineFinished || storylineFrames.empty()) return true;
     
         storyFrameTimer += GetFrameTime();
     
         // Skip functionality
         if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
         {
             storylineFinished = true;
             return true;
         }
     
         if (storyFrameTimer >= storyFrameDuration)
         {
             storyFrameTimer = 0.0f;
             currentStoryFrame++;
             if (currentStoryFrame >= (int)storylineFrames.size())
             {
                 storylineFinished = true;
                 return true;
             }
         }
     
         return false;
     }

     void DrawStoryline()
    {
        if (storylineFinished || storylineFrames.empty()) return;

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        float alpha = storyFrameTimer / storyFadeDuration;
        if (alpha > 1.0f) alpha = 1.0f;

        ClearBackground(BLACK);
        if (currentStoryFrame > 0 && alpha < 1.0f)
        {
            Texture2D prevFrame = storylineFrames[currentStoryFrame - 1];
            DrawTexturePro(
                prevFrame,
                {0, 0, (float)prevFrame.width, (float)prevFrame.height},
                {0, 0, (float)sw, (float)sh},
                {0, 0}, 0.0f, WHITE
            );
        }

        Texture2D frame = storylineFrames[currentStoryFrame];
        DrawTexturePro(
            frame,
            {0, 0, (float)frame.width, (float)frame.height},
            {0, 0, (float)sw, (float)sh},
            {0, 0}, 0.0f,
            Fade(WHITE, alpha)
        );
        
            // skip button
            const char* skipText = "SKIP (SPACE)";
        float fontSize = 30.0f;

        Vector2 textSize = MeasureTextEx(scoreFont, skipText, fontSize, 0);

        float padX = 10.0f, padY = 8.0f;
        Rectangle skipBtn = {
            (float)sw - textSize.x - padX * 2 - 30,
            (float)sh - textSize.y - padY * 2 - 30,
            textSize.x + padX * 2,
            textSize.y + padY * 2
        };

        bool hover = CheckCollisionPointRec(GetMousePosition(), skipBtn);
        DrawRectangleRounded(skipBtn, 0.3f, 6, Fade(hover ? GRAY : DARKGRAY, 0.6f));

        // Shadow
        DrawTextEx(scoreFont, skipText,
            {skipBtn.x + padX + 2, skipBtn.y + padY + 2},
            fontSize, 0, Fade(BLACK, 0.5f));
        // Text
        DrawTextEx(scoreFont, skipText,
            {skipBtn.x + padX, skipBtn.y + padY},
            fontSize, 0, WHITE);

        if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            storylineFinished = true;
        }
    }
     
     void UnloadStoryline()
     {
         for (auto &tex : storylineFrames)
         {
             UnloadTexture(tex);
         }
         storylineFrames.clear();
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
static const int maxPages = 3;
static Texture2D infoTextures[3];
static Texture2D playButtonTex;
static Texture2D exitButtonTex;
static Texture2D infoButtonTex;
static Texture2D closeButtonTex;
static Texture2D nextButtonTex;
static Texture2D prevButtonTex;
static Texture2D WallTex;

// This is the instance of your struct
static IntroButtons introButtons;

// =====================================================
// BUTTON DRAW HELPER
// =====================================================
static void DrawButton(Texture2D tex, Rectangle rect, bool *prevHovered)
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
    
    if (isHovered && prevHovered && !(*prevHovered)) PlaySound(hoverSound);
    if (prevHovered) *prevHovered = isHovered;

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
    infoButtonTex  = LoadTexture("assets/buttons/info.png");
    closeButtonTex = LoadTexture("assets/buttons/close.png");
    nextButtonTex  = LoadTexture("assets/buttons/next.png");
    prevButtonTex  = LoadTexture("assets/buttons/prev.png");
    WallTex = LoadTexture("assets/buttons/wall1.png");

    infoTextures[0] = LoadTexture("assets/images/info.png");
    infoTextures[1] = LoadTexture("assets/images/info2.png");
    infoTextures[2] = LoadTexture("assets/images/info3.png");

    if (scoreFont.texture.id == 0)
    scoreFont = LoadFontEx("assets/font/Quantico-Regular.ttf", 64, 0, 0);
    buttonClickSound = LoadSound("assets/sounds/button_click.mp3");
    hoverSound = LoadSound("assets/sounds/hoverSound.mp3");
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
    UnloadSound(buttonClickSound);
    UnloadSound(hoverSound);
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
        PlaySound(buttonClickSound);
        return 1; // Play
    }

    if (CheckCollisionPointRec(mouse, introButtons.exit) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        PlaySound(buttonClickSound);
        return 2; // Exit
    }

    if (!showInfo &&
        CheckCollisionPointRec(mouse, introButtons.info) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        PlaySound(buttonClickSound);
        showInfo = true;
        infoPage = 1; // reset to first page
        return 3; // Info opened
    }

    if (showInfo)
    {
        if (CheckCollisionPointRec(mouse, introButtons.close) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            PlaySound(buttonClickSound);
            showInfo = false;
            return 3; // Info closed
        }

        if (infoPage < maxPages &&
            CheckCollisionPointRec(mouse, introButtons.next) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            PlaySound(buttonClickSound);
            infoPage++;
        }

        if (infoPage > 1 &&
            CheckCollisionPointRec(mouse, introButtons.prev) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            PlaySound(buttonClickSound);
            infoPage--;
        }
    }

    return 0;
}


// =====================================================
// DRAW INTRO (Right-Side Menu Alignment Fixed)
// =====================================================
void SetupButtons(Rectangle wallRect, float startY)
{
    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float btnWidth   = 340.0f;   
    float btnHeight  = 85.0f;  
    float gap        = 18.0f; 

    float buttonX = wallRect.x + (wallRect.width / 2.0f) - (btnWidth / 2.0f);

    introButtons.play = { buttonX, startY, btnWidth, btnHeight };
    introButtons.info = { buttonX, introButtons.play.y + btnHeight + gap, btnWidth, btnHeight };
    introButtons.exit = { buttonX, introButtons.info.y + btnHeight + gap, btnWidth, btnHeight };

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
    
    DrawTexturePro(introTex,
                   {0,0,(float)introTex.width,(float)introTex.height},
                   {0,0,(float)screenWidth,(float)screenHeight},
                   {0,0}, 0.0f, WHITE);

    
    float wallWidth = screenWidth * 0.36f; 
    float wallX = screenWidth - wallWidth; 
    Rectangle wallRect = { wallX, 0.0f, wallWidth, (float)screenHeight };

    DrawTexturePro(WallTex,
                   {0, 0, (float)WallTex.width, (float)WallTex.height},
                   wallRect,
                   {0,0}, 0.0f, WHITE);


    // =====================================================

    // 4. Optimal High Score Placement
    const char* scoreText = TextFormat("High Score: %d", highScore);
    int scoreFontSize = 45; 

    Vector2 scoreSize = MeasureTextEx(scoreFont, scoreText, (float)scoreFontSize, 0);

    float scoreX = wallRect.x + (wallRect.width / 2.0f) - (scoreSize.x / 2.0f);
    float scoreY = screenHeight * 0.41f; 
    
    
    DrawTextEx(scoreFont, scoreText, {scoreX + 2, scoreY + 2}, (float)scoreFontSize, 0, BLACK);
    DrawTextEx(scoreFont, scoreText, {scoreX, scoreY}, (float)scoreFontSize, 0, WHITE);


    
    float buttonStartY = scoreY + scoreSize.y + 30.0f;
    SetupButtons(wallRect, buttonStartY);

    static bool hPlay = false, hInfo = false, hExit = false;
    static bool hClose = false, hNext = false, hPrev = false;

    DrawButton(playButtonTex, introButtons.play, &hPlay);
    DrawButton(infoButtonTex, introButtons.info, &hInfo);
    DrawButton(exitButtonTex, introButtons.exit, &hExit);

    
    if (showInfo)
    {
        DrawRectangle(0,0,screenWidth,screenHeight, Fade(BLACK,0.85f));

        Texture2D currentInfo = infoTextures[infoPage-1];
        float maxW = screenWidth * 0.8f;
        float maxH = screenHeight * 0.75f;
        
        float scale = fminf(maxW / currentInfo.width, maxH / currentInfo.height);
        float overlayW = currentInfo.width * scale;
        float overlayH = currentInfo.height * scale;

        Rectangle infoRect = { screenWidth/2.0f - overlayW/2.0f, screenHeight/2.0f - overlayH/2.0f, overlayW, overlayH };

        DrawTexturePro(currentInfo,
                       {0,0,(float)currentInfo.width,(float)currentInfo.height},
                       infoRect, {0,0}, 0.0f, WHITE);

        DrawButton(closeButtonTex, introButtons.close, &hClose);
        if (infoPage < maxPages)
            DrawButton(nextButtonTex, introButtons.next, &hNext);
        else hNext = false;

        if (infoPage > 1)
            DrawButton(prevButtonTex, introButtons.prev, &hPrev);
        else hPrev = false;
    }
}