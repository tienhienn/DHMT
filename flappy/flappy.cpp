#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <math.h> // Them thu vien toan hoc cho sin/cos (TP3, TP4)
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// --- CAC HAM CO BAN ---
void playBackgroundMusic();
void stopBackgroundMusic();
void playJumpSound();
void playCollisionSound();
void stopMusic();
void settingMenu();

struct toado { int x, y; };
struct flap { toado td; float angle; } bird; 

// Cap nhat loai vat pham: Khien, Phong To, Thu Nho
enum ItemType { ITEM_NONE = 0, ITEM_SHIELD, ITEM_BIG, ITEM_SMALL };
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

// Trang thai Vat pham (Power-up States)
bool hasShield = false;
int bigTimer = 0;   // Thoi gian hieu luc cua Phong to
int smallTimer = 0; // Thoi gian hieu luc cua Thu nho

/* * 1. THUAT TOAN BRESENHAM VE DUONG THANG TONG QUAT */
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

/* * 2. THUAT TOAN MIDPOINT / BRESENHAM VE DUONG TRON */
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

/* * 3. THUAT TOAN TO MAU DA GIAC - BOUNDARY FILL 4 HUONG */
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

/* * HAM VE DUONG CONG KOCH BANG DE QUY */
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
    setfillstyle(1, LIGHTRED); setcolor(LIGHTRED); fillellipse(cx, cy, 42, 42);
    setfillstyle(1, YELLOW); setcolor(YELLOW); fillellipse(cx, cy, 35, 35);
    setfillstyle(1, WHITE); setcolor(WHITE); fillellipse(cx, cy, 20, 20);
}

// GUI VAT PHAM MOI
void DrawItem(int x, int y, ItemType type) {
    if (type == ITEM_SHIELD) {
        setfillstyle(1, LIGHTBLUE); setcolor(LIGHTCYAN); fillellipse(x, y, 12, 12);
        BresenhamLine(x - 6, y, x + 6, y, WHITE);
        BresenhamLine(x, y - 6, x, y + 6, WHITE);
    } else if (type == ITEM_BIG) { // Bieu tuong Phong To (Mui ten len mau do)
        setfillstyle(1, LIGHTRED); setcolor(RED); fillellipse(x, y, 12, 12);
        BresenhamLine(x, y - 6, x, y + 6, WHITE);
        BresenhamLine(x, y - 6, x - 4, y - 2, WHITE);
        BresenhamLine(x, y - 6, x + 4, y - 2, WHITE);
    } else if (type == ITEM_SMALL) { // Bieu tuong Thu nho (Mui ten xuong mau xanh)
        setfillstyle(1, LIGHTGREEN); setcolor(GREEN); fillellipse(x, y, 12, 12);
        BresenhamLine(x, y - 6, x, y + 6, WHITE);
        BresenhamLine(x, y + 6, x - 4, y + 2, WHITE);
        BresenhamLine(x, y + 6, x + 4, y + 2, WHITE);
    }
}

void DrawGround() {
    setfillstyle(1, 6); bar(0, 450, 1050, 600);
    setfillstyle(1, 10); bar(0, 450, 1050, 470);
}

void DrawPipe(int x, int y, bool isTop) {
    int w = 40, cap_w = 48, cap_h = 24;
    int offset = (cap_w - w) / 2; 
    int col_left = x, col_right = x + w;
    int cap_left = x - offset, cap_right = x + w + offset;

    if (!isTop) {
        int cap_top = y, cap_bottom = y + cap_h;
        int col_top = cap_bottom, col_bottom = 450;
        if (col_bottom > col_top) {
            setfillstyle(1, 2); bar(col_left, col_top, col_right, col_bottom);
            setfillstyle(1, 10); bar(col_left + 6, col_top, col_left + 14, col_bottom);
            setfillstyle(1, 15); bar(col_left + 8, col_top, col_left + 10, col_bottom);
            setfillstyle(1, 8); bar(col_right - 4, col_top, col_right - 1, col_bottom);
            setcolor(0); rectangle(col_left, col_top, col_right, col_bottom);
        }
        setfillstyle(1, 2); bar(cap_left, cap_top, cap_right, cap_bottom);
        setfillstyle(1, 10); bar(cap_left + 8, cap_top, cap_left + 18, cap_bottom);
        setfillstyle(1, 15); bar(cap_left + 10, cap_top, cap_left + 12, cap_bottom);
        setfillstyle(1, 8); bar(cap_right - 5, cap_top, cap_right - 1, cap_bottom);
        setcolor(0); rectangle(cap_left, cap_top, cap_right, cap_bottom);
    } else {
        int cap_top = y - cap_h, cap_bottom = y;
        int col_top = y - 400, col_bottom = cap_top;
        if (col_bottom > col_top) {
            setfillstyle(1, 2); bar(col_left, col_top, col_right, col_bottom);
            setfillstyle(1, 10); bar(col_left + 6, col_top, col_left + 14, col_bottom);
            setfillstyle(1, 15); bar(col_left + 8, col_top, col_left + 10, col_bottom);
            setfillstyle(1, 8); bar(col_right - 4, col_top, col_right - 1, col_bottom);
            setcolor(0); rectangle(col_left, col_top, col_right, col_bottom);
        }
        setfillstyle(1, 2); bar(cap_left, cap_top, cap_right, cap_bottom);
        setfillstyle(1, 10); bar(cap_left + 8, cap_top, cap_left + 18, cap_bottom);
        setfillstyle(1, 15); bar(cap_left + 10, cap_top, cap_left + 12, cap_bottom);
        setfillstyle(1, 8); bar(cap_right - 5, cap_top, cap_right - 1, cap_bottom);
        setcolor(0); rectangle(cap_left, cap_top, cap_right, cap_bottom);
    }
}

/* * PHEP QUAY (ROTATION) */
void RotatePoint(int &x, int &y, int cx, int cy, float angleDegree) {
    float rad = angleDegree * 3.14159 / 180.0;
    int tempX = x - cx;
    int tempY = y - cy;
    x = cx + tempX * cos(rad) - tempY * sin(rad);
    y = cy + tempX * sin(rad) + tempY * cos(rad);
}

/* * PHEP CO GIAN (SCALING) */
void ScalePoint(int &x, int &y, int cx, int cy, float s) {
    x = cx + (int)((x - cx) * s);
    y = cy + (int)((y - cy) * s);
}

// --- LOGIC GAME CHINH ---

void init() {
    bird.td.x = 100; bird.td.y = 250; bird.angle = 0; 
    hasShield = false;
    bigTimer = 0;
    smallTimer = 0;
    
    for (int i = 0; i < 7; i++) {
        BR.br[i].x = 900 + i * 160;
        BR.br[i].y = rand() % 200 + 240;
        
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

// KHOI PHUC LAI TAO HINH CHIM CU 
void BIRD() {
    int bx = bird.td.x, by = bird.td.y;
    float a = bird.angle;
    
    // Ty le scale phu thuoc vao viec an duoc vat pham nao
    float scale = 1.0;
    if (bigTimer > 0) scale = 1.4;       // Phong to chim gap 1.4 lan
    else if (smallTimer > 0) scale = 0.6; // Thu nho chim chi con 0.6 lan

    // Than chim
    int radius = (int)(20 * scale);
    BresenhamCircle(bx, by, radius, YELLOW); 
    BoundaryFill(bx, by, YELLOW, YELLOW);

    // Canh chim
    int w1x = bx - 15, w1y = by;
    int w2x = bx - 35, w2y = by - 10;
    int w3x = bx - 35, w3y = by + 10;
    int w4x = bx - 15, w4y = by + 5;
    ScalePoint(w1x, w1y, bx, by, scale); ScalePoint(w2x, w2y, bx, by, scale);
    ScalePoint(w3x, w3y, bx, by, scale); ScalePoint(w4x, w4y, bx, by, scale);
    RotatePoint(w1x, w1y, bx, by, a); RotatePoint(w2x, w2y, bx, by, a);
    RotatePoint(w3x, w3y, bx, by, a); RotatePoint(w4x, w4y, bx, by, a);
    BresenhamLine(w1x, w1y, w2x, w2y, LIGHTRED);
    BresenhamLine(w2x, w2y, w3x, w3y, LIGHTRED);
    BresenhamLine(w3x, w3y, w4x, w4y, LIGHTRED);
    BresenhamLine(w4x, w4y, w1x, w1y, LIGHTRED);
    int mid_wing_x = (w1x + w2x + w3x) / 3;
    int mid_wing_y = (w1y + w2y + w3y) / 3;
    BoundaryFill(mid_wing_x, mid_wing_y, LIGHTRED, LIGHTRED);

    // Duoi chim
    int t1x = bx - 20, t1y = by - 5;
    int t2x = bx - 35, t2y = by - 15;
    int t3x = bx - 35, t3y = by - 5;
    int t4x = bx - 20, t4y = by;
    ScalePoint(t1x, t1y, bx, by, scale); ScalePoint(t2x, t2y, bx, by, scale);
    ScalePoint(t3x, t3y, bx, by, scale); ScalePoint(t4x, t4y, bx, by, scale);
    RotatePoint(t1x, t1y, bx, by, a); RotatePoint(t2x, t2y, bx, by, a);
    RotatePoint(t3x, t3y, bx, by, a); RotatePoint(t4x, t4y, bx, by, a);
    BresenhamLine(t1x, t1y, t2x, t2y, LIGHTRED);
    BresenhamLine(t2x, t2y, t3x, t3y, LIGHTRED);
    BresenhamLine(t3x, t3y, t4x, t4y, LIGHTRED);
    BresenhamLine(t4x, t4y, t1x, t1y, LIGHTRED);
    int mid_tail_x = (t1x + t2x + t3x) / 3;
    int mid_tail_y = (t1y + t2y + t3y) / 3;
    BoundaryFill(mid_tail_x, mid_tail_y, LIGHTRED, LIGHTRED);

    // Mo chim
    int m1x = bx + 20, m1y = by - 5;
    int m2x = bx + 35, m2y = by;
    int m3x = bx + 20, m3y = by + 5;
    ScalePoint(m1x, m1y, bx, by, scale); ScalePoint(m2x, m2y, bx, by, scale);
    ScalePoint(m3x, m3y, bx, by, scale); 
    RotatePoint(m1x, m1y, bx, by, a); RotatePoint(m2x, m2y, bx, by, a);
    RotatePoint(m3x, m3y, bx, by, a); 
    BresenhamLine(m1x, m1y, m2x, m2y, LIGHTRED);
    BresenhamLine(m2x, m2y, m3x, m3y, LIGHTRED);
    BresenhamLine(m3x, m3y, m1x, m1y, LIGHTRED);
    int mid_beak_x = (m1x + m2x + m3x) / 3;
    int mid_beak_y = (m1y + m2y + m3y) / 3;
    BoundaryFill(mid_beak_x, mid_beak_y, LIGHTRED, LIGHTRED);

    // Mat chim 
    int eye_x = bx + 12, eye_y = by - 8;
    ScalePoint(eye_x, eye_y, bx, by, scale); 
    RotatePoint(eye_x, eye_y, bx, by, a);
    BresenhamCircle(eye_x, eye_y, (int)(8 * scale), WHITE);
    BoundaryFill(eye_x, eye_y, WHITE, WHITE);

    int pupil_x = bx + 14, pupil_y = by - 8;
    ScalePoint(pupil_x, pupil_y, bx, by, scale); 
    RotatePoint(pupil_x, pupil_y, bx, by, a);
    BresenhamCircle(pupil_x, pupil_y, (int)(3 * scale), BLACK);
    BoundaryFill(pupil_x, pupil_y, BLACK, BLACK);

    // Ve Hieu ung bao ve neu co Khien
    if (hasShield) {
        BresenhamCircle(bx, by, (int)(28 * scale), LIGHTCYAN);
        BresenhamCircle(bx, by, (int)(29 * scale), BLUE);
    }
}

void DLBIRD() {
    setfillstyle(1, 0);
    bar(bird.td.x - 60, bird.td.y - 50, bird.td.x + 60, bird.td.y + 50);
}

void control() {
    // Tru gian thoi gian vat pham dang hoat dong
    if (bigTimer > 0) bigTimer--;
    if (smallTimer > 0) smallTimer--;

    bird.td.y += gravity; 
    
    // Hieu ung quay cua chim
    if (gravity > 0) bird.angle = 15; 
    else bird.angle = -20; 

    if (kbhit()) { 
        char ch = getch(); 
        
        // Phep dieu khien toc do doc lap (TP4)
        if (ch == '0') { speed = 0; gravity = 0; } 
        else if (ch == '1') { base_speed = 3; speed = base_speed; }
        else if (ch == '2') { base_speed = 5; speed = base_speed; }
        else if (ch == '3') { base_speed = 8; speed = base_speed; }
        else if (ch == ' ') { 
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
            
            if (rand() % 100 < 15) {
                BR.items[i].type = (ItemType)(rand() % 3 + 1);
                BR.items[i].active = true;
                BR.items[i].y = BR.br[i].y - 140 + rand() % 90;
                BR.items[i].x_offset = 20;
            } else {
                BR.items[i].type = ITEM_NONE;
                BR.items[i].active = false;
            }
        }
        
        // Kiem tra cham cham vao Vat pham
        if (BR.items[i].active && BR.items[i].type != ITEM_NONE) {
            int dx = bird.td.x - (BR.br[i].x + BR.items[i].x_offset);
            int dy = bird.td.y - BR.items[i].y;
            if (dx * dx + dy * dy <= 1024) {
                BR.items[i].active = false;
                if (BR.items[i].type == ITEM_SHIELD) {
                    hasShield = true;
                } else if (BR.items[i].type == ITEM_BIG) {
                    bigTimer = 250;
                    smallTimer = 0; // Huy trang thai nho (neu co)
                } else if (BR.items[i].type == ITEM_SMALL) {
                    smallTimer = 250;
                    bigTimer = 0; // Huy trang thai to (neu co)
                }
                playJumpSound();
            }
        }
    }
    
    for (int i = 0; i < 7; i++) {
        DrawPipe(BR.br[i].x, BR.br[i].y, false);         
        DrawPipe(BR.br[i].x, BR.br[i].y - 190, true);    
        if (BR.items[i].active && BR.items[i].type != ITEM_NONE) {
            DrawItem(BR.br[i].x + BR.items[i].x_offset, BR.items[i].y, BR.items[i].type);
        }
    }
}

void gameover(int &thua) {
    // Tinh lai scale de Hitbox co gian chinh xac voi kich thuoc chim
    float scale = 1.0;
    if (bigTimer > 0) scale = 1.4;
    else if (smallTimer > 0) scale = 0.6;

    int hit_w = (int)(25 * scale);
    int hit_left = (int)(30 * scale);
    int hit_h = (int)(20 * scale);

    for (int i = 0; i < 7; i++) {
        if ((BR.br[i].x < bird.td.x + hit_w && BR.br[i].x + 40 > bird.td.x - hit_left) && 
            (bird.td.y + hit_h > BR.br[i].y || bird.td.y - hit_h < BR.br[i].y - 190)) {
            
            if (hasShield) {
                hasShield = false; 
                BR.br[i].x = -50; // Bop nat ong nuoc neu co khien
                playCollisionSound();
            } else {
                thua = 1;
                playCollisionSound();
            }
        }
    }
    
    if (bird.td.y < 0 || bird.td.y > 450) {
        if (hasShield) {
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

// ... [Cac ham saveHighScore, loadHighScore, showHighScore, settingMenu, login giu nguyen nhu cu, minh da an di de code ngan hon, ban cu copy full nhe] ...
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

void playBackgroundMusic() { 
    if (musicOn) PlaySound(TEXT("background.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP); 
}
void stopBackgroundMusic() { 
    PlaySound(NULL, NULL, 0); 
}
void playJumpSound() { 
    if (soundOn) Beep(750, 30); // Tieng bip nho, ngan (30ms) khong ngat nhac nen
}
void playCollisionSound() { 
    if (soundOn) Beep(300, 100); // Tieng thud tram (100ms) khi va cham
}

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
            
            // HUD HUONG DAN MOI Nhat
            outtextxy(10, 5, "Vat pham: Khien Xanh (Bao ve) | Mui ten Do (Phong to) | Mui ten Xanh (Thu nho)");
            setcolor(14); 
            outtextxy(10, 25, "Dieu khien: [SPACE] Nhay | [0] Dung | [1] Cham | [2] Binh Thuong | [3] Nhanh");

            // Render power-up HUD and Score
            char statusText[100] = "";
            if (hasShield) strcat(statusText, "KHIEN [BAT] ");
            if (bigTimer > 0) {
                char temp[30];
                sprintf(temp, "TO RA [%d] ", bigTimer);
                strcat(statusText, temp);
            }
            if (smallTimer > 0) {
                char temp[30];
                sprintf(temp, "NHO LAI [%d] ", smallTimer);
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
