// Include raylib library header (graphics, input, window functions)
#include "raylib.h"

// Include C++ standard vector container (dynamic arrays)
#include <vector>

// Include C time library (for time functions like time(NULL))
#include <ctime>

// Include C standard library (for random number functions like rand, srand)
#include <cstdlib>

// Use the standard namespace so we can write 'vector' instead of 'std::vector'
using namespace std;

// Define possible game states using an enum (enumeration)
enum GameState {
    MENU,       // Main menu screen
    PLAYING,    // Gameplay loop
    GAME_OVER   // Game over screen
};

// Define an Item struct to represent falling objects
struct Item {
    Rectangle rect;  // Position and size of the item (x, y, width, height)
    int type;        // Item type: 0 = bomb, 1 = food, 2 = medkit, 3 = poison
    float speed;     // Falling speed of the item
    bool active;     // Whether the item is currently active (true = visible, false = removed)
};

int main() {
    // Create a window 800x600 pixels with a title
    InitWindow(800, 600, "Curse of the Wings: Rage Edition");

    // Set the game to run at 60 frames per second
    SetTargetFPS(60);

    // Seed the random number generator with the current time
    // This ensures different random values each run
    srand(time(NULL));

    // Start the game in the MENU state
    GameState state = MENU;

    // Define the player rectangle (Manananggal)
    // Positioned at x=400, y=500, with width=50, height=50
    Rectangle player = {400, 500, 50, 50};
    int hp = 3;                // Player health points
    float speedBoost = 1.0f;   // Speed multiplier (affected by poison)

    // Vector to store all active falling items
    vector<Item> items;

    // Timer to control item spawning
    float spawnTimer = 0;

    // Main game loop (runs until window is closed)
    while (!WindowShouldClose()) {

        // ================= MENU =================
        if (state == MENU) {
            // Press ENTER to start playing
            if (IsKeyPressed(KEY_ENTER)) {
                state = PLAYING;
            }
        }

        // ================= GAME =================
        if (state == PLAYING) {

            // Player movement speed (affected by speedBoost)
            float moveSpeed = 5 * speedBoost;

            // Move player left or right with arrow keys
            if (IsKeyDown(KEY_LEFT)) player.x -= moveSpeed;
            if (IsKeyDown(KEY_RIGHT)) player.x += moveSpeed;

            // Keep player inside screen boundaries
            if (player.x < 0) player.x = 0;
            if (player.x > 750) player.x = 750;

            // Spawn items every 1 second
            spawnTimer += GetFrameTime(); // Add time per frame
            if (spawnTimer > 1.0f) {
                spawnTimer = 0;

                Item it;
                it.rect.x = rand() % 750;   // Random x position
                it.rect.y = 0;              // Start at top
                it.rect.width = 30;         // Item width
                it.rect.height = 30;        // Item height
                it.speed = 2 + rand() % 3;  // Random falling speed
                it.active = true;           // Item is active
                it.type = rand() % 4;       // Random type (0–3)

                items.push_back(it);        // Add item to vector
            }

            // Update items
            for (auto &it : items) {
                if (!it.active) continue;   // Skip inactive items

                // Move item down
                it.rect.y += it.speed;

                // Check collision with player
                if (CheckCollisionRecs(player, it.rect)) {

                    // Apply item effects
                    if (it.type == 0) { // bomb
                        hp -= 1;
                    }
                    if (it.type == 1) { // food
                        hp += 1;
                        if (hp > 3) hp = 3; // Max HP = 3
                    }
                    if (it.type == 2) { // medkit
                        hp = 3; // Full heal
                    }
                    if (it.type == 3) { // poison
                        hp -= 1;
                        speedBoost = 0.7f; // Slow player
                    }

                    it.active = false; // Remove item after collision
                }

                // Deactivate item if it falls off screen
                if (it.rect.y > 600) it.active = false;
            }

            // Gradually reset speedBoost back to 1.0
            speedBoost += (1.0f - speedBoost) * 0.01f;

            // If HP reaches 0, go to GAME_OVER state
            if (hp <= 0) {
                state = GAME_OVER;
            }
        }

        // ================= DRAW =================
        BeginDrawing();
        ClearBackground(BLACK);

        if (state == MENU) {
            DrawText("CURSE OF THE WINGS", 200, 200, 30, RED);
            DrawText("Press ENTER to Start", 250, 300, 20, WHITE);
        }

        else if (state == PLAYING) {
            // Draw player rectangle
            DrawRectangleRec(player, RED);

            // Draw items
            for (auto &it : items) {
                if (!it.active) continue;

                Color col = WHITE;
                if (it.type == 0) col = GRAY;      // bomb
                if (it.type == 1) col = GREEN;     // food
                if (it.type == 2) col = BLUE;      // medkit
                if (it.type == 3) col = PURPLE;    // poison

                DrawRectangleRec(it.rect, col);
            }

            // Display HP at top-left corner
            // %d means "print integer value" inside the string
            DrawText(TextFormat("HP: %d", hp), 10, 10, 20, WHITE);
        }

        else if (state == GAME_OVER) {
            DrawText("GAME OVER", 300, 250, 40, RED);
            DrawText("Press ENTER to Restart", 230, 320, 20, WHITE);

            // Restart game if ENTER is pressed
            if (IsKeyPressed(KEY_ENTER)) {
                state = MENU;
                hp = 3;
                items.clear();     // Remove all items
                player.x = 400;    // Reset player position
                speedBoost = 1.0f; // Reset speed
            }
        }

        EndDrawing();
    }

    // Close window and clean up
    CloseWindow();
    return 0;
}
