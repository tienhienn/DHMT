$path = "c:\Users\admin\Downloads\flappy\flappy\flappy\flappy.cpp"
$encoding = [System.Text.Encoding]::Default
$content = [System.IO.File]::ReadAllText($path, $encoding)

# 1. Insert DrawClouds and DrawGround
$target1 = "void DrawFractalBackground() {"
$target1_full = @"
void DrawFractalBackground() {
    DrawKoch(0, 400, 350, 400, 3, DARKGRAY);
    DrawKoch(350, 400, 700, 400, 3, DARKGRAY);
    DrawKoch(700, 400, 1050, 400, 3, DARKGRAY);
}
"@
$replacement1 = $target1_full + "`r`n`r`n" + @"
void DrawClouds() {
    setfillstyle(1, 15); setcolor(15);
    fillellipse(150, 100, 40, 20); fillellipse(130, 110, 30, 15); fillellipse(170, 110, 30, 15);
    fillellipse(400, 80, 50, 25); fillellipse(370, 90, 35, 20); fillellipse(430, 90, 35, 20);
    fillellipse(700, 120, 45, 20); fillellipse(670, 130, 35, 15); fillellipse(730, 130, 35, 15);
    fillellipse(900, 70, 40, 20); fillellipse(880, 80, 30, 15); fillellipse(920, 80, 30, 15);
}

void DrawGround() {
    setfillstyle(1, 6); bar(0, 450, 1050, 600);
    setfillstyle(1, 10); bar(0, 450, 1050, 470);
}
"@

$content = $content.Replace($target1_full, $replacement1)
# if Windows LF vs CRLF
$content = $content.Replace($target1_full.Replace("`r`n", "`n"), $replacement1.Replace("`r`n", "`n"))


# 2. Modify display()
$target2 = @"
void display() {
    for (int i = 0; i < 5; i++) {
        setfillstyle(1, 0);
        bar(BR.br[i].x, BR.br[i].y, BR.br[i].x + 40, 450);
        bar(BR.br[i].x, BR.br[i].y - 190, BR.br[i].x + 40, BR.br[i].y - 400);
    }
    
    for (int i = 0; i < 5; i++) {
"@
$replacement2 = @"
void display() {
    for (int i = 0; i < 5; i++) {
"@
$content = $content.Replace($target2, $replacement2)
$content = $content.Replace($target2.Replace("`r`n", "`n"), $replacement2.Replace("`r`n", "`n"))


# 3. Modify main()
$target3 = @"
            setfillstyle(1, 0); bar(0, 0, 1050, 600);
            
            DrawFractalBackground();
            setfillstyle(1, 15); bar(0, 450, 1050, 600);
            
            setcolor(8); settextstyle(3, 0, 1); setbkcolor(0);
            outtextxy(10, 10, "Phim: 0(Dung) 1(Cham) 2(BinhThuong) 3(Nhanh)");

            setcolor(12); settextstyle(6, 0, 9); setbkcolor(15); outtextxy(200, 460, "Flappy Bird");

            DLBIRD();
"@
$replacement3 = @"
            setfillstyle(1, 9); bar(0, 0, 1050, 600);
            
            DrawClouds();
            DrawFractalBackground();
            DrawGround();
            
            setcolor(15); settextstyle(3, 0, 1); setbkcolor(9);
            outtextxy(10, 10, "Phim: 0(Dung) 1(Cham) 2(BinhThuong) 3(Nhanh)");

            setcolor(15); settextstyle(6, 0, 9); setbkcolor(6); outtextxy(200, 460, "Flappy Bird");

            // DLBIRD();
"@
$content = $content.Replace($target3, $replacement3)
$content = $content.Replace($target3.Replace("`r`n", "`n"), $replacement3.Replace("`r`n", "`n"))


# 4. Modify score background
$target4 = @"
            char scoreText[50]; sprintf(scoreText, "Diem: %d", currentScore);
            setfillstyle(1, 0); bar(800, 10, 1050, 50);
            setbkcolor(0); settextstyle(6, 0, 2); setcolor(12); outtextxy(800, 10, scoreText);
"@
$replacement4 = @"
            char scoreText[50]; sprintf(scoreText, "Diem: %d", currentScore);
            setfillstyle(1, 9); bar(800, 10, 1050, 50);
            setbkcolor(9); settextstyle(6, 0, 2); setcolor(15); outtextxy(800, 10, scoreText);
"@
$content = $content.Replace($target4, $replacement4)
$content = $content.Replace($target4.Replace("`r`n", "`n"), $replacement4.Replace("`r`n", "`n"))

[System.IO.File]::WriteAllText($path, $content, $encoding)
Write-Output "Script replaced strings"
