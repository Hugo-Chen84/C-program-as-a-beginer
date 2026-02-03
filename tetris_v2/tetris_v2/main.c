#include <windows.h>
#include <stdio.h>
#include <time.h>

// 游戏常量
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define CELL_SIZE 30
#define PREVIEW_SIZE 4
#define TIMER_ID 1

// 窗口大小
#define WINDOW_WIDTH (BOARD_WIDTH * CELL_SIZE + 200)
#define WINDOW_HEIGHT (BOARD_HEIGHT * CELL_SIZE + 60)

// 颜色定义
const COLORREF COLORS[] = {
    RGB(0, 0, 0),       // 0: 空白 (黑色)
    RGB(0, 255, 255),   // 1: I (青色)
    RGB(0, 0, 255),     // 2: J (蓝色)
    RGB(255, 165, 0),   // 3: L (橙色)
    RGB(255, 255, 0),   // 4: O (黄色)
    RGB(0, 255, 0),     // 5: S (绿色)
    RGB(128, 0, 128),   // 6: T (紫色)
    RGB(255, 0, 0)      // 7: Z (红色)
};

// 方块形状定义 (7种方块，每种4个旋转状态，每个状态4个格子)
// 坐标基于 4x4 网格
const int SHAPES[7][4][4] = {
    // I
    {
        {1, 5, 9, 13}, {4, 5, 6, 7}, {1, 5, 9, 13}, {4, 5, 6, 7}
    },
    // J
    {
        {1, 2, 5, 9}, {0, 4, 5, 6}, {1, 5, 9, 8}, {4, 5, 6, 10}
    },
    // L
    {
        {1, 2, 6, 10}, {5, 6, 7, 9}, {2, 6, 10, 11}, {3, 5, 6, 7}
    },
    // O
    {
        {1, 2, 5, 6}, {1, 2, 5, 6}, {1, 2, 5, 6}, {1, 2, 5, 6}
    },
    // S
    {
        {2, 6, 5, 9}, {4, 5, 1, 2}, {2, 6, 5, 9}, {4, 5, 1, 2}
    },
    // T
    {
        {1, 4, 5, 6}, {1, 5, 9, 6}, {4, 5, 6, 9}, {1, 5, 9, 4}
    },
    // Z
    {
        {1, 5, 6, 10}, {1, 2, 5, 6}, {1, 5, 6, 10}, {1, 2, 5, 6}
    }
};

// 全局变量
int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};
int currentX, currentY;
int currentType, currentRotation;
int nextType;
int score = 0;
int gameOver = 0;
int isPaused = 0;

// 函数声明
void InitGame();
void DrawGame(HDC hdc);
void DrawCell(HDC hdc, int x, int y, int colorIdx);
void NewPiece();
int CheckCollision(int x, int y, int type, int rotation);
void LockPiece();
void ClearLines();
void RotatePiece();
void MovePiece(int dx, int dy);
void DrawBoard(HDC hdc);
void DrawPiece(HDC hdc, int x, int y, int type, int rotation);
void DrawNextPiece(HDC hdc);
void DrawScore(HDC hdc);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    srand((unsigned int)time(NULL));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "TetrisWindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 计算窗口实际大小以适应客户区
    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "TetrisWindowClass",
        "Tetris - C Win32 API",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    InitGame();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 设置定时器，每500ms触发一次
    SetTimer(hwnd, TIMER_ID, 500, NULL);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // 双缓冲绘图防止闪烁
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // 填充背景
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            FillRect(memDC, &clientRect, (HBRUSH)GetStockObject(WHITE_BRUSH));

            DrawGame(memDC);

            BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_TIMER: {
            if (!gameOver && !isPaused) {
                MovePiece(0, 1);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_KEYDOWN: {
            if (gameOver) {
                if (wParam == VK_RETURN) { // Enter 重开
                    InitGame();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                break;
            }

            switch (wParam) {
                case VK_LEFT:
                    if (!isPaused) MovePiece(-1, 0);
                    break;
                case VK_RIGHT:
                    if (!isPaused) MovePiece(1, 0);
                    break;
                case VK_DOWN:
                    if (!isPaused) MovePiece(0, 1);
                    break;
                case VK_UP:
                    if (!isPaused) RotatePiece();
                    break;
                case 'P':
                    isPaused = !isPaused;
                    break;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void InitGame() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }
    score = 0;
    gameOver = 0;
    isPaused = 0;
    nextType = rand() % 7;
    NewPiece();
}

void NewPiece() {
    currentType = nextType;
    nextType = rand() % 7;
    currentRotation = 0;
    currentX = BOARD_WIDTH / 2 - 2;
    currentY = 0;

    if (CheckCollision(currentX, currentY, currentType, currentRotation)) {
        gameOver = 1;
    }
}

// 检查碰撞
// 返回1表示碰撞，0表示无碰撞
int CheckCollision(int x, int y, int type, int rotation) {
    for (int i = 0; i < 4; i++) {
        int index = SHAPES[type][rotation][i];
        int r = index / 4;
        int c = index % 4;
        int worldY = y + r;
        int worldX = x + c;

        // 忽略空行/列的优化这里未做，直接判断
        // 注意：SHAPES定义的4x4矩阵中有些是空的，我们只关心非空的格子
        // 但这里SHAPES存储的是索引，说明这4个位置是有方块的
        
        if (worldX < 0 || worldX >= BOARD_WIDTH || worldY >= BOARD_HEIGHT) {
            return 1;
        }
        if (worldY >= 0 && board[worldY][worldX] != 0) {
            return 1;
        }
    }
    return 0;
}

void MovePiece(int dx, int dy) {
    if (!CheckCollision(currentX + dx, currentY + dy, currentType, currentRotation)) {
        currentX += dx;
        currentY += dy;
    } else {
        if (dy > 0) { // 向下移动碰到东西，锁定
            LockPiece();
            ClearLines();
            NewPiece();
        }
    }
}

void RotatePiece() {
    int newRotation = (currentRotation + 1) % 4;
    if (!CheckCollision(currentX, currentY, currentType, newRotation)) {
        currentRotation = newRotation;
    } else {
        // 简单的踢墙测试 (Wall Kick) - 尝试左右移动一格
        if (!CheckCollision(currentX - 1, currentY, currentType, newRotation)) {
            currentX -= 1;
            currentRotation = newRotation;
        } else if (!CheckCollision(currentX + 1, currentY, currentType, newRotation)) {
            currentX += 1;
            currentRotation = newRotation;
        }
    }
}

void LockPiece() {
    for (int i = 0; i < 4; i++) {
        int index = SHAPES[currentType][currentRotation][i];
        int r = index / 4;
        int c = index % 4;
        int worldY = currentY + r;
        int worldX = currentX + c;

        if (worldY >= 0 && worldY < BOARD_HEIGHT && worldX >= 0 && worldX < BOARD_WIDTH) {
            board[worldY][worldX] = currentType + 1; // +1 因为0是背景
        }
    }
}

void ClearLines() {
    int linesCleared = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        int full = 1;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                full = 0;
                break;
            }
        }

        if (full) {
            linesCleared++;
            // 移动上面的行下来
            for (int k = y; k > 0; k--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board[k][x] = board[k - 1][x];
                }
            }
            // 清空第一行
            for (int x = 0; x < BOARD_WIDTH; x++) {
                board[0][x] = 0;
            }
            y++; // 重新检查这一行，因为上面的移下来了
        }
    }
    if (linesCleared > 0) {
        score += linesCleared * 100; // 简单计分
    }
}

void DrawCell(HDC hdc, int x, int y, int colorIdx) {
    HBRUSH hBrush = CreateSolidBrush(COLORS[colorIdx]);
    RECT rect;
    rect.left = x * CELL_SIZE + 20; // 偏移量
    rect.top = y * CELL_SIZE + 20;
    rect.right = rect.left + CELL_SIZE;
    rect.bottom = rect.top + CELL_SIZE;

    FillRect(hdc, &rect, hBrush);
    
    // 绘制网格线
    FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    DeleteObject(hBrush);
}

void DrawGame(HDC hdc) {
    // 绘制游戏区域边框
    RECT border;
    border.left = 19;
    border.top = 19;
    border.right = 20 + BOARD_WIDTH * CELL_SIZE + 1;
    border.bottom = 20 + BOARD_HEIGHT * CELL_SIZE + 1;
    FrameRect(hdc, &border, (HBRUSH)GetStockObject(BLACK_BRUSH));

    DrawBoard(hdc);
    if (!gameOver) {
        DrawPiece(hdc, currentX, currentY, currentType, currentRotation);
    }

    DrawNextPiece(hdc);
    DrawScore(hdc);

    if (gameOver) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 0, 0));
        TextOut(hdc, WINDOW_WIDTH / 2 - 40, WINDOW_HEIGHT / 2, "GAME OVER", 9);
        TextOut(hdc, WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 + 20, "Press ENTER", 11);
    } else if (isPaused) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 255));
        TextOut(hdc, WINDOW_WIDTH / 2 - 30, WINDOW_HEIGHT / 2, "PAUSED", 6);
    }
}

void DrawBoard(HDC hdc) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] != 0) {
                DrawCell(hdc, x, y, board[y][x]);
            }
        }
    }
}

void DrawPiece(HDC hdc, int x, int y, int type, int rotation) {
    for (int i = 0; i < 4; i++) {
        int index = SHAPES[type][rotation][i];
        int r = index / 4;
        int c = index % 4;
        int worldY = y + r;
        int worldX = x + c;

        if (worldY >= 0) { // 顶部可能在屏幕外
            DrawCell(hdc, worldX, worldY, type + 1);
        }
    }
}

void DrawNextPiece(HDC hdc) {
    int startX = BOARD_WIDTH * CELL_SIZE + 50;
    int startY = 50;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    TextOut(hdc, startX, 20, "NEXT:", 5);

    for (int i = 0; i < 4; i++) {
        int index = SHAPES[nextType][0][i];
        int r = index / 4;
        int c = index % 4;
        
        HBRUSH hBrush = CreateSolidBrush(COLORS[nextType + 1]);
        RECT rect;
        rect.left = startX + c * CELL_SIZE;
        rect.top = startY + r * CELL_SIZE;
        rect.right = rect.left + CELL_SIZE;
        rect.bottom = rect.top + CELL_SIZE;
        
        FillRect(hdc, &rect, hBrush);
        FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        DeleteObject(hBrush);
    }
}

void DrawScore(HDC hdc) {
    int startX = BOARD_WIDTH * CELL_SIZE + 50;
    int startY = 200;
    
    char scoreText[32];
    sprintf(scoreText, "Score: %d", score);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    TextOut(hdc, startX, startY, scoreText, strlen(scoreText));

    TextOut(hdc, startX, startY + 50, "Controls:", 9);
    TextOut(hdc, startX, startY + 70, "Arrows: Move/Rotate", 19);
    TextOut(hdc, startX, startY + 90, "P: Pause", 8);
}
