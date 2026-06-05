#include "intro.h" 
#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cmath>
#include "raymath.h"
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

enum GameState
{
    LOGO,
    TITLE_LOGO,
    MENU,
    STORYLINE,
    TRANSITION,
    PLAYING,
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
    DICE,
    MUSHROOM,
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

// Background frames
vector<Texture2D> bgFrames;
int currentBgFrame = 0;
float bgFrameTimer = 0.0f;
float bgFrameDelay = 0.05f;

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

struct Notif {
    string text;
    Color  color;
    float  timer;
    float  maxTime;
};
vector<Notif> notifs;

void PushNotif(vector<Notif>& v, const string& text, Color col, float dur = 2.0f) {
    if (v.size() >= 5) v.erase(v.begin()); // Increased room for events
    v.push_back({text, col, dur, dur});
}

string GetEventName(EventType e) {
    if (e == SPEED_BOOST)    return "SPEED BOOST";
    if (e == SLOW_BOOST)     return "SLOW CURSE";
    if (e == SWAP_CONTROLS)  return "SWAPPED CONTROLS";
    if (e == LOW_GRAVITY)    return "LOW GRAVITY";
    if (e == FOG_BLIND)      return "CURSED FOG";
    if (e == INVERTED_SCREEN) return "UPSIDE DOWN";
    if (e == EARTHQUAKE)     return "EARTHQUAKE";
    if (e == LUCKY_PARTY)    return "JACKPOT";
    if (e == MISFORTUNE)     return "MISFORTUNE";
    return "";
}

int main()
{
    SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Raylib - Wings of the Curse");

    InitAudioDevice();
    trollSound = LoadSound("assets/sounds/trollFace.mp3");

    Sound goodItemSound = LoadSound("assets/sounds/good_item2.mp3");
    Sound badItemSound  = LoadSound("assets/sounds/bad_item.mp3");

    Sound walkSound = LoadSound("assets/sounds/walk.mp3");
    Sound jumpSound = LoadSound("assets/sounds/jump.mp3");
    SetSoundVolume(walkSound, 2.0f);
    SetSoundVolume(jumpSound, 0.2f);

    Music bgEasy   = LoadMusicStream("assets/sounds/bg_easy.mp3");
    Music bgMedium = LoadMusicStream("assets/sounds/bg_medium.mp3");
    Music bgHard   = LoadMusicStream("assets/sounds/bg_hard.mp3");

    float bgTargetVolume = 1.0f;
    SetMusicVolume(bgEasy,   bgTargetVolume);
    SetMusicVolume(bgMedium, bgTargetVolume);
    SetMusicVolume(bgHard,   bgTargetVolume);

    Music* currentBg = &bgEasy; 

    bool isCrossfading = false;
    Music* nextBg = nullptr;
    float crossfadeTimer = 0.0f;
    float crossfadeDuration = 0.5f; // seconds, change if you want longer/shorter fades

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
    for (int i = 1; i <= 110; i++)  videoFrames.push_back(LoadTexture(TextFormat("assets/videos/trollFace/ezgif-frame-%03d.png", i)));

    // Load background frames
    for (int i = 1; i <= 100; i++) bgFrames.push_back(LoadTexture(TextFormat("assets/videos/bg/ezgif-frame-%03d.png", i)));

    
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

    // chaos metrics
    float totalTimePlayed = 0.0f;
    int eventsSurvived = 0;
    int totalItemsCollected = 0;
    float chaosLevel = 0.0f; 

    // load images
    Texture2D bgTex = LoadTexture("assets/images/bg.png");
    Texture2D introTex = LoadTexture("assets/images/background.png");
    Texture2D gameOverBg = LoadTexture("assets/images/gameover.png");
    Texture2D titleTex = LoadTexture("assets/images/title1.png");

    // MEME POP-UP TEXTURES
    // ADD MORE MEMES HERE: Just add more textures to the memeTextures vector
    // 1. Kapre
    memeTextures.push_back(LoadTexture("assets/images/kapre.png"));
    memeSounds.push_back(LoadSound("assets/sounds/Creepy Hello Kapre.mp3"));

    // 2. Broomwitch
    memeTextures.push_back(LoadTexture("assets/images/broomwitch.png"));
    memeSounds.push_back(LoadSound("assets/sounds/Witch Broom.mp3"));

    // 3. Witch
    memeTextures.push_back(LoadTexture("assets/images/witch.png"));
    memeSounds.push_back(LoadSound("assets/sounds/Witch Potion.mp3"));

    // 4. Mermaid
    memeTextures.push_back(LoadTexture("assets/images/mermaid.png"));
    memeSounds.push_back(LoadSound("assets/sounds/mermaid.mp3"));

    // 5. Minion
    memeTextures.push_back(LoadTexture("assets/images/minion.png"));
    memeSounds.push_back(LoadSound("assets/sounds/Minion.mp3"));

    // 6. Tiyanak 
    memeTextures.push_back(LoadTexture("assets/images/tiyanak.png"));
    memeSounds.push_back(LoadSound("assets/sounds/tiyanak.mp3")); 

    // 7. Tikbalang
    memeTextures.push_back(LoadTexture("assets/images/tikbalang.png"));
    memeSounds.push_back(LoadSound("assets/sounds/tiktik.mp3"));

    // 8. Dwende
    memeTextures.push_back(LoadTexture("assets/images/dwende.png"));
    memeSounds.push_back(LoadSound("assets/sounds/dwende.mp3"));


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

    GameState state = LOGO;
    InitGroupLogo();
     InitTitleLogo();
     InitIntro();

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

    bool isMuted = false;

    bool showPauseMenu = false;

    // SCORE & HEALTH
    int score = 0;
    int highScore = 0;
    int hp = 3;
    //saved highScore
    ifstream file("score.txt");

    if (file.is_open())
    {
        file >> highScore;
        file.close();
    }

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
    Sound uiClickSound = LoadSound("assets/sounds/button_click.mp3");
    Sound hoverSound = LoadSound("assets/sounds/hoverSound.mp3");

    //extra 
    float medkitCooldown = 0;

    // EVENTS----------------------------------------------
    EventType currentEvent = NONE;
    EventType secondEvent = NONE; // for HARD MODE extra event

    float eventTimer = 0.0f;
    
    // --- ADJUST EVENT COOLDOWNS HERE ---
    float cooldownEasy   = 20.0f; // How many seconds between events in EASY
    float cooldownMedium = 15.0f; // How many seconds between events in MEDIUM
    float cooldownHard   = 10.0f; // How many seconds between events in HARD
    float initialCooldown = 30.0f; // Initial wait before the first event starts
    // ------------------------------------

    float eventCooldown = initialCooldown; 
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
    int fallingPitIndex = -1; // track which pit for boundary clamping

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
    Difficulty diff = EASY;
    Difficulty lastDiff = EASY; // track difficulty changes for grace periods
    bool bg1Triggered = false;
    bool bg2Triggered = false;

    // jump
    float gravity = (diff == HARD) ? 700.0f : 1800.0f;   // pull down
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
    float hitTimer = 0.0f; 
    float nearMissTimer = 0.0f;
    float milestoneCelebrationTimer = 0.0f;
    vector<Warning> activeWarnings;

    // JUICE & FEEL VARIABLES
    Vector2 squashStretch = { 1.0f, 1.0f };
    float playerRotation = 0.0f;
    float hitStopTimer = 0.0f;
    float landSquashTimer = 0.0f;

    while (!WindowShouldClose()) {
    
      // intro logo
        if (state == LOGO)
    {
        UpdateMusicStream(introMusic);
        bool done = UpdateGroupLogo();
        if (done) {
            UnloadGroupLogo();
            state = TITLE_LOGO;
        }
    }

    else if (state == TITLE_LOGO)
    {
        UpdateMusicStream(introMusic);
        bool done = UpdateTitleLogo();
        if (done) {
            UnloadTitleLogo();
            state = MENU;
        }
    }

            // MENU-----------------------------------------------
            else if (state == MENU)
    {
    UpdateMusicStream(introMusic);

    int action = UpdateIntro();

    if (action == 1) // Play
    {
        InitStoryline();
        state = STORYLINE;
}
    else if (action == 2) // Exit
    {
        StopMusicStream(introMusic);
        UnloadMusicStream(introMusic);
        break;
    }
    else if (action == 3) // Info opened/closed
    {
        
    }

    DrawIntro(highScore, introTex, titleTex);
}

         else if (state == STORYLINE) {
            UpdateMusicStream(introMusic);
             bool done = UpdateStoryline();
             
             if (done) {
                 UnloadStoryline();
                 StopMusicStream(introMusic);
                 UnloadMusicStream(introMusic);
                
                 if (currentBg) {
                SetMusicVolume(*currentBg, bgTargetVolume);
                PlayMusicStream(*currentBg);
            }
            state = PLAYING;
        }

            
         }


        else if (state == PAUSED) {
        float panelW = 340, panelH = 340; 
        float panelX = screenWidth / 2.0f - panelW / 2.0f;
        float panelY = screenHeight / 2.0f - panelH / 2.0f;
        float btnW = 220, btnH = 50;
        float btnX = panelX + panelW/2 - btnW/2;
        
        // Consistent spacing with drawing section (20px)
        float resumeY = panelY + 90;
        float muteY   = resumeY + btnH + 20;
        float exitY   = muteY + btnH + 20;

        Rectangle resumeRect = {btnX, resumeY, btnW, btnH};
        Rectangle muteRect   = {btnX, muteY,   btnW, btnH};
        Rectangle exitRect   = {btnX, exitY,   btnW, btnH};
        
        bool hoverResume = CheckCollisionPointRec(GetMousePosition(), resumeRect);
        bool hoverMute   = CheckCollisionPointRec(GetMousePosition(), muteRect);
        bool hoverExit   = CheckCollisionPointRec(GetMousePosition(), exitRect);

        if ((hoverResume && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ||
            IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
            PlaySound(uiClickSound);
            state = PLAYING;
        }

        if (hoverMute && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlaySound(uiClickSound);
            isMuted = !isMuted;
            SetMasterVolume(isMuted ? 0.0f : 1.0f);
        }

        if (hoverExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlaySound(uiClickSound);
            StopMusicStream(bgEasy);
            StopMusicStream(bgMedium);
            StopMusicStream(bgHard);
            state = MENU;
            hp = 3; score = 0; combo = 0;
            diff = EASY; bg1Triggered = false; bg2Triggered = false;
            lastDiff = EASY; comboBroken = false; comboBrokenTimer = 0; comboTime = 0;
            items.clear(); popEffects.clear();
            notifs.clear(); 
            player.x = (screenWidth - player.width) / 2;
            player.y = screenHeight * 0.75f;
            move = 1.0f; chiliBoost = 1.0f; eventBoost = 1.0f;
            gravity = 1800.0f; velocityY = 0; velocityX = 0; isGrounded = true;
            spawnTimer = 0; eventCooldown = initialCooldown; eventTimer = 0;
            currentEvent = NONE; secondEvent = NONE;
            lastEvent = NONE; secondLastEvent = NONE; eventWarningTimer = 0;
            slowTimer = 0; speedBoostTimer = 0; medkitCooldown = 0;
            shakeTime = 0; shakePower = 0; hitFlash = 0;
            fogActive = false; fogAlpha = 0;
            fallingInPit = false; pitCreated = false; pitSoundPlayed = false;
            quakeActive = false; quakeTimer = 0;
            pits.clear(); pitWidths.clear(); pitCenters.clear(); pitOpens.clear();
            camera.rotation = 0; camera.zoom = 1.30f;
            camera.target = {player.x + player.width/2, player.y + player.height/2};
            invertedScreen = false; gameOverAnimTimer = 0.0f;
            currentBgFrame = 0; bgFrameTimer = 0.0f;
            UnloadBg1TransitionVideo(); UnloadBg2TransitionVideo();
            InitBg1TransitionVideo();   InitBg2TransitionVideo();
            introMusic = LoadMusicStream("assets/sounds/intro.mp3");
            SetMusicVolume(introMusic, 0.5f);
            PlayMusicStream(introMusic);
        }
    }

       else if (state == TROLL_VIDEO)
        {
            frameTimer += GetFrameTime();

            if (frameTimer >= 0.2f)
            {
                frameTimer = 0;
                currentFrame++;
            }

            if (currentFrame >= (int)videoFrames.size())
            {
                StopSound(trollSound);
                currentFrame = 0;
                state = PLAYING;
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                StopSound(trollSound);
                currentFrame = 0;
                state = PLAYING;
            }
        }

        // GAMEPLAY-----------------------------------------
        else if (state == PLAYING)
        {
            Difficulty prevDiff = diff;

            if (score >= 400)
                diff = HARD;
            else if (score >= 100)
                diff = MEDIUM;
            else
                diff = EASY;

            if (diff != prevDiff) {
                if (diff > prevDiff) {
                    // Use difficulty-based cooldown
                    eventCooldown = (diff == MEDIUM) ? cooldownMedium : cooldownHard;
                    eventWarningTimer = 0;
                    PushNotif(notifs, diff == MEDIUM ? "LEVEL: MEDIUM" : "LEVEL: HARD", diff == MEDIUM ? YELLOW : RED, 3.0f);
                }
                lastDiff = diff; 
                gravity = (diff == HARD) ? 700.0f : 1800.0f;
            }

            // --- Music update + crossfade handling ---
            if (isCrossfading && currentBg && nextBg)
            {
                crossfadeTimer += GetFrameTime();
                float t = crossfadeTimer / crossfadeDuration;
                if (t > 1.0f) t = 1.0f;

                float volOut = Lerp(bgTargetVolume, 0.0f, t);
                float volIn  = Lerp(0.0f, bgTargetVolume, t);

                SetMusicVolume(*currentBg, volOut);
                SetMusicVolume(*nextBg, volIn);

                UpdateMusicStream(*currentBg);
                UpdateMusicStream(*nextBg);

                if (t >= 1.0f)
                {
                    StopMusicStream(*currentBg);
                    SetMusicVolume(*nextBg, bgTargetVolume);

                    currentBg = nextBg;
                    nextBg = nullptr;
                    isCrossfading = false;
                    crossfadeTimer = 0.0f;
                }
            }
            else
            {
                if (currentBg) UpdateMusicStream(*currentBg);
            }

            if (diff != prevDiff)
            {
                Music* chosen = nullptr;
                if (diff == EASY)    chosen = &bgEasy;
                else if (diff == MEDIUM) chosen = &bgMedium;
                else /* HARD */      chosen = &bgHard;

                if (chosen != nullptr && chosen != currentBg)
                {
                    nextBg = chosen;
                    isCrossfading = true;
                    crossfadeTimer = 0.0f;

                    SetMusicVolume(*nextBg, 0.0f);
                    
                    float startTime = 0.0f;
                    if (nextBg == &bgMedium) startTime = 2.0f * 60.0f + 25.0f; 
                    else if (nextBg == &bgHard)   startTime = 1.0f * 60.0f + 10.0f; 

                    SeekMusicStream(*nextBg, startTime);
                    PlayMusicStream(*nextBg);
                }
            }

           if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
             state = PAUSED;
            }

            //BACKGROUND VIDEO
            if (diff == EASY) {
                currentBgFrame = 0; 
            } else if (diff == MEDIUM) {
                if (currentBgFrame < 49) { 
                    bgFrameTimer += GetFrameTime();
                    if (bgFrameTimer >= bgFrameDelay) {
                        bgFrameTimer = 0;
                        currentBgFrame++;
                    }
                } else {
                    currentBgFrame = 49; 
                }
            } else if (diff == HARD) {
                if (currentBgFrame < 99) { 
                    bgFrameTimer += GetFrameTime();
                    if (bgFrameTimer >= bgFrameDelay) {
                        bgFrameTimer = 0;
                        currentBgFrame++;
                    }
                } else {
                    currentBgFrame = 99; 
                }
            }

            // movements 
            float accel = 2200.0f * move * baseMove;  // how fast player gains speed
            float friction = 0.92f; // slows naturally
            float maxSpeed = 520.0f * move * baseMove * chiliBoost * eventBoost;
            // Gravity
            velocityY += gravity * GetFrameTime();

            // Update player position
            player.y += velocityY * GetFrameTime();

            // Milestone Celebration Logic
            static int lastMilestone = 0;
            if (score / 100 > lastMilestone) {
                lastMilestone = score / 100;
                milestoneCelebrationTimer = 2.0f;
                PushNotif(notifs, TextFormat("MILESTONE: %d!", lastMilestone * 100), GOLD, 3.0f);
            }
            
            // RESET lastMilestone if game restarts
            if (score == 0) lastMilestone = 0;

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
            float groundY = screenHeight * 0.85f; //ground level

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
                
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
                velocityX -= accel * GetFrameTime() * chiliBoost * eventBoost * dir;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
                velocityX += accel * GetFrameTime() * chiliBoost * eventBoost * dir;

            player.x += velocityX * GetFrameTime();

            // left and right boundaries
            if (fallingInPit && fallingPitIndex != -1)
            {
                // Clamp to pit walls
                float pitL = pits[fallingPitIndex].x;
                float pitR = pits[fallingPitIndex].x + pits[fallingPitIndex].width;
                if (player.x < pitL) player.x = pitL;
                if (player.x + player.width > pitR) player.x = pitR - player.width;
            }
            else
            {
                if (player.x < 0)
                    player.x = 0;
                if (player.x + player.width > screenWidth)
                    player.x = screenWidth - player.width;
            }

            // --- CAMERA AND FLIP LOGIC ---
            Vector2 shakeOffset = {0, 0};
            if (shakeTime > 0)
            {
                float intensity = shakePower * (shakeTime / 0.25f);
                shakeOffset.x = (GetRandomValue(-100, 100) / 100.0f) * intensity;
                shakeOffset.y = (GetRandomValue(-100, 100) / 100.0f) * intensity;
            }

            if (!fallingInPit)
            {
                // Instant flip: 180 degrees (upside down) if event is active, otherwise 0
                camera.rotation = invertedScreen ? 180.0f : 0.0f;
                camera.zoom = 1.30f; // Keep zoom simple and fixed

                // Camera follows player vertical position
                camera.target.y = player.y + player.height / 2;
            }

            // Center camera offset (This makes the 180-degree flip perfectly symmetrical)
            camera.offset = {
                screenWidth / 2.0f + shakeOffset.x,
                screenHeight / 2.0f + shakeOffset.y 
            };

            // Smoothly follow player horizontal position
            float wantedX = player.x + player.width / 2;
            camera.target.x += (wantedX - camera.target.x) * 0.12f;

            // Simple clamping: keep the camera target within bounds so we don't see black edges
            float minX = (screenWidth / 2.0f) / camera.zoom;
            float maxX = screenWidth - (screenWidth / 2.0f) / camera.zoom;
            float minY = (screenHeight / 2.0f) / camera.zoom;
            float maxY = screenHeight - (screenHeight / 2.0f) / camera.zoom;

            camera.target.x = Clamp(camera.target.x, minX, maxX);
            camera.target.y = Clamp(camera.target.y, minY, maxY);

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
                        int goodPool[] = {BABY, HEART, BLOOD, ATAY, DICE, CHILI};
                        it.type = goodPool[rand() % 6];
                    }
                    else if (currentEvent == MISFORTUNE) {
                        int badPool[] = {BOMB, POISON, POO, GARLIC, SALT, MUSHROOM, TROLLFACE, BOMB, POISON};
                        it.type = badPool[rand() % 9];
                    }
                    else {
                        // Priority: Medkit if hp is low
                        if (hp == 1 && medkitCooldown <= 0 && rand() % 100 < 12) {
                            it.type = MEDKIT;
                            medkitCooldown = 15.0f; // shorter cooldown for survival
                        }
                        else if (hp < 3 && rand() % 100 < 5) it.type = BANDAGE;
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
                            int pool[] = {POO, GARLIC, BABY, BLOOD, BOMB, POISON, ATAY, HEART, MUSHROOM, DICE};
                            it.type = pool[rand() % 10];
                            if (rand() % 100 < 22) it.isIllusion = true; 
                        }
                    }

                    if (it.isIllusion) {
                        if (it.type == BABY || it.type == HEART || it.type == BLOOD || it.type == ATAY) 
                            it.illusionTarget = BOMB;
                        else if (it.type == BOMB || it.type == POISON || it.type == POO || it.type == GARLIC) 
                            it.illusionTarget = HEART;
                        else 
                            it.isIllusion = false; 
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
                if (eventCooldown <= 3.0f)
                {
                    if (eventCooldown > 2.95f)  // fires once as it crosses 3.0
                        PushNotif(notifs, "INGAT... MAY PAPARATING!", {255, 68, 68, 255}, 3.0f);
                    eventWarningTimer = eventCooldown;
                }
            }

            // start new event
            if (currentEvent == NONE && eventCooldown <= 0)
            {
                // reset previous
                secondEvent = NONE;
                eventWarningTimer = 0;

                if (diff == EASY)
                {
                    // EASY MODE: Only Slowness and Swapped Controls
                    int easyEvents[] = {SLOW_BOOST, SWAP_CONTROLS};
                    EventType chosen = (EventType)easyEvents[rand() % 2];

                    currentEvent = chosen;
                    secondLastEvent = lastEvent;
                    lastEvent = currentEvent;

                    eventTimer = 11.0f; 
                    eventCooldown = 20.0f; // longer cooldown for easy

                    PushNotif(notifs, GetEventName(currentEvent), {136, 255, 136, 255}, eventTimer);
                }
                else if (diff == MEDIUM)
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

                    eventTimer = 18.0f;
                    eventCooldown = 15.0f;

                    PushNotif(notifs, GetEventName(currentEvent), {255, 221, 51, 255}, eventTimer);
                }

                else if (diff == HARD)
                {
                    // For Hard mode, LOW_GRAVITY is now a constant baseline, so removed from random events
                    int hardEvents[] = {SWAP_CONTROLS, SPEED_BOOST, SLOW_BOOST, FOG_BLIND, INVERTED_SCREEN, EARTHQUAKE, LUCKY_PARTY, MISFORTUNE};

                    // FAIRNESS: Same repeat-prevention for primary event
                    EventType chosen;
                    int attempts = 0;
                    do {
                        chosen = (EventType)hardEvents[rand() % 8];
                        attempts++;
                    } while ((chosen == lastEvent || chosen == secondLastEvent) && attempts < 10);

                    currentEvent = chosen;
                    secondLastEvent = lastEvent;
                    lastEvent = currentEvent;
                    
                    eventTimer = 25.0f;
                    eventCooldown = 10.0f;

                    // Event-specific notif colors (HARD)
                    if (currentEvent == LUCKY_PARTY)
                        PushNotif(notifs, "JACKPOT!", {255, 221, 51, 255}, eventTimer);
                    else if (currentEvent == MISFORTUNE)
                        PushNotif(notifs, "MALAS!", {221, 68, 68, 255}, eventTimer);
                    else if (currentEvent == INVERTED_SCREEN)
                        PushNotif(notifs, "UPSIDE DOWN!", {170, 136, 255, 255}, eventTimer);
                    else if (currentEvent == SWAP_CONTROLS)
                        PushNotif(notifs, "CONTROLS SWAPPED!", {255, 136, 34, 255}, eventTimer);
                    else if (currentEvent == SPEED_BOOST)
                        PushNotif(notifs, "BILIS!!", {255, 221, 51, 255}, eventTimer);
                    else if (currentEvent == SLOW_BOOST)
                        PushNotif(notifs, "SUMPANG PAMPABAGAL....", {136, 204, 255, 255}, eventTimer);
                    else if (currentEvent == LOW_GRAVITY)
                        PushNotif(notifs, "LOW GRAVITY!", {170, 255, 221, 255}, eventTimer);
                    else if (currentEvent == FOG_BLIND)
                        PushNotif(notifs, "CURSED FOG!", {200, 200, 200, 255}, eventTimer);
                    else if (currentEvent == EARTHQUAKE)
                        PushNotif(notifs, "LINDOL!", {255, 136, 34, 255}, eventTimer);
                    else
                        PushNotif(notifs, GetEventName(currentEvent), {255, 170, 34, 255}, eventTimer);

                    if (rand() % 100 < 40)
                    {
                        secondEvent = (EventType)hardEvents[rand() % 8];
                        // prevent same event twice or contradicting events
                        while (secondEvent == currentEvent || 
                               (currentEvent == MISFORTUNE && secondEvent == LUCKY_PARTY) ||
                               (currentEvent == LUCKY_PARTY && secondEvent == MISFORTUNE) ||
                               (currentEvent == SPEED_BOOST && secondEvent == SLOW_BOOST) ||
                               (currentEvent == SLOW_BOOST && secondEvent == SPEED_BOOST))
                        {
                            secondEvent = (EventType)hardEvents[rand() % 8];
                        }
                        
                        // Notify about the second event too!
                        PushNotif(notifs, GetEventName(secondEvent) + " TOO!", {255, 85, 85, 255}, eventTimer);
                    }
                }
            }

            // EVENTS SYSTEM---------------------------------------------------
            if (currentEvent != NONE)
            {

                eventTimer -= GetFrameTime();
                // reset effects every frame first
                eventBoost = 1.0f;
                gravity = (diff == HARD) ? 700.0f : 1800.0f;
                

                // helper lambda
                auto ApplyEvent = [&](EventType e)
                {
                    if (e == SPEED_BOOST)
                        eventBoost *= 1.7f;
                    if (e == SLOW_BOOST)
                        eventBoost *= 0.65f;
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
                if (eventTimer <= 0) {

                    currentEvent = NONE;
                    secondEvent = NONE;

                    fogActive = false;
                    fogAlpha = 0.0f;
                    fogFadingOut = false;

                    gravity = (diff == HARD) ? 700.0f : 1800.0f;
                    eventBoost = 1.0f;

                    invertedScreen = false;

                    quakeActive = false;
                    fallingInPit = false;

                    // RESET COOLDOWN
                    if (diff == MEDIUM)
                       eventCooldown = cooldownMedium;
                    else if (diff == HARD)
                       eventCooldown = cooldownHard;
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
                if (quakeTimer > 1.0f && quakeTimer < 1.05f)
                    PushNotif(notifs, "THE EARTH TREMBLES!", {255, 170, 34, 255}, 2.5f);
                if (quakeTimer > 3.5f && quakeTimer < 3.55f)
                    PushNotif(notifs, "FLEE! FLEE! FLEE!", {238, 238, 238, 255}, 2.0f);
            
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
                            screenHeight * 0.82f,
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
                            screenHeight * 0.82f,
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
                            fallingPitIndex = i;
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
                    fallingPitIndex = -1;
            
                    pits.clear();
                    pitWidths.clear();
                    pitCenters.clear();
                    pitOpens.clear();
                }
            }

            // UPDATE ITEMS & COLLISION -----------------
            // ... (rest of the loop) ...
            // [I need to find where to insert the clamping logic]
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
                if (it.isIllusion && it.rect.y > screenHeight * 0.50f) {
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
                bool isGood = (it.type == BABY || it.type == HEART || it.type == BLOOD || it.type == ATAY || it.type == CHILI);
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

                    // ── ITEM SOUND EFFECT ──
                    bool isBadItem  = (it.type == POO    || it.type == BOMB   || it.type == SALT  ||
                                    it.type == GARLIC || it.type == POISON || it.type == MUSHROOM);
                    bool isGoodItem = (it.type == BABY   || it.type == HEART  || it.type == BLOOD ||
                                    it.type == ATAY   || it.type == CHILI  || it.type == BANDAGE ||
                                    it.type == MEDKIT || it.type == DICE);
                    if (isBadItem)  PlaySound(badItemSound);
                    if (isGoodItem) PlaySound(goodItemSound);

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
                        PushNotif(notifs, "STREAK BROKEN!", {255, 68, 68, 255}, 1.5f);

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
                        if (combo == 10)
                            PushNotif(notifs, "NAPAKAGALING!", {255, 68, 68, 255}, 2.0f);
                        else if (combo == 5)
                            PushNotif(notifs, "HOTSTREAK!!", {255, 68, 68, 255}, 2.0f);
                        else if (combo > 1)
                            PushNotif(notifs, TextFormat("COMBO x%d", combo), {255, 221, 51, 255}, 1.6f);
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

                    else if (it.type == BLOOD || it.type == ATAY)
                    { // blood
                        pe.color = RED;
                        score += 3;

                        combo++;
                        comboTime = 2.5f;
                        if (combo == 10)
                            PushNotif(notifs, "NAPAKAGALING!", {255, 68, 68, 255}, 2.0f);
                        else if (combo == 5)
                            PushNotif(notifs, "HOTSTREAK!!", {255, 68, 68, 255}, 2.0f);
                        else if (combo > 1)
                            PushNotif(notifs, TextFormat("COMBO x%d", combo), {255, 221, 51, 255}, 1.6f);
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
                            PushNotif(notifs, "LUCKY STAR!  +10", {255, 221, 51, 255});
                        }
                    }
                    else if (it.type == MUSHROOM)
                    { // mushroom(bad effects)
                        pe.color = MAGENTA;
                        int randomIndex = rand() % 4;
                    if (randomIndex == 1)
                    {
                        score -= 10;
                        hitFlash = 0.35f;
                        PushNotif(notifs, "CURSED!  -10 HUHU", {221, 136, 255, 255});
                    }
                    else if (randomIndex == 2)
                    {
                        move = 0.45f;
                        slowTimer = 4.0f;
                        PushNotif(notifs, "HEXED... SLOWED", {221, 136, 255, 255}, 2.2f);
                        }
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
                StopMusicStream(bgEasy);
                StopMusicStream(bgMedium);
                StopMusicStream(bgHard);
    
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

           // Notif update
            for (int i = notifs.size() - 1; i >= 0; i--) {
                notifs[i].timer -= GetFrameTime();
                if (notifs[i].timer <= 0)
                    notifs.erase(notifs.begin() + i);
            }

            if (slowTimer > 0)
            {
                slowTimer -= GetFrameTime();
                if (slowTimer <= 0)
                    move = 1.0f;
            }


         // PLAYER INPUT & ANIMATIONS-----------------------------
            // Common Jump Input
            if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && isGrounded) {
                velocityY = jumpForce;
                isGrounded = false;
                if (diff != HARD) currentAnim = JUMP;
                playerFrame = 0;
                pframeTimer = 0.0f;
                StopSound(walkSound);
                PlaySound(jumpSound);
            }

            if (diff == HARD) {
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))       currentMananAnim = FLY_RIGHT;
                else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))   currentMananAnim = FLY_LEFT;
                else                            currentMananAnim = FLY_FRONT;

                pframeTimer += GetFrameTime();
                if (pframeTimer >= pframeDelay) {
                    pframeTimer = 0.0f;
                    playerFrame++;
                    if (playerFrame >= 6) playerFrame = 0;
                }
            }
            else if (diff == EASY || diff == MEDIUM) {
                // Determine anim state
                if (!isGrounded) {
                    currentAnim = JUMP;
                } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                    currentAnim = WALK_LEFT;
                } else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                    currentAnim = WALK_RIGHT;
                } else {
                    currentAnim = IDLE;
                }

                // Advance frames
                if (currentAnim == JUMP) {
                    pframeTimer += GetFrameTime();
                    if (pframeTimer >= 0.01f && playerFrame < 4) {
                        pframeTimer = 0.0f;
                        playerFrame++;
                    }
                    if (playerFrame >= 4) playerFrame = 4;

                    if (isGrounded) {
                        playerFrame = 0;
                    }
               } else {
                    pframeTimer += GetFrameTime();
                    if (pframeTimer >= pframeDelay) {
                        pframeTimer = 0.0f;
                        playerFrame++;
                        if (playerFrame >= 6) playerFrame = 0;
                    }
                    if (currentAnim == WALK_LEFT || currentAnim == WALK_RIGHT) {
                        if (!IsSoundPlaying(walkSound))
                            PlaySound(walkSound);
                    } else {
                        StopSound(walkSound);
                    }
                }
            }

        // MEME POP-UP UPDATE
        if (state == PLAYING) {
            totalTimePlayed += GetFrameTime();
            
            // CHAOS LEVEL CALCULATION 
            float scoreTarget = 1000.0f;
            chaosLevel = Clamp((float)score / scoreTarget, 0.0f, 0.7f);
            if (diff == MEDIUM) chaosLevel += 0.15f;
            if (diff == HARD) chaosLevel += 0.3f;
            chaosLevel = Clamp(chaosLevel, 0.0f, 1.0f);

            memeSpawnTimer += GetFrameTime();
            
            if (!currentMeme.active && (memeSpawnTimer > GetRandomValue(45, 100))) {
                memeSpawnTimer = 0;
                int index;
                if (!memeTextures.empty()) {
                do {
                index = GetRandomValue(0, memeTextures.size() - 1);
                } while (index == currentMeme.lastIndex && memeTextures.size() > 1);

                    currentMeme.tex = memeTextures[index];
                    currentMeme.lastIndex = index;
                    currentMeme.active = true;
                    currentMeme.speed = (float)GetRandomValue(2200, 3200);

            // Play the matching sound
            if (index < memeSounds.size()) {
                 currentMeme.soundIndex = index;
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

       if (state == LOGO)
        {
            DrawGroupLogo();
            EndDrawing();
            continue;
        }

        if (state == TITLE_LOGO)
        {
            DrawTitleLogo();
            EndDrawing();
            continue;
        }

        if (state == STORYLINE)
        {
            DrawStoryline();
            EndDrawing();
            continue;
        }

       if (state == TROLL_VIDEO)
        {
            ClearBackground(BLACK);
            if (currentFrame < (int)videoFrames.size())
            {
                DrawTexturePro(
                    videoFrames[currentFrame],
                    { 0, 0, (float)videoFrames[currentFrame].width, (float)videoFrames[currentFrame].height },
                    { 0, 0, (float)screenWidth, (float)screenHeight },
                    { 0, 0 }, 0.0f, WHITE
                );
            }
            // "relapse ka muna boi" 
            const char* trollText = "relapse ka muna boi :((";
            Vector2 trollSize = MeasureTextEx(tinyFont, trollText, 48, 0);
            float trollX = screenWidth / 2.0f - trollSize.x / 2.0f;
            float trollY = screenHeight - 130.0f;
            DrawTextEx(tinyFont, trollText, { trollX + 3, trollY + 3 }, 48, 0, Fade(BLACK, 0.8f));
            DrawTextEx(tinyFont, trollText, { trollX, trollY }, 48, 0, RED);

            // "press enter to skip" text
            const char* skipText = "PRESS ENTER TO SKIP";
            Vector2 skipSize = MeasureTextEx(tinyFont, skipText, 36, 0);
            float skipX = screenWidth / 2.0f - skipSize.x / 2.0f;
            float skipY = screenHeight - 70.0f;
            float pulse = (sinf(GetTime() * 4.0f) + 1.0f) / 2.0f;
            DrawTextEx(tinyFont, skipText, { skipX + 2, skipY + 2 }, 36, 0, Fade(BLACK, pulse));
            DrawTextEx(tinyFont, skipText, { skipX, skipY }, 36, 0, Fade(WHITE, pulse));
        }
        
        // game
        if (state == PLAYING)
        {
            BeginMode2D(camera);
    
            // DRAW BACKGROUND (Frame-based transition)
            if (!bgFrames.empty())
            {
                Texture2D currentBg = bgFrames[currentBgFrame];
                DrawTexturePro(
                    currentBg,
                    {0, 0, (float)currentBg.width, (float)currentBg.height},
                    {0, 0, (float)screenWidth, (float)screenHeight},
                    {0, 0},
                    0.0f,
                    WHITE
                );
            }

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

            if (currentAnim == JUMP && playerFrame > 4) playerFrame = 4;
            if (playerFrame >= 6) playerFrame = 0;

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

                Rectangle drawRect = it.rect;

                float itemScale = 0.80f;

                if (it.type == HEART || it.type == BABY || it.type == SALT || it.type == BLOOD) itemScale = 0.95f;
                if (it.type == TROLLFACE || it.type == GARLIC) itemScale = 0.75f;
                if (it.type == CHILI) itemScale = 0.80f;

                float finalScale = itemScale * visualScale;  // Apply pulsing visual scale

                Rectangle smallRect = {
                    drawRect.x + (drawRect.width * (1.0f - finalScale) / 2),
                    drawRect.y + (drawRect.height * (1.0f - finalScale) / 2),
                    drawRect.width * finalScale,
                    drawRect.height * finalScale
                };

                if (it.type == BOMB) DrawTexturePro(bombTex, {0, 0, (float)bombTex.width, (float)bombTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == BABY) DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)babyTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == MEDKIT) DrawTexturePro(potionMedkitTex, {0, 0, (float)potionMedkitTex.width, (float)potionMedkitTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == BANDAGE) DrawTexturePro(potionBandageTex, {0, 0, (float)potionBandageTex.width, (float)potionBandageTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == GARLIC) DrawTexturePro(garlicTex, {0, 0, (float)garlicTex.width, (float)garlicTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == CHILI) DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)chiliTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == TROLLFACE) DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)trollFaceTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == HEART) DrawTexturePro(heartTex, {0, 0, (float)heartTex.width, (float)heartTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == BLOOD) DrawTexturePro(bloodTex, {0, 0, (float)bloodTex.width, (float)bloodTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == POO) DrawTexturePro(pooTex, {0, 0, (float)pooTex.width, (float)pooTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == DICE) DrawTexturePro(diceTex, {0, 0, (float)diceTex.width, (float)diceTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == MUSHROOM) DrawTexturePro(mushroomTex, {0, 0, (float)mushroomTex.width, (float)mushroomTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == POISON) DrawTexturePro(poisonTex, {0, 0, (float)poisonTex.width, (float)poisonTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == SALT) DrawTexturePro(saltTex, {0, 0, (float)saltTex.width, (float)saltTex.height}, smallRect, {0, 0}, 0.0f, col);
                if (it.type == ATAY) DrawTexturePro(atayTex, {0, 0, (float)atayTex.width, (float)atayTex.height}, smallRect, {0, 0}, 0.0f, col);                   
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

            // NOTIF DRAW
            {
                float startY = 100.0f;
                float fontSize = 48.0f;
                float fadeDur = 0.5f;

                for (int i = 0; i < (int)notifs.size(); i++) {
                    auto& n = notifs[i];
                    float alpha = 1.0f;
                    
                    if (n.timer > n.maxTime - fadeDur) alpha = (n.maxTime - n.timer) / fadeDur;
                    else if (n.timer < fadeDur) alpha = n.timer / fadeDur;

                    Vector2 tSize = MeasureTextEx(tinyFont, n.text.c_str(), fontSize, 0);
                    float tx = screenWidth / 2.0f - tSize.x / 2.0f;
                    float ty = startY + i * 55.0f;

                    // black pixel outline (8 directions)
                    Color outline = Fade(BLACK, alpha);
                    for (int ox = -2; ox <= 2; ox += 2) {
                        for (int oy = -2; oy <= 2; oy += 2) {
                            if (ox == 0 && oy == 0) continue;
                            DrawTextEx(tinyFont, n.text.c_str(), {tx + ox, ty + oy}, fontSize, 0, outline);
                        }
                    }
                    // drop shadow
                    DrawTextEx(tinyFont, n.text.c_str(), {tx + 2, ty + 3}, fontSize, 0,
                        Fade({40, 0, 0, 255}, alpha * 0.8f));
                    // main colored text
                    DrawTextEx(tinyFont, n.text.c_str(), {tx, ty}, fontSize, 0,
                        Fade(n.color, alpha));
                }
            }
            if (hitFlash > 0) DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, hitFlash));

            // UI   
            // Milestone Text
            if (milestoneCelebrationTimer > 0) {
                float alpha = milestoneCelebrationTimer > 0.5f ? 1.0f : milestoneCelebrationTimer / 0.5f;
                const char* mileText = "MILESTONE REACHED!";
                Vector2 mileSize = MeasureTextEx(tinyFont, mileText, 80, 2);
                DrawTextEx(tinyFont, mileText, {screenWidth / 2.0f - mileSize.x / 2.0f + 4, screenHeight / 2.0f - 100 + 4}, 80, 2, Fade(BLACK, alpha));
                DrawTextEx(tinyFont, mileText, {screenWidth / 2.0f - mileSize.x / 2.0f, screenHeight / 2.0f - 100}, 80, 2, Fade(GOLD, alpha));
            }

            // CHAOS LEVEL UI 
            DrawRectangle(screenWidth - 220, 20, 200, 25, Fade(BLACK, 0.4f));
            DrawRectangle(screenWidth - 215, 25, (int)(190 * chaosLevel), 15, ColorLerp(GREEN, RED, chaosLevel));
            DrawTextEx(tinyFont, "CHAOS LEVEL", {(float)(screenWidth - 215), 50.0f}, 20, 0, RED);

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
        }

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
            float panelW = 340, panelH = 340; // Increased height for 3 buttons
            float panelX = screenWidth / 2.0f - panelW / 2.0f;
            float panelY = screenHeight / 2.0f - panelH / 2.0f;
            float btnW = 220, btnH = 50;
            float btnX = panelX + panelW/2 - btnW/2;
            
            // Calculate vertical positions with consistent spacing
            float resumeY = panelY + 90;
            float muteY   = resumeY + btnH + 20;
            float exitY   = muteY + btnH + 20;

            Rectangle resumeRect = {btnX, resumeY, btnW, btnH};
            Rectangle muteRect   = {btnX, muteY,   btnW, btnH};
            Rectangle exitRect   = {btnX, exitY,   btnW, btnH};
            
            bool hoverResume = CheckCollisionPointRec(GetMousePosition(), resumeRect);
            bool hoverMute   = CheckCollisionPointRec(GetMousePosition(), muteRect);
            bool hoverExit   = CheckCollisionPointRec(GetMousePosition(), exitRect);

            static bool prevHoverResume = false;
            static bool prevHoverMute   = false;
            static bool prevHoverExit   = false;

            if (hoverResume && !prevHoverResume) PlaySound(hoverSound);
            if (hoverMute && !prevHoverMute) PlaySound(hoverSound);
            if (hoverExit && !prevHoverExit) PlaySound(hoverSound);

            prevHoverResume = hoverResume;
            prevHoverMute   = hoverMute;
            prevHoverExit   = hoverExit;

            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.75f));
            DrawRectangleRounded({panelX, panelY, panelW, panelH}, 0.15f, 8, {20, 20, 20, 230});
            DrawRectangleRoundedLines({panelX, panelY, panelW, panelH}, 0.15f, 8, Color{180, 180, 180, 200});

            Vector2 titleSize = MeasureTextEx(tinyFont, "PAUSED", 52, 0);
            DrawTextEx(tinyFont, "PAUSED",
                {panelX + panelW/2 - titleSize.x/2, panelY + 18}, 52, 0, WHITE);

            // RESUME BUTTON
            DrawRectangleRounded(resumeRect, 0.3f, 6, hoverResume ? Color{60, 180, 60, 255} : Color{40, 120, 40, 220});
            Vector2 resumeSize = MeasureTextEx(tinyFont, "RESUME", 30, 0);
            DrawTextEx(tinyFont, "RESUME", {btnX + btnW/2 - resumeSize.x/2, resumeY + btnH/2 - resumeSize.y/2}, 30, 0, WHITE);

            // MUTE BUTTON
            DrawRectangleRounded(muteRect, 0.3f, 6, hoverMute ? Color{80, 80, 200, 255} : Color{50, 50, 150, 220});
            const char* muteText = isMuted ? "UNMUTE" : "MUTE";
            Vector2 muteSize = MeasureTextEx(tinyFont, muteText, 30, 0);
            DrawTextEx(tinyFont, muteText, {btnX + btnW/2 - muteSize.x/2, muteY + btnH/2 - muteSize.y/2}, 30, 0, WHITE);

            // EXIT BUTTON
            DrawRectangleRounded(exitRect, 0.3f, 6, hoverExit ? Color{200, 40, 40, 255} : Color{130, 20, 20, 220});
            Vector2 exitSize = MeasureTextEx(tinyFont, "EXIT TO MENU", 30, 0);
            DrawTextEx(tinyFont, "EXIT TO MENU", {btnX + btnW/2 - exitSize.x/2, exitY + btnH/2 - exitSize.y/2}, 30, 0, WHITE);
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

        Vector2 gameSize = MeasureTextEx(tinyFont, "GAME", (float)fontSize, 4);
        Vector2 overSize = MeasureTextEx(tinyFont, "OVER", (float)fontSize, 4);

        float gameX = screenWidth / 2.0f - gameSize.x / 2.0f;
        float overX = screenWidth / 2.0f - overSize.x / 2.0f;
        float gameY = screenHeight / 2.0f - gameSize.y - 10;
        float overY = screenHeight / 2.0f;

        DrawTextEx(tinyFont, "GAME", {gameX + 5, gameY + 5}, (float)fontSize, 4, {40, 40, 40, 255});
        DrawTextEx(tinyFont, "GAME", {gameX, gameY}, (float)fontSize, 4, WHITE);
        DrawTextEx(tinyFont, "OVER", {overX + 5, overY + 5}, (float)fontSize, 4, {60, 0, 0, 255});
        DrawTextEx(tinyFont, "OVER", {overX, overY}, (float)fontSize, 4, {200, 20, 20, 255});
    }
       if (gameOverAnimTimer >= 2.5f)
{
    // ── Save high score ──
    if (score > highScore) {
        highScore = score;
        ofstream file("score.txt");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }

    float cx = screenWidth / 2.0f;
    float cy = screenHeight / 2.0f;

    // ── Calculate total block height first ──
    float titleH    = 260.0f;
    float divider1H = 20.0f;
    float scoresH   = 100.0f;
    float divider2H = 20.0f;
    float tryAgainH = 60.0f;
    float yesH      = 68.0f;
    float noH       = 68.0f;
    float gapH      = 20.0f;

    float totalH = titleH + divider1H + scoresH + divider2H + tryAgainH + yesH + noH + gapH;

    // ── Start Y so entire block is vertically centered ──
    float startY = cy - totalH / 2.0f;

    // ── GAME OVER TITLE ──
    Vector2 goSize = MeasureTextEx(tinyFont, "GAME OVER", 280, 4);
    float goX = cx - goSize.x / 2.0f;
    float goY = startY;

    DrawTextEx(tinyFont, "GAME OVER", {goX + 8, goY + 8}, 280, 4, {40, 40, 40, 255});
    Vector2 gameWordSize = MeasureTextEx(tinyFont, "GAME ", 280, 4);
    DrawTextEx(tinyFont, "GAME ", {goX, goY}, 280, 4, WHITE);
    DrawTextEx(tinyFont, "OVER", {goX + gameWordSize.x, goY}, 280, 4, {200, 20, 20, 255});

    float cursorY = startY + titleH;

    // ── DIVIDER 1 ──
    DrawRectangle(cx - 300, cursorY, 600, 2, {100, 100, 100, 180});
    cursorY += divider1H;

    // ── SCORE + HIGH SCORE ──
    float scoresY = cursorY;
    float col1X   = cx - 180.0f;
    float col2X   = cx + 180.0f;

    Vector2 sLabel = MeasureTextEx(tinyFont, "SCORE", 24, 1);
    DrawTextEx(tinyFont, "SCORE",
        {col1X - sLabel.x / 2.0f, scoresY}, 24, 1, {200, 200, 200, 255});
    Vector2 sVal = MeasureTextEx(tinyFont, TextFormat("%d", score), 52, 1);
    DrawTextEx(tinyFont, TextFormat("%d", score),
        {col1X - sVal.x / 2.0f, scoresY + 30}, 52, 1, {220, 40, 40, 255});

    Vector2 hsLabel = MeasureTextEx(tinyFont, "HIGH SCORE", 24, 1);
    DrawTextEx(tinyFont, "HIGH SCORE",
        {col2X - hsLabel.x / 2.0f, scoresY}, 24, 1, {170, 170, 204, 255});
    Vector2 hsVal = MeasureTextEx(tinyFont, TextFormat("%d", highScore), 52, 1);
    DrawTextEx(tinyFont, TextFormat("%d", highScore),
        {col2X - hsVal.x / 2.0f, scoresY + 30}, 52, 1, {100, 180, 255, 255});

    cursorY += scoresH;

    // ── DIVIDER 2 ──
    DrawRectangle(cx - 300, cursorY, 600, 2, {100, 100, 100, 180});
    cursorY += divider2H;

    // ── TRY AGAIN? ──
    float menuY = cursorY;
    Vector2 trySize = MeasureTextEx(tinyFont, "TRY AGAIN?", 55, 1);
    DrawTextEx(tinyFont, "TRY AGAIN?",
        {cx - trySize.x / 2.0f, menuY}, 55, 1, {220, 220, 220, 255});

    menuY += 65;

    // ── YES ──
    static bool prevHoverPlay = false;
    bool hoverPlay = CheckCollisionPointRec(GetMousePosition(), {cx - 90, menuY - 5, 180, 55});
    if (hoverPlay && !prevHoverPlay) PlaySound(hoverSound);
    prevHoverPlay = hoverPlay;
    if (hoverPlay) DrawRectangleRounded({cx - 90, menuY - 5, 180, 55}, 0.3f, 6, {0, 120, 0, 180});
    Vector2 yesSize = MeasureTextEx(tinyFont, hoverPlay ? "> YES <" : "YES", 55, 1);
    DrawTextEx(tinyFont, hoverPlay ? "> YES <" : "YES",
        {cx - yesSize.x / 2.0f, menuY}, 55, 1, hoverPlay ? GREEN : WHITE);

    menuY += 65;

    // ── NO ──
    static bool prevHoverMenu = false;
    bool hoverMenu = CheckCollisionPointRec(GetMousePosition(), {cx - 90, menuY - 5, 180, 55});
    if (hoverMenu && !prevHoverMenu) PlaySound(hoverSound);
    prevHoverMenu = hoverMenu;
    if (hoverMenu) DrawRectangleRounded({cx - 90, menuY - 5, 180, 55}, 0.3f, 6, {120, 0, 0, 180});
    Vector2 noSize = MeasureTextEx(tinyFont, hoverMenu ? "> NO <" : "NO", 55, 1);
    DrawTextEx(tinyFont, hoverMenu ? "> NO <" : "NO",
        {cx - noSize.x / 2.0f, menuY}, 55, 1, hoverMenu ? RED : WHITE);

    // ── INPUT ──
    if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverPlay))
    {
        StopMusicStream(gameOverMusic);
        currentBg = &bgEasy;
        SetMusicVolume(*currentBg, bgTargetVolume);
        SeekMusicStream(*currentBg, 0.0f);
        PlayMusicStream(*currentBg);

        state = PLAYING; diff = EASY; lastDiff = EASY;
        hp = 3; score = 0; combo = 0;
        comboBroken = false; comboBrokenTimer = 0; comboTime = 0;
        items.clear(); popEffects.clear(); notifs.clear();
        player.x = (screenWidth - player.width) / 2;
        player.y = screenHeight * 0.75f;
        move = 1.0f; chiliBoost = 1.0f; eventBoost = 1.0f;
        gravity = 1800.0f; velocityY = 0; velocityX = 0; isGrounded = true;
        spawnTimer = 0; eventCooldown = initialCooldown; eventTimer = 0;
        currentEvent = NONE; secondEvent = NONE;
        lastEvent = NONE; secondLastEvent = NONE; eventWarningTimer = 0;
        slowTimer = 0; speedBoostTimer = 0; medkitCooldown = 0;
        shakeTime = 0; shakePower = 0; hitFlash = 0;
        fogActive = false; fogAlpha = 0;
        fallingInPit = false; pitCreated = false; pitSoundPlayed = false;
        quakeActive = false; quakeTimer = 0;
        pits.clear(); pitWidths.clear(); pitCenters.clear(); pitOpens.clear();
        camera.rotation = 0; camera.zoom = 1.30f;
        camera.target = { player.x + player.width / 2, player.y + player.height / 2 };
        invertedScreen = false; gameOverAnimTimer = 0.0f;
        currentBgFrame = 0; bgFrameTimer = 0.0f;
        bg1Triggered = false; bg2Triggered = false;
        UnloadBg1TransitionVideo(); UnloadBg2TransitionVideo();
        InitBg1TransitionVideo();   InitBg2TransitionVideo();
    }

    if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverMenu))
    {
        StopMusicStream(gameOverMusic);
        state = MENU; diff = EASY; lastDiff = EASY;
        hp = 3; score = 0; combo = 0;
        comboBroken = false; comboBrokenTimer = 0; comboTime = 0;
        items.clear(); popEffects.clear(); notifs.clear();
        player.x = (screenWidth - player.width) / 2;
        player.y = screenHeight * 0.75f;
        move = 1.0f; chiliBoost = 1.0f; eventBoost = 1.0f;
        gravity = 1800.0f; velocityY = 0; velocityX = 0; isGrounded = true;
        spawnTimer = 0; eventCooldown = initialCooldown; eventTimer = 0;
        currentEvent = NONE; secondEvent = NONE;
        lastEvent = NONE; secondLastEvent = NONE; eventWarningTimer = 0;
        slowTimer = 0; speedBoostTimer = 0; medkitCooldown = 0;
        shakeTime = 0; shakePower = 0; hitFlash = 0;
        fogActive = false; fogAlpha = 0;
        fallingInPit = false; pitCreated = false; pitSoundPlayed = false;
        quakeActive = false; quakeTimer = 0;
        pits.clear(); pitWidths.clear(); pitCenters.clear(); pitOpens.clear();
        camera.rotation = 0; camera.zoom = 1.30f;
        camera.target = { player.x + player.width / 2, player.y + player.height / 2 };
        invertedScreen = false; gameOverAnimTimer = 0.0f;
        currentBgFrame = 0; bgFrameTimer = 0.0f;
        bg1Triggered = false; bg2Triggered = false;
        UnloadBg1TransitionVideo(); UnloadBg2TransitionVideo();
        InitBg1TransitionVideo();   InitBg2TransitionVideo();
        introMusic = LoadMusicStream("assets/sounds/intro.mp3");
        SetMusicVolume(introMusic, 0.5f);
        PlayMusicStream(introMusic);
    }
}
        }
        EndDrawing();
    }

    for (auto &t : videoFrames) UnloadTexture(t);

    for (auto &t : bgFrames) UnloadTexture(t);

    UnloadBg1TransitionVideo();
    UnloadBg2TransitionVideo();

    for (auto &t : memeTextures) UnloadTexture(t);

    UnloadTexture(introTex);
    UnloadTexture(gameOverBg);  
    UnloadTexture(hpTex);


    currentFrame = 0;
    frameTimer = 0;

    for (auto &s : memeSounds)  UnloadSound(s);

    // unload
    UnloadSound(trollSound);
    UnloadSound(goodItemSound);
    UnloadSound(badItemSound);
    UnloadSound(walkSound);
    UnloadSound(jumpSound);
    UnloadSound(gameOverSound);
    UnloadSound(pitSound);
    UnloadSound(uiClickSound);
    UnloadMusicStream(gameOverMusic);
    UnloadMusicStream(bgMusic);
    UnloadFont(nosifer);
    UnloadFont(gamefont);
    UnloadFont(tinyFont);

    if (currentBg) StopMusicStream(*currentBg);
    UnloadMusicStream(bgEasy);
    UnloadMusicStream(bgMedium);
    UnloadMusicStream(bgHard);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
