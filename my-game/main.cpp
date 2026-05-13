#include "intro.h"
#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cmath>
#include "raymath.h"
using namespace std;

enum GameState{
    MENU,
    PLAYING,
    TROLL_VIDEO,
    GAMEOVER
};

enum ItemType{
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

struct Item{
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

struct Smoke{
    float x;
    float y;
    float size;
    float speed;
    float alpha;
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

    // load intro music
    Music introMusic = LoadMusicStream("assets/sounds/intro.mp3");
    SetMusicVolume(introMusic, 0.5f);  
    PlayMusicStream(introMusic);


    // load frames
    for (int i = 1; i <= 110; i++)
    {
        videoFrames.push_back(LoadTexture(TextFormat("assets/videos/trollFace/ezgif-frame-%03d.png", i)));
    }
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


    srand(time(NULL));
    SetTargetFPS(60);
    
    GameState state = MENU;
    // player
    Rectangle player;
    player.width = 50;
    player.height = 50;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    //CAMERA
    player.x = (screenWidth - player.width) / 2;
    player.y = screenHeight * 0.85f;

    //SCORE & HEALTH
    int score = 0;
    int highScore = 0;
    int hp = 3;
    
    //ITEMS
    vector<Item> items;
    float spawnTimer = 0; // time has spawned
    
    //COMBO
    int combo = 0;
    float comboTime = 0;
    int comboPop = 0;
    
    //SHAKE EFFECT
    float shakeTime = 0;     // how long screen shakes
    float shakePower = 0;     // strength of shake
    float hitFlash = 0;       // red flash when damaged

    float medkitCooldown = 0;

    //FOG EFFECT
    float fogAlpha = 0.0f;
    float fogTimer = 0.0f;
    float fogMoveX = -900.0f;
    bool fogActive = false;
    float fogCooldown = 0.0f; 
    float nextFogTime = 3.0; 


    //MOVEMENTS
    float velocityX = 0;      // player movement momentum
    //slow effect
    float slowTime = 0;
    float move = 1.0f;
    float baseMove = 1.0f;
    float velocityY = 0;     // vertical speed
    float gravity = 1800.0f; // pull down
    float jumpForce = -700.0f; // jump strength (negative = up)
    bool isGrounded = true;

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

    
        // menu
        if (state == MENU)
        {
            UpdateMusicStream(introMusic);

            if (UpdateIntro())
            {
                StopMusicStream(introMusic);
                UnloadMusicStream(introMusic);
                state = PLAYING;
            }

            if (UpdateExit())
            {
                StopMusicStream(introMusic);
                UnloadMusicStream(introMusic);
                break;            
            }

            DrawIntro(highScore, introTex);
        }

        // during play
        if (state == PLAYING)
        {
            UpdateMusicStream(bgMusic);
            float moveSpeed = 400 * move;

            //movements
            float accel = 2200.0f;      // how fast player gains speed
            float friction = 0.92f;     // slows naturally
            float maxSpeed = 520.0f * move;

            

            if(IsKeyDown(KEY_LEFT)) velocityX -= accel * GetFrameTime();
            if(IsKeyDown(KEY_RIGHT)) velocityX += accel * GetFrameTime();
            if (IsKeyDown(KEY_UP) && isGrounded){
                velocityY = jumpForce;
                isGrounded = false;
            }

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
            if (diff == EASY){
                spawnDelay = 1.0f;
                spawnAmount = 1;
                baseMove = 1.0f;
            }else if (diff == MEDIUM){
                spawnDelay = 0.65f;
                spawnAmount = 2;
                baseMove = 1.2f;
            }
            else if (diff == HARD){
                spawnDelay = 0.40f;
                spawnAmount = 3;
                baseMove = 1.4f;
            }

            if (spawnTimer > spawnDelay){
                spawnTimer = 0;
                // spawnn items
                for (int i = 0; i < spawnAmount; i++)
                {
                    Item it;
                    it.rect.width = 40;
                    it.rect.height = 40;
                    it.rect.x = rand() % (screenWidth - (int)it.rect.width); // random x position
                    it.rect.y = 0;

                    if (diff == EASY)
                        it.speed = 140.0f;
                    else if (diff == MEDIUM)
                        it.speed = 200.0f;
                    else if (diff == HARD)
                        it.speed = 320.0f;

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
                    else if (diff == MEDIUM){
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, BOMB,
                                      POISON, MEAT, HEART, TROLLFACE, ATAY};
                        int randomIndex = rand() % 11;
                        it.type = pool[randomIndex];
                    }
                    else if (diff == HARD){
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, BOMB, POISON, MEAT, HEART, TROLLFACE, ATAY};

                        int chance = rand() % 100;
                        if (chance < 70){
                            int randomIndex = rand() % 11;
                            it.type = pool[randomIndex];
                        }
                        else if (chance < 85) it.type = MUSHROOM;
                        else if (chance < 95) it.type = DICE;
                        else it.type = STAR;
                    }

                    items.push_back(it); // add item to vector
                }
            }

            // FOG EFFECT APPEARANCE -----
            if(diff == HARD){
                float dt = GetFrameTime();

                fogCooldown += dt;

                if(!fogActive && fogCooldown >= nextFogTime){
                    fogActive = true;
                    fogTimer = 0;
                    fogAlpha = 0;
                    fogCooldown = 0;
                }

                if(fogActive){
                    fogTimer += dt;

                    // Fade in 
                    if(fogTimer < 10.0f) fogAlpha = Lerp(fogAlpha, 0.40f, 0.30f * dt);
                    // Stay foggy
                    else if(fogTimer < 28.0f) fogAlpha = Lerp(fogAlpha, 0.42f, 0.6f * dt);
                    // Fade out 
                    else fogAlpha = Lerp(fogAlpha, 0.0f, 0.22f * dt);
                    // End 
                    if(fogTimer >= 28.0f  && fogAlpha <= 0.01f){ 
                        fogActive = false;
                        fogAlpha = 0.0f;
                        fogCooldown = 0.0f;
                        nextFogTime = 280 + rand()%41;    // around 280 to 320 sec 
                    }  
                }
            }
            else fogAlpha = Lerp(fogAlpha, 0.0f, 2.0f * GetFrameTime());


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
                    }
                    else if (it.type == CHILI)
                    {
                        move += 1.0f;
                    }
                    // SCORE++
                    else if (it.type == BABY || it.type == HEART)
                    { // baby
                        score += 5;
                    }
                    else if (it.type == ATAY) score += 8;

                    else if (it.type == BLOOD || it.type == MEAT){ // blood
                        score += 3;
                    }
                    // HEAL
                    else if (it.type == BANDAGE){ // bandage
                        hp += 1;
                        if (hp > 3)
                            hp = 3;
                    }
                    else if (it.type == MEDKIT) hp = 3;
                    // RANDOMNESS
                    else if (it.type == TROLLFACE){
                        state = TROLL_VIDEO;
                        currentFrame = 0;
                        frameTimer = 0;
                        PlaySound(trollSound);
                    }
                    else if (it.type == POISON) move -= 2.0f;
                    
                    // SPECIAL ITEMS
                    else if (it.type == DICE){ // dice(good effects)
                        int randomIndex = rand() % 4;
                        if (randomIndex == 1)
                        {
                            score += 10;
                            DrawText("STAR!", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
                        }
                    }
                    else if (it.type == MUSHROOM)
                    { // mushroom(bad effects)
                        int randomIndex = rand() % 4;
                        if (randomIndex == 1)
                        {
                            score -= 10;
                            DrawText("MINUS 10 HUHU", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
                        }
                        else if (randomIndex == 2)
                        {
                            move -= 2.0f;
                            DrawText("SLOW MO", screenWidth / 2 - 220, screenHeight - 100, 40, WHITE);
                        }
                    }
                    else if (it.type == STAR) score += 10;

                    // special prize(super rare)
                    else if (it.type == PRIZE)
                    { // gift?
                    }
                   
                    

                    it.active = false; // remove item after collision
                }
            }
            move += (1.0f - move) * 0.01f;
            if (hp <= 0) state = GAMEOVER;
        }
        if (state == TROLL_VIDEO){
            // add time every frame
            frameTimer += GetFrameTime();

            // when enough time passed, next frame
            if (frameTimer >= 0.2f){
                frameTimer = 0;
                currentFrame++;
            }

            // if video ended, go back to game
            if (currentFrame >= videoFrames.size()){
                StopSound(trollSound); // stop audio
                currentFrame = 0;      // reset video
                state = PLAYING;       // resume game
            }
        }

        // drawing
        BeginDrawing();
        ClearBackground(BLACK);
        // menu
       // if (state == MENU){
            //DrawText("Welcome, Type ENTER to play", 190, 200, 20, LIGHTGRAY);
            //DrawText(TextFormat("High Score: %d", highScore), 20, 20, 40, WHITE);
        //if (!IsMusicStreamPlaying(bgMusic)){
               // DrawText("Music not playing!", 10, 50, 20, RED);
           // }
        //}
        // game
        if (state == PLAYING){
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
                    DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)babyTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == MEDKIT)
                    DrawTexturePro(potionMedkitTex, {0, 0, (float)potionMedkitTex.width, (float)potionMedkitTex.height},  {it.rect.x, it.rect.y, potionMedkitTex.width * 0.15f, potionMedkitTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == BANDAGE)
                    DrawTexturePro(potionBandageTex, {0, 0, (float)potionBandageTex.width, (float)potionBandageTex.height},  {it.rect.x, it.rect.y, potionBandageTex.width * 0.15f, potionBandageTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == GARLIC)
                    DrawTexturePro(garlic1Tex, {0, 0, (float)garlic1Tex.width, (float)garlic1Tex.height},  {it.rect.x, it.rect.y, garlic1Tex.width * 0.15f, garlic1Tex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == CHILI)
                    DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)chiliTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == TROLLFACE)
                    DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)trollFaceTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == HEART)
                    DrawTexturePro(heartTex, {0, 0, (float)heartTex.width, (float)heartTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == BLOOD)
                    DrawTexturePro(bloodTex, {0, 0, (float)bloodTex.width, (float)bloodTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == POO)
                    DrawTexturePro(pooTex, {0, 0, (float)pooTex.width, (float)pooTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == DICE)
                    DrawTexturePro(diceTex, {0, 0, (float)diceTex.width, (float)diceTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == MUSHROOM)
                    DrawTexturePro(mushroomTex, {0, 0, (float)mushroomTex.width, (float)mushroomTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == POISON)
                   DrawTexturePro(poisonTex,{0, 0, (float)poisonTex.width, (float)poisonTex.height},  {it.rect.x, it.rect.y, poisonTex.width * 0.15f, poisonTex.height * 0.15f}, {0, 0}, 0.0f, col);
                if (it.type == SALT)
                    DrawTexturePro(saltTex, {0, 0, (float)saltTex.width, (float)saltTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == HOLYWATER)
                    DrawTexturePro(holyTex, {0, 0, (float)holyTex.width, (float)holyTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == KRUS)
                    DrawTexturePro(crossTex, {0, 0, (float)crossTex.width, (float)crossTex.height}, it.rect, {0, 0}, 0.0f, col);
                if (it.type == ATAY)
                    DrawTexturePro(atayTex, {0, 0, (float)atayTex.width, (float)atayTex.height}, it.rect, {0, 0}, 0.0f, col);
            }

            //TEXTURE OF FOG EFFECT
            if(diff == HARD && fogAlpha > 0){
                float camLeft  = camera.target.x - screenWidth / (2 * camera.zoom);
                float camRight = camera.target.x + screenWidth / (2 * camera.zoom);

                for(int row = 0; row < 6; row++){
                    for(float x = camLeft - 300; x < camRight + 300; x += 240){
                        float y = row * 160;

                        DrawCircleGradient({x, y},220,Fade(LIGHTGRAY, fogAlpha),Fade(WHITE, 0.0f));
                        DrawCircleGradient({x + 100, y + 50},260,Fade(GRAY, fogAlpha * 0.8f),Fade(WHITE, 0.0f));
                    }
                }
            }
            

            EndMode2D();

            

            // UI
            DrawText(TextFormat("hp: %d", hp), 10, 10, 20, WHITE);
            DrawText(TextFormat("score: %d", score), 20, 20, 40, WHITE);
        }
        else if (state == TROLL_VIDEO){
            ClearBackground(WHITE);
            if (!videoFrames.empty() && currentFrame < videoFrames.size()){
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

        if (hitFlash > 0) DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, hitFlash));

        // heartbeat text
        if (hp == 1){
            int pulse = 20 + sin(GetTime() * 8) * 10;
            DrawText("WARNING!", screenWidth / 2 - 100, 50, pulse, RED);
        }

        // combo text
        if (combo > 1)DrawText(TextFormat("COMBO x%d", combo), screenWidth / 2 - 100, 20, 35, YELLOW);
        if (combo == 5)DrawText(TextFormat("HOTSTREAK!!", combo), screenWidth / 2 - 100, 20, 35, ORANGE);
        if (combo == 10)DrawText(TextFormat("UNSTOPPABLE", combo), screenWidth / 2 - 100, 20, 35, RED);

        else if (state == GAMEOVER){
            DrawText("GAME OVER", 300, 250, 40, RED);
            DrawText("Press enter to restart", 230, 320, 20, WHITE);
            // restart
            if (IsKeyPressed(KEY_ENTER)){
                if (highScore < score)
                    highScore = score;
                state = MENU;
                hp = 3;
                score = 0; 
                items.clear();
                player.x = 400;
                move = 1.0f;
            }
        }

        EndDrawing();
    }

    for (auto &t : videoFrames)UnloadTexture(t);

    UnloadTexture(introTex);
    
    currentFrame = 0;
    frameTimer = 0;

    //unload
    UnloadSound(trollSound);
    UnloadMusicStream(bgMusic);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
