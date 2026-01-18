#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

// 游戏区域尺寸
#define WIDTH 10
#define HEIGHT 20

// 方块类型定义
typedef struct {
    int x, y;
    int shape[4][4];
    int color;
} Tetromino;

// 全局变量
int gameBoard[HEIGHT][WIDTH] = {0};
int score = 0;
int level = 1;
int lines = 0;
Tetromino currentPiece;
Tetromino nextPiece;
int gameOver = 0;

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
    {{4, 4, 0, 0},
     {4, 4, 0, 0},
     {0, 0, 0, 0},
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

// 颜色定义
int colors[8] = {0, 11, 12, 14, 10, 13, 9, 15};

// 函数声明
void initGame();
void createPiece(Tetromino *piece);
void rotatePiece(Tetromino *piece);
int checkCollision(int x, int y, int shape[4][4]);
void placePiece(int shape[4][4], int x, int y, int value);
void clearLines();
void drawBoard();
void drawPiece(Tetromino piece);
void gameLoop();
void handleInput();
void setCursorPosition(int x, int y);
void hideCursor();
void showStartScreen();
void showGameOverScreen();

// 主函数
int main() {
    hideCursor();
    showStartScreen();
    initGame();
    gameLoop();
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
    
    // 初始化随机数种子
    srand(time(NULL));
    
    // 创建初始方块
    createPiece(&currentPiece);
    createPiece(&nextPiece);
    
    // 设置初始位置
    currentPiece.x = WIDTH / 2 - 2;
    currentPiece.y = 0;
}

// 创建新方块
void createPiece(Tetromino *piece) {
    int shapeIndex = rand() % 7;
    
    // 复制形状
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            piece->shape[i][j] = shapes[shapeIndex][i][j];
        }
    }
    
    // 设置颜色
    piece->color = colors[shapeIndex + 1];
    
    // 初始位置
    piece->x = 0;
    piece->y = 0;
}

// 旋转方块
void rotatePiece(Tetromino *piece) {
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

// 绘制游戏板
void drawBoard() {
    // 清空屏幕
    system("cls");
    
    // 绘制标题
    setCursorPosition(15, 1);
    printf("俄罗斯方块");
    
    // 绘制分数信息
    setCursorPosition(30, 3);
    printf("分数: %d", score);
    setCursorPosition(30, 4);
    printf("行数: %d", lines);
    setCursorPosition(30, 5);
    printf("等级: %d", level);
    
    // 绘制下一个方块
    setCursorPosition(30, 7);
    printf("下一个方块:");
    for (int i = 0; i < 4; i++) {
        setCursorPosition(30, 8 + i);
        for (int j = 0; j < 4; j++) {
            if (nextPiece.shape[i][j]) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nextPiece.color);
                printf("[]");
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            } else {
                printf("  ");
            }
        }
    }
    
    // 绘制游戏区域边框
    for (int i = 0; i < HEIGHT + 2; i++) {
        setCursorPosition(2, 3 + i);
        if (i == 0 || i == HEIGHT + 1) {
            printf("+");
            for (int j = 0; j < WIDTH; j++) {
                printf("--");
            }
            printf("+");
        } else {
            printf("|");
            for (int j = 0; j < WIDTH; j++) {
                if (gameBoard[i - 1][j]) {
                    // 绘制已放置的方块
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colors[gameBoard[i - 1][j]]);
                    printf("[]");
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                } else {
                    printf("  ");
                }
            }
            printf("|");
        }
    }
    
    // 绘制当前方块
    drawPiece(currentPiece);
    
    // 绘制控制说明
    setCursorPosition(2, 26);
    printf("控制说明:");
    setCursorPosition(2, 27);
    printf("← →: 左右移动");
    setCursorPosition(2, 28);
    printf("↓: 加速下落");
    setCursorPosition(2, 29);
    printf("空格: 旋转");
    setCursorPosition(2, 30);
    printf("ESC: 退出游戏");
}

// 绘制当前方块
void drawPiece(Tetromino piece) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece.shape[i][j]) {
                int screenX = 2 + (piece.x + j) * 2 + 1;
                int screenY = 3 + piece.y + i + 1;
                
                if (screenY > 3 && screenY < 3 + HEIGHT + 1) {
                    setCursorPosition(screenX, screenY);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), piece.color);
                    printf("[]");
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                }
            }
        }
    }
}

// 游戏主循环
void gameLoop() {
    int fallDelay = 500 - (level - 1) * 50; // 下落延迟，随等级增加而减少
    int lastFallTime = GetTickCount();
    
    while (!gameOver) {
        // 处理输入
        handleInput();
        
        // 检查是否需要下落
        if (GetTickCount() - lastFallTime > fallDelay) {
            // 尝试向下移动
            if (!checkCollision(currentPiece.x, currentPiece.y + 1, currentPiece.shape)) {
                currentPiece.y++;
            } else {
                // 放置方块
                placePiece(currentPiece.shape, currentPiece.x, currentPiece.y, currentPiece.color);
                
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
        
        // 绘制游戏板
        drawBoard();
        
        // 小延迟，防止CPU占用过高
        Sleep(30);
    }
    
    // 显示游戏结束画面
    showGameOverScreen();
}

// 处理用户输入
void handleInput() {
    if (_kbhit()) {
        int key = _getch();
        
        switch (key) {
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
            case 32: // 空格
                rotatePiece(&currentPiece);
                break;
            case 80: // 下箭头
                if (!checkCollision(currentPiece.x, currentPiece.y + 1, currentPiece.shape)) {
                    currentPiece.y++;
                }
                break;
            case 27: // ESC
                gameOver = 1;
                break;
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

// 显示开始画面
void showStartScreen() {
    system("cls");
    setCursorPosition(15, 5);
    printf("俄罗斯方块");
    setCursorPosition(10, 10);
    printf("按任意键开始游戏...");
    _getch();
}

// 显示游戏结束画面
void showGameOverScreen() {
    system("cls");
    setCursorPosition(15, 5);
    printf("游戏结束!");
    setCursorPosition(15, 7);
    printf("最终分数: %d", score);
    setCursorPosition(15, 8);
    printf("消除行数: %d", lines);
    setCursorPosition(15, 9);
    printf("达到等级: %d", level);
    setCursorPosition(10, 12);
    printf("按任意键退出...");
    _getch();
}
