#include "intro.h"
#include "raylib.h"



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

    DrawText(
        title,
        screenWidth / 2 - titleWidth / 2,
        screenHeight / 2 - 150,
        titleSize,
        RED
    );

    // ===== BUTTON =====
    Color buttonColor = hovered ? SKYBLUE : RED;

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

    // ===== HIGH SCORE =====
    DrawText(
        TextFormat("High Score: %d", highScore),
        20,
        20,
        30,
        WHITE
    );
}