#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

// 游戏区域尺寸
#define WIDTH 10
#define HEIGHT 20

// 方块类型定义
typedef struct {
    int x, y;
    int shape[4][4];
    int id;     // 新增：方块类型索引（1..7）
    int color;  // 控制台颜色属性（从 colors[id] 取得）
} Tetromino;

// 全局变量
int gameBoard[HEIGHT][WIDTH] = { 0 };
int score = 0;
int level = 1;
int lines = 0;
Tetromino currentPiece;
Tetromino nextPiece;
int gameOver = 0;
int paused = 0;            // 新增：暂停标志
DWORD lastFallTime = 0;    // 新增：用于记录上次下落时间，便于暂停后重置

// 方块形状定义
int shapes[7][4][4] = {
    // I 型
    {{0, 0, 0, 0},
     {1, 1, 1, 1},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
     // J 型
     {{2, 0, 0, 0},
      {2, 2, 2, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
      // L 型
      {{0, 0, 3, 0},
       {3, 3, 3, 0},
       {0, 0, 0, 0},
       {0, 0, 0, 0}},
       // O 型
       {{0, 0, 0, 0},
        {0, 4, 4, 0},
        {0, 4, 4, 0},
        {0, 0, 0, 0}},
        // S 型
        {{0, 5, 5, 0},
         {5, 5, 0, 0},
         {0, 0, 0, 0},
         {0, 0, 0, 0}},
         // T 型
         {{0, 6, 0, 0},
          {6, 6, 6, 0},
          {0, 0, 0, 0},
          {0, 0, 0, 0}},
          // Z 型
          {{7, 7, 0, 0},
           {0, 7, 7, 0},
           {0, 0, 0, 0},
           {0, 0, 0, 0}}
};

// 颜色定义（索引 0..7，对应 gameBoard 存储的 id）
// WORD attributes 用于 WriteConsoleOutput 的属性
int colors[8] = { 0, 11, 12, 14, 10, 13, 9, 15 };

// 函数声明
void initGame();
void createPiece(Tetromino* piece);
void rotatePiece(Tetromino* piece);
int checkCollision(int x, int y, int shape[4][4]);
void placePiece(int shape[4][4], int x, int y, int value);
void clearLines();
void drawBoard();
void drawPiece(Tetromino piece); // 仍保留但不直接写屏
void gameLoop();
void handleInput();
void setCursorPosition(int x, int y);
void hideCursor();
void showStartScreen();
int showGameOverScreen(); // 修改：返回 1 表示重新开始，0 表示退出
void clearScreen(); // 备用

// 主函数
int main() {
    hideCursor();
    showStartScreen();

    while (1) {
        initGame();
        gameLoop();

        int res = showGameOverScreen();
        if (res == 1) {
            // 重新开始：继续循环，showStartScreen 不再显示，直接重置并开始新局
            continue;
        }
        else {
            break; // 退出程序
        }
    }

    return 0;
}

// 初始化游戏
void initGame() {
    // 清空游戏板
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            gameBoard[i][j] = 0;
        }
    }

    // 初始化分数和等级
    score = 0;
    lines = 0;
    level = 1;
    gameOver = 0;
    paused = 0;

    // 初始化随机数种子
    srand((unsigned)time(NULL));

    // 创建初始方块
    createPiece(&currentPiece);
    createPiece(&nextPiece);

    // 设置初始位置
    currentPiece.x = WIDTH / 2 - 2;
    currentPiece.y = 0;

    // 初始化下落计时
    lastFallTime = GetTickCount();
}

// 创建新方块
void createPiece(Tetromino* piece) {
    int shapeIndex = rand() % 7;

    // 复制形状
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            piece->shape[i][j] = shapes[shapeIndex][i][j];
        }
    }

    // 设置 id 和颜色
    piece->id = shapeIndex + 1; // 1..7
    piece->color = colors[piece->id];

    // 初始位置
    piece->x = 0;
    piece->y = 0;
}

// 旋转方块
void rotatePiece(Tetromino* piece) {
    int temp[4][4];

    // 保存当前形状
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = piece->shape[i][j];
        }
    }

    // 旋转
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            piece->shape[i][j] = temp[3 - j][i];
        }
    }

    // 检查碰撞
    if (checkCollision(piece->x, piece->y, piece->shape)) {
        // 如果碰撞，恢复原来的形状
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                piece->shape[i][j] = temp[i][j];
            }
        }
    }
}

// 检查碰撞
int checkCollision(int x, int y, int shape[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int newX = x + j;
                int newY = y + i;

                // 检查边界
                if (newX < 0 || newX >= WIDTH || newY >= HEIGHT) {
                    return 1;
                }

                // 检查是否与已放置的方块碰撞
                if (newY >= 0 && gameBoard[newY][newX]) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// 放置方块到游戏板
// value 现在应为方块 id（1..7），而不是控制台颜色
void placePiece(int shape[4][4], int x, int y, int value) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j]) {
                int boardX = x + j;
                int boardY = y + i;

                if (boardY >= 0 && boardY < HEIGHT && boardX >= 0 && boardX < WIDTH) {
                    gameBoard[boardY][boardX] = value;
                }
            }
        }
    }
}

// 消除满行
void clearLines() {
    int linesCleared = 0;

    for (int i = HEIGHT - 1; i >= 0; i--) {
        int fullLine = 1;

        // 检查是否满行
        for (int j = 0; j < WIDTH; j++) {
            if (!gameBoard[i][j]) {
                fullLine = 0;
                break;
            }
        }

        if (fullLine) {
            // 消除该行
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < WIDTH; j++) {
                    gameBoard[k][j] = gameBoard[k - 1][j];
                }
            }

            // 清空第一行
            for (int j = 0; j < WIDTH; j++) {
                gameBoard[0][j] = 0;
            }

            // 增加行数
            linesCleared++;
            i++; // 重新检查当前行
        }
    }

    // 更新分数和等级
    if (linesCleared > 0) {
        // 计算分数
        int points = linesCleared * linesCleared * 100 * level;
        score += points;
        lines += linesCleared;

        // 更新等级
        level = lines / 10 + 1;
    }
}

// 清空控制台（备用）
void clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD written;
    COORD home = { 0, 0 };

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(hConsole, ' ', cells, home, &written);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(hConsole, home);
}

// 绘制游戏板（重写：使用缓冲区并一次性写出，且居中布局，信息区显示分数/控制/暂停）
void drawBoard() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    int screenW = csbi.dwSize.X;
    int screenH = csbi.dwSize.Y;
    int bufSize = screenW * screenH;

    CHAR_INFO* buf = (CHAR_INFO*)malloc(sizeof(CHAR_INFO) * bufSize);
    if (!buf) return;

    // 填充空白（默认属性 7）
    for (int i = 0; i < bufSize; i++) {
        buf[i].Char.AsciiChar = ' ';
        buf[i].Attributes = 7;
    }

    // 帮助：在缓冲区 (x,y) 位置设置字符和属性
    auto setBufChar = [&](int x, int y, char c, WORD attr) {
        if (x < 0 || x >= screenW || y < 0 || y >= screenH) return;
        int idx = y * screenW + x;
        buf[idx].Char.AsciiChar = c;
        buf[idx].Attributes = attr;
        };

    // 写字符串到缓冲区
    auto writeStr = [&](int x, int y, const char* s, WORD attr) {
        int cx = x;
        for (const char* p = s; *p; ++p) {
            setBufChar(cx++, y, *p, attr);
        }
        };

    // 计算居中布局
    int boardW = WIDTH * 2 + 2; // 每列2字符 + 左右边框
    int boardH = HEIGHT + 2; // 顶底边框
    int infoW = 28; // 右侧信息宽度（估计）
    int gap = 4;
    int totalW = boardW + gap + infoW;
    int startX = (screenW - totalW) / 2;
    if (startX < 0) startX = 0;
    int startY = (screenH - boardH) / 2;
    if (startY < 0) startY = 0;

    int boardX = startX;
    int boardY = startY;
    int infoX = boardX + boardW + gap;
    int infoY = boardY;

    // 标题居中（相对于整个屏幕）
    const char* title = "俄罗斯方块";
    int titleLen = (int)strlen(title);
    int titleX = (screenW - titleLen) / 2;
    int titleY = (boardY - 2 >= 0) ? boardY - 2 : 0;
    writeStr(titleX, titleY, title, 7);

    // 绘制游戏区域边框和已放置方块（以 boardX,boardY 为基准）
    for (int i = 0; i < boardH; i++) {
        int y = boardY + i;
        if (i == 0 || i == boardH - 1) {
            setBufChar(boardX, y, '+', 7);
            for (int j = 0; j < WIDTH; j++) {
                setBufChar(boardX + 1 + j * 2, y, '-', 7);
                setBufChar(boardX + 2 + j * 2, y, '-', 7);
            }
            setBufChar(boardX + 1 + WIDTH * 2, y, '+', 7);
        }
        else {
            setBufChar(boardX, y, '|', 7);
            for (int j = 0; j < WIDTH; j++) {
                int boardVal = gameBoard[i - 1][j];
                int px = boardX + 1 + j * 2;
                if (boardVal) {
                    WORD attr = (boardVal >= 0 && boardVal <= 7) ? (WORD)colors[boardVal] : 7;
                    setBufChar(px, y, '[', attr);
                    setBufChar(px + 1, y, ']', attr);
                }
                else {
                    setBufChar(px, y, ' ', 7);
                    setBufChar(px + 1, y, ' ', 7);
                }
            }
            setBufChar(boardX + 1 + WIDTH * 2, y, '|', 7);
        }
    }

    // 绘制右侧信息（分数/行数/等级）
    char tmp[64];
    sprintf(tmp, "分数: %d", score);
    writeStr(infoX, infoY + 0, tmp, 7);
    sprintf(tmp, "行数: %d", lines);
    writeStr(infoX, infoY + 1, tmp, 7);
    sprintf(tmp, "等级: %d", level);
    writeStr(infoX, infoY + 2, tmp, 7);

    // 绘制下一个方块（固定在信息区）
    writeStr(infoX, infoY + 4, "下一个方块:", 7);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int px = infoX + j * 2;
            int py = infoY + 5 + i;
            if (nextPiece.shape[i][j]) {
                WORD attr = (nextPiece.id >= 0 && nextPiece.id <= 7) ? (WORD)colors[nextPiece.id] : 7;
                setBufChar(px, py, '[', attr);
                setBufChar(px + 1, py, ']', attr);
            }
            else {
                setBufChar(px, py, ' ', 7);
                setBufChar(px + 1, py, ' ', 7);
            }
        }
    }

    // 绘制控制说明（放在信息区下方）
    const char* controls[] = { "← →: 左右移动", "↓: 加速下落", "空格: 旋转", "P: 暂停/继续", "ESC: 退出游戏" };
    int controlsCount = 5;
    int ctrlStartY = infoY + 11;
    for (int i = 0; i < controlsCount; i++) {
        writeStr(infoX, ctrlStartY + i, controls[i], 7);
    }

    // 如果暂停，显示暂停提示在信息区显眼位置
    if (paused) {
        writeStr(infoX, ctrlStartY + controlsCount + 1, "=== PAUSED ===", 7);
        writeStr(infoX, ctrlStartY + controlsCount + 2, "按 P 继续", 7);
    }

    // 绘制当前方块（覆盖在缓冲上）
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece.shape[i][j]) {
                int screenX = boardX + 1 + (currentPiece.x + j) * 2;
                int screenY = boardY + 1 + currentPiece.y + i;
                if (screenY >= boardY + 1 && screenY <= boardY + boardH - 2) {
                    WORD attr = (currentPiece.id >= 0 && currentPiece.id <= 7) ? (WORD)colors[currentPiece.id] : 7;
                    setBufChar(screenX, screenY, '[', attr);
                    setBufChar(screenX + 1, screenY, ']', attr);
                }
            }
        }
    }

    // 一次性写出缓冲区到控制台
    COORD bufSizeCoord = { (SHORT)screenW, (SHORT)screenH };
    COORD bufCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (SHORT)(screenW - 1), (SHORT)(screenH - 1) };
    WriteConsoleOutputA(hConsole, buf, bufSizeCoord, bufCoord, &writeRegion);

    free(buf);
}

// 保留但不直接向控制台写入的画块绘制（若需局部绘制可复用）
void drawPiece(Tetromino piece) {
    // 此函数不再直接使用 setCursorPosition/printf。
    // drawBoard 已在缓冲中绘制当前方块。
}

// 游戏主循环
void gameLoop() {
    int fallDelay = 500 - (level - 1) * 50; // 下落延迟，随等级增加而减少
    if (fallDelay < 50) fallDelay = 50;

    while (!gameOver) {
        // 处理输入（暂停仍可响应）
        handleInput();

        // 当未暂停时推进下落逻辑
        if (!paused) {
            if (GetTickCount() - lastFallTime > (DWORD)fallDelay) {
                // 尝试向下移动
                if (!checkCollision(currentPiece.x, currentPiece.y + 1, currentPiece.shape)) {
                    currentPiece.y++;
                }
                else {
                    // 放置方块（传入 id，而不是 color）
                    placePiece(currentPiece.shape, currentPiece.x, currentPiece.y, currentPiece.id);

                    // 检查游戏是否结束
                    if (currentPiece.y < 1) {
                        gameOver = 1;
                        break;
                    }

                    // 消除满行
                    clearLines();

                    // 生成新方块
                    currentPiece = nextPiece;
                    createPiece(&nextPiece);
                    currentPiece.x = WIDTH / 2 - 2;
                    currentPiece.y = 0;
                }
                lastFallTime = GetTickCount();
            }
        }

        // 绘制游戏板（改为一次性写入缓冲，减少闪烁）
        drawBoard();

        // 小延迟，防止CPU占用过高
        Sleep(30);
    }

    // 不在此处显示结束画面，改由 main 处理以支持重开
}

// 处理用户输入
void handleInput() {
    if (_kbhit()) {
        int first = _getch();

        // 区分普通键（first != 0 && first != 224）和扩展键（first == 0或224）
        if (first == 0 || first == 224) {
            // 扩展键：读取第二个码并只按扩展键处理（避免与普通字符冲突）
            int ext = _getch();
            // 扩展键允许在暂停时仍能响应方向键以便查看（保持现有行为：在暂停时忽略移动）
            if (paused) return;

            switch (ext) {
            case 75: // 左箭头
                if (!checkCollision(currentPiece.x - 1, currentPiece.y, currentPiece.shape)) {
                    currentPiece.x--;
                }
                break;
            case 77: // 右箭头
                if (!checkCollision(currentPiece.x + 1, currentPiece.y, currentPiece.shape)) {
                    currentPiece.x++;
                }
                break;
            case 72: // 上箭头
                rotatePiece(&currentPiece);
                break;
            case 80: // 下箭头
                if (!checkCollision(currentPiece.x, currentPiece.y + 1, currentPiece.shape)) {
                    currentPiece.y++;
                }
                break;
            default:
                break;
            }
        }
        else {
            // 普通键：按字符处理（ESC、P、空格等）
            int key = first;

            // 优先处理退出
            if (key == 27) { // ESC
                gameOver = 1;
                return;
            }

            // 暂停/继续（无论是否暂停都允许 P 切换）
            if (key == 'p' || key == 'P') {
                paused = !paused;
                if (!paused) {
                    // 恢复计时，避免一次性快速下落
                    lastFallTime = GetTickCount();
                }
                return;
            }

            // 如果处于暂停状态，其他按键忽略
            if (paused) {
                return;
            }

            // 非暂停时处理游戏按键（空格: 旋转，其他普通键可扩展）
            switch (key) {
            case 32: // 空格
                rotatePiece(&currentPiece);
                break;
            default:
                break;
            }
        }
    }
}

// 设置光标位置
void setCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 隐藏光标
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// 显示开始画面（居中）
void showStartScreen() {
    clearScreen();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    int w = csbi.dwSize.X;
    int h = csbi.dwSize.Y;

    const char* title = "俄罗斯方块";
    const char* prompt = "按任意键开始游戏...";
    int tx = (w - (int)strlen(title)) / 2;
    int px = (w - (int)strlen(prompt)) / 2;
    int ty = h / 2 - 1;
    int py = h / 2 + 1;

    setCursorPosition(tx, ty);
    printf("%s", title);
    setCursorPosition(px, py);
    printf("%s", prompt);
    _getch();
}

// 显示游戏结束画面（居中），并返回是否重新开始（1=重开，0=退出）
int showGameOverScreen() {
    clearScreen();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return 0;
    int w = csbi.dwSize.X;
    int h = csbi.dwSize.Y;

    char buf[64];
    const char* over = "游戏结束!";

    int ox = (w - (int)strlen(over)) / 2;
    int oy = h / 2 - 2;
    setCursorPosition(ox, oy);
    printf("%s", over);

    sprintf(buf, "最终分数: %d", score);
    int bx = (w - (int)strlen(buf)) / 2;
    setCursorPosition(bx, oy + 2);
    printf("%s", buf);

    sprintf(buf, "消除行数: %d", lines);
    bx = (w - (int)strlen(buf)) / 2;
    setCursorPosition(bx, oy + 3);
    printf("%s", buf);

    sprintf(buf, "达到等级: %d", level);
    bx = (w - (int)strlen(buf)) / 2;
    setCursorPosition(bx, oy + 4);
    printf("%s", buf);

    const char* prompt = "按 R 重新开始，ESC 退出...";
    int px = (w - (int)strlen(prompt)) / 2;
    setCursorPosition(px, oy + 6);
    printf("%s", prompt);

    // 等待用户按键：R/r -> 重开；ESC -> 退出；其他任意键 -> 退出
    while (1) {
        int k = _getch();
        if (k == 27) { // ESC
            return 0;
        }
        if (k == 'r' || k == 'R') {
            return 1;
        }
        // 对于特殊键（0/224），读取并忽略后面的码
        if (k == 0 || k == 224) {
            (void)_getch();
            continue;
        }
        // 其他键视为退出
        return 0;
    }
}