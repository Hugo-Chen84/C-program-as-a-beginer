#include <stdio.h>
#define N 8

int queen[N]; // queen[i] = j 表示第i行皇后在第j列
int col[N] = { 0 }; // col[j] = 1 表示第j列有皇后
int leftDiag[2 * N - 1] = { 0 }; // leftDiag[i+j] = 1 表示左对角线有皇后
int rightDiag[2 * N - 1] = { 0 }; // rightDiag[i-j+N-1] = 1 表示右对角线有皇后
int count = 0; // 解的计数器

// 打印棋盘
void printSolution() {
    count++;
    printf("解 #%d:\n", count);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (queen[i] == j) {
                printf("Q ");
            }
            else {
                printf(". ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// 递归求解
void solve(int row) {
    if (row == N) {
        printSolution(); // 找到一个解
        return;
    }

    for (int c = 0; c < N; c++) {
        // 检查是否可以放置皇后
        if (!col[c] && !leftDiag[row + c] && !rightDiag[row - c + N - 1]) {
            // 放置皇后
            queen[row] = c;
            col[c] = leftDiag[row + c] = rightDiag[row - c + N - 1] = 1;

            // 递归放置下一行
            solve(row + 1);

            // 回溯
            col[c] = leftDiag[row + c] = rightDiag[row - c + N - 1] = 0;
        }
    }
}

int main() {
    solve(0);
    printf("八皇后问题共有 %d 种解法\n", count);
    return 0;
}