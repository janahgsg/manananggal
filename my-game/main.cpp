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
    ATAY,
    KRUS,
    HOLYWATER,
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
    Texture2D pauseBg = LoadTexture("assets/images/PAUSED UI (3).png");
    InitInfoTexture();

    // MEME POP-UP TEXTURES
    // ADD MORE MEMES HERE: Just add more textures to the memeTextures vector
    memeTextures.push_back(LoadTexture("assets/images/nyek.png"));
    memeTextures.push_back(LoadTexture("assets/images/nani.png"));
    memeTextures.push_back(LoadTexture("assets/images/sigma.png"));
    memeTextures.push_back(LoadTexture("assets/images/blush.png"));
    memeTextures.push_back(LoadTexture("assets/images/trollFace.png"));

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
    Texture2D fetusTex = LoadTexture("assets/images/fetus1.png");
    Texture2D poisonTex = LoadTexture("assets/images/poison.png");
    Texture2D saltTex = LoadTexture("assets/images/salt.png");
    Texture2D potionBandageTex = LoadTexture("assets/images/potion-bandage (2).png");
    Texture2D potionMedkitTex = LoadTexture("assets/images/potion-medkit (2).png");
    Texture2D garlic1Tex = LoadTexture("assets/images/garlic-1.png");
    Texture2D bloodTex = LoadTexture("assets/images/blood.png");
    Texture2D pooTex = LoadTexture("assets/images/poo.png");
    Texture2D heartTex = LoadTexture("assets/images/heart.png");
    Texture2D atayTex = LoadTexture("assets/images/atay.png");
    Texture2D crossTex = LoadTexture("assets/images/cross.png");
    Texture2D diceTex = LoadTexture("assets/images/dice.png");
    Texture2D holyTex = LoadTexture("assets/images/holywater.png");
    Texture2D mushroomTex = LoadTexture("assets/images/mushroom.png");
    Texture2D hpTex = LoadTexture("assets/images/hearty.png");

    // font
    Font nosifer = LoadFontEx("assets/font/Nosifer-Regular.ttf", 64, 0, 0);
    Font gamefont = LoadFontEx("assets/font/Chewy-Regular.ttf", 64, 0, 0);

    //character
    Texture2D playerTex = LoadTexture("assets/character/human/player.png");

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

    // SCORE & HEALTH
    int score = 0;
    int highScore = 0;
    int hp = 3;

    // ITEMS
    vector<Item> items;
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
        BeginDrawing();
        ClearBackground(BLACK);
        DrawBg1TransitionVideo();

        EndDrawing();

        if (IsBg1TransitionFinished()) {
            float fadeAlpha = GetBg1TransitionAlpha(); 
           Color fadeColor = Fade(BLACK, fadeAlpha);
           DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), fadeColor);
            UnloadBg1TransitionVideo();
            state = PLAYING;
        }
        continue; 
        }


        else if (state == BG2_TRANSITION) {
             UpdateBg2TransitionVideo();

             BeginDrawing();
             ClearBackground(BLACK);
             DrawBg2TransitionVideo();
             

         EndDrawing();

        if (IsBg2TransitionFinished()) {
            float fadeAlpha = GetBg2TransitionAlpha(); 
             Color fadeColor = Fade(BLACK, fadeAlpha);
             DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), fadeColor);
             UnloadBg2TransitionVideo();
             state = PLAYING;   // resume gameplay with HARD background
             }
         continue;
            }


else if (state == PAUSED)
{
    BeginDrawing();

    ClearBackground(BLACK);

    // Centered "PAUSED" text
    const char* pausedText = "PAUSED";
    int fontSize = 60;
    int textWidth = MeasureText(pausedText, fontSize);
    DrawText(pausedText,
             screenWidth/2 - textWidth/2,
             screenHeight/2 - fontSize,
             fontSize,
             WHITE);

    // Witty quotation
    const char* quote = "\"Press SPACE to breathe life again.\"";
    int quoteSize = 30;
    int quoteWidth = MeasureText(quote, quoteSize);
    DrawText(quote,
             screenWidth/2 - quoteWidth/2,
             screenHeight/2 + 40,
             quoteSize,
             GRAY);

    EndDrawing();

    // Resume check
    if (IsKeyPressed(KEY_SPACE)) {
            if (state == PLAYING) state = PAUSED;
            else if (state == PAUSED) state = PLAYING;
           }

    }



        // GAMEPLAY-----------------------------------------
        else if (state == PLAYING)
        {
            UpdateMusicStream(bgMusic);

            if (IsKeyPressed(KEY_SPACE)) {
            if (state == PLAYING) state = PAUSED;
            else if (state == PAUSED) state = PLAYING;
           }

            // Update difficulty and handle grace periods
            lastDiff = diff;
            if (score >= 400) 
                diff = HARD;
            else if (score >= 150)
                diff = MEDIUM;
            else
                diff = EASY;


            // If difficulty just increased, give the player a "Grace Period"
            if (diff > lastDiff) 
            {
                eventCooldown = 15.0f; 
                eventWarningTimer = 0;
            }

             // >>> Trigger transition video when EASY → MEDIUM
            if (diff == MEDIUM && lastDiff == EASY) {
                 InitBg1TransitionVideo();
                 state = BG1_TRANSITION;
                 continue;
                   
                 }

             else if (diff == HARD && lastDiff == MEDIUM) {
                 InitBg2TransitionVideo();
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
                spawnDelay = 1.1f; 
                spawnAmount = 1;
                baseMove = 1.0f;
            }
            else if (diff == MEDIUM)
            {
                spawnDelay = 0.75f; 
                spawnAmount = 2;
                baseMove = 1.15f;
            }
            else if (diff == HARD)
            {
                spawnDelay = 0.55f; 
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
                    it.rect.width = 65;
                    it.rect.height = 65;

                    // Pick a segment from the shuffled list to guarantee horizontal separation
                    int segIdx = currentIndices[i % segments];
                    float margin = 60.0f; // More margin to prevent clustering
                    float minX = (segIdx * segWidth) + margin;
                    float maxX = ((segIdx + 1) * segWidth) - it.rect.width - margin;
                    
                    it.rect.x = minX + (rand() % (int)(maxX - minX + 1));
                    it.rect.y = -100 - rand() % 250;

                    // Physics based on difficulty 
                    if (diff == EASY) {
                        it.speed = 180 + rand() % 40;
                        it.acceleration = 25;
                        it.lateralSpeed = 0;
                    } else if (diff == MEDIUM) {
                        it.speed = 250 + rand() % 50;
                        it.acceleration = 35;
                        it.lateralSpeed = (rand() % 100 < 30) ? (rand() % 81 - 40) : 0; 
                    } else {
                        it.speed = 320 + rand() % 80;
                        it.acceleration = 50;
                        it.lateralSpeed = (rand() % 91 - 45) * 1.2f; 
                    }

                    it.sinTime = (float)(rand() % 1000) / 100.0f;
                    it.active = true;
                    it.isIllusion = false;

                    // ITEM SELECTION LOGIC
                    if (currentEvent == LUCKY_PARTY) {
                        int goodPool[] = {BABY, HEART, BLOOD, MEAT, ATAY, STAR, DICE, MEDKIT, CHILI};
                        it.type = goodPool[rand() % 9];
                    }
                    else if (currentEvent == MISFORTUNE) {
                        int badPool[] = {BOMB, POISON, POO, GARLIC, SALT, MUSHROOM, TROLLFACE, MEDKIT, BANDAGE};
                        it.type = badPool[rand() % 9];
                    }
                    else {
                        if (hp == 1 && medkitCooldown <= 0 && rand() % 100 < 5) {
                            it.type = MEDKIT;
                            medkitCooldown = 15.0f;
                        }
                        else if (hp < 3 && rand() % 100 < 8) it.type = BANDAGE;
                        else if (rand() % 100 < 3) it.type = TROLLFACE;
                        else if (rand() % 100 < 2) it.type = DICE; 
                        else if (diff == EASY) {
                            int pool[] = {POO, GARLIC, BABY, BLOOD, BABY, ATAY};
                            it.type = pool[rand() % 6];
                        }
                        else if (diff == MEDIUM) {
                            int pool[] = {POO, GARLIC, BABY, BLOOD, BOMB, POISON, HEART, ATAY, BABY};
                            it.type = pool[rand() % 9];
                            if (rand() % 100 < 12) it.isIllusion = true; // Slightly higher chance
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

                    eventTimer = 18.0f;
                    eventCooldown = 20.0f;
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

                    eventTimer = 25.0f;
                    eventCooldown = 12.0f;
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
                       eventCooldown = 20.0f;
                    else if (diff == HARD)
                       eventCooldown = 12.0f;
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

                // TIGHT COLLISION LOGIC (Padding)
                // We shrink the effective hitbox to match the character's visual core
                Rectangle playerHitbox = {
                    player.x + player.width * 0.25f, // 25% padding on sides
                    player.y + player.height * 0.15f, // 15% padding on top
                    player.width * 0.5f,             // 50% width
                    player.height * 0.75f            // 75% height
                };

                Rectangle itemHitbox = {
                    it.rect.x + it.rect.width * 0.15f,
                    it.rect.y + it.rect.height * 0.15f,
                    it.rect.width * 0.7f,
                    it.rect.height * 0.7f
                };

                if (CheckCollisionRecs(playerHitbox, itemHitbox))
                {
                    // BAD ITEMS
                    if (it.type == POO || it.type == BOMB || it.type == SALT || it.type == GARLIC)
                    {
                        hp--;
                        shakeTime = 0.22f;
                        shakePower = 14.0f;
                        combo = 0;
                        comboTime = 0;
                        comboBroken = true;
                        comboBrokenTimer = 1.5f;
                    }
                    else if (it.type == CHILI)
                    {
                        chiliBoost = 1.8f;      
                        speedBoostTimer = 5.0f; 
                    }
                    
                    // SCORE++
                    else if (it.type == BABY || it.type == HEART)
                    {
                        score += 5;

                        combo++;
                        comboTime = 2.5f;
                        if (combo > 1)
                            score += combo;
                    }
                    else if (it.type == ATAY)
                        score += 8;

                    else if (it.type == BLOOD || it.type == MEAT)
                    { // blood
                        score += 3;

                        combo++;
                        comboTime = 2.5f;
                        if (combo > 1)
                            score += combo;
                    }
                    // HEAL
                    else if (it.type == BANDAGE)
                    { // bandage
                        hp += 1;
                        if (hp > 3)
                            hp = 3;
                    }
                    else if (it.type == MEDKIT)
                        hp = 3;
                    // RANDOMNESS
                    else if (it.type == TROLLFACE)
                    {
                        state = TROLL_VIDEO;
                        currentFrame = 0;
                        frameTimer = 0;
                        PlaySound(trollSound);
                    }
                    else if (it.type == POISON)
                    {
                        move = 0.45f;
                        slowTimer = 4.0f;
                    }

                    // SPECIAL ITEMS
                    else if (it.type == DICE)
                    { // dice(good effects)
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
                        int randomIndex = rand() % 4;
                        if (randomIndex == 1)
                        { // -10
                            score -= 10;
                            showMinusText = true;
                            minusTextTimer = 2.0f;
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
                        score += 10;

                    // special prize(super rare)
                    else if (it.type == PRIZE)
                    { // gift?
                    }

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
        }

             // If airborne, force jump animation
                if (!isGrounded) {
                currentAnim = JUMP;
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
    
            Texture2D currentBg;

            if (diff == EASY)      currentBg = bgEasy;
            else if (diff == MEDIUM) currentBg = bgMedium;
            else if (diff == HARD)   currentBg = bgHard;
          
            DrawTexturePro(
                 currentBg,
                 {0, 0, (float)currentBg.width, (float)currentBg.height},
                 {0, 0, (float)screenWidth, (float)screenHeight},
                 {0, 0},
                 0,
                 WHITE
                 );

            
            DrawTexturePro(
                currentBg,
                {0, 0, (float)currentBg.width, (float)currentBg.height},
                {0, 0, (float)screenWidth, (float)screenHeight},
                {0, 0},
                0,
                ColorAlpha(WHITE, 0.7f) 
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
                dest,
                {0, 0},
                0.0f,
                WHITE
                );
            

            for (auto &it : items)
            {
                if (!it.active)
                    continue;

                Color col = WHITE;
                
                // Intensified ILLUSION HINT: Pulsing purple tint
                if (it.isIllusion) {
                    float pulse = (sin(GetTime() * 12.0f) + 1.0f) / 2.0f; // More aggressive pulse
                    col = ColorLerp(WHITE, PURPLE, 0.45f * pulse); // Stronger tint
                }

                if (it.type == BOMB)
                    DrawTexturePro(bombTex, {0, 0, (float)bombTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == BABY)
                    DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)babyTex.height}, {it.rect}, {0, 0}, 0.0f, col);
                if (it.type == MEDKIT)
                    DrawTexturePro(potionMedkitTex, {0, 0, (float)potionMedkitTex.width, (float)potionMedkitTex.height}, {it.rect}, {0, 0}, 0.0f, col);
                if (it.type == BANDAGE)
                    DrawTexturePro(potionBandageTex, {0, 0, (float)potionBandageTex.width, (float)potionBandageTex.height}, {it.rect.x, it.rect.y, potionBandageTex.width * 0.15f, potionBandageTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == GARLIC)
                    DrawTexturePro(garlic1Tex, {0, 0, (float)garlic1Tex.width, (float)garlic1Tex.height}, {it.rect.x, it.rect.y, garlic1Tex.width * 0.22f, garlic1Tex.height * 0.22f}, {0, 0}, 0.0f, col);
                if (it.type == CHILI)
                    DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)chiliTex.height}, {it.rect.x, it.rect.y, chiliTex.width * 0.22f, chiliTex.height * 0.22f}, {0, 0}, 0.0f, col);
                if (it.type == TROLLFACE)
                    DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)trollFaceTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == HEART)
                    DrawTexturePro(heartTex, {0, 0, (float)heartTex.width, (float)heartTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == BLOOD)
                    DrawTexturePro(bloodTex, {0, 0, (float)bloodTex.width, (float)bloodTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == POO)
                    DrawTexturePro(pooTex, {0, 0, (float)pooTex.width, (float)pooTex.height}, {it.rect.x, it.rect.y, pooTex.width * 0.22f, pooTex.height * 0.22f}, {0, 0}, 0.0f, col);
                if (it.type == DICE)
                    DrawTexturePro(diceTex, {0, 0, (float)diceTex.width, (float)diceTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == MUSHROOM)
                    DrawTexturePro(mushroomTex, {0, 0, (float)mushroomTex.width, (float)mushroomTex.height}, {it.rect.x, it.rect.y, mushroomTex.width * 0.22f, mushroomTex.height * 0.22f}, {0, 0}, 0.0f, col);
                if (it.type == POISON)
                    DrawTexturePro(poisonTex, {0, 0, (float)poisonTex.width, (float)poisonTex.height}, {it.rect.x, it.rect.y, poisonTex.width * 0.22f, poisonTex.height * 0.22f}, {0, 0}, 0.0f, col);
                if (it.type == SALT)
                    DrawTexturePro(saltTex, {0, 0, (float)saltTex.width, (float)saltTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == HOLYWATER)
                    DrawTexturePro(holyTex, {0, 0, (float)holyTex.width, (float)holyTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == KRUS)
                    DrawTexturePro(crossTex, {0, 0, (float)crossTex.width, (float)crossTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == ATAY)
                    DrawTexturePro(atayTex, {0, 0, (float)atayTex.width, (float)atayTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == PRIZE) //INSERT PRIZE IMAGE
                    DrawTexturePro(potionMedkitTex, {0, 0, (float)potionMedkitTex.width, (float)potionMedkitTex.height}, {it.rect.x, it.rect.y, potionMedkitTex.width * 0.22f, potionMedkitTex.height * 0.22f}, {0, 0}, 0.0f, col);
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
                            {x, y},
                            220,
                            Fade(LIGHTGRAY, fogAlpha),
                            Fade(WHITE, 0.0f)
                        );
            
                        DrawCircleGradient(
                            {x + 100, y + 50},
                            260,
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
            for (int i = 0; i < hp; i++) DrawTextureEx(hpTex, (Vector2){10 + i * (hpTex.width * hpScale + 5), 10}, 0.0f, hpScale, WHITE);
            
            //score 
            DrawText(TextFormat("score: %d", score), 20, 50, 40, WHITE);

            // EVENT WARNING UI
            if (eventWarningTimer > 0)
            {
                // Pulsing effect for the warning
                float pulse = abs(sin(GetTime() * 10.0f));
                DrawText("!!! PREPARE FOR CHAOS !!!", screenWidth / 2 - 240, 150, 40, Fade(RED, 0.5f + pulse * 0.5f));
            }

            // POP UP TEXTS--------------------------------------
            
            if (showStarText)
                DrawText("STAR!", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
            if (showMinusText)
                DrawText("MINUS 10 HUHU", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
            if (showSlowText)
                DrawText("SLOW MO", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);

            if (quakeTimer > 1.0f && quakeTimer < 3.0f)
                DrawText("THE GROUND IS SHAKING", screenWidth / 2 - 250, screenHeight - 100, 40, RED);
            
            if (quakeTimer > 3.0f && quakeTimer < 5.5f)
                DrawText("RUN AWAY!!!", screenWidth / 2 - 180, screenHeight - 100, 50, WHITE);

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
                DrawText(eventName.c_str(), 25, 100, 28, RED);
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
                DrawText("UNSTOPPABLE", screenWidth / 2 - 140, 20, 45, RED);
            else if (combo >= 5)
                DrawText("HOTSTREAK!!", screenWidth / 2 - 120, 20, 40, ORANGE);
            else if (combo > 1)
                DrawText(TextFormat("COMBO x%d", combo), screenWidth / 2 - 100, 20, 35, YELLOW);

            if (comboBroken)
                DrawText("COMBO LOST!", screenWidth / 2 - 120, 70, 35, RED);
            if (quakeTimer > 1.2f && quakeTimer < 2.0f)
            {
                DrawText(
                    "THE GROUND IS CRACKING!",
                    screenWidth / 2 - 220,
                    120,
                    35,
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

                DrawText("RELAPSE ", screenWidth / 2 - 350, screenHeight - 100, 40, BLUE);
                DrawText("KA ", screenWidth / 2 - 350 + MeasureText("RELAPSE ", 40), screenHeight - 100, 40, YELLOW);
                DrawText("MUNA ", screenWidth / 2 - 350 + MeasureText("RELAPSE KA ", 40), screenHeight - 100, 40, BLUE);
                DrawText("BOI HAHA :((", screenWidth / 2 - 350 + MeasureText("RELAPSE KA MUNA ", 40), screenHeight - 100, 40, YELLOW);

                DrawText(
                    "Press ENTER to skip",
                    screenWidth / 2 - 130,
                    screenHeight - 50,
                    25,
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
            int pulse = 20 + sin(GetTime() * 8) * 10;
            DrawText("WARNING!", screenWidth / 2 - 100, 50, pulse, RED);
        }

        // GAME OVER----------------------------------------------------
        if (state == GAMEOVER_ANIM) {

             UpdateMusicStream(gameOverMusic);
             ClearBackground(BLACK);
            

    // BACKGROUND
    DrawTexturePro(
        gameOverBg,
        {0, 0, (float)gameOverBg.width, (float)gameOverBg.height},
        {0, 0, (float)screenWidth, (float)screenHeight},
        {0, 0},
        0.0f,
        WHITE
    );

    // dark overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.55f));

    if (gameOverAnimTimer < 2.5f)
    {
        if (gameOverFlash > 0)
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, gameOverFlash));

        // GAME (white) stacked on OVER (red)
        float scale = Clamp(gameOverAnimTimer / 0.6f, 0.0f, 1.0f);
        int fontSize = (int)(160 * scale);
        
        Vector2 gameSize = MeasureTextEx(nosifer, "GAME", (float)fontSize, 4);
        Vector2 overSize = MeasureTextEx(nosifer, "OVER", (float)fontSize, 4);

        float gameX = screenWidth / 2.0f - gameSize.x / 2.0f;
        float overX = screenWidth / 2.0f - overSize.x / 2.0f;
        float gameY = screenHeight / 2.0f - gameSize.y - 10;
        float overY = screenHeight / 2.0f;

        // GAME - white with shadow
        DrawTextEx(nosifer, "GAME", {gameX + 5, gameY + 5}, (float)fontSize, 4, {40, 40, 40, 255});
        DrawTextEx(nosifer, "GAME", {gameX, gameY}, (float)fontSize, 4, WHITE);

        // OVER - red with shadow
        DrawTextEx(nosifer, "OVER", {overX + 5, overY + 5}, (float)fontSize, 4, {60, 0, 0, 255});
        DrawTextEx(nosifer, "OVER", {overX, overY}, (float)fontSize, 4, {200, 20, 20, 255});
    }

    if (gameOverAnimTimer >= 2.5f)
{
    if (score > highScore) highScore = score;

        // ===== UPPER LEFT INFO =====
        float infoX = 60;
        float infoY = 60;

        // YOUR SCORE
        DrawTextEx(nosifer, "YOUR SCORE", {infoX, infoY}, 18, 1, {160, 160, 160, 255});
        DrawTextEx(nosifer, TextFormat("%d", score), {infoX, infoY + 24}, 52, 1, {220, 150, 30, 255});

        // HIGHEST SCORE
        DrawTextEx(nosifer, "HIGHEST SCORE", {infoX, infoY + 90}, 18, 1, {160, 160, 160, 255});
        DrawTextEx(nosifer, TextFormat("%d", highScore), {infoX, infoY + 114}, 52, 1, {60, 140, 220, 255});

        // RESEARCH METRICS
        DrawTextEx(nosifer, "RESEARCH DATA", {infoX, infoY + 180}, 18, 1, {160, 160, 160, 255});
        DrawTextEx(gamefont, TextFormat("Time Played: %.1fs", totalTimePlayed), {infoX, infoY + 204}, 22, 1, LIGHTGRAY);
        DrawTextEx(gamefont, TextFormat("Max Chaos: %.0f%%", chaosLevel * 100), {infoX, infoY + 230}, 22, 1, LIGHTGRAY);

        // DIFFICULTY
        const char* diffLabel = (diff == HARD) ? "HARD MODE" : (diff == MEDIUM) ? "MEDIUM MODE" : "EASY MODE";
        Color diffCol = (diff == HARD) ? RED : (diff == MEDIUM) ? ORANGE : GREEN;
        DrawTextEx(nosifer, "DIFFICULTY", {infoX, infoY + 280}, 18, 1, {160, 160, 160, 255});
        DrawTextEx(nosifer, diffLabel, {infoX, infoY + 304}, 28, 1, diffCol);

    // NEW HIGH SCORE badge
    if (score > 0 && score >= highScore)
    {
            DrawTextEx(nosifer, "* NEW HIGH SCORE!", {infoX, infoY + 348}, 18, 1, {80, 220, 120, 255});
        }

        // ===== STACKED TITLE =====
        Vector2 gameSize = MeasureTextEx(nosifer, "GAME", 160, 4);
        Vector2 overSize = MeasureTextEx(nosifer, "OVER", 160, 4);

        float gameX = screenWidth / 2.0f - gameSize.x / 2.0f;
        float overX = screenWidth / 2.0f - overSize.x / 2.0f;
        float gameY = screenHeight / 2.0f - gameSize.y - 80;
        float overY = screenHeight / 2.0f - 70;
        
        DrawTextEx(nosifer, "GAME", {gameX + 5, gameY + 5}, 160, 4, {40, 40, 40, 255});
        DrawTextEx(nosifer, "GAME", {gameX, gameY}, 160, 4, WHITE);
        DrawTextEx(nosifer, "OVER", {overX + 5, overY + 5}, 160, 4, {60, 0, 0, 255});
        DrawTextEx(nosifer, "OVER", {overX, overY}, 160, 4, {200, 20, 20, 255});

        // ===== BUTTONS =====
        float btnW = 340, btnH = 52;
    float btnX = screenWidth / 2.0f - btnW / 2.0f;
        float btnStartY = overY + overSize.y + 40;

        // PLAY AGAIN button — thin red border style
        Rectangle btnPlay = {btnX, btnStartY, btnW, btnH};
    bool hoverPlay = CheckCollisionPointRec(GetMousePosition(), btnPlay);
        Color btnPlayBg = hoverPlay ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
        Color btnPlayBorder = {139, 0, 0, 255};
        DrawRectangleRounded(btnPlay, 0.3f, 10, btnPlayBg);

        Vector2 playSize = MeasureTextEx(gamefont, "PLAY AGAIN ", 26, 1);
        DrawTextEx(gamefont, "PLAY AGAIN",
            {btnX + btnW / 2 - playSize.x / 2, btnStartY + btnH / 2 - playSize.y / 2},
            26, 1, hoverPlay ? WHITE : Color{200, 200, 200, 255});

        // MAIN MENU button 
        float btn2Y = btnStartY + btnH + 10;
    Rectangle btnMenu = {btnX, btn2Y, btnW, btnH};
    bool hoverMenu = CheckCollisionPointRec(GetMousePosition(), btnMenu);
        Color btnMenuBg = hoverMenu ? Color{139, 0, 0, 255} : Color{90, 0, 0, 255};
        DrawRectangleRounded(btnMenu, 0.3f, 10, btnMenuBg);

        Vector2 menuSize = MeasureTextEx(gamefont, "MAIN MENU", 26, 1);
        DrawTextEx(gamefont, "MAIN MENU",
        {btnX + btnW / 2 - menuSize.x / 2, btn2Y + btnH / 2 - menuSize.y / 2},
            26, 1, hoverMenu ? WHITE : Color{180, 180, 180, 255});

    // INPUT
    if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverPlay))
    {
        StopMusicStream(gameOverMusic);  
        PlayMusicStream(bgMusic);     
        //RESET EVERYTHING   
        state = PLAYING;
        hp = 3;
        score = 0;
        combo = 0;
        comboBroken = false;
        comboBrokenTimer = 0;
        comboTime = 0;
        items.clear();
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
    }
    if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverMenu))
    {
        StopMusicStream(gameOverMusic);
        //RESET EVERYTHING
        state = MENU;
        hp = 3;
        score = 0;
        combo = 0;
        comboBroken = false;
        comboBrokenTimer = 0;
        comboTime = 0;
        items.clear();
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

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
