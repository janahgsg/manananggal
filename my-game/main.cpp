#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cmath>
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
    TROLLFACE
};

struct Item{
    Rectangle rect; // items
    int type; 
    float speed;
    bool active;
};

enum Difficulty {
    EASY,
    MEDIUM,
    HARD
};
//stores every frame (Texture2D frame1 and so on)
vector<Texture2D> videoFrames;
int currentFrame = 0;
float frameTimer = 0;
Sound trollSound;

int main() {
    SetWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Raylib - Wings of the Curse");

    //load sound
    InitAudioDevice();
    trollSound = LoadSound("assets/sounds/trollFace.mp3");
    //load frames
    for(int i = 1; i <= 110; i++){
        videoFrames.push_back(LoadTexture(TextFormat("assets/videos/trollFace/ezgif-frame-%03d.png", i)));
    }
    //load images
    //bg
    Texture2D bgTex = LoadTexture("assets/images/bg.png");
    Texture2D groundTex = LoadTexture("assets/images/ground.png");
    Texture2D wallTex = LoadTexture("assets/images/wall.png");

    //items
    Texture2D bombTex = LoadTexture("assets/images/bomb.png");
    Texture2D babyTex = LoadTexture("assets/images/baby.png");
    Texture2D medkitTex = LoadTexture("assets/images/med_kit.png");
    Texture2D bandageTex = LoadTexture("assets/images/bandage.png");
    Texture2D trollFaceTex = LoadTexture("assets/images/trollFace.png");
    Texture2D chiliTex = LoadTexture("assets/images/chili.png");
    Texture2D garlicTex = LoadTexture("assets/images/garlic.png");

    srand(time(NULL));
    SetTargetFPS(60);
    GameState state = MENU;
    //player
    Rectangle player;
    player.width = 50;
    player.height = 50;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    //center horizontally
    player.x = (screenWidth - player.width) / 2;
    player.y = screenHeight * 0.85f;
    int score = 0;
    int highScore = 0;
    int hp = 3;
    float move = 1.0f;
    vector<Item> items;
    float spawnTimer = 0; // time has spawned

    // movements
    float velocityX = 0;      // player movement momentum
    

    
    //shake effect
    float shakeTime = 0;     // how long screen shakes
    float shakePower = 0;     // strength of shake
    float hitFlash = 0;       // red flash when damaged

    Difficulty diff = EASY;
    //camera 
    Camera2D camera = {0};
    camera.offset = {
        screenWidth / 2.0f,
        screenHeight * 0.85f
    };
    camera.target = {
        player.x + player.width / 2,
        player.y + player.height / 2
    };
    camera.rotation = 0.0f;
    camera.zoom = 1.30f;

    while(!WindowShouldClose()){
        if (IsKeyPressed(KEY_Q)) {
        CloseWindow();
        return 0;   // exit program 
    }
        //menu
        if(state == MENU){
            if(IsKeyPressed(KEY_ENTER)) state = PLAYING;
        }
        //during play
        if(state == PLAYING){
            float moveSpeed = 400 * move;

            float accel = 2200.0f;      // how fast player gains speed
            float friction = 0.92f;     // slows naturally
            float maxSpeed = 520.0f * move;

            //movements
            if(IsKeyDown(KEY_LEFT)) velocityX -= accel * GetFrameTime();
            if(IsKeyDown(KEY_RIGHT)) velocityX += accel * GetFrameTime();

            //slow down
            velocityX *= friction;

            //limt max
            if(velocityX > maxSpeed) velocityX = maxSpeed;
            if(velocityX < -maxSpeed) velocityX = -maxSpeed;

            player.x += velocityX * GetFrameTime();
        
            //left and right boundaries
            if(player.x < 0) player.x = 0;
            if(player.x + player.width > screenWidth)
                player.x = screenWidth - player.width;

            float wantedX = player.x + player.width / 2;

            // smooth follow
            camera.target.x += (wantedX - camera.target.x) * 0.12f;

            // keep Y fixed so player remains bottom
            camera.target.y = player.y + player.height / 2;
            
            //camera shake
            if (shakeTime > 0) {
                shakeTime -= GetFrameTime();
                if (shakeTime < 0) shakeTime = 0;
            }

            Vector2 shakeOffset = {0,0};

            if (shakeTime > 0) {
                float intensity = shakePower * (shakeTime / 0.25f);

                shakeOffset.x = (GetRandomValue(-100, 100) / 100.0f) * intensity;
                shakeOffset.y = (GetRandomValue(-100, 100) / 100.0f) * intensity;
            }

            camera.offset = {
                screenWidth / 2.0f + shakeOffset.x,
                screenHeight * 0.85f + shakeOffset.y
            };

            //difficulty
            if(score >= 120) diff = HARD;
            else if(score >= 50) diff = MEDIUM;
            else diff = EASY;

            //SPAWNING------------

            // spawn items every 1 second
            spawnTimer += GetFrameTime(); // Add time per frame
            float spawnDelay;
            int spawnAmount;
            if(diff == EASY){
                spawnDelay = 1.0f;
                spawnAmount = 1;
            }else if(diff == MEDIUM){
                spawnDelay = 0.65f;
                spawnAmount = 2;
                move = 1.2f;
            }else if(diff == HARD){
                spawnDelay = 0.40f;
                spawnAmount = 3;
                move = 1.4f;
            }

            if(spawnTimer > spawnDelay){
                spawnTimer = 0;
                //spawnn items
                for(int i = 0; i < spawnAmount; i++){
                    Item it;
                    it.rect.width = 90;
                    it.rect.height = 90;
                    it.rect.x = rand() % (screenWidth - (int)it.rect.width); //random x position
                    it.rect.y = 0;  

                    if(diff == EASY) it.speed = 140.0f;
                    else if(diff == MEDIUM) it.speed = 200.0f;
                    else if(diff == HARD)it.speed = 320.0f;

                    it.active = true;

                    static bool prizeSpawn = false;
                    if(score >= 500 && rand()%100 < 15){
                        prizeSpawn = true;
                        it.type = PRIZE;
                    }else if(hp == 1 && rand()%100 < 10) it.type = MEDKIT;
                    else if(diff == EASY){
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, TROLLFACE};
                        int randomIndex =  rand() % 6;
                        it.type = pool[randomIndex];
                    }else if(diff == MEDIUM){
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, BOMB, POISON, MEAT, HEART, TROLLFACE};
                        int randomIndex =  rand() % 10;
                        it.type = pool[randomIndex];
                    }else if(diff == HARD){
                        int pool[] = {POO, GARLIC, BANDAGE, BABY, BLOOD, BOMB, POISON, MEAT, HEART,TROLLFACE};

                        int chance = rand() % 100;
                        if(chance < 70){
                            int randomIndex =  rand() % 10;
                            it.type = pool[randomIndex];
                        }else if(chance < 85) it.type = MUSHROOM;
                        else if(chance < 95) it.type = DICE;
                        else it.type = STAR;
                    }

                    items.push_back(it); //add item to vector
                }
            }
            

            //UPDATE ITEMS & COLLISION -----------------
            for (auto &it : items){
                if(!it.active) continue;
                it.rect.y += it.speed * GetFrameTime();

                //remove if off-screen
                if(it.rect.y > screenHeight) it.active = false;

                if(CheckCollisionRecs(player, it.rect)){
                    //BAD ITEMS
                    if (it.type == POO || it.type == BOMB || it.type == SALT || it.type == GARLIC) {
                        hp--;
                        shakeTime = 0.25f;
                        shakePower = 12.0f;
                    }else if(it.type == CHILI){//
                        move += 1.0f;
                    }
                    //SCORE++
                    else if(it.type == BABY || it.type == HEART){ //baby
                        score += 5;
                    }else if(it.type == BLOOD || it.type == MEAT){ //blood
                        score += 3;
                    }
                    //HEAL
                    else if(it.type == BANDAGE){//bandage
                        hp += 1;
                        if(hp > 3) hp = 3;
                    }else if(it.type == MEDKIT){ //medkit
                        hp = 3; 
                    }
                    //RANDOMNESS
                    else if(it.type == TROLLFACE){
                        state = TROLL_VIDEO;
                        currentFrame = 0;
                        frameTimer = 0;
                        PlaySound(trollSound);

                    }else if(it.type == POISON){//poison
                        move -= 2.0f;
                    }   
                    //SPECIAL ITEMS
                    else if(it.type == DICE){//dice(good effects)
                        int randomIndex = rand() % 4;
                        if(randomIndex == 1){
                            score += 10;
                            DrawText("STAR!", screenWidth/2 - 220, screenHeight - 100, 40, WHITE);
                        }
                    }
                    else if(it.type == MUSHROOM){//mushroom(bad effects)
                        int randomIndex = rand() % 4;
                        if(randomIndex == 1){
                            score -= 10;
                            DrawText("MINUS 10 HUHU", screenWidth/2 - 220, screenHeight - 100, 40, WHITE);
                        }else if(randomIndex == 2){
                            move -= 2.0f;
                            DrawText("SLOW MO", screenWidth/2 - 220, screenHeight - 100, 40, WHITE);
                        }
                    }
                    else if(it.type == STAR){//star 
                        score += 10;
                    }

                    //special prize(super rare)
                    else if(it.type == PRIZE){ //gift?
                        
                    }

                    it.active = false; //remove item after collision
                }
            }
            move += (1.0f - move) * 0.01f; 
            if(hp <= 0) state = GAMEOVER;
        }
        if(state == TROLL_VIDEO){
            // add time every frame 
            frameTimer += GetFrameTime();

            // when enough time passed, next frame
            if(frameTimer >= 0.2f){
                frameTimer = 0;
                currentFrame++;
            }

                // if video ended, go back to game
                if(currentFrame >= videoFrames.size()){
                StopSound(trollSound);  // stop audio
                currentFrame = 0;       // reset video
                state = PLAYING;        // resume game
            }
        }

        //drawing
        BeginDrawing();
        ClearBackground(BLACK);
        //menu
        if(state == MENU){
            DrawText("Welcome, Type ENTER to play", 190, 200, 20, LIGHTGRAY);
            DrawText(TextFormat("High Score: %d", highScore), 20, 20, 40,  WHITE);
        }
        //game
        if(state == PLAYING){
            ClearBackground(SKYBLUE); 
            //camera
            BeginMode2D(camera);
            DrawTexturePro(
                bgTex,
                {0,0,(float)bgTex.width,(float)bgTex.height},
                {0,0,(float)screenWidth,(float)screenHeight},
                {0,0},
                0,
                WHITE
            );
            DrawRectangleRec(player, RED);
            //draw items
            for(auto &it : items){
                if(!it.active) continue;
                Color col = WHITE;
                if(it.type == BOMB) DrawTexturePro(bombTex, {0, 0, (float)bombTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == BABY) DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == MEDKIT) DrawTexturePro(medkitTex, {0, 0, (float)medkitTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == BANDAGE) DrawTexturePro(bandageTex, {0, 0, (float)bandageTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == GARLIC) DrawTexturePro(garlicTex, {0, 0, (float)garlicTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == CHILI) DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == TROLLFACE) DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == HEART) DrawRectangleRec(it.rect, PINK);
                if(it.type == BLOOD) DrawRectangleRec(it.rect, RED);
                if(it.type == POO) DrawRectangleRec(it.rect, BROWN);
                if(it.type == DICE) DrawRectangleRec(it.rect, WHITE);
                if(it.type == MUSHROOM) DrawRectangleRec(it.rect, GREEN);
                if(it.type == POISON) DrawRectangleRec(it.rect, BLACK);
                if(it.type == SALT) DrawRectangleRec(it.rect, LIGHTGRAY);
            
            }

            EndMode2D();  

            //UI
            DrawText(TextFormat("hp: %d", hp), 10, 10, 20,  WHITE);
            DrawText(TextFormat("score: %d", score), 20, 20, 40,  WHITE);

        }else if(state == TROLL_VIDEO){
            ClearBackground(WHITE);
            if(!videoFrames.empty() && currentFrame < videoFrames.size()){
                DrawTexturePro(
                    videoFrames[currentFrame],
                    {0,0,
                    (float)videoFrames[currentFrame].width,
                    (float)videoFrames[currentFrame].height},
                    {0,0,
                    (float)screenWidth,
                    (float)screenHeight},
                    {0,0},
                    0,
                    WHITE
                );
        
                DrawText(
                    "RELAPSE KA MUNA BOI",
                    screenWidth/2 - 220,
                    screenHeight - 100,
                    40,
                    RED
                );
            }
        }

        if(hitFlash > 0) DrawRectangle(0,0, screenWidth,screenHeight,Fade(RED, hitFlash));

        //heartbeat
        if(hp == 1){
            int pulse = 20 + sin(GetTime()*8)*10;
            DrawText("WARNING!", screenWidth/2 - 100, 50, pulse, RED);
        }


        else if(state == GAMEOVER){
            DrawText("GAME OVER", 300, 250, 40, RED);
            DrawText("Press enter to restart", 230, 320, 20, WHITE);
            //restart
            if(IsKeyPressed(KEY_ENTER)){
                state = MENU;
                hp = 3;
                items.clear();
                player.x = 400;    // Reset player position
                move = 1.0f; // Reset speed
                if(highScore < score) highScore = score;
            }
        }
        
        EndDrawing();
    }
    for(auto &t : videoFrames){
        UnloadTexture(t);
    }

    currentFrame = 0;
    frameTimer = 0;

    UnloadSound(trollSound);
    CloseWindow();
    return 0;
}

