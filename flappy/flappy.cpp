#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <math.h> // Them thu vien toan hoc cho sin/cos (TP3, TP4)
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// --- C�C H�M CO B?N ---
void playBackgroundMusic();
void stopBackgroundMusic();
void playJumpSound();
void playCollisionSound();
void stopMusic();
void settingMenu();

struct toado { int x, y; };
struct flap { toado td; float angle; } bird; 

enum ItemType { ITEM_NONE = 0, ITEM_SHIELD, ITEM_SLOW, ITEM_FAST };
struct Item {
    ItemType type;
    int y;
    int x_offset;
    bool active;
};

struct Barrier { 
    toado br[10]; 
    Item items[10];
} BR;

int highScore = 0;
int currentScore = 0;
char playerName[50];
struct HighScoreEntry { char name[50]; int score; } topScores[3];
int musicOn = 1, soundOn = 1, difficulty = 2, speed = 5, gravity = 5;
int base_speed = 5, base_gravity = 5; 

// Power-up States
bool hasShield = false;
int slowTimer = 0;
int fastTimer = 0; 

// =========================================================================
// �I?M TP2: THU?T TO�N V? �U?NG TH?NG, �U?NG TR�N & T� M�U �? QUY
// =========================================================================

void BresenhamLine(int x1, int y1, int x2, int y2, int color) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        putpixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; } 
        if (e2 < dx) { err += dx; y1 += sy; }  
    }
}

void BresenhamCircle(int xc, int yc, int r, int color) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        putpixel(xc + x, yc + y, color); putpixel(xc - x, yc + y, color);
        putpixel(xc + x, yc - y, color); putpixel(xc - x, yc - y, color);
        putpixel(xc + y, yc + x, color); putpixel(xc - y, yc + x, color);
        putpixel(xc + y, yc - x, color); putpixel(xc - y, yc - x, color);
        x++;
        if (d > 0) { y--; d = d + 4 * (x - y) + 10; } 
        else { d = d + 4 * x + 6; }
    }
}

void BoundaryFill(int x, int y, int fill_color, int boundary_color) {
    int current = getpixel(x, y);
    if (current != boundary_color && current != fill_color) {
        putpixel(x, y, fill_color);
        BoundaryFill(x + 1, y, fill_color, boundary_color);
        BoundaryFill(x - 1, y, fill_color, boundary_color);
        BoundaryFill(x, y + 1, fill_color, boundary_color);
        BoundaryFill(x, y - 1, fill_color, boundary_color);
    }
}

// =========================================================================
// �I?M TP3: H�NH H?C FRACTAL 
// =========================================================================
void DrawKoch(int x1, int y1, int x2, int y2, int iter, int color) {
    float angle = 60 * 3.14159 / 180;
    if (iter == 0) {
        BresenhamLine(x1, y1, x2, y2, color);
    } else {
        int x3 = x1 + (x2 - x1) / 3;
        int y3 = y1 + (y2 - y1) / 3;
        int x4 = x1 + 2 * (x2 - x1) / 3;
        int y4 = y1 + 2 * (y2 - y1) / 3;
        
        int x_top = x3 + (x4 - x3) * cos(angle) - (y4 - y3) * sin(angle);
        int y_top = y3 + (x4 - x3) * sin(angle) + (y4 - y3) * cos(angle);

        DrawKoch(x1, y1, x3, y3, iter - 1, color);
        DrawKoch(x3, y3, x_top, y_top, iter - 1, color);
        DrawKoch(x_top, y_top, x4, y4, iter - 1, color);
        DrawKoch(x4, y4, x2, y2, iter - 1, color);
    }
}

void DrawFractalBackground() {
    DrawKoch(0, 400, 350, 400, 3, DARKGRAY);
    DrawKoch(350, 400, 700, 400, 3, DARKGRAY);
    DrawKoch(700, 400, 1050, 400, 3, DARKGRAY);
}

void DrawClouds() {
    setfillstyle(1, 15); setcolor(15);
    fillellipse(150, 100, 40, 20); fillellipse(130, 110, 30, 15); fillellipse(170, 110, 30, 15);
    fillellipse(400, 80, 50, 25); fillellipse(370, 90, 35, 20); fillellipse(430, 90, 35, 20);
    fillellipse(700, 120, 45, 20); fillellipse(670, 130, 35, 15); fillellipse(730, 130, 35, 15);
    fillellipse(900, 70, 40, 20); fillellipse(880, 80, 30, 15); fillellipse(920, 80, 30, 15);
}

void DrawSun() {
    int cx = 550, cy = 80;
    
    // 1. Draw sun rays
    int numRays = 12;
    int rInner = 48;
    int rOuter = 65;
    for (int i = 0; i < numRays; i++) {
        float angle = i * 2 * 3.14159265 / numRays;
        int x1 = cx + rInner * cos(angle);
        int y1 = cy + rInner * sin(angle);
        int x2 = cx + rOuter * cos(angle);
        int y2 = cy + rOuter * sin(angle);
        BresenhamLine(x1, y1, x2, y2, YELLOW);
    }
    
    // 2. Outer glow (LIGHTRED/Orange)
    setfillstyle(1, LIGHTRED);
    setcolor(LIGHTRED);
    fillellipse(cx, cy, 42, 42);
    
    // 3. Middle body (YELLOW)
    setfillstyle(1, YELLOW);
    setcolor(YELLOW);
    fillellipse(cx, cy, 35, 35);
    
    // 4. Inner core (WHITE glow)
    setfillstyle(1, WHITE);
    setcolor(WHITE);
    fillellipse(cx, cy, 20, 20);
}

void DrawItem(int x, int y, ItemType type) {
    if (type == ITEM_SHIELD) {
        setfillstyle(1, LIGHTBLUE);
        setcolor(LIGHTCYAN);
        fillellipse(x, y, 12, 12);
        BresenhamLine(x - 6, y, x + 6, y, WHITE);
        BresenhamLine(x, y - 6, x, y + 6, WHITE);
    } else if (type == ITEM_SLOW) {
        setfillstyle(1, LIGHTGREEN);
        setcolor(GREEN);
        fillellipse(x, y, 12, 12);
        setcolor(WHITE);
        BresenhamLine(x, y - 5, x, y + 2, WHITE);
        BresenhamLine(x, y + 2, x + 4, y + 2, WHITE);
    } else if (type == ITEM_FAST) {
        setfillstyle(1, YELLOW);
        setcolor(LIGHTRED);
        fillellipse(x, y, 12, 12);
        BresenhamLine(x - 4, y - 5, x, y, LIGHTRED);
        BresenhamLine(x, y, x - 4, y + 5, LIGHTRED);
        BresenhamLine(x, y - 5, x + 4, y, LIGHTRED);
        BresenhamLine(x + 4, y, x, y + 5, LIGHTRED);
    }
}

void DrawGround() {
    setfillstyle(1, 6); bar(0, 450, 1050, 600);
    setfillstyle(1, 10); bar(0, 450, 1050, 470);
}

void DrawPipe(int x, int y, bool isTop) {
    int w = 40;        // Width of the column
    int cap_w = 48;    // Width of the cap
    int cap_h = 24;    // Height of the cap
    int offset = (cap_w - w) / 2; // Offset for cap width centering

    // Left and right bounds
    int col_left = x;
    int col_right = x + w;
    int cap_left = x - offset;
    int cap_right = x + w + offset;

    if (!isTop) {
        // --- BOTTOM PIPE ---
        int cap_top = y;
        int cap_bottom = y + cap_h;
        int col_top = cap_bottom;
        int col_bottom = 450;

        // 1. Column (only draw if there is space)
        if (col_bottom > col_top) {
            // Base dark green
            setfillstyle(1, 2);
            bar(col_left, col_top, col_right, col_bottom);

            // Shading/Highlight: Light green band on the left-middle
            setfillstyle(1, 10);
            bar(col_left + 6, col_top, col_left + 14, col_bottom);

            // Shading/Highlight: Brightest white specular line
            setfillstyle(1, 15);
            bar(col_left + 8, col_top, col_left + 10, col_bottom);

            // Darker shadow on the right edge
            setfillstyle(1, 8); // DARKGRAY
            bar(col_right - 4, col_top, col_right - 1, col_bottom);

            // Outline the column in black
            setcolor(0);
            rectangle(col_left, col_top, col_right, col_bottom);
        }

        // 2. Draw Cap
        // Base dark green
        setfillstyle(1, 2);
        bar(cap_left, cap_top, cap_right, cap_bottom);

        // Highlight band
        setfillstyle(1, 10);
        bar(cap_left + 8, cap_top, cap_left + 18, cap_bottom);

        // Brightest white line
        setfillstyle(1, 15);
        bar(cap_left + 10, cap_top, cap_left + 12, cap_bottom);

        // Shadow on the right edge of cap
        setfillstyle(1, 8);
        bar(cap_right - 5, cap_top, cap_right - 1, cap_bottom);

        // Outline cap in black
        setcolor(0);
        rectangle(cap_left, cap_top, cap_right, cap_bottom);
    } else {
        // --- TOP PIPE ---
        int cap_top = y - cap_h;
        int cap_bottom = y;
        int col_top = y - 400;
        int col_bottom = cap_top;

        // 1. Column
        if (col_bottom > col_top) {
            // Base dark green
            setfillstyle(1, 2);
            bar(col_left, col_top, col_right, col_bottom);

            // Highlight band
            setfillstyle(1, 10);
            bar(col_left + 6, col_top, col_left + 14, col_bottom);

            // Brightest white line
            setfillstyle(1, 15);
            bar(col_left + 8, col_top, col_left + 10, col_bottom);

            // Shadow on the right edge
            setfillstyle(1, 8);
            bar(col_right - 4, col_top, col_right - 1, col_bottom);

            // Outline column
            setcolor(0);
            rectangle(col_left, col_top, col_right, col_bottom);
        }

        // 2. Draw Cap
        // Base dark green
        setfillstyle(1, 2);
        bar(cap_left, cap_top, cap_right, cap_bottom);

        // Highlight band
        setfillstyle(1, 10);
        bar(cap_left + 8, cap_top, cap_left + 18, cap_bottom);

        // Brightest white line
        setfillstyle(1, 15);
        bar(cap_left + 10, cap_top, cap_left + 12, cap_bottom);

        // Shadow on the right edge of cap
        setfillstyle(1, 8);
        bar(cap_right - 5, cap_top, cap_right - 1, cap_bottom);

        // Outline cap
        setcolor(0);
        rectangle(cap_left, cap_top, cap_right, cap_bottom);
    }
}

// =========================================================================
// �I?M TP4: PH�P BI?N �?I AFFINE
// =========================================================================
void RotatePoint(int &x, int &y, int cx, int cy, float angleDegree) {
    float rad = angleDegree * 3.14159 / 180.0;
    int tempX = x - cx;
    int tempY = y - cy;
    x = cx + tempX * cos(rad) - tempY * sin(rad);
    y = cy + tempX * sin(rad) + tempY * cos(rad);
}

// --- LOGIC GAME CH�NH ---

void init() {
    bird.td.x = 100; bird.td.y = 250; bird.angle = 0; 
    hasShield = false;
    slowTimer = 0;
    fastTimer = 0;
    
    for (int i = 0; i < 7; i++) {
        BR.br[i].x = 900 + i * 160;
        BR.br[i].y = rand() % 200 + 240;
        
        // Spawn item (First 2 columns are empty to let players adjust, 15% spawn chance in random scattered positions)
        if (i >= 2 && rand() % 100 < 15) {
            BR.items[i].type = (ItemType)(rand() % 3 + 1);
            BR.items[i].active = true;
            BR.items[i].y = BR.br[i].y - 140 + rand() % 90;
            BR.items[i].x_offset = 20;
        } else {
            BR.items[i].type = ITEM_NONE;
            BR.items[i].active = false;
            BR.items[i].y = 0;
            BR.items[i].x_offset = 0;
        }
    }
    currentScore = 0;
    base_speed = (difficulty == 1) ? 4 : (difficulty == 2) ? 5 : 7;
    base_gravity = base_speed;
    speed = base_speed; gravity = base_gravity;
}

// KH�I PH?C L?I T?O H�NH CHIM CU (�p d?ng TP2 & TP4)
void BIRD() {
    int bx = bird.td.x, by = bird.td.y;
    float a = bird.angle; 

    // 1. Th�n chim
    BresenhamCircle(bx, by, 20, YELLOW); 
    BoundaryFill(bx, by, YELLOW, YELLOW);

    // 2. C�nh chim
    int w1x = bx - 15, w1y = by;
    int w2x = bx - 35, w2y = by - 10;
    int w3x = bx - 35, w3y = by + 10;
    int w4x = bx - 15, w4y = by + 5;
    RotatePoint(w1x, w1y, bx, by, a); RotatePoint(w2x, w2y, bx, by, a);
    RotatePoint(w3x, w3y, bx, by, a); RotatePoint(w4x, w4y, bx, by, a);
    BresenhamLine(w1x, w1y, w2x, w2y, LIGHTRED);
    BresenhamLine(w2x, w2y, w3x, w3y, LIGHTRED);
    BresenhamLine(w3x, w3y, w4x, w4y, LIGHTRED);
    BresenhamLine(w4x, w4y, w1x, w1y, LIGHTRED);
    int mid_wing_x = (w1x + w2x + w3x) / 3;
    int mid_wing_y = (w1y + w2y + w3y) / 3;
    BoundaryFill(mid_wing_x, mid_wing_y, LIGHTRED, LIGHTRED);

    // 3. �u�i chim
    int t1x = bx - 20, t1y = by - 5;
    int t2x = bx - 35, t2y = by - 15;
    int t3x = bx - 35, t3y = by - 5;
    int t4x = bx - 20, t4y = by;
    RotatePoint(t1x, t1y, bx, by, a); RotatePoint(t2x, t2y, bx, by, a);
    RotatePoint(t3x, t3y, bx, by, a); RotatePoint(t4x, t4y, bx, by, a);
    BresenhamLine(t1x, t1y, t2x, t2y, LIGHTRED);
    BresenhamLine(t2x, t2y, t3x, t3y, LIGHTRED);
    BresenhamLine(t3x, t3y, t4x, t4y, LIGHTRED);
    BresenhamLine(t4x, t4y, t1x, t1y, LIGHTRED);
    int mid_tail_x = (t1x + t2x + t3x) / 3;
    int mid_tail_y = (t1y + t2y + t3y) / 3;
    BoundaryFill(mid_tail_x, mid_tail_y, LIGHTRED, LIGHTRED);

    // 4. M? chim
    int m1x = bx + 20, m1y = by - 5;
    int m2x = bx + 35, m2y = by;
    int m3x = bx + 20, m3y = by + 5;
    RotatePoint(m1x, m1y, bx, by, a); RotatePoint(m2x, m2y, bx, by, a);
    RotatePoint(m3x, m3y, bx, by, a); 
    BresenhamLine(m1x, m1y, m2x, m2y, LIGHTRED);
    BresenhamLine(m2x, m2y, m3x, m3y, LIGHTRED);
    BresenhamLine(m3x, m3y, m1x, m1y, LIGHTRED);
    int mid_beak_x = (m1x + m2x + m3x) / 3;
    int mid_beak_y = (m1y + m2y + m3y) / 3;
    BoundaryFill(mid_beak_x, mid_beak_y, LIGHTRED, LIGHTRED);

    // 5. M?t chim (Tr�ng tr?ng & tr�ng den)
    int eye_x = bx + 12, eye_y = by - 8;
    RotatePoint(eye_x, eye_y, bx, by, a);
    BresenhamCircle(eye_x, eye_y, 8, WHITE);
    BoundaryFill(eye_x, eye_y, WHITE, WHITE);

    int pupil_x = bx + 14, pupil_y = by - 8;
    RotatePoint(pupil_x, pupil_y, bx, by, a);
    BresenhamCircle(pupil_x, pupil_y, 3, BLACK);
    BoundaryFill(pupil_x, pupil_y, BLACK, BLACK);

    // 6. Draw visual effects for active power-ups
    if (hasShield) {
        BresenhamCircle(bx, by, 28, LIGHTCYAN);
        BresenhamCircle(bx, by, 29, BLUE);
    }
    if (fastTimer > 0) {
        int tx1 = bx - 30, ty1 = by;
        int tx2 = bx - 42, ty2 = by;
        RotatePoint(tx1, ty1, bx, by, a);
        RotatePoint(tx2, ty2, bx, by, a);
        
        setfillstyle(1, LIGHTRED);
        setcolor(LIGHTRED);
        fillellipse(tx1, ty1, 8, 6);
        
        setfillstyle(1, YELLOW);
        setcolor(YELLOW);
        fillellipse(tx2, ty2, 5, 4);
    }
}

void DLBIRD() {
    setfillstyle(1, 0);
    // X�a r?ng hon d? d?m b?o kh�ng d? l?i v?t c?a du�i/c�nh khi di chuy?n
    bar(bird.td.x - 40, bird.td.y - 30, bird.td.x + 40, bird.td.y + 30);
}

void control() {
    // Decrement active timers
    if (slowTimer > 0) slowTimer--;
    if (fastTimer > 0) fastTimer--;

    // Dynamic Speed & Gravity adjustment based on timers
    if (fastTimer > 0) {
        speed = base_speed * 2;
        gravity = base_gravity;
    } else if (slowTimer > 0) {
        speed = (base_speed / 2 < 2) ? 2 : base_speed / 2;
        gravity = (base_gravity / 2 < 2) ? 2 : base_gravity / 2;
    } else {
        speed = base_speed;
        gravity = base_gravity;
    }

    bird.td.y += gravity; 
    
    // Xoay chim t?nh ti?n (TP4)
    if (gravity > 0) bird.angle = 15; 
    else bird.angle = -20; 

    if (kbhit()) { 
        char ch = getch(); 
        
        if (ch == '0') { 
            speed = 0; 
            gravity = 0; 
        } 
        else { 
            bird.td.y -= 70; 
            bird.angle = -30; 
            playJumpSound();
            if (speed == 0) { speed = base_speed; gravity = base_gravity; }
        }
    }
}

void display() {
    for (int i = 0; i < 7; i++) {
        BR.br[i].x -= speed;
        if (BR.br[i].x <= -40) {
            int max_x = 0;
            for (int j = 0; j < 7; j++) if (BR.br[j].x > max_x) max_x = BR.br[j].x;
            BR.br[i].x = max_x + 160;
            BR.br[i].y = rand() % 200 + 200;
            currentScore++;
            
            // Spawn random item for reset pipe (15% spawn chance in random scattered positions)
            if (rand() % 100 < 15) {
                BR.items[i].type = (ItemType)(rand() % 3 + 1);
                BR.items[i].active = true;
                BR.items[i].y = BR.br[i].y - 140 + rand() % 90;
                BR.items[i].x_offset = 20;
            } else {
                BR.items[i].type = ITEM_NONE;
                BR.items[i].active = false;
                BR.items[i].y = 0;
                BR.items[i].x_offset = 0;
            }
        }
        
        // Item collision detection (scattered random positions, away from pipe mouths)
        if (BR.items[i].active && BR.items[i].type != ITEM_NONE) {
            int bird_x = bird.td.x;
            int bird_y = bird.td.y;
            int item_x = BR.br[i].x + BR.items[i].x_offset;
            int item_y = BR.items[i].y;
            
            int dx = bird_x - item_x;
            int dy = bird_y - item_y;
            if (dx * dx + dy * dy <= 1024) {
                BR.items[i].active = false;
                if (BR.items[i].type == ITEM_SHIELD) {
                    hasShield = true;
                } else if (BR.items[i].type == ITEM_SLOW) {
                    slowTimer = 250;
                    fastTimer = 0;
                } else if (BR.items[i].type == ITEM_FAST) {
                    fastTimer = 150;
                    slowTimer = 0;
                }
                playJumpSound();
            }
        }
    }
    
    for (int i = 0; i < 7; i++) {
        DrawPipe(BR.br[i].x, BR.br[i].y, false);         // Bottom pipe
        DrawPipe(BR.br[i].x, BR.br[i].y - 190, true);    // Top pipe
        if (BR.items[i].active && BR.items[i].type != ITEM_NONE) {
            DrawItem(BR.br[i].x + BR.items[i].x_offset, BR.items[i].y, BR.items[i].type);
        }
    }
}

void gameover(int &thua) {
    for (int i = 0; i < 7; i++) {
        if ((BR.br[i].x < bird.td.x + 25 && BR.br[i].x + 40 > bird.td.x - 30) && 
            (bird.td.y + 20 > BR.br[i].y || bird.td.y - 20 < BR.br[i].y - 190)) {
            
            if (fastTimer > 0) {
                BR.br[i].x = -50; // Destroy pipe instantly!
                playJumpSound();
            } else if (hasShield) {
                hasShield = false; // Consume shield
                BR.br[i].x = -50; // Destroy pipe instantly to prevent getting stuck
                playCollisionSound();
            } else {
                thua = 1;
                playCollisionSound();
            }
        }
    }
    
    if (bird.td.y < 0 || bird.td.y > 450) {
        if (fastTimer > 0 || hasShield) {
            if (bird.td.y < 0) bird.td.y = 10;
            if (bird.td.y > 450) bird.td.y = 440;
        } else {
            thua = 1;
            playCollisionSound();
        }
    }

    if (thua == 1 && currentScore > highScore) {
        highScore = currentScore;
    }
}

void saveHighScore(const char* name, int score) {
    FILE* file = fopen("highscore.txt", "r");
    struct HighScoreEntry scores[4]; 
    int count = 0;
    for (int i = 0; i < 4; i++) { strcpy(scores[i].name, ""); scores[i].score = 0; }
    if (file != NULL) {
        while (count < 3 && fscanf(file, "%s %d", scores[count].name, &scores[count].score) == 2) count++;
        fclose(file);
    }
    strcpy(scores[count].name, name); scores[count].score = score; count++;
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                struct HighScoreEntry temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }
    file = fopen("highscore.txt", "w");
    if (file != NULL) {
        int saveCount = (count < 3) ? count : 3;
        for (int i = 0; i < saveCount; i++) fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
        fclose(file);
    }
}

void loadHighScore() {
    FILE* file = fopen("highscore.txt", "r");
    int count = 0;
    for (int i = 0; i < 3; i++) { strcpy(topScores[i].name, ""); topScores[i].score = 0; }
    if (file != NULL) {
        while (count < 3 && fscanf(file, "%s %d", topScores[count].name, &topScores[count].score) == 2) count++;
        fclose(file);
        if (count > 0) { highScore = topScores[0].score; strcpy(playerName, topScores[0].name); }
    }
}

void showHighScore() {
    loadHighScore();
    setfillstyle(1, 0); bar(0, 0, 1050, 600);
    setbkcolor(0); settextstyle(6, 0, 5); setcolor(12);
    outtextxy(200, 100, "BANG XEP HANG");
    char scoreText[100]; int yPos = 200;
    for (int i = 0; i < 3; i++) {
        if (strlen(topScores[i].name) > 0) {
            sprintf(scoreText, "%d. %s: %d", i + 1, topScores[i].name, topScores[i].score);
            outtextxy(200, yPos, scoreText); yPos += 100;
        }
    }
    outtextxy(200, yPos, "Nhan phim bat ky de quay lai..."); getch();
}

void settingMenu() {
    int choice = 1;
    while (1) {
        setfillstyle(1, 0); bar(0, 0, 1050, 600);
        setcolor(14); setlinestyle(0, 0, 3); rectangle(250, 150, 800, 500);
        setbkcolor(0); settextstyle(6, 0, 6); setcolor(12); outtextxy(350, 170, "SETTING");
        settextstyle(6, 0, 4); setcolor(14);
        
        char musicText[30], soundText[30], diffText[40];
        sprintf(musicText, "1. Nhac nen: %s", musicOn ? "BAT" : "TAT");
        sprintf(soundText, "2. Am thanh: %s", soundOn ? "BAT" : "TAT");
        const char* diffName = (difficulty == 1) ? "De" : (difficulty == 2) ? "Vua" : "Kho";
        sprintf(diffText, "3. Do kho: %s", diffName);
        
        outtextxy(300, 250, musicText); outtextxy(300, 320, soundText);
        outtextxy(300, 390, diffText); outtextxy(300, 460, "4. Quay lai");

        setcolor(10);
        if (choice == 1) outtextxy(270, 250, "> ");
        if (choice == 2) outtextxy(270, 320, "> ");
        if (choice == 3) outtextxy(270, 390, "> ");
        if (choice == 4) outtextxy(270, 460, "> ");

        if (kbhit()) {
            char ch = getch();
            if (ch == 72) choice = (choice == 1) ? 4 : choice - 1;
            if (ch == 80) choice = (choice == 4) ? 1 : choice + 1;
            if (ch == 13) {
                if (choice == 1) { musicOn = !musicOn; musicOn ? playBackgroundMusic() : stopBackgroundMusic(); }
                if (choice == 2) soundOn = !soundOn;
                if (choice == 3) { difficulty = (difficulty % 3) + 1; }
                if (choice == 4) break;
            }
        }
        delay(80);
    }
}

void login() {
    int choice = 1;
    while (1) {
        setfillstyle(1, 0); bar(0, 0, 1050, 600);
        setcolor(14); setlinestyle(0, 0, 3); rectangle(150, 120, 900, 500); 
        setbkcolor(0); settextstyle(6, 0, 7); setcolor(12); outtextxy(230, 150, "FLAPPY BIRD");
        settextstyle(6, 0, 5); setcolor(14);
        outtextxy(300, 250, "1. New Game"); outtextxy(300, 320, "2. Ky luc"); outtextxy(300, 390, "3. Setting");

        setcolor(10);
        if (choice == 1) outtextxy(270, 250, ">");
        if (choice == 2) outtextxy(270, 320, ">");
        if (choice == 3) outtextxy(270, 390, ">");

        if (kbhit()) {
            char ch = getch();
            if (ch == 72) choice = (choice == 1) ? 3 : choice - 1;
            if (ch == 80) choice = (choice == 3) ? 1 : choice + 1;
            if (ch == 13) break;
        }
        delay(80);
    }

    if (choice == 1) {
        char name[50]; setfillstyle(1, 0); bar(0, 0, 1050, 600);
        setbkcolor(0); settextstyle(6, 0, 5); setcolor(12); outtextxy(200, 200, "Nhap ten cua ban: ");
        int i = 0;
        while (true) {
            if (kbhit()) {
                char ch = getch();
                if (ch == 13) break; 
                if (ch == 8 && i > 0) { i--; name[i] = '\0'; } 
                else if (i < 49 && ch >= 32 && ch <= 126) { name[i] = ch; i++; name[i] = '\0'; }
                setfillstyle(1, 0); bar(400, 250, 1050, 300); outtextxy(400, 250, name);
            }
        }
        strcpy(playerName, name);
    } else if (choice == 2) { showHighScore(); login(); }
    else if (choice == 3) { settingMenu(); login(); }
}

void playBackgroundMusic() { if (musicOn) PlaySound(TEXT("background.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP); }
void stopBackgroundMusic() { PlaySound(NULL, NULL, 0); }
void playJumpSound() { if (soundOn) PlaySound(TEXT("jump.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT); }
void playCollisionSound() { if (soundOn) PlaySound(TEXT("collision.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT); }

int main() {
    initwindow(1050, 600);
    playBackgroundMusic();
    int thua, page = 0;
    loadHighScore(); 
    
    while (true) {  
        login(); 
        thua = 0; init();
        
        while (thua == 0) {
            setactivepage(page);
            setvisualpage(1 - page);
            
            setfillstyle(1, 9); bar(0, 0, 1050, 600);
            
            DrawSun();
            DrawClouds();
            DrawFractalBackground();
            DrawGround();
            
            setcolor(15); settextstyle(6, 0, 1); setbkcolor(9);
            outtextxy(10, 10, "Vat pham: Khien Xanh (Bao ve) | Dong ho XanhLa (Cham) | Set Vang (Bay nhanh)");

            // Render power-up HUD and Score
            char statusText[100] = "";
            if (hasShield) strcat(statusText, "KHIEN [BAT] ");
            if (slowTimer > 0) {
                char temp[30];
                sprintf(temp, "CHAM [%d] ", slowTimer);
                strcat(statusText, temp);
            }
            if (fastTimer > 0) {
                char temp[30];
                sprintf(temp, "NHANH [%d] ", fastTimer);
                strcat(statusText, temp);
            }
            settextstyle(6, 0, 2); setcolor(14); outtextxy(10, 40, statusText);

            char scoreText[50]; sprintf(scoreText, "Diem: %d", currentScore);
            settextstyle(6, 0, 2); setcolor(15); outtextxy(850, 40, scoreText);

            control();
            BIRD();
            display();
            gameover(thua);

            delay(20); 
            page = 1 - page;
        }
        
        if (thua == 1) {
            setvisualpage(0); setactivepage(0);
            setfillstyle(1, 0); bar(0, 0, 1050, 600);
            setbkcolor(0); settextstyle(6, 0, 9); setcolor(12); outtextxy(100, 200, "GAME OVER");
            char finalScore[50]; sprintf(finalScore, "Diem: %d", currentScore);
            settextstyle(6, 0, 5); outtextxy(100, 300, finalScore);
            saveHighScore(playerName, currentScore);
            delay(2000); 
            playBackgroundMusic(); 
        }
    }
    closegraph();
    return 0;
}

