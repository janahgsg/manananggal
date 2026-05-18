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
    TROLL_VIDEO,
    GAMEOVER_ANIM,
    GAMEOVER
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
    HOLYWATER
};

struct Item
{
    Rectangle rect; // items
    int type;
    float speed;
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
    SPIKES,
    FOG_BLIND
};

// stores every frame (Texture2D frame1 and so on)
vector<Texture2D> videoFrames;
int currentFrame = 0;
float frameTimer = 0;
Sound trollSound;
Music bgMusic;

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


    // load intro music
    Music introMusic = LoadMusicStream("assets/sounds/intro.mp3");
    SetMusicVolume(introMusic, 0.5f);
    PlayMusicStream(introMusic);

    // load frames
    for (int i = 1; i <= 110; i++)
        videoFrames.push_back(LoadTexture(TextFormat("assets/videos/trollFace/ezgif-frame-%03d.png", i)));

    // load images
    // bg
    Texture2D bgTex = LoadTexture("assets/images/bg.png");
    Texture2D groundTex = LoadTexture("assets/images/ground.png");
    Texture2D wallTex = LoadTexture("assets/images/wall.png");

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
    Texture2D introTex = LoadTexture("assets/images/intro1.png");
    Texture2D atayTex = LoadTexture("assets/images/atay.png");
    Texture2D crossTex = LoadTexture("assets/images/cross.png");
    Texture2D diceTex = LoadTexture("assets/images/dice.png");
    Texture2D holyTex = LoadTexture("assets/images/holywater.png");
    Texture2D mushroomTex = LoadTexture("assets/images/mushroom.png");

    // font
    Font nosifer = LoadFontEx("assets/font/Nosifer-Regular.ttf", 64, 0, 0);

    srand(time(NULL));
    SetTargetFPS(60); // 60fps 1sec/60frame

    // VARIABLES-----------------------------------------

    GameState state = MENU;
    // player
    Rectangle player;
    player.width = 50;
    player.height = 50;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // CAMERA
    player.x = (screenWidth - player.width) / 2;
    player.y = screenHeight * 0.85f;

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
    Sound gameOverSound = LoadSound("assets/sounds/game_over.mp3"); // add sfx

    float medkitCooldown = 0;

    // EVENTS----------------------------------------------
    EventType currentEvent = NONE;
    EventType secondEvent = NONE; // for HARD MODE extra event

    float eventTimer = 0.0f;
    float eventCooldown = 10.0f;
    // FOG EFFECT
    float fogAlpha = 0.0f;
    float fogTimer = 0.0f;
    float fogMoveX = -900.0f;
    bool fogActive = false;
    float fogCooldown = 0.0f;
    float nextFogTime = 3.0;

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
    // camera
    Camera2D camera = {0};
    camera.offset = {
        screenWidth / 2.0f,
        screenHeight * 0.85f};
    camera.target = {
        player.x + player.width / 2,
        player.y + player.height / 2};
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

            DrawIntro(highScore, introTex);
        }

        if (state == TRANSITION) {
            UpdateMusicStream(batmusic);
             UpdateIntroVideo();
             BeginDrawing();
             ClearBackground(BLACK);
             DrawIntroVideo();
             EndDrawing();

             if (IsVideoFinished()) {
             UnloadIntroVideo();
             state = PLAYING;
             }
         }

        // GAMEPLAY-----------------------------------------
        if (state == PLAYING)
        {
            UpdateMusicStream(bgMusic);

            // movements
            float accel = 2200.0f;  // how fast player gains speed
            float friction = 0.92f; // slows naturally
            float maxSpeed = 520.0f * move * chiliBoost * eventBoost;
            // Gravity
            velocityY += gravity * GetFrameTime();

            // Update player position
            player.y += velocityY * GetFrameTime();

            // Ground collision
            if (player.y >= screenHeight * 0.85f)
            {
                player.y = screenHeight * 0.85f;
                velocityY = 0;
                isGrounded = true;
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

            // keep Y fixed so player remains bottom
            camera.target.y = player.y + player.height / 2;

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

            camera.offset = {
                screenWidth / 2.0f + shakeOffset.x,
                screenHeight * 0.85f + shakeOffset.y};

            // difficulty
            if (score >= 50)
                diff = HARD;
            else if (score >= 20)
                diff = MEDIUM;
            else
                diff = EASY;

            // SPAWNING------------

            // spawn items every 1 second
            spawnTimer += GetFrameTime(); // Add time per frame
            float spawnDelay;
            int spawnAmount;
            if (diff == EASY)
            {
                spawnDelay = 1.0f;
                spawnAmount = 1;
                baseMove = 1.0f;
            }
            else if (diff == MEDIUM)
            {
                spawnDelay = 0.65f;
                spawnAmount = 2;
                baseMove = 1.2f;
            }
            else if (diff == HARD)
            {
                spawnDelay = 0.40f;
                spawnAmount = 3;
                baseMove = 1.4f;
            }

            if (spawnTimer > spawnDelay)
            {
                spawnTimer = 0;
                // spawnn items
                for (int i = 0; i < spawnAmount; i++)
                {
                    Item it;
                    it.rect.width = 40;
                    it.rect.height = 40;
                    it.rect.x = rand() % (screenWidth - (int)it.rect.width); // random x position
                    it.rect.y = -(rand() % 400);

                    if (diff == EASY)
                        it.speed = 120 + rand() % 80;
                    else if (diff == MEDIUM)
                        it.speed = 180 + rand() % 120;
                    else if (diff == HARD)
                        it.speed = 260 + rand() % 180;

                    it.active = true;

                    if (hp == 1 && medkitCooldown <= 0 && rand() % 100 < 12)
                    {
                        it.type = MEDKIT;
                        medkitCooldown = 8.0f;
                    }

                    static bool prizeSpawn = false;
                    if (score >= 500 && rand() % 100 < 15)
                    {
                        prizeSpawn = true;
                        it.type = PRIZE;
                    }
                    else if (hp == 1 && rand() % 100 < 10)
                        it.type = MEDKIT;
                    else if (diff == EASY)
                    {
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, TROLLFACE};
                        int randomIndex = rand() % 6;
                        it.type = pool[randomIndex];
                    }
                    else if (diff == MEDIUM)
                    {
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, BOMB,
                                      POISON, MEAT, HEART, TROLLFACE, ATAY};
                        int randomIndex = rand() % 11;
                        it.type = pool[randomIndex];
                    }
                    else if (diff == HARD)
                    {
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, BOMB, POISON, MEAT, HEART, TROLLFACE, ATAY};

                        int chance = rand() % 100;
                        if (chance < 70)
                        {
                            int randomIndex = rand() % 11;
                            it.type = pool[randomIndex];
                        }
                        else if (chance < 85)
                            it.type = MUSHROOM;
                        else if (chance < 95)
                            it.type = DICE;
                        else
                            it.type = STAR;
                    }

                    items.push_back(it); // add item to vector
                }
            }

            // CHALLENGES--------------------------------------------

            // countdown before next event
            if (currentEvent == NONE)
                eventCooldown -= GetFrameTime();

            // start new event
            if (currentEvent == NONE && eventCooldown <= 0)
            {

                // reset previous
                secondEvent = NONE;

                if (diff == MEDIUM)
                {

                    int mediumEvents[] = {SWAP_CONTROLS, SPEED_BOOST, SLOW_BOOST};
                    currentEvent = (EventType)mediumEvents[rand() % 3];
                    eventTimer = 18.0f;
                    // next event
                    eventCooldown = 20.0f;
                }

                else if (diff == HARD)
                {

                    int hardEvents[] = {SWAP_CONTROLS, SPEED_BOOST, SLOW_BOOST, LOW_GRAVITY, FOG_BLIND};

                    currentEvent = (EventType)hardEvents[rand() % 5];

                    // 40% chance for DOUBLE EVENT
                    if (rand() % 100 < 40)
                    {
                        secondEvent = (EventType)hardEvents[rand() % 5];
                        // prevent same event twice
                        while (secondEvent == currentEvent)
                            secondEvent = (EventType)hardEvents[rand() % 5];
                    }

                    eventTimer = 25.0f;

                    // HARD mode gets more frequent chaos
                    eventCooldown = 12.0f;
                }
            }

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
                    {

                        fogActive = true;
                        fogTimer += GetFrameTime();

                        if (fogTimer < 2.0f)
                            fogAlpha = Lerp(fogAlpha, 0.45f, 2.0f * GetFrameTime());

                        else
                            fogAlpha = Lerp(fogAlpha, 0.55f, 2.0f * GetFrameTime());
                    }
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

                    gravity = 1800.0f;
                    eventBoost = 1.0f;

                    fogTimer = 0.0f;

                    // RESET COOLDOWN
                    if (diff == MEDIUM)
                        eventCooldown = 20.0f;
                    else if (diff == HARD)
                        eventCooldown = 12.0f;
                }
            }

            // UPDATE ITEMS & COLLISION -----------------
            for (auto &it : items)
            {
                if (!it.active)
                    continue;
                it.rect.y += it.speed * GetFrameTime();

                // remove if off-screen
                if (it.rect.y > screenHeight)
                    it.active = false;

                if (CheckCollisionRecs(player, it.rect))
                {
                    // BAD ITEMS
                    if (it.type == POO || it.type == BOMB || it.type == SALT || it.type == GARLIC)
                    {
                        hp--;
                        shakeTime = 0.25f;
                        shakePower = 12.0f;
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
                        if (combo >= 2)
                            score += combo;
                    }
                    else if (it.type == ATAY)
                        score += 8;

                    else if (it.type == BLOOD || it.type == MEAT)
                    { // blood
                        score += 3;

                        combo++;
                        comboTime = 2.5f;
                        if (combo >= 2)
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

                if (comboBrokenTimer <= 0)
                    comboBroken = false;
            }

            // speedboost
            if (speedBoostTimer > 0)
            {
                speedBoostTimer -= GetFrameTime();
                if (speedBoostTimer <= 0)
                    chiliBoost = 1.0f;
            }

            // STAR
            if (showStarText)
            {
                starTextTimer -= GetFrameTime();
                if (starTextTimer <= 0)
                    showStarText = false;
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
        }

        if (state == GAMEOVER_ANIM)
        {
            gameOverAnimTimer += GetFrameTime();
            gameOverFlash -= GetFrameTime() * 1.5f;
            if (gameOverFlash < 0)
                gameOverFlash = 0;

            // After ~2.5 seconds, transition to actual game over menu
            if (gameOverAnimTimer >= 2.5f)
                state = GAMEOVER;
        }

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // game
        if (state == PLAYING)
        {
            ClearBackground(SKYBLUE);
            // camera
            BeginMode2D(camera);

            DrawTexturePro(
                bgTex,
                {0, 0, (float)bgTex.width, (float)bgTex.height},
                {0, 0, (float)screenWidth, (float)screenHeight},
                {0, 0},
                0,
                WHITE);
            DrawRectangleRec(player, RED);
            // draw items
            for (auto &it : items)
            {
                if (!it.active)
                    continue;
                Color col = WHITE;
                if (it.type == BOMB)
                    DrawTexturePro(bombTex, {0, 0, (float)bombTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == BABY)
                    DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)babyTex.height}, {it.rect.x, it.rect.y, babyTex.width * 0.15f, babyTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == MEDKIT)
                    DrawTexturePro(potionMedkitTex, {0, 0, (float)potionMedkitTex.width, (float)potionMedkitTex.height}, {it.rect.x, it.rect.y, potionMedkitTex.width * 0.15f, potionMedkitTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == BANDAGE)
                    DrawTexturePro(potionBandageTex, {0, 0, (float)potionBandageTex.width, (float)potionBandageTex.height}, {it.rect.x, it.rect.y, potionBandageTex.width * 0.15f, potionBandageTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == GARLIC)
                    DrawTexturePro(garlic1Tex, {0, 0, (float)garlic1Tex.width, (float)garlic1Tex.height}, {it.rect.x, it.rect.y, garlic1Tex.width * 0.2f, garlic1Tex.height * 0.2f}, {0, 0}, 0.0f, col);
                if (it.type == CHILI)
                    DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)chiliTex.height}, {it.rect.x, it.rect.y, chiliTex.width * 0.15f, chiliTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == TROLLFACE)
                    DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)trollFaceTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == HEART)
                    DrawTexturePro(heartTex, {0, 0, (float)heartTex.width, (float)heartTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == BLOOD)
                    DrawTexturePro(bloodTex, {0, 0, (float)bloodTex.width, (float)bloodTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == POO)
                    DrawTexturePro(pooTex, {0, 0, (float)pooTex.width, (float)pooTex.height}, {it.rect.x, it.rect.y, pooTex.width * 0.15f, pooTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == DICE)
                    DrawTexturePro(diceTex, {0, 0, (float)diceTex.width, (float)diceTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == MUSHROOM)
                    DrawTexturePro(mushroomTex, {0, 0, (float)mushroomTex.width, (float)mushroomTex.height}, {it.rect.x, it.rect.y, mushroomTex.width * 0.15f, mushroomTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == POISON)
                    DrawTexturePro(poisonTex, {0, 0, (float)poisonTex.width, (float)poisonTex.height}, {it.rect.x, it.rect.y, poisonTex.width * 0.15f, poisonTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == SALT)
                    DrawTexturePro(saltTex, {0, 0, (float)saltTex.width, (float)saltTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == HOLYWATER)
                    DrawTexturePro(holyTex, {0, 0, (float)holyTex.width, (float)holyTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == KRUS)
                    DrawTexturePro(crossTex, {0, 0, (float)crossTex.width, (float)crossTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == ATAY)
                    DrawTexturePro(atayTex, {0, 0, (float)atayTex.width, (float)atayTex.height}, it.rect, {0, 0}, 0.0f, col);
            }

            // TEXTURE OF FOG EFFECT
            if (fogActive && fogAlpha > 0)
            {
                float camLeft = camera.target.x - screenWidth / (2 * camera.zoom);
                float camRight = camera.target.x + screenWidth / (2 * camera.zoom);

                for (int row = 0; row < 6; row++)
                {
                    for (float x = camLeft - 300; x < camRight + 300; x += 240)
                    {
                        float y = row * 160;

                        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(LIGHTGRAY, fogAlpha * 0.5f));
                    }
                }
            }

            EndMode2D();

            // UI
            DrawText(TextFormat("hp: %d", hp), 10, 10, 20, WHITE);
            DrawText(TextFormat("score: %d", score), 20, 20, 40, WHITE);

            // POP UP TEXTS--------------------------------------

            if (showStarText)
                DrawText("STAR!", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
            if (showMinusText)
                DrawText("MINUS 10 HUHU", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
            if (showSlowText)
                DrawText("SLOW MO", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);

            // events
            string eventName = "";

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
                DrawRectangle(15, 60, 420, 40, Fade(BLACK, 0.5f));
                DrawText(eventName.c_str(), 25, 70, 28, RED);
            }

            // combo
            if (combo >= 2)
                DrawText(TextFormat("COMBO x%d", combo), screenWidth / 2 - 100, 20, 35, YELLOW);
            if (combo >= 5)
                DrawText(TextFormat("HOTSTREAK!!", combo), screenWidth / 2 - 120, 20, 40, ORANGE);
            if (combo >= 10)
                DrawText(TextFormat("UNSTOPPABLE", combo), screenWidth / 2 - 140, 20, 45, RED);
            if (comboBroken)
                DrawText("COMBO LOST!", screenWidth / 2 - 120, 70, 35, RED);
        }
        else if (state == TROLL_VIDEO)
        {
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

                DrawText(
                    "RELAPSE KA MUNA BOI",
                    screenWidth / 2 - 220,
                    screenHeight - 100,
                    40,
                    RED);
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

        else if (state == GAMEOVER_ANIM)
        {
            ClearBackground(BLACK);

            // Red flash overlay
            if (gameOverFlash > 0)
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, gameOverFlash));

            // Pulsing "GAME OVER" text that scales in
            float scale = Clamp(gameOverAnimTimer / 0.6f, 0.0f, 1.0f);
            int fontSize = (int)(80 * scale);
            int textW = MeasureText("GAME OVER", fontSize);
            DrawText("GAME OVER",
                     screenWidth / 2 - textW / 2,
                     screenHeight / 2 - fontSize / 2,
                     fontSize,
                     RED);

            // Optional blinking subtext after 1.2s
            if (gameOverAnimTimer > 1.2f)
            {
                int subW = MeasureText("your soul has been claimed...", 24);
                DrawText("your soul has been claimed...",
                         screenWidth / 2 - subW / 2,
                         screenHeight / 2 + 60,
                         24,
                         Fade(WHITE, (float)sin(gameOverAnimTimer * 6) * 0.5f + 0.5f));
            }
        }

        else if (state == GAMEOVER)
        {
            DrawText("GAME OVER", 300, 250, 40, RED);
            DrawText("Press enter to restart", 230, 320, 20, WHITE);
            // restart
            if (IsKeyPressed(KEY_ENTER))
            {
                if (highScore < score)
                    highScore = score;
                state = MENU;
                hp = 3;
                score = 0;
                combo = 0;
                items.clear();
                player.x = (screenWidth - player.width) / 2;
                move = 1.0f;
                gravity = 1800.0f;
                currentEvent = NONE;
                introMusic = LoadMusicStream("assets/sounds/intro.mp3");
                SetMusicVolume(introMusic, 0.5f);
                PlayMusicStream(introMusic);
            }
        }

        EndDrawing();
    }

    for (auto &t : videoFrames)
        UnloadTexture(t);

    UnloadTexture(introTex);

    currentFrame = 0;
    frameTimer = 0;

    // unload
    UnloadSound(trollSound);
    UnloadSound(gameOverSound);
    UnloadMusicStream(bgMusic);
    UnloadFont(nosifer);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
