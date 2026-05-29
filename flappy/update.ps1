$path = "c:\Users\admin\Downloads\flappy\flappy\flappy\flappy.cpp"
$content = Get-Content $path -Raw -Encoding Default

$insertStr = @"
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

$content = $content -replace '(?s)(void DrawFractalBackground\(\) \{.*?\})', "`$1`r`n`r`n$insertStr"

$content = $content -replace '(?s)void display\(\) \{\s*for \(int i = 0; i < 5; i\+\+\) \{\s*setfillstyle\(1, 0\);\s*bar[^}]+\}\s*for', "void display() {`r`n    for"

$oldMain = '            setfillstyle\(1, 0\); bar\(0, 0, 1050, 600\);\s+DrawFractalBackground\(\);\s+setfillstyle\(1, 15\); bar\(0, 450, 1050, 600\);\s+setcolor\(8\); settextstyle\(3, 0, 1\); setbkcolor\(0\);\s+outtextxy\(10, 10, "Phim: 0\(Dung\) 1\(Cham\) 2\(BinhThuong\) 3\(Nhanh\)"\);\s+setcolor\(12\); settextstyle\(6, 0, 9\); setbkcolor\(15\); outtextxy\(200, 460, "Flappy Bird"\);\s+DLBIRD\(\);'
$newMain = @"
            setfillstyle(1, 9); bar(0, 0, 1050, 600);
            
            DrawClouds();
            DrawFractalBackground();
            DrawGround();
            
            setcolor(15); settextstyle(3, 0, 1); setbkcolor(9);
            outtextxy(10, 10, "Phim: 0(Dung) 1(Cham) 2(BinhThuong) 3(Nhanh)");

            setcolor(15); settextstyle(6, 0, 9); setbkcolor(6); outtextxy(200, 460, "Flappy Bird");

            // DLBIRD();
"@
$content = $content -replace $oldMain, $newMain

$oldScore = 'char scoreText\[50\]; sprintf\(scoreText, "Diem: %d", currentScore\);\s+setfillstyle\(1, 0\); bar\(800, 10, 1050, 50\);\s+setbkcolor\(0\); settextstyle\(6, 0, 2\); setcolor\(12\); outtextxy\(800, 10, scoreText\);'
$newScore = @"
            char scoreText[50]; sprintf(scoreText, "Diem: %d", currentScore);
            setfillstyle(1, 9); bar(800, 10, 1050, 50);
            setbkcolor(9); settextstyle(6, 0, 2); setcolor(15); outtextxy(800, 10, scoreText);
"@
$content = $content -replace $oldScore, $newScore

Set-Content -Path $path -Value $content -Encoding Default
Write-Output "Script finished"
