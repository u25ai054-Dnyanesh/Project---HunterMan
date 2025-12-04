#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#define width 70
#define height 20
#define ghost_no 4

//----structure for characters----
typedef struct {
    int x, y;
    int dx, dy;     // Velocity
    char symbol;
    int speed;      // Steps per frame
} character;

// map
int map[height][width];

// make objects for maze
void make_maze(int x1, int x2, int y1, int y2) {

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            
            if (y == y1 || y == y2) {
                map[y][x] = 1;
            }
            else { 
                map[y][x] = 2;
            }
        }
    }

}

// Initialize the Map Layout
void init_map() {
    // Clear Map
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (y == 0 || y == height - 1) {
                 map[y][x] = 1;
            }
            else if (x == 0 || x == width - 1) {
                 map[y][x] = 2;
            }
            else {
                 map[y][x] = 0;
            }
        }
    }

    // I
    make_maze(9, 14, 5, 15);
    // filling the hollow gap
    for(int y=6; y<15; y++) {
        for(int x=10; x<14; x++) {
            map[y][x] = 2;
        }
    }

    // Shape 'C' (Top part)
    make_maze(28, 38, 6, 8); 
    // Shape 'C' (Bottom part)
    make_maze(28, 38, 11, 13);
    // Shape 'C' (Connector)
    make_maze(28, 28, 8, 11);

    // Shape 'P'
    make_maze(50, 57, 6, 14);
    // Clear middle of P
    for(int y=7; y<=9; y++) {
        for(int x=51; x<57; x++) { 
            map[y][x] = 0;
        }
    }
    // Clear leg of P (right side)
    for(int y=10; y<=14; y++) for(int x=51; x<57; x++) map[y][x] = 0;
}

// Check if a coordinate is valid to walk on
int is_walkable(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
         return 0;
    }
    return map[y][x] == 0;
}

// Manhattan Distance
int get_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// Spawn an character at a random valid location
void spawn_character(character *c) {
    do {
        c->x = rand() % (width - 2) + 1;
        c->y = rand() % (height - 2) + 1;
    } while (!is_walkable(c->x, c->y));
}


int main() {
    // Setup Console
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\033[?25l"); // Hide Cursor
    printf("\033[2J");   // Clear Screen

    srand(time(NULL));
    init_map();

    int score = 0;

    // Init Player
    character player;
    player.x = width/2;
    player.y = height/2;
    player.dx = 0;
    player.dy = 0;
    player.symbol = '@';
    player.speed = 2;
    
    // Init Ghosts
    character ghosts[ghost_no];
    for(int i=0; i<ghost_no; i++) {
        spawn_character(&ghosts[i]);
        ghosts[i].symbol = 'G';
        ghosts[i].dx = (rand() % 2 == 0) ? 1 : -1; // Random X direction
        ghosts[i].dy = (rand() % 2 == 0) ? 1 : -1; // Random Y direction
        ghosts[i].speed = 1;
    }

    int running = 1;
    int collision_detected = 0;

    while (running) {
        // key controller
        if (_kbhit()) {
            char key = _getch();
            if (key == 'w') { player.dy = -1; player.dx = 0; player.speed = 1; } // Vertical is slower
            if (key == 's') { player.dy =  1; player.dx = 0; player.speed = 1; }
            if (key == 'a') { player.dx = -1; player.dy = 0; player.speed = 2; } // Horizontal is faster
            if (key == 'd') { player.dx =  1; player.dy = 0; player.speed = 2; }
            if (key == 'q') running = 0;
        }

        // Player logic
        for (int i = 0; i < player.speed; i++) {
            int next_x = player.x + player.dx;
            int next_y = player.y + player.dy;

            // Wrap Around Logic
            if (next_x <= 0) {
                next_x = width - 2;
            }
            if (next_x >= width - 1) {
                next_x = 1;
            }
            if (next_y <= 0) {
                next_y = height - 2;
            }
            if (next_y >= height - 1)  {
                next_y = 1;
            }

            // Check collision with walls
            if (is_walkable(next_x, next_y)) {
                player.x = next_x;
                player.y = next_y;
            }
            
            // Check collision with Ghosts
            for(int j=0; j<ghost_no; j++) {
                if (get_distance(player.x, player.y, ghosts[j].x, ghosts[j].y) <= 1) {
                    collision_detected = 1;
                }
            }
        }

        // GHOST LOGIC
        for (int i = 0; i < ghost_no; i++) {
            int gx = ghosts[i].x + ghosts[i].dx;
            int gy = ghosts[i].y + ghosts[i].dy;

            // Bounce Logic
            int hit_wall = 0;
            if (!is_walkable(gx, ghosts[i].y)) {
                ghosts[i].dx = -ghosts[i].dx;
                hit_wall = 1;
            }
            if (!is_walkable(ghosts[i].x, gy)) {
                ghosts[i].dy = -ghosts[i].dy;
                hit_wall = 1;
            }

            // Apply movement if no bounce happened this frame, or apply bounce result
            if (!hit_wall) {
                ghosts[i].x = gx;
                ghosts[i].y = gy;
            } else {
                // Apply new direction immediately to prevent getting stuck
                ghosts[i].x += ghosts[i].dx;
                ghosts[i].y += ghosts[i].dy;
            }
        }

        score++;
        if (collision_detected) {
            // printf("\033[H\033[J"); // Clear screen
            printf("\nGAME OVER! Collision Detected. %d\n", score-1);
            break;
        }

        // RENDERING (Draw Map + Entities)
        printf("\033[H"); // Move cursor to top-left
        
        // Output buffer for single printf (Optional optimization, strictly confusing so we print char by char)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                
                // Priority 1: Ghosts
                int is_ghost = 0;
                for(int i=0; i<ghost_no; i++) {
                    if (ghosts[i].x == x && ghosts[i].y == y) {
                        printf("\033[1;31m%c\033[0m", ghosts[i].symbol); // Red Ghost
                        is_ghost = 1;
                        break;
                    }
                }
                if (is_ghost) continue;

                // Priority 2: Player
                if (player.x == x && player.y == y) {
                    printf("\033[1;32m%c\033[0m", player.symbol); // Green Player
                    continue;
                }

                // Priority 3: Map
                switch (map[y][x]) {
                    case 1: printf("-"); break;
                    case 2: printf("|"); break;
                    default:     printf(" "); break;
                }
            }
            printf("\n");
        }
        
        printf("score: %d", score);
        Sleep(30);
    }

    printf("\033[?25h"); // Show cursor
    return 0;
}