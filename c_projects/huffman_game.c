#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

// ========== 哈夫曼树节点 ==========
typedef struct HuffmanNode {
    char ch;                    // 字符
    int freq;                   // 频率
    struct HuffmanNode *left;   // 左子树
    struct HuffmanNode *right;  // 右子树
} HuffmanNode;

// ========== 哈夫曼编码表 ==========
typedef struct {
    char ch;
    char code[32];  // 存储 0/1 字符串
} HuffmanCode;

// ========== 全局变量 ==========
char *g_text = NULL;               // 随机生成的原文
int g_textLen = 0;                 // 原文长度
HuffmanNode *g_root = NULL;        // 哈夫曼树根
HuffmanCode g_codeTable[128];      // 编码表（按 ASCII 索引）
int g_codeCount = 0;               // 编码表条目数
char g_cipherText[1024] = "";      // 密文（0/1 串）
int g_level = 1;                   // 当前关卡

// ========== 工具函数 ==========
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// ========== 哈夫曼树节点操作 ==========
HuffmanNode* createNode(char ch, int freq) {
    HuffmanNode *node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    if (node == NULL) {
    // 内存分配失败，处理错误
    exit(1);
    }
    node->ch = ch;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 释放哈夫曼树
void freeTree(HuffmanNode *root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// ========== 最小堆（用于构建哈夫曼树） ==========
#define MAX_HEAP 256
HuffmanNode *heap[MAX_HEAP];
int heapSize = 0;

void heapPush(HuffmanNode *node) {
    heap[heapSize] = node;
    int i = heapSize;
    heapSize++;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap[parent]->freq <= heap[i]->freq) break;
        // 交换
        HuffmanNode *tmp = heap[parent];
        heap[parent] = heap[i];
        heap[i] = tmp;
        i = parent;
    }
}

HuffmanNode* heapPop() {
    if (heapSize == 0) return NULL;
    HuffmanNode *result = heap[0];
    heapSize--;
    heap[0] = heap[heapSize];
    int i = 0;
    while (1) {
        int left = i * 2 + 1;
        int right = i * 2 + 2;
        int smallest = i;
        if (left < heapSize && heap[left]->freq < heap[smallest]->freq)
            smallest = left;
        if (right < heapSize && heap[right]->freq < heap[smallest]->freq)
            smallest = right;
        if (smallest == i) break;
        HuffmanNode *tmp = heap[smallest];
        heap[smallest] = heap[i];
        heap[i] = tmp;
        i = smallest;
    }
    return result;
}

int heapIsEmpty() {
    return heapSize == 0;
}

// ========== 构建哈夫曼树 ==========
void buildHuffmanTree(int freq[128]) {
    heapSize = 0;
    for (int i = 0; i < 128; i++) {
        if (freq[i] > 0) {
            HuffmanNode *node = createNode((char)i, freq[i]);
            heapPush(node);
        }
    }

    while (heapSize > 1) {
        HuffmanNode *left = heapPop();
        HuffmanNode *right = heapPop();
        HuffmanNode *parent = createNode('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        heapPush(parent);
    }
    g_root = heapPop();
}

// ========== 生成哈夫曼编码 ==========
void generateCodes(HuffmanNode *root, char *path, int depth) {
    if (!root) return;
    if (root->left == NULL && root->right == NULL) {
        path[depth] = '\0';
        g_codeTable[root->ch].ch = root->ch;
        strcpy(g_codeTable[root->ch].code, path);
        g_codeCount++;
        return;
    }
    path[depth] = '0';
    generateCodes(root->left, path, depth + 1);
    path[depth] = '1';
    generateCodes(root->right, path, depth + 1);
}

// ========== 编码 ==========
void encodeText(const char *text, char *cipher) {
    cipher[0] = '\0';
    for (int i = 0; text[i] != '\0'; i++) {
        strcat(cipher, g_codeTable[(unsigned char)text[i]].code);
    }
}

// ========== 解码（用于验证答案） ==========
int decodeCipher(const char *cipher, char *output) {
    output[0] = '\0';
    HuffmanNode *current = g_root;
    for (int i = 0; cipher[i] != '\0'; i++) {
        if (cipher[i] == '0') {
            current = current->left;
        } else if (cipher[i] == '1') {
            current = current->right;
        } else {
            return 0; // 非法字符
        }
        if (!current) return 0; // 路径无效
        if (current->left == NULL && current->right == NULL) {
            // 叶子节点，输出字符
            char tmp[2] = {current->ch, '\0'};
            strcat(output, tmp);
            current = g_root;
        }
    }
    return 1;
}

// ========== 生成随机文本 ==========
char* generateRandomText(int length) {
    char *text = (char*)malloc(length + 1);
    char chars[] = "ABCDEF";
    for (int i = 0; i < length; i++) {
        text[i] = chars[rand() % 6];
    }
    text[length] = '\0';
    return text;
}

// ========== 游戏 UI ==========
void drawGame() {
    system("cls");
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║      破译密码 - 哈夫曼树解码器            ║\n");
    printf("║      关卡 %d                               ║\n", g_level);
    printf("╚═══════════════════════════════════════════╝\n\n");

    // 显示原文长度
    printf(" 原文长度: %d 字符\n", g_textLen);
    printf(" 字符频率:\n");
    int freq[128] = {0};
    for (int i = 0; i < g_textLen; i++) {
        freq[(unsigned char)g_text[i]]++;
    }
    for (int i = 0; i < 128; i++) {
        if (freq[i] > 0) {
            printf("   %c : %d", (char)i, freq[i]);
            // 显示对应编码
            if (strlen(g_codeTable[i].code) > 0) {
                printf("  -> %s", g_codeTable[i].code);
            }
            printf("\n");
        }
    }

    printf("\n 密文 (0/1 串):\n");
    // 如果密文太长，只显示前 80 个字符
    if (strlen(g_cipherText) <= 80) {
        printf("   %s\n", g_cipherText);
    } else {
        printf("   %.80s... (共 %d 位)\n", g_cipherText, (int)strlen(g_cipherText));
    }

    printf("\n 请输入你解码后的原文: ");
}

// ========== 重置游戏 ==========
void resetGame() {
    // 清空编码表
    for (int i = 0; i < 128; i++) {
        g_codeTable[i].ch = '\0';
        g_codeTable[i].code[0] = '\0';
    }
    g_codeCount = 0;

    // 释放旧树
    if (g_root) {
        freeTree(g_root);
        g_root = NULL;
    }

    // 生成随机文本
    int length = 8 + g_level * 4; // 关卡越高，文本越长
    if (g_text) free(g_text);
    g_text = generateRandomText(length);
    g_textLen = length;

    // 统计频率
    int freq[128] = {0};
    for (int i = 0; i < g_textLen; i++) {
        freq[(unsigned char)g_text[i]]++;
    }

    // 构建哈夫曼树
    buildHuffmanTree(freq);

    // 生成编码
    char path[64] = "";
    generateCodes(g_root, path, 0);

    // 编码
    encodeText(g_text, g_cipherText);
}

// ========== 主游戏循环 ==========
int main() {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(NULL));
    hideCursor();

    int score = 0;

    while (1) {
        resetGame();
        drawGame();

        char input[256] = "";
        scanf("%s", input);

        // 跳过空格
        while (getchar() != '\n');

        // 检查是否为空
        if (strlen(input) == 0) {
            printf(" 请输入内容！");
            Sleep(800);
            continue;
        }

        // 解码验证：用玩家的输入去匹配密文
        // 方法：将玩家输入的字符逐个编码，看是否等于密文
        char encodedInput[1024] = "";
        int valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (strlen(g_codeTable[(unsigned char)input[i]].code) == 0) {
                valid = 0;
                break;
            }
            strcat(encodedInput, g_codeTable[(unsigned char)input[i]].code);
        }

        if (!valid) {
            printf("\n 包含无法识别的字符！只能用 A-F。");
            Sleep(1500);
            continue;
        }

        // 比较编码结果和密文
        if (strcmp(encodedInput, g_cipherText) == 0) {
            printf("\n 解码成功！原文: %s\n", input);
            score++;
            printf(" 当前得分: %d\n", score);
            printf("\n按任意键进入下一关...");
            getch();
            g_level++;
        } else {
            printf("\n 解码失败！编码结果和密文不匹配。");
            printf("\n你输入的编码: %s", encodedInput);
            printf("\n正确的密文:   %s", g_cipherText);
            printf("\n\n按任意键重试...");
            getch();
        }
    }

    free(g_text);
    freeTree(g_root);
    return 0;
}