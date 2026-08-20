#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#define WIDTH 40
#define HEIGHT 20
#define INIT_LEN 3

int score = 0;
int gameOver = 0;
int foodX, foodY;
int dir = 1;          // 初始方向改为 1（向右）
int speed = 150;

typedef struct SnakeNode {
   short int x, y;
    struct SnakeNode *next;
} SnakeNode;

SnakeNode *head = NULL;

void gotoxy(short int x,short int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void initSnake() {
    head = (SnakeNode*)malloc(sizeof(SnakeNode));
    SnakeNode *mid = (SnakeNode*)malloc(sizeof(SnakeNode));
    SnakeNode *tail = (SnakeNode*)malloc(sizeof(SnakeNode));

    head->x = WIDTH / 2;
    head->y = HEIGHT / 2;

    mid->x = head->x - 1;
    mid->y = head->y;

    tail->x = mid->x - 1;
    tail->y = mid->y;

    head->next = mid;
    mid->next = tail;
    tail->next = NULL;
}

void freeSnake() {
    SnakeNode *current = head;
    while (current) {
        SnakeNode *temp = current;
        current = current->next;
        free(temp);
    }
    head = NULL;
}

void insertHead(int x, int y) {
    SnakeNode *newHead = (SnakeNode*)malloc(sizeof(SnakeNode));
    newHead->x = x;
    newHead->y = y;
    newHead->next = head;
    head = newHead;
}

void removeTail() {
    if (!head || !head->next) return;
    SnakeNode *current = head;
    while (current->next && current->next->next) {
        current = current->next;
    }
    free(current->next);
    current->next = NULL;
}

void generateFood() {
    int valid = 0;
    int attempts = 0;
    while (!valid && attempts < 100) {
        attempts++;
        foodX = rand() % WIDTH;
        foodY = rand() % HEIGHT;
        valid = 1;
        SnakeNode *current = head;
        while (current) {
            if (current->x == foodX && current->y == foodY) {
                valid = 0;
                break;
            }
            current = current->next;
        }
    }
    if (!valid) gameOver = 1;
}

void draw() {
    gotoxy(0, 0);

    for (int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    for (int i = 0; i < HEIGHT; i++) {
        printf("#");
        for (int j = 0; j < WIDTH; j++) {
            if (i == foodY && j == foodX) {
                printf("*");
            } else {
                SnakeNode *current = head;
                int isSnakePart = 0;
                while (current) {
                    if (current->x == j && current->y == i) {
                        printf("O");
                        isSnakePart = 1;
                        break;
                    }
                    current = current->next;
                }
                if (!isSnakePart) printf(" ");
            }
        }
        printf("#\n");
    }
    for (int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\nScore: %d  |  WASD控制  |  Q退出\n", score);
}

void moveSnake() {
    int newX = head->x;
    int newY = head->y;

    switch (dir) {
        case 0: newY--; break; // 上
        case 1: newX++; break; // 右
        case 2: newY++; break; // 下
        case 3: newX--; break; // 左
        default: break;
    }

    // 撞墙检测
    if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT) {
        gameOver = 1;
        return;
    }

    // 撞自身检测（从第二个节点开始）
    SnakeNode *current = head->next;
    while (current) {
        if (current->x == newX && current->y == newY) {
            gameOver = 1;
            return;
        }
        current = current->next;
    }

    insertHead(newX, newY);

    if (newX == foodX && newY == foodY) {
        score++;
        generateFood();
        if (score % 5 == 0 && speed > 50) {
            speed -= 10;
        }
    } else {
        removeTail();
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(NULL));

    hideCursor();
    system("cls");

    printf("欢迎来到贪吃蛇游戏！\n");
    printf("使用 WASD 控制蛇移动，按任意键开始，按 Q 键退出。\n");
    getch();

    initSnake();
    generateFood();

    while (!gameOver) {
        draw();
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'q' || ch == 'Q') {
                gameOver = 1;
            } else if (ch == 'w' || ch == 'W') {
                if (dir != 2) dir = 0;   // 上
            } else if (ch == 's' || ch == 'S') {
                if (dir != 0) dir = 2;   // 下
            } else if (ch == 'a' || ch == 'A') {
                if (dir != 1) dir = 3;   // 左
            } else if (ch == 'd' || ch == 'D') {
                if (dir != 3) dir = 1;   // 右
            }
        }
        moveSnake();
        Sleep(speed);
    }

    gotoxy(0, HEIGHT + 4);
    if (gameOver) {
        printf("游戏结束！你的得分是: %d\n", score);
    } else {
        printf("你已退出游戏。你的得分是: %d\n", score);
    }

    freeSnake();
    printf("按任意键退出...");
    getch();
    return 0;
}