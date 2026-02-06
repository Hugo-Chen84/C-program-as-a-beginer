#define N 8
#include <stdio.h>

int board[8][8] = { 0 };
//0 refers to none and 1 refers to queen.

void printboard() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", board[i][j]);
		}
		printf("\n");
	}
}

int checksafe(int row, int col) {
	for (int i = 0; i < row; i++) {
		if (board[i][col]) {
			return 0;
		}
	}//check col

	for (int i = row, j = col; i >= 0 && j >= 0; i--, j--) {
		if (board[i][j]==1) {
			return 0;
		}
	}//check left up

	for (int i = row, j = col; i >= 0 && j < N; i--, j++) {
		if (board[i][j]==1) {
			return 0;
		}
	}//check right up

	return 1;
}

int placequeen(int row) {
	if (row == N) {
		printboard();
		printf("\n");
		return 1;
	}
	int count = 0;
	for (int col = 0; col < N; col++) {
		if (checksafe(row, col)) {
			board[row][col] = 1;

			count += placequeen(row + 1);

			board[row][col] = 0;
		}
	}
	return count;
}

int main(void) {
	int total = placequeen(0);
	printf("Total solution: %d\n", total);
	return 0;
}

