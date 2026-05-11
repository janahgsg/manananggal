#include "intro.h"
#include "raylib.h"
#include <cmath>



// ================= UPDATE =================
bool UpdateIntro() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    float buttonWidth = 300;
    float buttonHeight = 80;

    Rectangle playButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        screenHeight / 2.0f,
        buttonWidth,
        buttonHeight
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, playButton);

    if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true; // start game
    }

    return false;
}

// ================= DRAW =================
void DrawIntro(int highScore, Texture2D introTex) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

     // ===== BACKGROUND =====
    DrawTextureEx(introTex, {0, 0}, 0.0f, 
        (float)screenWidth / introTex.width, WHITE);

    float buttonWidth = 300;
    float buttonHeight = 80;

    Rectangle playButton = {
        screenWidth / 2.0f - buttonWidth / 2,
        screenHeight / 2.0f,
        buttonWidth,
        buttonHeight
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, playButton);

    // ===== TITLE =====
    const char* title = "Curse of the Wings";
    int titleSize = 60;

    int titleWidth = MeasureText(title, titleSize);

    //DrawText(
       // title,
        //screenWidth / 2 - titleWidth / 2,
       // screenHeight / 2 - 150,
       // titleSize,
       // RED
   //);

   float fadeAlpha = fminf(GetTime() * 0.5f, 1.0f); // fade in over ~2 seconds

   //shadow
   DrawText(title, 
         screenWidth / 2 - titleWidth / 2 + 2,
         screenHeight / 2 - 150 + 2,
         titleSize,
         BLACK);

    // main title wl fade effect
    DrawText(title,
         screenWidth / 2 - titleWidth / 2,
         screenHeight / 2 - 150,
         titleSize,
         Fade(Color{255, 215, 0, 255}, fadeAlpha)); //fade in gold color


    // ===== BUTTON =====
    if (hovered) {
    Rectangle glowRect = {
            playButton.x - 5,
            playButton.y - 5,
            playButton.width + 10,
            playButton.height + 10
         };
         DrawRectangleRounded(glowRect, 0.3f, 10, Fade(SKYBLUE, 0.5f));
    }
    Color buttonColor = hovered ? Color{50, 205, 50, 255} : Color{34, 139, 34, 255};

    DrawRectangleRounded(playButton, 0.3f, 10, buttonColor);

    // ===== PLAY TEXT =====
    const char* playText = "PLAY";
    int playFontSize = 30;

    int textWidth = MeasureText(playText, playFontSize);

    DrawText(
        playText,
        playButton.x + playButton.width / 2 - textWidth / 2,
        playButton.y + playButton.height / 2 - playFontSize / 2,
        playFontSize,
        WHITE
    );


    const char* scoreText = TextFormat("High Score: %d", highScore);
       int scoreWidth = MeasureText(scoreText, 30);

     DrawRectangle(0, 0, screenWidth, 60, Fade(BLACK, 0.5f)); // semi-transparent bar

     DrawText(scoreText,
         screenWidth / 2 - scoreWidth / 2,
         screenHeight - 50,
         30,
         Color{245, 245, 220, 255});

}