#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <climits>
#include <ctime>
using namespace std;

enum GameState{
    MENU,
    PLAYING,
    TROLL_VIDEO,
    GAMEOVER
};

struct Item{
    Rectangle rect; // items
    int type; 
    float speed;
    bool active;
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
    Texture2D bombTex   = LoadTexture("assets/images/bomb.png");
    Texture2D babyTex   = LoadTexture("assets/images/baby.png");
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
    
    int score = 0;
    int point = 0;
    int hp = 3;
    float move = 1.0f;
    vector<Item> items;
    float spawnTimer = 0;
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    //center horizontally
    player.x = (screenWidth - player.width) / 2;
    player.y = screenHeight - player.height;

    while(!WindowShouldClose()){
        //menu
        if(state == MENU){
            if(IsKeyPressed(KEY_ENTER)) state = PLAYING;
        }
        //during play
        if(state == PLAYING){
            float moveSpeed =  5 * move;

            //movements
            if(IsKeyDown(KEY_LEFT)) player.x -= moveSpeed;
            if(IsKeyDown(KEY_RIGHT)) player.x += moveSpeed;

            //left and right boundaries
            if(player.x < 0) player.x = 0;
            if(player.x + player.width > screenWidth)
                player.x = screenWidth - player.width;

            // spawn items every 1 second
            spawnTimer += GetFrameTime(); // Add time per frame
            if(spawnTimer > 1.0f){
                spawnTimer = 0;
                Item it;
                it.rect.x = rand() % (screenWidth - (int)it.rect.width); //random x position
                it.rect.y = 0; // y starts at top
                it.rect.width = 80;
                it.rect.height = 80;
                it.speed = 2 + rand() % 3;
                it.active = true;
                it.type = rand() % 7;

                items.push_back(it); //add item to vector
            }

            //update items
            for (auto &it : items){
                if(!it.active) continue;
                it.rect.y += it.speed;
                //remove if off-screen
                if(it.rect.y > screenHeight) it.active = false;

                if(CheckCollisionRecs(player, it.rect)){
                    if(it.type == 0){ //bomb
                        hp -= 1;
                    }else if(it.type == 1){ //food
                            score += 5;
                            if(hp > 3) hp = 3;
                        }else if(it.type == 2){ //medkit
                                hp = 3; 
                            }else if(it.type == 3){//bandage
                                    hp += 1;
                                    if(hp > 3) hp = 3;
                                }else if(it.type == 4){//poison
                                        hp -= 1;
                                    }else if(it.type == 5){//chili
                                            move = 2.0f;
                                        }else if(it.type == 6){
                                            state = TROLL_VIDEO;
                                            currentFrame = 0;
                                            frameTimer = 0;
                                            PlaySound(trollSound);
                                        }
                    it.active = false; //remove item after collision
                }
            }
            move += (1.0f - move) * 0.01f; 
            if(hp <= 0) state = GAMEOVER;
        }
        if(state == TROLL_VIDEO){
            // add time every frame (Raylib gives delta time)
            frameTimer += GetFrameTime();

            // when enough time passed → next frame
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
        //character
        if(state == MENU) DrawText("Welcome, Type ENTER to play", 190, 200, 20, LIGHTGRAY);
        //game
        if(state == PLAYING){
            ClearBackground(SKYBLUE);
            DrawRectangleRec(player, RED);
            //draw items
            for(auto &it : items){
                if(!it.active) continue;
                Color col = WHITE;
                if(it.type == 0) DrawTexturePro(bombTex, {0, 0, (float)bombTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == 1) DrawTexturePro(babyTex, {0, 0, (float)babyTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == 2) DrawTexturePro(medkitTex, {0, 0, (float)medkitTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == 3) DrawTexturePro(bandageTex, {0, 0, (float)bandageTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == 4) DrawTexturePro(garlicTex, {0, 0, (float)garlicTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == 5) DrawTexturePro(chiliTex, {0, 0, (float)chiliTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);
                if(it.type == 6) DrawTexturePro(trollFaceTex, {0, 0, (float)trollFaceTex.width, (float)bombTex.height}, it.rect, {0, 0}, 0.0f, col);

            }

            DrawText(TextFormat("hp: %d", hp), 10, 10, 20,  WHITE);
            DrawText(TextFormat("score: %d", score), 20, 20, 40,  WHITE);
        }else if(state == TROLL_VIDEO){
            ClearBackground(WHITE);
            if(!videoFrames.empty() && currentFrame < videoFrames.size()){
                DrawTexture(videoFrames[currentFrame], 0, 0, WHITE);
                DrawText("RELAPSE KA MUNA BOI", 250, 500, 30, RED);
            }
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
//run it 
// g++ raylib-proj/my-game/main.cpp -o game.exe -I"C:/Users/norja/Documents/PUP-Subs/COMP PROG/C++/raylib-proj/raylib/raylib-6.0_win64_mingw-w64/include" -L"C:/Users/norja/Documents/PUP-Subs/COMP PROG/C++/raylib-proj/raylib/raylib-6.0_win64_mingw-w64/lib" -lraylib -lopengl32 -lgdi32 -lwinmm
