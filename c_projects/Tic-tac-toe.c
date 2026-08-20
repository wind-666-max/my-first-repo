#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char board[3][3];
char currentPlayer = 'X';
int gameMode;          // 0=双人, 1=人机

// ========== 棋盘操作 ==========
void resetBoard() {
    char init[3][3] = {
        {'1','2','3'},
        {'4','5','6'},
        {'7','8','9'}
    };
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = init[i][j];
    currentPlayer = 'X';
}

void drawBoard() {
    system("cls");
    printf("\n  井字棋 （按数字键落子）\n\n");
    for (int i = 0; i < 3; i++) {
        printf("     %c | %c | %c\n", board[i][0], board[i][1], board[i][2]);
        if (i < 2) printf("    ---|---|---\n");
    }
    printf("\n  当前回合：%c\n", currentPlayer);
}

char checkWin() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return board[0][i];
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];
    return 0;
}

int isBoardFull() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] != 'X' && board[i][j] != 'O')
                return 0;
    return 1;
}

int isValidMove(int row, int col) {
    if (row < 0 || row > 2 || col < 0 || col > 2) return 0;
    if (board[row][col] == 'X' || board[row][col] == 'O') return 0;
    return 1;
}

void clearInput() {
    while (getchar() != '\n');
}

// ========== AI（Minimax 困难） ==========
int evaluate() {
    char w = checkWin();
    if (w == 'O') return 10;
    if (w == 'X') return -10;
    return 0;
}

int minimax(int depth, int isMax) {
    int score = evaluate();
    if (score == 10) return score - depth;
    if (score == -10) return score + depth;
    if (isBoardFull()) return 0;

    if (isMax) {
        int best = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (isValidMove(i, j)) {
                    char temp = board[i][j];
                    board[i][j] = 'O';
                    int val = minimax(depth + 1, 0);
                    board[i][j] = temp;
                    if (val > best) best = val;
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (isValidMove(i, j)) {
                    char temp = board[i][j];
                    board[i][j] = 'X';
                    int val = minimax(depth + 1, 1);
                    board[i][j] = temp;
                    if (val < best) best = val;
                }
            }
        }
        return best;
    }
}

void computerMove() {
    int bestVal = -1000;
    int bestRow = -1, bestCol = -1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (isValidMove(i, j)) {
                char temp = board[i][j];
                board[i][j] = 'O';
                int moveVal = minimax(0, 0);
                board[i][j] = temp;
                if (moveVal > bestVal) {
                    bestVal = moveVal;
                    bestRow = i;
                    bestCol = j;
                }
            }
        }
    }
    if (bestRow != -1 && bestCol != -1)
        board[bestRow][bestCol] = 'O';
}

// ========== 主函数 ==========
int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(NULL));

    printf("  欢迎来到井字棋！\n");
    printf("  请选择游戏模式：\n");
    printf("    1. 双人对战\n");
    printf("    2. 人机对战（你执 X，电脑执 O）\n");
    printf("  输入数字: ");
    int c = getchar();
    clearInput();
    if (c == '1') gameMode = 0;
    else if (c == '2') gameMode = 1;
    else {
        printf("无效输入，程序退出。\n");
        return 0;
    }
    int winX = 0, winO = 0, draw = 0;
    char playAgain;

    do {
        resetBoard();
        int gameOver = 0;
        int turn = 0;

        while (turn < 9 && !gameOver) {
            drawBoard();

            // 人机模式下电脑走棋（O）
            if (gameMode == 1 && currentPlayer == 'O') {
                printf("\n  电脑思考中...\n");
                Sleep(500);
                computerMove();
                char winner = checkWin();
                if (winner) {
                    drawBoard();
                    printf("\n  电脑 (O) 赢了！\n");
                    winO++;
                    gameOver = 1;
                    break;
                } else if (isBoardFull()) {
                    drawBoard();
                    printf("\n  平局！\n");
                    draw++;
                    gameOver = 1;
                    break;
                }
                currentPlayer = 'X';
                turn++;
                continue;
            }

            // 玩家输入
            int choice;
            printf("  请选择位置 (1-9): ");
            if (scanf("%d", &choice) != 1) {
                clearInput();
                printf("  输入无效，请重新输入数字。\n");
                continue;
            }
            clearInput();

            int pos = choice - 1;
            int row = pos / 3, col = pos % 3;

            if (pos < 0 || pos > 8) {
                printf("  位置超出范围，请重新输入。\n");
                continue;
            }

            if (isValidMove(row, col)) {
                board[row][col] = currentPlayer;
                char winner = checkWin();
                if (winner) {
                    drawBoard();
                    if (gameMode == 1 && winner == 'O') {
                        printf("\n  电脑 (O) 赢了！\n");
                        winO++;
                    } else if (gameMode == 1 && winner == 'X') {
                        printf("\n  你 (X) 赢了！\n");
                        winX++;
                    } else {
                        printf("\n  玩家 %c 赢了！\n", winner);
                        if (winner == 'X') winX++;
                        else winO++;
                    }
                    gameOver = 1;
                } else if (isBoardFull()) {
                    drawBoard();
                    printf("\n  平局！\n");
                    draw++;
                    gameOver = 1;
                } else {
                    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
                }
                turn++;
            } else {
                printf("  该位置已被占用，请重新选择。\n");
            }
        }

        // 显示当前比分
        printf("\n  ========== 当前比分 ==========\n");
        if (gameMode == 0) {
            printf("  玩家 X : 玩家 O : 平局\n");
            printf("    %d     :   %d    :  %d\n", winX, winO, draw);
        } else {
            printf("  你 (X) : 电脑 (O) : 平局\n");
            printf("    %d     :   %d    :  %d\n", winX, winO, draw);
        }
        printf("  ===============================\n");

        printf("  是否继续下一局？ (y/n): ");
        scanf(" %c", &playAgain);
        clearInput();

    } while (playAgain == 'y' || playAgain == 'Y');

    // 最终比分
    printf("\n  ========== 最终比分 ==========\n");
    if (gameMode == 0) {
        printf("  玩家 X 胜: %d\n", winX);
        printf("  玩家 O 胜: %d\n", winO);
    } else {
        printf("  你 (X) 胜: %d\n", winX);
        printf("  电脑 (O) 胜: %d\n", winO);
    }
    printf("  平局    : %d\n", draw);
    printf("  总对局  : %d\n", winX + winO + draw);
    printf("  ===============================\n");
    printf("  感谢游玩！\n");

    return 0;
}