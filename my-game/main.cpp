#include "intro.h"
#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cmath>
#include "raymath.h"
#include <string>
using namespace std;

enum GameState
{
    MENU,
    TRANSITION,
    PLAYING,
    BG1_TRANSITION,
    BG2_TRANSITION,
    TROLL_VIDEO,
    PAUSED,
    GAMEOVER_ANIM
};

enum ItemType
{
    BOMB,
    BABY,
    GARLIC,
    BANDAGE,
    MEDKIT,
    CHILI,
    POO,
    SALT,
    HEART,
    BLOOD,
    POISON,
    MEAT,
    DICE,
    MUSHROOM,
    PRIZE,
    STAR,
    TROLLFACE,
    ATAY
};

struct Item
{
    Rectangle rect; // items
    int type;
    float speed;
    float acceleration; // added for more natural, varied falling speeds
    float lateralSpeed; // horizontal movement (sway)
    float sinTime;      // used for sine-wave movement
    bool isIllusion;    // if true, it will change its type mid-fall
    int illusionTarget; // the type it will change into
    bool active;
};

struct PopEffect {
    Vector2 pos;
    float timer;
    float maxTime;
    Color color;
};

struct Warning {
    float x;
    float timer;
    int type;
    bool active;
};

enum Difficulty
{
    EASY,
    MEDIUM,
    HARD
};

enum EventType
{
    NONE,
    SWAP_CONTROLS,
    LOW_GRAVITY,
    SPEED_BOOST,
    SLOW_BOOST,
    LUCKY_PARTY,
    MISFORTUNE,
    INVERTED_SCREEN,
    FOG_BLIND,
    EARTHQUAKE
};


enum PlayerAnim { 
    IDLE,
    WALK_LEFT,
    WALK_RIGHT,
    JUMP 
};

enum MananAnim {
    FLY_FRONT,
    FLY_RIGHT,
    FLY_LEFT
};



// Animation
PlayerAnim currentAnim = IDLE;
MananAnim currentMananAnim = FLY_FRONT;


int playerFrame = 0;
float pframeTimer = 0.0f;
float pframeDelay = 0.15f;


// stores every frame (Texture2D frame1 and so on)
vector<Texture2D> videoFrames;
int currentFrame = 0;
float frameTimer = 0;
Sound trollSound;
Music bgMusic;

// Global Event History to prevent repeated events
EventType lastEvent = NONE;
EventType secondLastEvent = NONE;

struct FloatingText {
    Vector2 pos;
    string text;
    float timer;
    Color color;
};
vector<FloatingText> floatingTexts;

int main()
{
    SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Raylib - Wings of the Curse");

    // load sound
    InitAudioDevice();
    trollSound = LoadSound("assets/sounds/trollFace.mp3");

    // music
    bgMusic = LoadMusicStream("assets/sounds/bg_music.mp3");
    SetMusicVolume(bgMusic, 1.0f);
    PlayMusicStream(bgMusic);

    // bat music
    Music batmusic= LoadMusicStream("assets/sounds/bat flying.mp3");
    SetMusicVolume(batmusic, 1.0f);
    PlayMusicStream(batmusic);

    //game over music
    Music gameOverMusic = LoadMusicStream("assets/sounds/gameover_music.mp3"); // game over bg music

    // load intro music
    Music introMusic = LoadMusicStream("assets/sounds/intro.mp3");
    SetMusicVolume(introMusic, 0.5f);
    PlayMusicStream(introMusic);

    // load frames
    for (int i = 1; i <= 110; i++) 
    videoFrames.push_back(LoadTexture(TextFormat("assets/videos/trollFace/ezgif-frame-%03d.png", i)));

    
    // MEME POP-UP
    struct MemePop {
        Texture2D tex;
        Vector2 pos;
        float speed;
        bool active;
        int lastIndex;
        int soundIndex;
    };
    vector<Texture2D> memeTextures;
    vector<Sound> memeSounds;
    MemePop currentMeme = { {0}, {0, 0}, 0, false, -1, -1 };
    float memeSpawnTimer = 0.0f;

    // RESEARCH TRACKING
    float totalTimePlayed = 0.0f;
    int eventsSurvived = 0;
    int totalItemsCollected = 0;
    float chaosLevel = 0.0f; // 0.0 to 1.0 based on difficulty and score progress

    // load images
    // bg
    Texture2D bgTex = LoadTexture("assets/images/bg.png");
    Texture2D groundTex = LoadTexture("assets/images/ground.png");
    Texture2D wallTex = LoadTexture("assets/images/wall.jpg");
    Texture2D bgEasy   = LoadTexture("assets/images/easyy(1).png");
    Texture2D bgMedium = LoadTexture("assets/images/mediumm.png");
    Texture2D bgHard   = LoadTexture("assets/images/hardd.png");
    Texture2D introTex = LoadTexture("assets/images/intro2.png");
    Texture2D gameOverBg = LoadTexture("assets/images/gameoverbg.png");
     
    InitInfoTexture();

    // MEME POP-UP TEXTURES
    // ADD MORE MEMES HERE: Just add more textures to the memeTextures vector
    memeTextures.push_back(LoadTexture("assets/images/kapre.png"));
    memeTextures.push_back(LoadTexture("assets/images/broomwitch.png"));
    memeTextures.push_back(LoadTexture("assets/images/witch.png"));
    memeTextures.push_back(LoadTexture("assets/images/mermaid.png"));
    memeTextures.push_back(LoadTexture("assets/images/minion.png"));
    memeTextures.push_back(LoadTexture("assets/images/tiyanak.png"));
    memeTextures.push_back(LoadTexture("assets/images/tikbalang.png"));
    memeTextures.push_back(LoadTexture("assets/images/dwende.png"));


    // MEME SOUNDS
    memeSounds.push_back(LoadSound("assets/sounds/getOut.mp3"));
    memeSounds.push_back(LoadSound("assets/sounds/plankton.mp3"));
    memeSounds.push_back(LoadSound("assets/sounds/auugh.mp3"));
    memeSounds.push_back(LoadSound("assets/sounds/cookedDog.mp3"));
    memeSounds.push_back(LoadSound("assets/sounds/goodbye.mp3"));

    // items
    Texture2D bombTex = LoadTexture("assets/images/bomb.png");
    Texture2D babyTex = LoadTexture("assets/images/baby.png");
    Texture2D medkitTex = LoadTexture("assets/images/med_kit.png");
    Texture2D bandageTex = LoadTexture("assets/images/bandage.png");
    Texture2D trollFaceTex = LoadTexture("assets/images/trollFace.png");
    Texture2D chiliTex = LoadTexture("assets/images/chili.png");
    Texture2D garlicTex = LoadTexture("assets/images/garlic.png");
    Texture2D poisonTex = LoadTexture("assets/images/poison.png");
    Texture2D saltTex = LoadTexture("assets/images/salt.png");
    Texture2D potionBandageTex = LoadTexture("assets/images/potion-bandage.png");
    Texture2D potionMedkitTex = LoadTexture("assets/images/potion-medkit.png");
    Texture2D bloodTex = LoadTexture("assets/images/blood.png");
    Texture2D pooTex = LoadTexture("assets/images/poo.png");
    Texture2D heartTex = LoadTexture("assets/images/heart.png");
    Texture2D atayTex = LoadTexture("assets/images/atay.png");
    Texture2D diceTex = LoadTexture("assets/images/dice.png");
    Texture2D mushroomTex = LoadTexture("assets/images/mushroom.png");
    Texture2D hpTex = LoadTexture("assets/images/hearty.png");

    // font
    Font nosifer = LoadFontEx("assets/font/Nosifer-Regular.ttf", 64, 0, 0);
    Font gamefont = LoadFontEx("assets/font/Chewy-Regular.ttf", 64, 0, 0);
    Font tinyFont = LoadFontEx("assets/font/Jersey10-Regular.ttf", 64, 0, 0);

    //character
    Texture2D playerTex = LoadTexture("assets/character/human/player.png");
    Texture2D player1Tex = LoadTexture("assets/character/human/invisible.png");

    Texture2D RwalkFrames[6]; 
     RwalkFrames[0] = LoadTexture("assets/character/human/right/1.png");
     RwalkFrames[1] = LoadTexture("assets/character/human/right/2.png");
     RwalkFrames[2] = LoadTexture("assets/character/human/right/3.png");
    RwalkFrames[3] = LoadTexture("assets/character/human/right/4.png");
    RwalkFrames[4] = LoadTexture("assets/character/human/right/5.png");
    RwalkFrames[5] = LoadTexture("assets/character/human/right/6.png");

    Texture2D LwalkFrames[6]; 
     LwalkFrames[0] = LoadTexture("assets/character/human/left/1.png");
     LwalkFrames[1] = LoadTexture("assets/character/human/left/2.png");
     LwalkFrames[2] = LoadTexture("assets/character/human/left/3.png");
    LwalkFrames[3] = LoadTexture("assets/character/human/left/4.png");
    LwalkFrames[4] = LoadTexture("assets/character/human/left/5.png");
    LwalkFrames[5] = LoadTexture("assets/character/human/left/6.png");

    Texture2D JumpFrames[6]; 
     JumpFrames[0] = LoadTexture("assets/character/human/jump/1.png");
     JumpFrames[1] = LoadTexture("assets/character/human/jump/2.png");
     JumpFrames[2] = LoadTexture("assets/character/human/jump/3.png");
     JumpFrames[3] = LoadTexture("assets/character/human/jump/4.png");
     JumpFrames[4] = LoadTexture("assets/character/human/jump/5.png");


    Texture2D FlyFrames[6];   // front
        FlyFrames[0] = LoadTexture("assets/character/manananggal/front/1.png");
        FlyFrames[1] = LoadTexture("assets/character/manananggal/front/2.png");
        FlyFrames[2] = LoadTexture("assets/character/manananggal/front/3.png");
        FlyFrames[3] = LoadTexture("assets/character/manananggal/front/4.png");
        FlyFrames[4] = LoadTexture("assets/character/manananggal/front/5.png");
        FlyFrames[5] = LoadTexture("assets/character/manananggal/front/6.png");

    Texture2D RflyFrames[6];  // right
        RflyFrames[0] = LoadTexture("assets/character/manananggal/right/1.png");
        RflyFrames[1] = LoadTexture("assets/character/manananggal/right/2.png");
        RflyFrames[2] = LoadTexture("assets/character/manananggal/right/3.png");
        RflyFrames[3] = LoadTexture("assets/character/manananggal/right/4.png");
        RflyFrames[4] = LoadTexture("assets/character/manananggal/right/5.png");
        RflyFrames[5] = LoadTexture("assets/character/manananggal/right/6.png");

    Texture2D LflyFrames[6];  // left
        LflyFrames[0] = LoadTexture("assets/character/manananggal/left/1.png");
        LflyFrames[1] = LoadTexture("assets/character/manananggal/left/2.png");
        LflyFrames[2] = LoadTexture("assets/character/manananggal/left/3.png");
        LflyFrames[3] = LoadTexture("assets/character/manananggal/left/4.png");
        LflyFrames[4] = LoadTexture("assets/character/manananggal/left/5.png");
        LflyFrames[5] = LoadTexture("assets/character/manananggal/left/6.png");
        
   

    srand(time(NULL));
    SetTargetFPS(60); // 60fps 1sec/60frame

    // VARIABLES-----------------------------------------

    GameState state = MENU;
    // player
    Rectangle player;
    player.width = 300;
    player.height = 300;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // CAMERA
    player.x = (screenWidth - player.width) / 2;
    player.y = screenHeight * 0.75f;

    //INVERTED SCREEN
    bool invertedScreen = false;

    bool showPauseMenu = false;

    // SCORE & HEALTH
    int score = 0;
    int highScore = 0;
    int hp = 3;

    // ITEMS
    vector<Item> items;
    vector<PopEffect> popEffects;
    float spawnTimer = 0; // time has spawned

    // COMBO
    int combo = 0;
    float comboTime = 0;
    int comboPop = 0;
    bool comboBroken = false;
    float comboBrokenTimer = 0;
    
    // SHAKE EFFECT
    float shakeTime = 0;  // how long screen shakes
    float shakePower = 0; // strength of shake
    float hitFlash = 0;   // red flash when damaged

    // game over
    float gameOverAnimTimer = 0.0f;
    float gameOverFlash = 0.0f;
    Sound gameOverSound = LoadSound("assets/sounds/game_over.wav"); // not final?
    Sound pitSound = LoadSound("assets/sounds/pit_open.mp3");

    //extra 
    float medkitCooldown = 0;
    static bool prizeSpawn = false;


    // EVENTS----------------------------------------------
    EventType currentEvent = NONE;
    EventType secondEvent = NONE; // for HARD MODE extra event

    float eventTimer = 0.0f;
    float eventCooldown = 15.0f; // increased initial cooldown for fairer start
    float eventWarningTimer = 0.0f; // used to show "Warning" before an event
    
    // FOG EFFECT
    float fogAlpha = 0.0f;
    float fogTimer = 0.0f;
    bool fogActive = false;
    bool fogFadingOut = false;

    // EARTHQUAKE
    bool quakeActive = false;
    float quakeTimer = 0.0f;
    //pit 
    vector<Rectangle> pits;
    vector<float> pitWidths;
    vector<float> pitCenters;
    vector<float> pitOpens;
    bool pitCreated = false; //control creation
    bool pitSoundPlayed = false;
    int pitCount = 1; // d more pits, d merrier

    float pitAlpha = 1;
    bool fallingInPit = false;

    float glitchTimer = 0.0f;
    bool glitchActive = false;

    // MOVEMENTS
    float velocityX = 0; // player movement momentum

    // slow effect
    float slowTime = 0;
    float move = 1.0f;
    float baseMove = 1.0f;
    float velocityY = 0; // vertical speed
    float slowTimer = 0.0f;
    // jump
    float gravity = 1800.0f;   // pull down
    float jumpForce = -700.0f; // jump strength (negative = up)
    bool isGrounded = true;
    // speed boost
    float speedBoostTimer = 0.0f;
    // chili
    float chiliBoost = 1.0f;
    float eventBoost = 1.0f;

    // TEXTS POP UPS
    bool showStarText = false;
    bool showMinusText = false;
    bool showSlowText = false;
    bool showComboText = false;
    // timers
    float starTextTimer = 0;
    float minusTextTimer = 0;
    float slowTextTimer = 0;


    Difficulty diff = EASY;
    Difficulty lastDiff = EASY; // track difficulty changes for grace periods
    bool bg1Triggered = false;
    bool bg2Triggered = false;

    // camera
    Camera2D camera = {0};
    camera.offset = {
        screenWidth / 2.0f,
        screenHeight * 0.75f};
    camera.target = {
        player.x + player.width / 2,
        player.y + player.height / 2
    };
    camera.rotation = 0.0f;
    camera.zoom = 1.30f;

    InitBg1TransitionVideo();
    InitBg2TransitionVideo();

    // EXTRA SAFETY & FEEDBACK
    float hitTimer = 0.0f; // grace period after taking damage
    float nearMissTimer = 0.0f;
    float milestoneCelebrationTimer = 0.0f;
    vector<Warning> activeWarnings;

    // JUICE & FEEL VARIABLES
    Vector2 squashStretch = { 1.0f, 1.0f };
    float playerRotation = 0.0f;
    float hitStopTimer = 0.0f;
    float landSquashTimer = 0.0f;

    while (!WindowShouldClose())
    {
    
    

        // MENU-----------------------------------------------
        if (state == MENU)
        {
            UpdateMusicStream(introMusic);

            if (UpdateIntro()) 
            {
                StopMusicStream(introMusic);
                UnloadMusicStream(introMusic);
                InitIntroVideo();
                state = TRANSITION;
            }

            if (UpdateExit())
            {
                StopMusicStream(introMusic);
                UnloadMusicStream(introMusic);
                break;
            }

            UpdateInfo();
            DrawIntro(highScore, introTex);
        }

        else if (state == TRANSITION) {
            UpdateMusicStream(batmusic);
             UpdateIntroVideo();
             BeginDrawing();
             ClearBackground(BLACK);
             DrawIntroVideo();
             EndDrawing();


             if (IsVideoFinished()) 
             {
             UnloadIntroVideo();
             PlayMusicStream(bgMusic);  
             state = PLAYING;
             }
         }

            else if (state == BG1_TRANSITION) {
        UpdateBg1TransitionVideo();
        glitchTimer += GetFrameTime();   // ADD

        BeginDrawing();
        ClearBackground(BLACK);
        DrawBg1TransitionVideo();

        // ADD THIS ENTIRE BLOCK:
        if (glitchTimer < 0.6f) {
            float flicker = (float)(rand() % 100) / 100.0f;
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f + flicker * 0.4f));

            int sliceCount = 6 + rand() % 6;
            for (int i = 0; i < sliceCount; i++) {
                int sliceY      = rand() % screenHeight;
                int sliceHeight = 2 + rand() % 18;
                int sliceOffset = (rand() % 80) - 40;
                float sliceAlpha = 0.3f + (float)(rand() % 60) / 100.0f;
                DrawRectangle(sliceOffset, sliceY, screenWidth, sliceHeight, Fade(BLACK, sliceAlpha));
                if (rand() % 3 == 0) {
                    Color fringe = (rand() % 2 == 0) ? Color{180, 0, 0, 80} : Color{0, 180, 180, 60};
                    DrawRectangle(sliceOffset + (rand() % 20 - 10), sliceY, screenWidth, sliceHeight / 2, fringe);
                }
            }
            for (int y = 0; y < screenHeight; y += 4)
                DrawRectangle(0, y, screenWidth, 1, Fade(BLACK, 0.25f));
            if (glitchTimer < 0.12f)
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f * (1.0f - glitchTimer / 0.12f)));
        }

        EndDrawing();

       if (IsBg1TransitionFinished()) {
            float fadeAlpha = GetBg1TransitionAlpha(); 
            Color fadeColor = Fade(BLACK, fadeAlpha);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), fadeColor);

            // Reset player to center
            player.x = (screenWidth - player.width) / 2;
            player.y = screenHeight * 0.75f;
            velocityX = 0;
            velocityY = 0;
            isGrounded = true;

            // Reset items and spawn timer so they fall fresh from top
            items.clear();
            spawnTimer = 0;

            // Reset camera to follow new player position
            camera.target = { player.x + player.width / 2, player.y + player.height / 2 };
            camera.rotation = 0.0f;

            state = PLAYING;
        }

        continue; 
        }


         else if (state == BG2_TRANSITION) {
            UpdateBg2TransitionVideo();
            glitchTimer += GetFrameTime();   // ADD

            BeginDrawing();
            ClearBackground(BLACK);
            DrawBg2TransitionVideo();

            // SAME GLITCH BLOCK AS ABOVE
            if (glitchTimer < 0.6f) {
                float flicker = (float)(rand() % 100) / 100.0f;
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f + flicker * 0.4f));

                int sliceCount = 6 + rand() % 6;
                for (int i = 0; i < sliceCount; i++) {
                    int sliceY      = rand() % screenHeight;
                    int sliceHeight = 2 + rand() % 18;
                    int sliceOffset = (rand() % 80) - 40;
                    float sliceAlpha = 0.3f + (float)(rand() % 60) / 100.0f;
                    DrawRectangle(sliceOffset, sliceY, screenWidth, sliceHeight, Fade(BLACK, sliceAlpha));
                    if (rand() % 3 == 0) {
                        Color fringe = (rand() % 2 == 0) ? Color{180, 0, 0, 80} : Color{0, 180, 180, 60};
                        DrawRectangle(sliceOffset + (rand() % 20 - 10), sliceY, screenWidth, sliceHeight / 2, fringe);
                    }
                }
                for (int y = 0; y < screenHeight; y += 4)
                    DrawRectangle(0, y, screenWidth, 1, Fade(BLACK, 0.25f));
                if (glitchTimer < 0.12f)
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f * (1.0f - glitchTimer / 0.12f)));
            }

            EndDrawing();

        if (IsBg2TransitionFinished()) {
            float fadeAlpha = GetBg2TransitionAlpha(); 
            Color fadeColor = Fade(BLACK, fadeAlpha);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), fadeColor);

            // Reset player to center
            player.x = (screenWidth - player.width) / 2;
            player.y = screenHeight * 0.75f;
            velocityX = 0;
            velocityY = 0;
            isGrounded = true;

            // Reset items and spawn timer so they fall fresh from top
            items.clear();
            spawnTimer = 0;

            // Reset camera to follow new player position
            camera.target = { player.x + player.width / 2, player.y + player.height / 2 };
            camera.rotation = 0.0f;

            state = PLAYING;
        }

         continue;
            }

        else if (state == PAUSED)
          {
        // Input only — drawing happens inside BeginDrawing() below
        float panelW = 340, panelH = 260;
        float panelX = screenWidth / 2.0f - panelW / 2.0f;
        float panelY = screenHeight / 2.0f - panelH / 2.0f;
        float btnW = 220, btnH = 50;
        float btnX = panelX + panelW/2 - btnW/2;
        float resumeY = panelY + 90;
        float exitY = resumeY + btnH + 18;

        Rectangle resumeRect = {btnX, resumeY, btnW, btnH};
        Rectangle exitRect   = {btnX, exitY,   btnW, btnH};
        bool hoverResume = CheckCollisionPointRec(GetMousePosition(), resumeRect);
        bool hoverExit   = CheckCollisionPointRec(GetMousePosition(), exitRect);

        if ((hoverResume && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ||
            IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
            state = PLAYING;
        }

        if (hoverExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            StopMusicStream(bgMusic);
            state = MENU;
            hp = 3; score = 0; combo = 0;
            diff = EASY; bg1Triggered = false; bg2Triggered = false;
            lastDiff = EASY; comboBroken = false; comboBrokenTimer = 0; comboTime = 0;
            items.clear(); popEffects.clear();
            player.x = (screenWidth - player.width) / 2;
            player.y = screenHeight * 0.75f;
            move = 1.0f; chiliBoost = 1.0f; eventBoost = 1.0f;
            gravity = 1800.0f; velocityY = 0; velocityX = 0; isGrounded = true;
            spawnTimer = 0; eventCooldown = 15.0f; eventTimer = 0;
            currentEvent = NONE; secondEvent = NONE;
            lastEvent = NONE; secondLastEvent = NONE; eventWarningTimer = 0;
            slowTimer = 0; speedBoostTimer = 0; medkitCooldown = 0;
            showStarText = false; showMinusText = false; showSlowText = false;
            shakeTime = 0; shakePower = 0; hitFlash = 0;
            fogActive = false; fogAlpha = 0;
            fallingInPit = false; pitCreated = false; pitSoundPlayed = false;
            quakeActive = false; quakeTimer = 0;
            pits.clear(); pitWidths.clear(); pitCenters.clear(); pitOpens.clear();
            camera.rotation = 0; camera.zoom = 1.30f;
            camera.target = {player.x + player.width/2, player.y + player.height/2};
            invertedScreen = false; gameOverAnimTimer = 0.0f;
            UnloadBg1TransitionVideo(); UnloadBg2TransitionVideo();
            InitBg1TransitionVideo();   InitBg2TransitionVideo();
            introMusic = LoadMusicStream("assets/sounds/intro.mp3");
            SetMusicVolume(introMusic, 0.5f);
            PlayMusicStream(introMusic);
        }
    }



        // GAMEPLAY-----------------------------------------
        else if (state == PLAYING)
        {
            UpdateMusicStream(bgMusic);

           if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
             state = PAUSED;
            }

            // Update difficulty and handle grace periods
            Difficulty prevDiff = diff;
            if (score >= 100) 
                diff = HARD;
            else if (score >= 50)
                diff = MEDIUM;
            else
                diff = EASY;

            lastDiff = prevDiff; // lastDiff now correctly holds what diff WAS before this frame

            // If difficulty just increased, give the player a "Grace Period"
            if (diff > lastDiff) 
            {
                eventCooldown = 15.0f; 
                eventWarningTimer = 0;
            }

            // >>> Trigger transition video when EASY → MEDIUM
            if (diff == MEDIUM && lastDiff == EASY && !bg1Triggered) {
                bg1Triggered = true;
                items.clear();
                popEffects.clear();
                currentEvent = NONE;
                secondEvent = NONE;
                shakeTime = 0; shakePower = 0; hitFlash = 0;
                quakeActive = false;
                pits.clear(); pitWidths.clear();
                pitCenters.clear(); pitOpens.clear();
                pitCreated = false;
                glitchActive = true;   
                glitchTimer = 0.0f;    
                state = BG1_TRANSITION;
                continue;
            }

             else if (diff == HARD && lastDiff == MEDIUM && !bg2Triggered) {
                bg2Triggered = true;
                items.clear();
                popEffects.clear();
                currentEvent = NONE;
                secondEvent = NONE;
                shakeTime = 0; shakePower = 0; hitFlash = 0;
                quakeActive = false;
                pits.clear(); pitWidths.clear();
                pitCenters.clear(); pitOpens.clear();
                pitCreated = false;
                glitchActive = true;   
                glitchTimer = 0.0f;    
                state = BG2_TRANSITION;
                continue;
            }

            // movements 
            float accel = 2200.0f;  // how fast player gains speed
            float friction = 0.92f; // slows naturally
            float maxSpeed = 520.0f * move * chiliBoost * eventBoost;
            // Gravity
            velocityY += gravity * GetFrameTime();

            // Update player position
            player.y += velocityY * GetFrameTime();


            //TESTING EVENTS-------------------------------------
            if (IsKeyPressed(KEY_A))
            {
                currentEvent = EARTHQUAKE;
                secondEvent = NONE;

                quakeActive = true;
                quakeTimer = 0.0f;
                eventTimer = 25.0f;
            }

            if (IsKeyPressed(KEY_E))
            {
                currentEvent = INVERTED_SCREEN;
                secondEvent = NONE;

                invertedScreen = true;

                eventTimer = 25.0f;
            }

            bool overPit = false;

            for (auto &p : pits)
            {
                if (quakeActive &&
                    player.x + player.width > p.x &&
                    player.x < p.x + p.width)
                {
                    overPit = true;
                    break;
                }
            }
            //EARTHQUAKE ground-------------------------
            float groundY = screenHeight * 0.79f; //ground level

            // GROUND and PIT COLLISION
            float playerBottom = player.y + player.height;

            float playerCenter = player.x + player.width / 2;

            bool fullyInsidePit = false;

            for (auto &p : pits)
            {
                if (playerCenter > p.x &&
                    playerCenter < p.x + p.width)
                {
                    fullyInsidePit = true;
                    break;
                }
            }
            // normal ground
            if (!fallingInPit)
            {
                if (playerBottom >= groundY)
                {
                    // safe ground
                    if (!fullyInsidePit)
                    {
                        player.y = groundY - player.height;
                        velocityY = 0;
                        isGrounded = true;
                    }
                    else
                    {
                        // no ground in pit
                        isGrounded = false;
                    }
                }

                // falling
                if (fullyInsidePit &&
                    playerBottom > groundY + 35)
                {
                    fallingInPit = true;
                }
            }

            //FALLING INTO PIT DEAAADDDD
            if (fallingInPit)
            {
                // gravity fall
                velocityY += 3200 * GetFrameTime();
                player.y += velocityY * GetFrameTime();

                camera.rotation = 0.0f;
            
                // camera follows downward
                camera.target.y = Lerp(
                    camera.target.y,
                    player.y + 250,
                    4.0f * GetFrameTime()
                );
            
                // DIE ONLY WHEN DEEP
                if (player.y > screenHeight + 100)
                {
                    hp = 0;
                }

                // GAME OVER LATER -- to be fixed
                if (player.y > screenHeight + 350)
                {
                    hp = 0;
                }
            }

            // slow down
            velocityX *= friction;

            // limt max
            if (velocityX > maxSpeed)
                velocityX = maxSpeed;
            if (velocityX < -maxSpeed)
                velocityX = -maxSpeed;

                int dir = 1;

                // swapped controls
            if (currentEvent == SWAP_CONTROLS || secondEvent == SWAP_CONTROLS)
                 dir = -1;
                
            if (IsKeyDown(KEY_LEFT))
                velocityX -= accel * GetFrameTime() * chiliBoost * eventBoost * dir;
            if (IsKeyDown(KEY_RIGHT))
                velocityX += accel * GetFrameTime() * chiliBoost * eventBoost * dir;
            if (IsKeyDown(KEY_UP) && isGrounded)
            {
                    velocityY = jumpForce;
                    isGrounded = false;
                }

            player.x += velocityX * GetFrameTime();

            // left and right boundaries
            if (player.x < 0)
                player.x = 0;
            if (player.x + player.width > screenWidth)
                player.x = screenWidth - player.width;

            float wantedX = player.x + player.width / 2;

            // smooth follow
            camera.target.x += (wantedX - camera.target.x) * 0.12f;

            // Clamp camera to world boundaries
            float minX = (screenWidth / 2.0f) / camera.zoom;
            float maxX = screenWidth - (screenWidth / 2.0f) / camera.zoom;
            camera.target.x = Clamp(camera.target.x, minX, maxX);

            // keep Y fixed so player remains bottom
            camera.target.y = player.y + player.height / 2;

            //inverted screen
            if (!fallingInPit)
            {
                float targetRotation = 0.0f;

                if (invertedScreen)
                    targetRotation = 180.0f;

                camera.rotation = Lerp(camera.rotation, targetRotation, 5.0f * GetFrameTime());
                camera.zoom = Lerp(camera.zoom, 1.30f, 4.0f * GetFrameTime());
            }

            // camera shake
            if (shakeTime > 0)
            {
                shakeTime -= GetFrameTime();
                if (shakeTime < 0)
                    shakeTime = 0;
            }

            // hit flash
            if (hitFlash > 0)
            {
                hitFlash -= GetFrameTime() * 3.0f; // fade out fast
                if (hitFlash < 0) hitFlash = 0;
            }

            // hit timer (grace period)
            if (hitTimer > 0) {
                hitTimer -= GetFrameTime();
                if (hitTimer < 0) hitTimer = 0;
            }
            
            if (nearMissTimer > 0) {
                nearMissTimer -= GetFrameTime();
                if (nearMissTimer < 0) nearMissTimer = 0;
            }
            
            if (milestoneCelebrationTimer > 0) {
                milestoneCelebrationTimer -= GetFrameTime();
                if (milestoneCelebrationTimer < 0) milestoneCelebrationTimer = 0;
            }

            // JUICE: Kinematics (Squash & Stretch)
            if (landSquashTimer > 0) {
                landSquashTimer -= GetFrameTime();
                squashStretch.x = Lerp(squashStretch.x, 1.15f, 20.0f * GetFrameTime());
                squashStretch.y = Lerp(squashStretch.y, 0.85f, 20.0f * GetFrameTime());
                if (landSquashTimer <= 0) landSquashTimer = 0;
            } else if (!isGrounded) {
                // Stretching while in air
                squashStretch.x = Lerp(squashStretch.x, 0.90f, 15.0f * GetFrameTime());
                squashStretch.y = Lerp(squashStretch.y, 1.10f, 15.0f * GetFrameTime());
            } else {
                // return to normal squash/stretch
                squashStretch.x = Lerp(squashStretch.x, 1.0f, 10.0f * GetFrameTime());
                squashStretch.y = Lerp(squashStretch.y, 1.0f, 10.0f * GetFrameTime());
            }

            // (Removed playerRotation leaning logic)

            // update pop effects
            for (int i = popEffects.size() - 1; i >= 0; i--)
            {
                popEffects[i].timer -= GetFrameTime();
                if (popEffects[i].timer <= 0)
                {
                    popEffects.erase(popEffects.begin() + i);
                }
            }

            // JUICE: Update Floating Texts
            for (int i = floatingTexts.size() - 1; i >= 0; i--) {
                floatingTexts[i].timer -= GetFrameTime();
                floatingTexts[i].pos.y -= 80.0f * GetFrameTime(); // Float up
                if (floatingTexts[i].timer <= 0) {
                    floatingTexts.erase(floatingTexts.begin() + i);
                }
            }

            // JUICE: Hit-Stop Logic
            if (hitStopTimer > 0) {
                hitStopTimer -= GetFrameTime();
                if (hitStopTimer <= 0) hitStopTimer = 0;
            }

            Vector2 shakeOffset = {0, 0};
            if (shakeTime > 0)
            {
                float intensity = shakePower * (shakeTime / 0.25f);
                shakeOffset.x = (GetRandomValue(-100, 100) / 100.0f) * intensity;
                shakeOffset.y = (GetRandomValue(-100, 100) / 100.0f) * intensity;
            }

            //reverse screen
            float baseY = invertedScreen ? screenHeight / 2.0f + -220 : screenHeight * 0.75f;

            camera.offset = {
                screenWidth / 2.0f + shakeOffset.x,
                baseY + shakeOffset.y
            };

            
            // SPAWNING------------

            spawnTimer += GetFrameTime(); 
            float spawnDelay;
            int spawnAmount;
            if (diff == EASY)
            {
                spawnDelay = 1.35f; 
                spawnAmount = 1;
                baseMove = 1.0f;
            }
            else if (diff == MEDIUM)
            {
                spawnDelay = 0.95f; 
                spawnAmount = 2;
                baseMove = 1.15f;
            }
            else if (diff == HARD)
            {
                spawnDelay = 0.70f; 
                spawnAmount = 3;
                baseMove = 1.3f;
            }

            if (spawnTimer > spawnDelay)
            {
                spawnTimer = 0;

                // Segmented spawning to ensure spread and prevent overlapping
                int segments = (diff == EASY) ? 3 : 4;
                float segWidth = (float)screenWidth / segments;
                int currentIndices[4] = {0, 1, 2, 3};
                
                // Shuffle segments
                for (int s = 0; s < segments; s++) {
                    int r = rand() % segments;
                    int temp = currentIndices[s];
                    currentIndices[s] = currentIndices[r];
                    currentIndices[r] = temp;
                }

                for (int i = 0; i < spawnAmount; i++)
                {
                    Item it;
                    it.active = true;
                    it.isIllusion = false;

                    // 1. ITEM SELECTION LOGIC (Moved up to determine size)
                    if (currentEvent == LUCKY_PARTY) {
                        int goodPool[] = {BABY, HEART, BLOOD, MEAT, ATAY, STAR, DICE, CHILI};
                        it.type = goodPool[rand() % 8];
                    }
                    else if (currentEvent == MISFORTUNE) {
                        int badPool[] = {BOMB, POISON, POO, GARLIC, SALT, MUSHROOM, TROLLFACE, BOMB, POISON};
                        it.type = badPool[rand() % 9];
                    }
                    else {
                        if (hp == 1 && medkitCooldown <= 0 && rand() % 100 < 3) {
                            it.type = MEDKIT;
                            medkitCooldown = 20.0f;
                        }
                        else if (hp < 3 && rand() % 100 < 4) it.type = BANDAGE;
                        else if (rand() % 100 < 3) it.type = TROLLFACE;
                        else if (rand() % 100 < 2) it.type = DICE; 
                        else if (diff == EASY) {
                            int pool[] = {POO, GARLIC, BABY, BLOOD, BABY, ATAY};
                            it.type = pool[rand() % 6];
                        }
                        else if (diff == MEDIUM) {
                            int pool[] = {POO, GARLIC, BABY, BLOOD, BOMB, POISON, HEART, ATAY, BABY};
                            it.type = pool[rand() % 9];
                            if (rand() % 100 < 12) it.isIllusion = true; 
                        }
                        else { // HARD
                            int pool[] = {POO, GARLIC, BABY, BLOOD, BOMB, POISON, ATAY, HEART, MUSHROOM, DICE, STAR};
                            it.type = pool[rand() % 11];
                            if (rand() % 100 < 22) it.isIllusion = true; 
                        }
                    }

                    if (it.isIllusion) {
                        if (it.type == BABY || it.type == HEART || it.type == BLOOD) it.illusionTarget = BOMB;
                        else if (it.type == BOMB || it.type == POISON) it.illusionTarget = HEART;
                        else it.isIllusion = false; 
                    }

                    // 2. SET DIMENSIONS BASED ON TYPE (Larger and unsqueezed)
                    it.rect.width = 70;  // Slightly larger base
                    it.rect.height = 70;

                    if (it.type == POO || it.type == POISON || it.type == TROLLFACE) {
                        it.rect.width = 95;
                        it.rect.height = 95;
                    }
                    else if (it.type == GARLIC || it.type == MUSHROOM || it.type == CHILI || it.type == BOMB) {
                        it.rect.width = 85;
                        it.rect.height = 85;
                    }
                    else if (it.type == MEDKIT || it.type == BANDAGE || it.type == ATAY) {
                        it.rect.width = 90;
                        it.rect.height = 90;
                    }

                    // 3. POSITION LOGIC (using final width)
                    int segIdx = currentIndices[i % segments];
                    float margin = 60.0f; 
                    float minX = (segIdx * segWidth) + margin;
                    float maxX = ((segIdx + 1) * segWidth) - it.rect.width - margin;
                    
                    it.rect.x = minX + (rand() % (int)(maxX - minX + 1));
                    it.rect.y = -100 - rand() % 250;

                    // 4. PHYSICS
                    if (diff == EASY) {
                        it.speed = 140 + rand() % 40;
                        it.acceleration = 15;
                        it.lateralSpeed = 0;
                    } else if (diff == MEDIUM) {
                        it.speed = 195 + rand() % 50;
                        it.acceleration = 25;
                        it.lateralSpeed = (rand() % 100 < 40) ? (rand() % 61 - 30) : 0; 
                    } else {
                        it.speed = 270 + rand() % 70;
                        it.acceleration = 40;
                        it.lateralSpeed = (rand() % 91 - 45) * 1.1f; 
                    }

                    it.sinTime = (float)(rand() % 1000) / 100.0f;
                    items.push_back(it); 
                }
            }

            // CHALLENGES--------------------------------------------
           
            // countdown before next event
            if (currentEvent == NONE)
            {
                eventCooldown -= GetFrameTime();
                
                // Show a warning when the event is about to start (3 seconds before)
                if (eventCooldown <= 3.0f && diff != EASY)
                {
                    eventWarningTimer = eventCooldown;
                }
            }

            // start new event
            if (currentEvent == NONE && eventCooldown <= 0)
            {
                // reset previous
                secondEvent = NONE;
                eventWarningTimer = 0;

                if (diff == MEDIUM)
                {
                    int mediumEvents[] = {SWAP_CONTROLS, SPEED_BOOST, SLOW_BOOST, INVERTED_SCREEN, EARTHQUAKE, LUCKY_PARTY, MISFORTUNE};
                    
                    // FAIRNESS: Loop to ensure we don't repeat the last 2 events immediately
                    EventType chosen;
                    int attempts = 0;
                    do {
                        chosen = (EventType)mediumEvents[rand() % 7];
                        attempts++;
                    } while ((chosen == lastEvent || chosen == secondLastEvent) && attempts < 10);
                    
                    currentEvent = chosen;
                    secondLastEvent = lastEvent;
                    lastEvent = currentEvent;

                    eventTimer = 13.0f;
                    eventCooldown = 15.0f;
                }

                else if (diff == HARD)
                {
                    int hardEvents[] = {SWAP_CONTROLS, SPEED_BOOST, SLOW_BOOST, LOW_GRAVITY, FOG_BLIND, INVERTED_SCREEN, EARTHQUAKE, LUCKY_PARTY, MISFORTUNE};

                    // FAIRNESS: Same repeat-prevention for primary event
                    EventType chosen;
                    int attempts = 0;
                    do {
                        chosen = (EventType)hardEvents[rand() % 9];
                        attempts++;
                    } while ((chosen == lastEvent || chosen == secondLastEvent) && attempts < 10);

                    currentEvent = chosen;
                    secondLastEvent = lastEvent;
                    lastEvent = currentEvent;

                    // 40% chance for DOUBLE EVENT
                    if (rand() % 100 < 40)
                    {
                        secondEvent = (EventType)hardEvents[rand() % 9];
                        // prevent same event twice in the double-event slot
                        while (secondEvent == currentEvent)
                            secondEvent = (EventType)hardEvents[rand() % 9];
                    }

                    eventTimer = 16.0f;
                    eventCooldown = 10.0f;
                }
            }

            // EVENTS SYSTEM---------------------------------------------------
            if (currentEvent != NONE)
            {

                eventTimer -= GetFrameTime();
                // reset effects every frame first
                eventBoost = 1.0f;
                gravity = 1800.0f;
                

                // helper lambda
                auto ApplyEvent = [&](EventType e)
                {
                    if (e == SPEED_BOOST)
                    eventBoost = 1.7f;
                    if (e == SLOW_BOOST)
                    eventBoost = 0.65f;
                    if (e == LOW_GRAVITY)
                    gravity = 700.0f;
                    if (e == FOG_BLIND)
                    fogActive = true;
                    if (e == INVERTED_SCREEN)
                    invertedScreen = true;
                    if (e == EARTHQUAKE)
                    quakeActive = true;
                };

                // apply BOTH events
                ApplyEvent(currentEvent);
                ApplyEvent(secondEvent);

                // event ended
                if (eventTimer <= 0)
                {

                    currentEvent = NONE;
                    secondEvent = NONE;

                    fogActive = false;
                    fogAlpha = 0.0f;
                    fogFadingOut = false;

                    gravity = 1800.0f;
                    eventBoost = 1.0f;

                    invertedScreen = false;

                    quakeActive = false;
                    fallingInPit = false;

                    // RESET COOLDOWN
                    if (diff == MEDIUM)
                       eventCooldown = 15.0f;
                    else if (diff == HARD)
                       eventCooldown = 10.0f;
                }
            }

            //FOG EFFECT
            if (fogActive)
            {
                float fadeStart = 5.0f;

                if (eventTimer > fadeStart)
                {
                    // smooth fade in
                    fogAlpha = Lerp(fogAlpha, 0.55f, 1.5f * GetFrameTime());
                }
                else
                {
                    // smooth fade out
                    fogAlpha = Lerp(fogAlpha, 0.0f, 1.0f * GetFrameTime());

                    if (fogAlpha <= 0.02f)
                    {
                        fogActive = false;
                        fogAlpha = 0.0f;
                    }
                }
            }
        

            // EARTHQUAKE PIT
            if (quakeActive)
            {
                quakeTimer += GetFrameTime();
            
                if(shakeTime <= 0){
                    shakeTime = 0.1f;
                    shakePower = 6.0f;
                }
                // CREATE PIT
                if (quakeTimer > 1.0f && !pitCreated)                {
                    pitCreated = true;
                    PlaySound(pitSound);

                    pits.clear();
                    pitWidths.clear();
                    pitCenters.clear();

                    if (diff == HARD)
                        pitCount = GetRandomValue(2, 3);
                    else
                        pitCount = 1;

                    for (int i = 0; i < pitCount; i++)
                    {
                        float randomWidth;

                        // random widths
                        if (pitCount == 1)
                        {
                            randomWidth = GetRandomValue(260, 340);
                        }
                        else if (pitCount == 2)
                        {
                            randomWidth = GetRandomValue(170, 240);
                        }
                        else // 3 pits
                        {
                            randomWidth = GetRandomValue(120, 180);
                        }

                        //prevent pit overlap
                        float randomX;
                        bool tooClose;

                        do {
                            tooClose = false;

                            randomX = GetRandomValue(
                                100,
                                screenWidth - randomWidth - 100
                            );

                            // avoid overlap
                            for (int j = 0; j < pits.size(); j++)
                            {
                                if (fabs(randomX - pits[j].x) < 220)
                                {
                                    tooClose = true;
                                    break;
                                }
                            }

                        

                        } while (tooClose);

                        Rectangle newPit = {
                            randomX,
                            screenHeight * 0.79f,
                            0,
                            220
                        };

                        pits.push_back(newPit);

                        pitWidths.push_back(randomWidth);
                        pitOpens.push_back(0.0f);

                        pitCenters.push_back(randomX + randomWidth / 2);
                    }
                }
            
                // open pits
                if (pitCreated)
                {
                    for (int i = 0; i < pits.size(); i++)
                    {
                        // open individually
                        if (quakeTimer > 3.5f)
                        {
                            if (!pitSoundPlayed) {
                              PlaySound(pitSound);
                              pitSoundPlayed = true;
                 }
                            pitOpens[i] += 250 * GetFrameTime();

                            if (pitOpens[i] > pitWidths[i])
                                pitOpens[i] = pitWidths[i];
                        }

                        pits[i] = {
                            pitCenters[i] - pitOpens[i] / 2,
                            screenHeight * 0.79f,
                            pitOpens[i],
                            220
                        };

                        float playerCenterX = player.x + player.width / 2;
                        float playerBottom = player.y + player.height;

                        bool abovePit =
                            playerCenterX > pits[i].x &&
                            playerCenterX < pits[i].x + pits[i].width;

                        if (abovePit &&
                            playerBottom >= groundY &&
                            velocityY > 150)
                        {
                            fallingInPit = true;
                        }
                    }
                }
            }
            else
            {
                bool allClosed = true;
            
                for (int i = 0; i < pitOpens.size(); i++)
                {
                    pitOpens[i] = Lerp(
                        pitOpens[i],
                        0.0f,
                        5.0f * GetFrameTime()
                    );
            
                    if (pitOpens[i] > 1.0f)
                        allClosed = false;
            
                    pits[i] = {
                        pitCenters[i] - pitOpens[i] / 2,
                        groundY,
                        pitOpens[i],
                        screenHeight - groundY
                    };
                }
            
                // fully reset
                if (allClosed)
                {
                    pitCreated = false;
                    quakeTimer = 0;
                    pitSoundPlayed = false;
            
                    pits.clear();
                    pitWidths.clear();
                    pitCenters.clear();
                    pitOpens.clear();
                }
            }

            // UPDATE ITEMS & COLLISION -----------------
            for (auto &it : items)
            {
                if (!it.active)
                    continue;

                // Physics: items now accelerate at their own unique rates
                it.speed += it.acceleration * GetFrameTime();
                it.rect.y += it.speed * GetFrameTime();

                // Lateral movement (Sway / Zigzag)
                if (it.lateralSpeed != 0) {
                    it.sinTime += GetFrameTime() * 2.0f;
                    it.rect.x += it.lateralSpeed * GetFrameTime() + sin(it.sinTime) * 3.0f;
                    
                    // Keep within screen
                    if (it.rect.x < 10) { it.rect.x = 10; it.lateralSpeed *= -1; }
                    if (it.rect.x > screenWidth - it.rect.width - 10) { it.rect.x = screenWidth - it.rect.width - 10; it.lateralSpeed *= -1; }
                }

                // Illusion Shifting (Uncertainty)
                if (it.isIllusion && it.rect.y > screenHeight * 0.30f) {
                    // Shift to target type mid-air
                    it.type = it.illusionTarget;
                    it.isIllusion = false; // only shift once
                    // add a little "poof" shake to the item
                    it.rect.x += (rand() % 21 - 10);
                }

                // COLLISION LOGIC (Literal Visual-Only)
                // We use a centered "Core" for the player and a centered "Core" for items.
                
                float bodyWidth = player.width * 0.35f;   // Middle 35% of the sprite
                float bodyHeight = player.height * 0.80f;  // Top 80%
                
                Rectangle playerHitbox = {
                    player.x + (player.width - bodyWidth) / 2.0f, 
                    player.y + (player.height - bodyHeight) / 2.0f,
                    bodyWidth,             
                    bodyHeight            
                };

                // JUICE: Tongue Reach Passive (Snag good items from distance)
                bool isGood = (it.type == BABY || it.type == HEART || it.type == BLOOD || it.type == MEAT || it.type == ATAY || it.type == STAR || it.type == CHILI);
                if (isGood) {
                    float dist = Vector2Distance(
                        {it.rect.x + it.rect.width/2, it.rect.y + it.rect.height/2},
                        {player.x + player.width/2, player.y + player.height/2}
                    );
                    if (dist < 180.0f) {
                        // Pull towards player
                        Vector2 dir = Vector2Normalize(Vector2Subtract(
                            {player.x + player.width/2, player.y + player.height/2},
                            {it.rect.x + it.rect.width/2, it.rect.y + it.rect.height/2}
                        ));
                        it.rect.x += dir.x * 500.0f * GetFrameTime();
                        it.rect.y += dir.y * 500.0f * GetFrameTime();
                    }
                }

                // Match item collision to its visual size
                float itemCoreScale = 0.75f;
                Rectangle itemHitbox = {
                    it.rect.x + (it.rect.width * (1.0f - itemCoreScale) / 2.0f),
                    it.rect.y + (it.rect.height * (1.0f - itemCoreScale) / 2.0f),
                    it.rect.width * itemCoreScale,
                    it.rect.height * itemCoreScale
                };

                if (CheckCollisionRecs(playerHitbox, itemHitbox))
                {
                    // Trigger Pop Effect
                    PopEffect pe;
                    pe.pos = { it.rect.x + it.rect.width/2, it.rect.y + it.rect.height/2 };
                    pe.timer = 0.45f;
                    pe.maxTime = 0.45f;

                    // BAD ITEMS
                    if (it.type == POO || it.type == BOMB || it.type == SALT || it.type == GARLIC)
                    {
                        pe.color = RED;
                        hitFlash = 0.4f;
                        hp--;
                        shakeTime = 0.22f;
                        shakePower = 14.0f;
                        combo = 0;
                        comboTime = 0;
                        comboBroken = true;
                        comboBrokenTimer = 1.5f;

                        // JUICE: Hit-Stop for impact
                        hitStopTimer = 0.08f; 
                    }
                    else if (it.type == CHILI)
                    {
                        pe.color = ORANGE;
                        chiliBoost = 1.8f;      
                        speedBoostTimer = 5.0f; 
                    }
                    
                    // SCORE++
                    else if (it.type == BABY || it.type == HEART)
                    {
                        pe.color = GOLD;
                        score += 5;

                        combo++;
                        comboTime = 2.5f;
                        int added = 5;
                        if (combo > 1) {
                            score += combo;
                            added += combo;
                        }
                        
                        // JUICE: Floating text
                        floatingTexts.push_back({ {it.rect.x, it.rect.y}, "+" + to_string(added), 1.5f, GOLD });

                        // JUICE: Subtle shake on combo
                        if (combo % 5 == 0) {
                            shakeTime = 0.1f;
                            shakePower = 4.0f;
                        }
                    }
                    else if (it.type == ATAY)
                    {
                        pe.color = GOLD;
                        score += 8;
                        shakeTime = 0.08f;
                        shakePower = 3.0f;
                        floatingTexts.push_back({ {it.rect.x, it.rect.y}, "+8", 1.5f, GOLD });
                    }

                    else if (it.type == BLOOD || it.type == MEAT)
                    { // blood
                        pe.color = RED;
                        score += 3;

                        combo++;
                        comboTime = 2.5f;
                        int added = 3;
                        if (combo > 1) {
                            score += combo;
                            added += combo;
                        }
                        
                        floatingTexts.push_back({ {it.rect.x, it.rect.y}, "+" + to_string(added), 1.5f, RED });

                        if (combo % 5 == 0) {
                            shakeTime = 0.1f;
                            shakePower = 4.0f;
                        }
                    }
                    // HEAL
                    else if (it.type == BANDAGE)
                    { // bandage
                        pe.color = GREEN;
                        hp += 1;
                        if (hp > 3)
                            hp = 3;
                    }
                    else if (it.type == MEDKIT)
                    {
                        pe.color = GREEN;
                        hp = 3;
                    }
                    // RANDOMNESS
                    else if (it.type == TROLLFACE)
                    {
                        pe.color = WHITE;
                        state = TROLL_VIDEO;
                        currentFrame = 0;
                        frameTimer = 0;
                        PlaySound(trollSound);
                    }
                    else if (it.type == POISON)
                    {
                        pe.color = PURPLE;
                        move = 0.45f;
                        slowTimer = 4.0f;
                    }

                    // SPECIAL ITEMS
                    else if (it.type == DICE)
                    { // dice(good effects)
                        pe.color = WHITE;
                        int randomIndex = rand() % 4;
                        if (randomIndex == 1)
                        {
                            score += 10;
                            showStarText = true;
                            starTextTimer = 2.0f;
                        }
                    }
                    else if (it.type == MUSHROOM)
                    { // mushroom(bad effects)
                        pe.color = MAGENTA;
                        int randomIndex = rand() % 4;
                        if (randomIndex == 1)
                        { // -10
                            score -= 10;
                            showMinusText = true;
                            minusTextTimer = 2.0f;
                            hitFlash = 0.35f;
                        }
                        else if (randomIndex == 2)
                        { // slowness
                            move = 0.45f;
                            slowTimer = 4.0f;
                            showSlowText = true;
                            slowTextTimer = 2.0f;
                        }
                    }
                    else if (it.type == STAR)
                    {
                        pe.color = YELLOW;
                        score += 10;
                    }

                    // special prize(super rare)
                    else if (it.type == PRIZE)
                    { // gift?
                        pe.color = PINK;
                    }

                    popEffects.push_back(pe);
                    it.active = false; // remove item after collision
                }
            }

            if (hp <= 0 && state == PLAYING)
            {
                state = GAMEOVER_ANIM;
                gameOverAnimTimer = 0.0f;
                gameOverFlash = 1.0f;
                PlaySound(gameOverSound);
                shakeTime = 0.5f;
                shakePower = 20.0f;
                StopMusicStream(bgMusic);        
            }

            // TIMERS-----------------------------
            // combo
            if (comboTime > 0)
            {
                comboTime -= GetFrameTime();
                if (comboTime <= 0)
                {
                    comboTime = 0;
                    combo = 0;
                }
            }

            if (comboBroken)
            {
                comboBrokenTimer -= GetFrameTime();
            
                if (comboBrokenTimer <= 0){
                    comboBroken = false;
                    comboBrokenTimer = 0;
                    comboTime = 0;
                }   
            }

            // speedboost
            if (speedBoostTimer > 0)
            {
                speedBoostTimer -= GetFrameTime();
                if (speedBoostTimer <= 0)
                    chiliBoost = 1.0f;
            }

            // STAR 
            if(showStarText){
                starTextTimer -= GetFrameTime();
                if(starTextTimer <= 0) showStarText = false;
            }
            // STAR
            if (showStarText)
            {
                starTextTimer -= GetFrameTime();
                if (starTextTimer <= 0)
                    showStarText = false;
            }
            // MINUS
            if (showMinusText)
            {
                minusTextTimer -= GetFrameTime();
                if (minusTextTimer <= 0)
                    showMinusText = false;
            }
            // SLOW
            if (showSlowText)
            {
                slowTextTimer -= GetFrameTime();
                if (slowTextTimer <= 0)
                    showSlowText = false;
            }
            if (slowTimer > 0)
            {
                slowTimer -= GetFrameTime();
                if (slowTimer <= 0)
                    move = 1.0f;
            }


            // PLAYER ANIMATIONS-----------------------------
            
            if (diff == HARD) {
         
         if (IsKeyDown(KEY_RIGHT)) {
             currentMananAnim = FLY_RIGHT;
         } else if (IsKeyDown(KEY_LEFT)) {
             currentMananAnim = FLY_LEFT;
         } else {
             currentMananAnim = FLY_FRONT;
         }

         // Frame timing

         pframeTimer += GetFrameTime();
         if (pframeTimer >= pframeDelay) {
         pframeTimer = 0.0f;
         playerFrame++;
         if (playerFrame >= 6) playerFrame = 0;
         }
         }
            
          else if (diff == EASY || diff == MEDIUM) {
            currentAnim = IDLE;

            // Walking right
           if (IsKeyDown(KEY_RIGHT)) {
               currentAnim = WALK_RIGHT;
             }

            // Walking left
            if (IsKeyDown(KEY_LEFT)) {
               currentAnim = WALK_LEFT;
             }

            // Jump trigger (physics only)
            if (IsKeyPressed(KEY_UP) && isGrounded) {
            velocityY = jumpForce;
            isGrounded = false;

             // reset jump animation
             currentAnim = JUMP;
             playerFrame = 0;
             pframeTimer = 0.0f;
            }

             // If airborne, force jump animation
                if (!isGrounded) {
                currentAnim = JUMP;
             }

             // Animation state handling
            if (currentAnim == JUMP) {
            // advance jump frames only once
            pframeTimer += GetFrameTime();
             if (pframeTimer >= pframeDelay && playerFrame < 4) {
               pframeTimer = 0.0f;
               playerFrame++;
            }
            // hold last frame until landing
            if (playerFrame >= 4) playerFrame = 4;


              // when grounded again, switch back to idle/walk
         if (isGrounded) {
             if (IsKeyDown(KEY_LEFT)) currentAnim = WALK_LEFT;
             else if (IsKeyDown(KEY_RIGHT)) currentAnim = WALK_RIGHT;
             else currentAnim = IDLE;
                  playerFrame = 0; // reset for next cycle
            }
           }
         else {
              // normal walking/idle animation
              pframeTimer += GetFrameTime();
              if (pframeTimer >= pframeDelay) {
              pframeTimer = 0.0f;
               playerFrame++;
             if (playerFrame >= 6) playerFrame = 0;
            }
        }

             // Update frame timer for walk animations
            if (currentAnim == WALK_RIGHT || currentAnim == WALK_LEFT || currentAnim == JUMP) {
                pframeTimer += GetFrameTime();
                if (pframeTimer >= pframeDelay) {
                    pframeTimer = 0.0f;
                    playerFrame++;
                    if (playerFrame >= 6) playerFrame = 0;
                }
            } else playerFrame = 0;
            
        }


        


        if (state == TROLL_VIDEO)
        {
            // add time every frame
            frameTimer += GetFrameTime();

            // when enough time passed, next frame
            if (frameTimer >= 0.2f)
            {
                frameTimer = 0;
                currentFrame++;
            }

            // if video ended, go back to game
            if (currentFrame >= videoFrames.size())
            {
                StopSound(trollSound); // stop audio
                currentFrame = 0;      // reset video
                state = PLAYING;       // resume game
            }
            if (IsKeyPressed(KEY_ENTER))
            {
            StopSound(trollSound);
             currentFrame = 0;
             state = PLAYING;
            }
        }

        // MEME POP-UP UPDATE
        if (state == PLAYING) {
            totalTimePlayed += GetFrameTime();
            
            // CHAOS LEVEL CALCULATION (for research metrics)
            float scoreTarget = 1000.0f;
            chaosLevel = Clamp((float)score / scoreTarget, 0.0f, 0.7f);
            if (diff == MEDIUM) chaosLevel += 0.15f;
            if (diff == HARD) chaosLevel += 0.3f;
            chaosLevel = Clamp(chaosLevel, 0.0f, 1.0f);

            memeSpawnTimer += GetFrameTime();
            
            // Trigger: Random (Rarer: 45 to 100 seconds) or Press 'F'
            bool manualTrigger = IsKeyPressed(KEY_F);
            
            if (!currentMeme.active && (memeSpawnTimer > GetRandomValue(45, 100) || manualTrigger)) {
                memeSpawnTimer = 0;
                int index;
                if (!memeTextures.empty()) {
                    do {
                        index = GetRandomValue(0, memeTextures.size() - 1);
                    } while (index == currentMeme.lastIndex && memeTextures.size() > 1);

                    currentMeme.tex = memeTextures[index];
                    currentMeme.lastIndex = index;
                    currentMeme.active = true;
                    currentMeme.speed = (float)GetRandomValue(2200, 3200); // Much faster sliding
                    
                    // Rare Sound Trigger (25% chance or manual)
                    if (GetRandomValue(1, 100) <= 25 || manualTrigger) {
                        currentMeme.soundIndex = GetRandomValue(0, memeSounds.size() - 1);
                        PlaySound(memeSounds[currentMeme.soundIndex]);
                    } else {
                        currentMeme.soundIndex = -1;
                    }

                    bool fromLeft = GetRandomValue(0, 1) == 0;
                    if (fromLeft) {
                        currentMeme.pos.x = -(float)screenWidth * 1.5f; // Start further back
                    } else {
                        currentMeme.pos.x = (float)screenWidth * 1.5f;
                        currentMeme.speed = -currentMeme.speed;
                    }
                    currentMeme.pos.y = 0;
                }
            }

            if (currentMeme.active) {
                currentMeme.pos.x += currentMeme.speed * GetFrameTime();
                
                // End conditions (Accounting for extra width)
                if (currentMeme.speed > 0 && currentMeme.pos.x > screenWidth * 1.5f) currentMeme.active = false;
                if (currentMeme.speed < 0 && currentMeme.pos.x < -screenWidth * 1.5f) currentMeme.active = false;
            }
        }
    }

        if (state == GAMEOVER_ANIM)
        {
            gameOverAnimTimer += GetFrameTime();
            gameOverFlash -= GetFrameTime() * 1.5f;
            if (gameOverFlash < 0)
                gameOverFlash = 0;

            // play music AFTER game over sound finishes (~2 seconds)
            if (gameOverAnimTimer >= 2.0f && !IsMusicStreamPlaying(gameOverMusic))
                PlayMusicStream(gameOverMusic);  
        }
        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // game
        if (state == PLAYING)
        {
            BeginMode2D(camera);
    
            // DRAW BACKGROUND (Clean, single layer)
            Texture2D currentBg;
            if (diff == EASY)      currentBg = bgEasy;
            else if (diff == MEDIUM) currentBg = bgMedium;
            else if (diff == HARD)   currentBg = bgHard;

            DrawTexturePro(
                currentBg,
                {0, 0, (float)currentBg.width, (float)currentBg.height},
                {0, 0, (float)screenWidth, (float)screenHeight},
                {0, 0},
                0.0f,
                ColorAlpha(WHITE, 0.8f) 
            );

            // WARNING BEFORE PIT OPENS
            if (quakeTimer > 1.5f && quakeTimer < 3.5f)
            {
                for (int i = 0; i < pitCenters.size(); i++)
                {
                    DrawText(
                        "!",
                        pitCenters[i] - 10,
                        screenHeight * 0.75f,
                        60,
                        RED
                    );
                }
            }

            //TEXTURE OF EARTHQUAKE
            for (int i = 0; i < pits.size(); i++)
            {
                if (pitOpens[i] > 1)
                {
                    DrawRectangle(
                        pits[i].x,
                        pits[i].y,
                        pits[i].width,
                        pits[i].height,
                        BLACK
                    );
                 
                    DrawRectangle(
                        pits[i].x,
                        pits[i].y - 5,
                        pits[i].width,
                        5,
                        DARKGRAY
                    );
                }
            }


            //character 

            // Draw invisible base 
           DrawTexturePro(
           player1Tex,
           {0,0,(float)player1Tex.width,(float)player1Tex.height},
           {player.x, player.y, player.width, player.height},
           {0,0}, 0.0f, WHITE
           );
            
            Texture2D texToDraw; 
            
            float scale = 0.85f;

            // keep player rect consistent with texture size
            player.width  = playerTex.width * scale;
            player.height = playerTex.height * scale;

             Rectangle dest = {
                player.x,
                player.y,
                player.width,   
                player.height   
            };


             if (diff == EASY || diff == MEDIUM) {

            switch (currentAnim) {
                case WALK_RIGHT: texToDraw = RwalkFrames[playerFrame]; break;
                case WALK_LEFT:  texToDraw = LwalkFrames[playerFrame]; break;
                case JUMP:       texToDraw = JumpFrames[playerFrame];  break;
                default:         texToDraw = playerTex;                 break; 
            }}

           else if (diff == HARD) {

             if (currentMananAnim == FLY_FRONT) 
                  texToDraw = FlyFrames[playerFrame];
             else if (currentMananAnim == FLY_RIGHT) 
                  texToDraw = RflyFrames[playerFrame];
             else if (currentMananAnim == FLY_LEFT) 
                  texToDraw = LflyFrames[playerFrame];
                }

            DrawTexturePro(
                texToDraw,
                {0, 0, (float)texToDraw.width, (float)texToDraw.height},
                { dest.x + dest.width/2, dest.y + dest.height/2, dest.width * squashStretch.x, dest.height * squashStretch.y },
                { (dest.width * squashStretch.x)/2.0f, (dest.height * squashStretch.y)/2.0f },
                playerRotation,
                WHITE
                );


            for (auto &it : items)
            {
                if (!it.active)
                    continue;

                Color col = WHITE;
                float visualScale = 1.0f;
                
                // HIGHLY VISIBLE ILLUSION HINT: Pulsing magenta tint + size oscillation
                if (it.isIllusion) {
                    float pulse = (sin(GetTime() * 15.0f) + 1.0f) / 2.0f; // Fast pulse
                    col = ColorLerp(WHITE, MAGENTA, 0.7f * pulse);       // Strong magenta tint
                    visualScale = 1.0f + (0.15f * pulse);               // Grow up to 15% larger
                }

                // Smaller item size + apply illusion visual scale
                Rectangle drawRect = it.rect;
                float finalScale = 0.80f * visualScale; // Apply pulsing visual scale

                Rectangle smallRect = {
                    drawRect.x + (drawRect.width * (1.0f - finalScale) / 2),
                    drawRect.y + (drawRect.height * (1.0f - finalScale) / 2),
                    drawRect.width * finalScale,
                    drawRect.height * finalScale
                };

                if (it.type == BOMB)
                    DrawTexturePro(bombTex, {0, 0, (float)bombTex.width, (float)bombTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == BABY)
                    DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)babyTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == MEDKIT)
                    DrawTexturePro(potionMedkitTex, {0, 0, (float)potionMedkitTex.width, (float)potionMedkitTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == BANDAGE)
                    DrawTexturePro(potionBandageTex, {0, 0, (float)potionBandageTex.width, (float)potionBandageTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == GARLIC)
                    DrawTexturePro(garlicTex, {0, 0, (float)garlicTex.width, (float)garlicTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == CHILI)
                    DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)chiliTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == TROLLFACE)
                    DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)trollFaceTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == HEART)
                    DrawTexturePro(heartTex, {0, 0, (float)heartTex.width, (float)heartTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == BLOOD)
                    DrawTexturePro(bloodTex, {0, 0, (float)bloodTex.width, (float)bloodTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == POO)
                    DrawTexturePro(pooTex, {0, 0, (float)pooTex.width, (float)pooTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == DICE)
                    DrawTexturePro(diceTex, {0, 0, (float)diceTex.width, (float)diceTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == MUSHROOM)
                    DrawTexturePro(mushroomTex, {0, 0, (float)mushroomTex.width, (float)mushroomTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == POISON)
                    DrawTexturePro(poisonTex, {0, 0, (float)poisonTex.width, (float)poisonTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == SALT)
                    DrawTexturePro(saltTex, {0, 0, (float)saltTex.width, (float)saltTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == ATAY)
                    DrawTexturePro(atayTex, {0, 0, (float)atayTex.width, (float)atayTex.height}, smallRect, {0, 0}, 0.0f, col);
                //if (it.type == PRIZE) //INSERT PRIZE IMAGE
                   
            }

            // draw pop effects
            for (auto &pe : popEffects)
            {
                float t = 1.0f - (pe.timer / pe.maxTime); // 0 to 1
                for (int i = 0; i < 8; i++)
                {
                    float angle = i * 45.0f * DEG2RAD;
                    Vector2 p = {
                        pe.pos.x + cos(angle) * t * 80.0f,
                        pe.pos.y + sin(angle) * t * 80.0f
                    };
                    DrawCircleV(p, 6.0f * (1.0f - t), Fade(pe.color, 1.0f - t));
                }
            }

            
             // JUICE: Draw Floating Texts
                for (auto &ft : floatingTexts) {
                    float alpha = ft.timer / 1.5f;
                    DrawTextEx(tinyFont, ft.text.c_str(),
                        { ft.pos.x + 2, ft.pos.y + 2 }, 28, 0, Fade(BLACK, alpha * 0.85f));
                    DrawTextEx(tinyFont, ft.text.c_str(),
                        ft.pos, 28, 0, Fade(ft.color, alpha));
                }

            //EVENTS DESIGNS------------------------------------------------------
            // TEXTURE OF FOG EFFECT
            if(fogActive && fogAlpha > 0){

                float camLeft  = camera.target.x - screenWidth / (2 * camera.zoom);
                float camRight = camera.target.x + screenWidth / (2 * camera.zoom);
            
                for(int row = 0; row < 6; row++){
            
                    for(float x = camLeft - 300; x < camRight + 300; x += 240){
            
                        float y = row * 160;
            
                        DrawCircleGradient(
                            Vector2{x, y},
                            220.0f,
                            Fade(LIGHTGRAY, fogAlpha),
                            Fade(WHITE, 0.0f)
                        );
            
                        DrawCircleGradient(
                            Vector2{x + 100.0f, y + 50.0f},
                            260.0f,
                            Fade(GRAY, fogAlpha * 0.8f),
                            Fade(WHITE, 0.0f)
                        );
                    }
                }
            }

            EndMode2D();

            // DRAW MEME POP-UP (Stretched to full height)
            if (currentMeme.active) {
                DrawTexturePro(
                    currentMeme.tex,
                    { 0, 0, (float)currentMeme.tex.width, (float)currentMeme.tex.height },
                    { currentMeme.pos.x, 0, (float)screenWidth, (float)screenHeight }, // Stretched to screen height
                    { 0, 0 },
                    0,
                    Fade(WHITE, 0.4f)
                );
            }

            // UI   
            // CHAOS LEVEL UI (for research visibility)
            DrawRectangle(screenWidth - 220, 20, 200, 25, Fade(BLACK, 0.4f));
            DrawRectangle(screenWidth - 215, 25, (int)(190 * chaosLevel), 15, ColorLerp(GREEN, RED, chaosLevel));
            DrawText("CHAOS LEVEL", screenWidth - 215, 50, 20, WHITE);

            //health
            float hpScale = 0.1f; 
            for (int i = 0; i < hp; i++) DrawTextureEx(hpTex, Vector2{10.0f + i * (hpTex.width * hpScale + 5.0f), 10.0f}, 0.0f, hpScale, WHITE);

            //score 
            DrawTextEx(tinyFont, TextFormat("score: %d", score), Vector2{20.0f, 50.0f}, 45, 2, WHITE);

            // --- PAUSE BUTTON (below score, top-left) ---
            Rectangle pauseBtn = {10, 100, 44, 44};
            bool hoverPause = CheckCollisionPointRec(GetMousePosition(), pauseBtn);
            DrawRectangleRounded(pauseBtn, 0.2f, 6,
                hoverPause ? Color{80, 80, 80, 220} : Color{30, 30, 30, 180});
            DrawRectangle(pauseBtn.x + 11, pauseBtn.y + 11, 8, 22,
                hoverPause ? YELLOW : WHITE);
            DrawRectangle(pauseBtn.x + 25, pauseBtn.y + 11, 8, 22,
                hoverPause ? YELLOW : WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(GetMousePosition(), pauseBtn)) {
                state = PAUSED;
            }

            // === DIFFICULTY BADGE ===
            {
                const char* diffLabel;
                Color diffColor;

                if (diff == EASY) {
                    diffLabel = "EASY";
                    diffColor = { 74, 255, 106, 255 };
                } else if (diff == MEDIUM) {
                    diffLabel = "MEDIUM";
                    diffColor = { 255, 204, 0, 255 };
                } else {
                    diffLabel = "HARD";
                    diffColor = { 255, 48, 48, 255 };
                }

                float pulse = 0.65f + 0.35f * fabsf(sinf(
                    GetTime() * (diff == HARD ? 5.0f : diff == MEDIUM ? 3.5f : 2.0f)
                ));

                Vector2 labelSize = MeasureTextEx(tinyFont, diffLabel, 30, 0);
                float badgeW = labelSize.x + 20;
                float badgeH = 38;
                float badgeX = screenWidth - badgeW - 14;
                float badgeY = 75;

                DrawRectangle((int)badgeX, (int)badgeY, (int)badgeW, (int)badgeH, Color{0, 0, 0, 200});
                DrawRectangleLinesEx({ badgeX, badgeY, badgeW, badgeH }, 2, Fade(diffColor, pulse));
                DrawTextEx(tinyFont, diffLabel,
                    { badgeX + 10, badgeY + (badgeH - labelSize.y) / 2 },
                    30, 0, Fade(diffColor, pulse));
            }

            // EVENT WARNING UI
            if (eventWarningTimer > 0)
            {
                // Pulsing effect for the warning
                float pulse = abs(sin(GetTime() * 10.0f));
                const char* warningText = "INGAT... MAY PAPARATING!";
                Vector2 textSize = MeasureTextEx(tinyFont, warningText, 30, 0);
                DrawTextEx(tinyFont, warningText, { (float)screenWidth / 2.0f - textSize.x / 2.0f, 150.0f }, 30, 0, Fade(RED, 0.5f + pulse * 0.5f));
            }

            // POP UP TEXTS--------------------------------------
            
            if (showStarText)
                DrawTextEx(tinyFont, "LUCKY STAR! +10", { (float)screenWidth / 2.0f - 220.0f, (float)screenHeight - 100.0f }, 45, 2, WHITE);
            if (showMinusText)
                DrawTextEx(tinyFont, "CURSED! -10", { (float)screenWidth / 2.0f - 220.0f, (float)screenHeight - 100.0f }, 45, 2, WHITE);
            if (showSlowText)
                DrawTextEx(tinyFont, "HEXED! SLOWED...", { (float)screenWidth / 2.0f - 220.0f, (float)screenHeight - 100.0f }, 45, 2, WHITE);

            if (quakeTimer > 1.0f && quakeTimer < 3.0f)
                DrawTextEx(tinyFont, "THE EARTH TREMBLES!", { (float)screenWidth / 2.0f - 250.0f, (float)screenHeight - 100.0f }, 45, 2, RED);
            
            if (quakeTimer > 3.0f && quakeTimer < 5.5f)
                DrawTextEx(tinyFont, "FLEE! FLEE! FLEE!", { (float)screenWidth / 2.0f - 180.0f, (float)screenHeight - 100.0f }, 55, 2, WHITE);

             string eventName = "";// events

            auto GetEventName = [&](EventType e)
            {
                if (e == SPEED_BOOST)
                    return "SPEED BOOST";
                if (e == SLOW_BOOST)
                    return "SLOW CURSE";
                if (e == SWAP_CONTROLS)
                    return "SWAPPED CONTROLS";
                if (e == LOW_GRAVITY)
                    return "LOW GRAVITY";
                if (e == FOG_BLIND)
                    return "CURSED FOG";
                if (e == INVERTED_SCREEN) return "UPSIDE DOWN";
                if (e == EARTHQUAKE) return "EARTHQUAKE";
                if (e == LUCKY_PARTY) return "JACKPOT";
                if (e == MISFORTUNE) return "MISFORTUNE";

                return "";
            };

            eventName = GetEventName(currentEvent);

            if (secondEvent != NONE)
            {
                eventName += " + ";
                eventName += GetEventName(secondEvent);
            }

            if (currentEvent != NONE)
            {
                DrawRectangle(15, 90, 420, 40, Fade(BLACK, 0.5f));
                DrawTextEx(tinyFont, eventName.c_str(), {25.0f, 100.0f}, 28, 0, RED);
            }
            //effect when the player fell into the pit
            if (fallingInPit)
            {
                float fallStartY = screenHeight * 0.75f;
                float fallDistance = player.y - fallStartY;

                float alpha = fallDistance / 400.0f;

                if (alpha < 0.0f) alpha = 0.0f;
                if (alpha > 1.0f) alpha = 1.0f;

                DrawRectangle(
                    0,
                    0,
                    screenWidth,
                    screenHeight,
                    Fade(BLACK, alpha)
                );
            }

            // combo 
            if (combo >= 10)
                DrawTextEx(tinyFont, "NAPAKAGALING!", { (float)screenWidth / 2.0f - 140.0f, 20.0f }, 50, 2, RED);
            else if (combo >= 5)
                DrawTextEx(tinyFont, "HOTSTREAK!!", { (float)screenWidth / 2.0f - 120.0f, 20.0f }, 45, 2, ORANGE);
            else if (combo > 1)
                DrawTextEx(tinyFont, TextFormat("COMBO x%d", combo), { (float)screenWidth / 2.0f - 100.0f, 20.0f }, 40, 2, YELLOW);

            if (comboBroken)
                DrawTextEx(tinyFont, "STREAK BROKEN!", { (float)screenWidth / 2.0f - 120.0f, 70.0f }, 40, 2, RED);
            if (quakeTimer > 1.2f && quakeTimer < 2.0f)
            {
                DrawTextEx(tinyFont,
                    "CRACKS BENEATH YOU!",
                    { (float)screenWidth / 2.0f - 220.0f, 120.0f },
                    40, 2,
                    RED
                );
            }
        }
        else if (state == TROLL_VIDEO)
        {
            frameTimer += GetFrameTime();
            if (frameTimer >= 0.2f) {   // adjust playback speed here
                 frameTimer = 0.0f;
                 currentFrame++;
        }

            ClearBackground(WHITE);
            if (!videoFrames.empty() && currentFrame < videoFrames.size())
            {
                DrawTexturePro(
                    videoFrames[currentFrame],
                    {0, 0,
                     (float)videoFrames[currentFrame].width,
                     (float)videoFrames[currentFrame].height},
                    {0, 0,
                     (float)screenWidth,
                     (float)screenHeight},
                    {0, 0},
                    0,
                    WHITE);

               DrawTextEx(tinyFont, "RELAPSE ", { (float)screenWidth / 2.0f - 350.0f, (float)screenHeight - 100.0f }, 45, 2, BLUE);
               DrawTextEx(tinyFont, "KA ", { (float)screenWidth / 2.0f - 350.0f + MeasureTextEx(tinyFont, "RELAPSE ", 45, 2).x, (float)screenHeight - 100.0f }, 45, 2, YELLOW);
               DrawTextEx(tinyFont, "MUNA ", { (float)screenWidth / 2.0f - 350.0f + MeasureTextEx(tinyFont, "RELAPSE KA ", 45, 2).x, (float)screenHeight - 100.0f }, 45, 2, BLUE);
               DrawTextEx(tinyFont, "BOI HAHA :((", { (float)screenWidth / 2.0f - 350.0f + MeasureTextEx(tinyFont, "RELAPSE KA MUNA ", 45, 2).x, (float)screenHeight - 100.0f }, 45, 2, YELLOW);

                DrawTextEx(tinyFont,
                    "Press ENTER to skip",
                    { (float)screenWidth / 2.0f - 130.0f, (float)screenHeight - 50.0f },
                    30, 2,
                    GRAY);
            }

            if (IsKeyPressed(KEY_ENTER)) {
             UnloadBg1TransitionVideo(); 
            state = PLAYING; 
            continue;          
             }

             if (!IsSoundPlaying(trollSound)) {   // or IsMusicStreamPlaying if you used LoadMusicStream
             state = PLAYING;
             continue;
              }
        }

        if (hitFlash > 0)
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, hitFlash));

        // heartbeat text
        if (hp == 1)
        {
            const char* hpWarning = "WARNING!";
            int pixelPulse = 24 + (int)(sinf(GetTime() * 8) * 6);
            Vector2 textSize = MeasureTextEx(tinyFont, hpWarning, (float)pixelPulse, 0);
            DrawTextEx(tinyFont, hpWarning, { (float)screenWidth / 2.0f - textSize.x / 2.0f, 50.0f }, (float)pixelPulse, 0, RED);
        }

        // PAUSED SCREEN DRAWING
        if (state == PAUSED)
        {
            float panelW = 340, panelH = 260;
            float panelX = screenWidth / 2.0f - panelW / 2.0f;
            float panelY = screenHeight / 2.0f - panelH / 2.0f;
            float btnW = 220, btnH = 50;
            float btnX = panelX + panelW/2 - btnW/2;
            float resumeY = panelY + 90;
            float exitY = resumeY + btnH + 18;

            Rectangle resumeRect = {btnX, resumeY, btnW, btnH};
            Rectangle exitRect   = {btnX, exitY,   btnW, btnH};
            bool hoverResume = CheckCollisionPointRec(GetMousePosition(), resumeRect);
            bool hoverExit   = CheckCollisionPointRec(GetMousePosition(), exitRect);

            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
            DrawRectangleRounded({panelX, panelY, panelW, panelH}, 0.15f, 8, {20, 20, 20, 230});
            DrawRectangleRoundedLines({panelX, panelY, panelW, panelH}, 0.15f, 8, Color{180, 180, 180, 200});

            Vector2 titleSize = MeasureTextEx(tinyFont, "PAUSED", 52, 0);
            DrawTextEx(tinyFont, "PAUSED",
                {panelX + panelW/2 - titleSize.x/2, panelY + 18}, 52, 0, WHITE);

            DrawRectangleRounded(resumeRect, 0.3f, 6, hoverResume ? Color{60, 180, 60, 255} : Color{40, 120, 40, 220});
            Vector2 resumeSize = MeasureTextEx(tinyFont, "RESUME", 30, 0);
            DrawTextEx(tinyFont, "RESUME",
                {btnX + btnW/2 - resumeSize.x/2, resumeY + btnH/2 - resumeSize.y/2}, 30, 0, WHITE);

            DrawRectangleRounded(exitRect, 0.3f, 6, hoverExit ? Color{200, 40, 40, 255} : Color{130, 20, 20, 220});
            Vector2 exitSize = MeasureTextEx(tinyFont, "EXIT TO MENU", 30, 0);
            DrawTextEx(tinyFont, "EXIT TO MENU",
                {btnX + btnW/2 - exitSize.x/2, exitY + btnH/2 - exitSize.y/2}, 30, 0, WHITE);
        }

        // GAME OVER----------------------------------------------------
        if (state == GAMEOVER_ANIM)
{
    UpdateMusicStream(gameOverMusic);
    ClearBackground(BLACK);

    // BACKGROUND
    DrawTexturePro(
        gameOverBg,
        {0, 0, (float)gameOverBg.width, (float)gameOverBg.height},
        {0, 0, (float)screenWidth, (float)screenHeight},
        {0, 0}, 0.0f, WHITE
    );

    // dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.45f));

    // FLASH EFFECT (first 2.5s)
    if (gameOverAnimTimer < 2.5f)
    {
        if (gameOverFlash > 0)
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, gameOverFlash));

        float scale = Clamp(gameOverAnimTimer / 0.6f, 0.0f, 1.0f);
        int fontSize = (int)(160 * scale);

        Vector2 gameSize = MeasureTextEx(nosifer, "GAME", (float)fontSize, 4);
        Vector2 overSize = MeasureTextEx(nosifer, "OVER", (float)fontSize, 4);

        float gameX = screenWidth / 2.0f - gameSize.x / 2.0f;
        float overX = screenWidth / 2.0f - overSize.x / 2.0f;
        float gameY = screenHeight / 2.0f - gameSize.y - 10;
        float overY = screenHeight / 2.0f;

        DrawTextEx(nosifer, "GAME", {gameX + 5, gameY + 5}, (float)fontSize, 4, {40, 40, 40, 255});
        DrawTextEx(nosifer, "GAME", {gameX, gameY}, (float)fontSize, 4, WHITE);
        DrawTextEx(nosifer, "OVER", {overX + 5, overY + 5}, (float)fontSize, 4, {60, 0, 0, 255});
        DrawTextEx(nosifer, "OVER", {overX, overY}, (float)fontSize, 4, {200, 20, 20, 255});
    }

    if (gameOverAnimTimer >= 2.5f)
    {
        if (score > highScore) highScore = score;
       

        // ── TOP-RIGHT: Score + High Score (no box) ──
        float scoreX = (float)(screenWidth - 220);

        DrawTextEx(gamefont, "SCORE", {scoreX, 20}, 28, 1, {200, 200, 200, 255});
        DrawTextEx(nosifer, TextFormat("%d", score), {scoreX, 52}, 48, 1, {200, 20, 20, 255});

        DrawTextEx(gamefont, "HIGH SCORE", {scoreX, 110}, 24, 1, {170, 170, 204, 255});
        DrawTextEx(nosifer, TextFormat("%d", highScore), {scoreX, 138}, 44, 1, {120, 200, 255, 255});

        if (score >= highScore)
             DrawTextEx(gamefont, "* NEW BEST!", {scoreX, 190}, 22, 1, {200, 20, 20, 255});

        // ── GAME OVER TITLE
        Vector2 goSize = MeasureTextEx(nosifer, "GAME OVER", 140, 4);
        float goX = screenWidth / 2.0f - goSize.x / 2.0f;
        float goY = screenHeight / 2.0f - goSize.y - 80;

        // Shadow
        DrawTextEx(nosifer, "GAME OVER", {goX + 5, goY + 5}, 140, 4, {40, 40, 40, 255});

        // Draw "GAME" in white
        Vector2 gameWordSize = MeasureTextEx(nosifer, "GAME ", 140, 4);
        DrawTextEx(nosifer, "GAME ", {goX, goY}, 140, 4, WHITE);

        // Draw "OVER" in red right after "GAME "
        DrawTextEx(nosifer, "OVER", {goX + gameWordSize.x, goY}, 140, 4, {200, 20, 20, 255});   

        // ── TRY AGAIN? + YES / NO ──
        float menuY = goY + goSize.y + 100;

        Vector2 trySize = MeasureTextEx(gamefont, "TRY AGAIN?", 60, 1);
        DrawTextEx(gamefont, "TRY AGAIN?",
         {screenWidth / 2.0f - trySize.x / 2.0f, menuY}, 60, 1, WHITE);

        menuY += 80;

        // YES
        bool hoverPlay = CheckCollisionPointRec(GetMousePosition(),
            {screenWidth / 2.0f - 80, menuY, 160, 50});
        Color yesCol = hoverPlay ? Color{200, 20, 20, 255} : WHITE;
        Vector2 yesSize = MeasureTextEx(gamefont, "YES", 60, 1);
        DrawTextEx(gamefont, hoverPlay ? "> YES" : "YES",
          {screenWidth / 2.0f - yesSize.x / 2.0f, menuY}, 60, 1, yesCol);

        menuY += 75;

        // NO
        bool hoverMenu = CheckCollisionPointRec(GetMousePosition(),
            {screenWidth / 2.0f - 80, menuY, 160, 50});
        Color noCol = hoverMenu ? Color{200, 20, 20, 255} : WHITE;
        Vector2 noSize = MeasureTextEx(gamefont, "NO", 60, 1);
        DrawTextEx(gamefont, hoverMenu ? "> NO" : "NO",
            {screenWidth / 2.0f - noSize.x / 2.0f, menuY}, 60, 1, noCol);

        // ── INPUT ──
        float btn2Y = goY + goSize.y + 30 + 46 + 38;

        if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverPlay))
        {
            StopMusicStream(gameOverMusic);
            PlayMusicStream(bgMusic);
            // ... your existing reset code ...
            state = PLAYING;
            hp = 3; score = 0; combo = 0;
            lastDiff = EASY;
            comboBroken = false;
        comboBrokenTimer = 0;
        comboTime = 0;
        items.clear();
        popEffects.clear();
        player.x = (screenWidth - player.width) / 2;
        player.y = screenHeight * 0.75f;
        move = 1.0f;
        chiliBoost = 1.0f;
        eventBoost = 1.0f;
        gravity = 1800.0f;
        velocityY = 0;
        velocityX = 0;
        isGrounded = true;
    
        spawnTimer = 0;
        eventCooldown = 15.0f; 
        eventTimer = 0;
        currentEvent = NONE;
        secondEvent = NONE;
        lastEvent = NONE;
        secondLastEvent = NONE;
        eventWarningTimer = 0;
        
        slowTimer = 0;
        speedBoostTimer = 0;
        medkitCooldown = 0;
        
        showStarText = false;
        showMinusText = false;
        showSlowText = false;
        starTextTimer = 0;
        minusTextTimer = 0;
        slowTextTimer = 0;
        
        shakeTime = 0;
        shakePower = 0;
        hitFlash = 0;
        
        fogActive = false;
        fogAlpha = 0;
        
        fallingInPit = false;
        pitCreated = false;
        pitSoundPlayed = false;
        quakeActive = false;
        quakeTimer = 0;
        pits.clear();
        pitWidths.clear();
        pitCenters.clear();
        pitOpens.clear();
    
        camera.rotation = 0;
        camera.zoom = 1.30f;
        camera.target = { player.x + player.width / 2, player.y + player.height / 2 };

        invertedScreen = false;
        gameOverAnimTimer = 0.0f;
        diff = EASY;
        bg1Triggered = false;
        bg2Triggered = false;
        UnloadBg1TransitionVideo();
        UnloadBg2TransitionVideo();
        InitBg1TransitionVideo();
        InitBg2TransitionVideo();

    }
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverMenu))
        {
            StopMusicStream(gameOverMusic);
            // ... your existing reset + go to MENU code ...
            state = MENU;
            hp = 3; score = 0; combo = 0;
            lastDiff = EASY;
            comboBroken = false;
        comboBrokenTimer = 0;
        comboTime = 0;
        items.clear();
        popEffects.clear();
        player.x = (screenWidth - player.width) / 2;
        player.y = screenHeight * 0.75f;
        move = 1.0f;
        chiliBoost = 1.0f;
        eventBoost = 1.0f;
        gravity = 1800.0f;
        velocityY = 0;
        velocityX = 0;
        isGrounded = true;

        spawnTimer = 0;
        eventCooldown = 15.0f; 
        eventTimer = 0;
        currentEvent = NONE;
        secondEvent = NONE;
        lastEvent = NONE;
        secondLastEvent = NONE;
        eventWarningTimer = 0;

        slowTimer = 0;
        speedBoostTimer = 0;
        medkitCooldown = 0;

        showStarText = false;
        showMinusText = false;
        showSlowText = false;
        starTextTimer = 0;
        minusTextTimer = 0;
        slowTextTimer = 0;

        shakeTime = 0;
        shakePower = 0;
        hitFlash = 0;

        fogActive = false;
        fogAlpha = 0;

        fallingInPit = false;
        pitCreated = false;
        pitSoundPlayed = false;
        quakeActive = false;
        quakeTimer = 0;
        pits.clear();
        pitWidths.clear();
        pitCenters.clear();
        pitOpens.clear();

        camera.rotation = 0;
        camera.zoom = 1.30f;
        camera.target = { player.x + player.width / 2, player.y + player.height / 2 };

        invertedScreen = false;
        gameOverAnimTimer = 0.0f;
        diff = EASY;
        bg1Triggered = false;
        bg2Triggered = false;
        UnloadBg1TransitionVideo();
        UnloadBg2TransitionVideo();
        InitBg1TransitionVideo();
        InitBg2TransitionVideo();

        introMusic = LoadMusicStream("assets/sounds/intro.mp3");
        SetMusicVolume(introMusic, 0.5f);
        PlayMusicStream(introMusic);
        }
    }
}
        EndDrawing();
    }

    for (auto &t : videoFrames)
        UnloadTexture(t);

    UnloadBg1TransitionVideo();
    UnloadBg2TransitionVideo();

    for (auto &t : memeTextures)
        UnloadTexture(t);

    UnloadTexture(introTex);
    UnloadInfoTexture(); 
    UnloadTexture(gameOverBg);  
    UnloadTexture(hpTex);


    currentFrame = 0;
    frameTimer = 0;

    for (auto &s : memeSounds)
        UnloadSound(s);

    // unload
    UnloadSound(trollSound);
    UnloadSound(gameOverSound);
    UnloadSound(pitSound);
    UnloadMusicStream(gameOverMusic);
    UnloadMusicStream(bgMusic);
    UnloadFont(nosifer);
    UnloadFont(gamefont);
    UnloadFont(tinyFont);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
