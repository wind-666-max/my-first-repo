#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

// ========== 迷宫尺寸配置 ==========
#define ROWS 21
#define COLS 21
#define MAX_SIZE (ROWS * COLS)

// ========== 游戏地图 ==========
int map[ROWS][COLS];
int playerRow = 1;
int playerCol = 1;
const int endRow = ROWS - 2;
const int endCol = COLS - 2;
int gameOver = 0;

// ========== 计时相关 ==========
clock_t startTime = 0;      // 游戏开始时间
clock_t endTime = 0;        // 游戏结束时间
int isTiming = 0;           // 是否正在计时（0=未开始/已结束，1=进行中）

// ========== 双缓冲显示 ==========
char displayBuffer[ROWS][COLS * 2 + 1];

// ========== 数据结构1：栈（用于迷宫生成的 DFS） ==========
typedef struct {
    int x, y;
} Point;

Point stack[MAX_SIZE];
int top = -1;

void push(int x, int y) {
    if (top < MAX_SIZE - 1) {
        top++;
        stack[top].x = x;
        stack[top].y = y;
    }
}

Point pop() {
    Point p = stack[top];
    top--;
    return p;
}

int isEmpty() {
    return top == -1;
}

// ========== 数据结构2：队列（用于 BFS 寻路） ==========
typedef struct {
    int x, y;
    int prev;
} QueueNode;

QueueNode queue[MAX_SIZE];
int front = 0, rear = 0;

void enqueue(int x, int y, int prev) {
    if (rear < MAX_SIZE) {
        queue[rear].x = x;
        queue[rear].y = y;
        queue[rear].prev = prev;
        rear++;
    }
}

QueueNode dequeue() {
    return queue[front++];
}

int isQueueEmpty() {
    return front == rear;
}

// ========== 工具函数 ==========
void gotoxy(int x, int y) {
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

// ========== 格式化时间 ==========
void formatTime(clock_t elapsed, char *buffer) {
    int totalMs = elapsed * 1000 / CLOCKS_PER_SEC;
    int minutes = totalMs / 60000;
    int seconds = (totalMs % 60000) / 1000;
    int millis = totalMs % 1000;
    if (minutes > 0) {
        sprintf(buffer, "%d分%d秒", minutes, seconds);
    } else {
        sprintf(buffer, "%d.%03d秒", seconds, millis);
    }
}

// ========== 用 DFS（栈）生成迷宫 ==========
void generateMaze() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            map[i][j] = 0;
        }
    }

    int startX = 1, startY = 1;
    map[startX][startY] = 1;
    push(startX, startY);

    int dirs[4][2] = {{-2, 0}, {0, 2}, {2, 0}, {0, -2}};

    while (!isEmpty()) {
        Point cur = pop();
        int x = cur.x;
        int y = cur.y;

        for (int i = 0; i < 4; i++) {
            int r = rand() % 4;
            int tempX = dirs[i][0], tempY = dirs[i][1];
            dirs[i][0] = dirs[r][0];
            dirs[i][1] = dirs[r][1];
            dirs[r][0] = tempX;
            dirs[r][1] = tempY;
        }

        for (int i = 0; i < 4; i++) {
            int nx = x + dirs[i][0];
            int ny = y + dirs[i][1];

            if (nx > 0 && nx < ROWS - 1 && ny > 0 && ny < COLS - 1 && map[nx][ny] == 0) {
                map[nx][ny] = 1;
                map[x + dirs[i][0] / 2][y + dirs[i][1] / 2] = 1;
                push(nx, ny);
            }
        }
    }

    map[endRow][endCol] = 1;
}

// ========== 绘制迷宫（双缓冲 + 计时显示） ==========
void drawMaze() {
    // 1. 在内存中构建画面
    for (int i = 0; i < ROWS; i++) {
        int idx = 0;
        for (int j = 0; j < COLS; j++) {
            char ch;
            if (i == playerRow && j == playerCol) {
                ch = 'P';
            } else if (i == endRow && j == endCol) {
                ch = 'E';
            } else if (map[i][j] == 1) {
                ch = ' ';
            } else {
                ch = '#';
            }
            displayBuffer[i][idx++] = ch;
            displayBuffer[i][idx++] = ' ';
        }
        displayBuffer[i][idx] = '\0';
    }

    // 2. 一次性输出
    gotoxy(0, 0);
    for (int i = 0; i < ROWS; i++) {
        printf("%s\n", displayBuffer[i]);
    }

    // 3. 显示控制提示 + 计时
    printf("WASD 移动 | P 显示路径 | Q 退出\n");

    // 显示计时
    if (isTiming) {
        clock_t current = clock();
        char timeStr[32];
        formatTime(current - startTime, timeStr);
        printf("⏱ 用时: %s\n", timeStr);
    } else if (endTime > 0) {
        char timeStr[32];
        formatTime(endTime - startTime, timeStr);
        printf("⏱ 完成! 用时: %s \n", timeStr);
    } else {
        printf("⏱ 等待开始...\n");
    }
}

// ========== BFS（队列）求最短路径 ==========
void bfsShowPath() {
    front = 0;
    rear = 0;

    int visited[ROWS][COLS] = {0};
    int parentRow[ROWS][COLS];
    int parentCol[ROWS][COLS];
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            parentRow[i][j] = -1;
            parentCol[i][j] = -1;
        }
    }

    enqueue(playerRow, playerCol, -1);
    visited[playerRow][playerCol] = 1;

    int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    int found = 0;

    while (!isQueueEmpty() && !found) {
        QueueNode cur = dequeue();
        int x = cur.x, y = cur.y;

        for (int i = 0; i < 4; i++) {
            int nx = x + dirs[i][0];
            int ny = y + dirs[i][1];

            if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS &&
                map[nx][ny] == 1 && !visited[nx][ny]) {
                visited[nx][ny] = 1;
                parentRow[nx][ny] = x;
                parentCol[nx][ny] = y;
                enqueue(nx, ny, -1);

                if (nx == endRow && ny == endCol) {
                    found = 1;
                    break;
                }
            }
        }
    }

    if (found) {
        int cx = endRow, cy = endCol;
        // 保存当前画面，防止被覆盖
        int savedRow = playerRow, savedCol = playerCol;
        while (cx != savedRow || cy != savedCol) {
            if (!(cx == savedRow && cy == savedCol) &&
                !(cx == endRow && cy == endCol)) {
                gotoxy(cy * 2, cx);
                printf("*");
            }
            int pr = parentRow[cx][cy];
            int pc = parentCol[cx][cy];
            cx = pr;
            cy = pc;
        }
        gotoxy(0, ROWS + 4);
        printf(" 找到最短路径！按任意键继续...");
        getch();
        // 不刷新，继续游戏
    } else {
        gotoxy(0, ROWS + 4);
        printf(" 当前无路可走？请检查迷宫生成。");
        getch();
    }
}

// ========== 主游戏循环 ==========
int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(NULL));

    hideCursor();
    system("cls");

    printf("生成随机迷宫中...");
    generateMaze();

    // 重置游戏状态
    playerRow = 1;
    playerCol = 1;
    gameOver = 0;
    startTime = 0;
    endTime = 0;
    isTiming = 0;

    system("cls");

    while (!gameOver) {
        drawMaze();

        if (_kbhit()) {
            char ch = _getch();
            int dx = 0, dy = 0;

            // WASD 方向控制
            if (ch == 'w' || ch == 'W') { dx = -1; dy = 0; }
            else if (ch == 's' || ch == 'S') { dx = 1; dy = 0; }
            else if (ch == 'a' || ch == 'A') { dx = 0; dy = -1; }
            else if (ch == 'd' || ch == 'D') { dx = 0; dy = 1; }
            else if (ch == 'p' || ch == 'P') {
                bfsShowPath();
                continue;
            }
            else if (ch == 'q' || ch == 'Q') {
                gameOver = 1;
                continue;
            }

            // 执行移动
            if (dx != 0 || dy != 0) {
                int newRow = playerRow + dx;
                int newCol = playerCol + dy;
                if (map[newRow][newCol] == 1) {
                    // 第一次移动时开始计时
                    if (!isTiming && !gameOver) {
                        startTime = clock();
                        isTiming = 1;
                    }
                    playerRow = newRow;
                    playerCol = newCol;
                }

                // 胜利检测
                if (playerRow == endRow && playerCol == endCol) {
                    if (isTiming) {
                        endTime = clock();
                        isTiming = 0;
                    }
                    drawMaze();
                    gotoxy(0, ROWS + 4);
                    char timeStr[32];
                    formatTime(endTime - startTime, timeStr);
                    printf(" 恭喜你逃出迷宫！用时: %s\n", timeStr);
                    printf("按任意键退出...");
                    getch();
                    gameOver = 1;
                }
            }
        }
        Sleep(50);
    }

    return 0;
}