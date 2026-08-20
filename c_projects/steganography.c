#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>

// ========== BMP 文件头结构体（按字节对齐） ==========
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;        // 文件类型，必须是 "BM" (0x4D42)
    uint32_t bfSize;        // 文件大小（字节）
    uint16_t bfReserved1;   // 保留
    uint16_t bfReserved2;   // 保留
    uint32_t bfOffBits;     // 像素数据偏移量
} BMPFileHeader;

typedef struct {
    uint32_t biSize;        // 信息头大小
    int32_t  biWidth;       // 图像宽度（像素）
    int32_t  biHeight;      // 图像高度（像素，正数表示从下到上）
    uint16_t biPlanes;      // 颜色平面数，必须为1
    uint16_t biBitCount;    // 每像素位数（24 = 24位真彩色）
    uint32_t biCompression; // 压缩方式（0 = 无压缩）
    uint32_t biSizeImage;   // 图像数据大小
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

// ========== 像素数据 ==========
typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    // 注：BMP 存储顺序是 BGR，不是 RGB
} Pixel;

// ========== 工具函数：获取最低位 ==========
uint8_t getLSB(uint8_t value) {
    return value & 0x01;
}

// ========== 工具函数：设置最低位 ==========
uint8_t setLSB(uint8_t value, uint8_t bit) {
    return (value & 0xFE) | (bit & 0x01);
}

// ========== 读取 BMP 文件 ==========
int readBMP(const char *filename, BMPFileHeader *fileHeader, BMPInfoHeader *infoHeader, Pixel **pixels, uint8_t **imageData) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf(" 无法打开文件: %s\n", filename);
        return 0;
    }

    // 读取文件头
    fread(fileHeader, sizeof(BMPFileHeader), 1, fp);
    if (fileHeader->bfType != 0x4D42) {
        printf(" 不是有效的 BMP 文件 (标志位 0x%X)\n", fileHeader->bfType);
        fclose(fp);
        return 0;
    }

    // 读取信息头
    fread(infoHeader, sizeof(BMPInfoHeader), 1, fp);

    // 检查是否为 24 位无压缩 BMP
    if (infoHeader->biBitCount != 24) {
        printf(" 仅支持 24 位 BMP，当前为 %d 位\n", infoHeader->biBitCount);
        fclose(fp);
        return 0;
    }
    if (infoHeader->biCompression != 0) {
        printf(" 仅支持无压缩 BMP (压缩方式: %d)\n", infoHeader->biCompression);
        fclose(fp);
        return 0;
    }

    // 计算每行字节数（4字节对齐）
    int width = infoHeader->biWidth;
    int height = abs(infoHeader->biHeight);
    int rowSize = (width * 3 + 3) & ~3;

    // 分配像素数据内存
    int dataSize = rowSize * height;
    *imageData = (uint8_t*)malloc(dataSize);
    if (!*imageData) {
        printf(" 内存分配失败\n");
        fclose(fp);
        return 0;
    }

    // 跳转到像素数据开始位置
    fseek(fp, fileHeader->bfOffBits, SEEK_SET);
    fread(*imageData, 1, dataSize, fp);

    fclose(fp);

    // 将数据转换为 Pixel 二维数组（方便操作）
    *pixels = (Pixel*)malloc(width * height * sizeof(Pixel));
    if (!*pixels) {
        printf(" 内存分配失败\n");
        free(*imageData);
        return 0;
    }

    // BMP 存储是 BGR 顺序，从底到顶
    for (int y = 0; y < height; y++) {
        int srcY = (infoHeader->biHeight > 0) ? (height - 1 - y) : y;
        for (int x = 0; x < width; x++) {
            int srcIdx = srcY * rowSize + x * 3;
            int dstIdx = y * width + x;
            (*pixels)[dstIdx].b = (*imageData)[srcIdx + 0];
            (*pixels)[dstIdx].g = (*imageData)[srcIdx + 1];
            (*pixels)[dstIdx].r = (*imageData)[srcIdx + 2];
        }
    }

    return 1;
}

// ========== 写入 BMP 文件 ==========
int writeBMP(const char *filename, BMPFileHeader *fileHeader, BMPInfoHeader *infoHeader, Pixel *pixels, uint8_t *imageData) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf(" 无法创建文件: %s\n", filename);
        return 0;
    }

    int width = infoHeader->biWidth;
    int height = abs(infoHeader->biHeight);
    int rowSize = (width * 3 + 3) & ~3;

    // 将 Pixel 数组写回 imageData（BGR 顺序）
    for (int y = 0; y < height; y++) {
        int dstY = (infoHeader->biHeight > 0) ? (height - 1 - y) : y;
        for (int x = 0; x < width; x++) {
            int srcIdx = y * width + x;
            int dstIdx = dstY * rowSize + x * 3;
            imageData[dstIdx + 0] = pixels[srcIdx].b;
            imageData[dstIdx + 1] = pixels[srcIdx].g;
            imageData[dstIdx + 2] = pixels[srcIdx].r;
        }
    }

    // 写入文件头
    fwrite(fileHeader, sizeof(BMPFileHeader), 1, fp);
    fwrite(infoHeader, sizeof(BMPInfoHeader), 1, fp);

    // 跳转到像素数据位置
    fseek(fp, fileHeader->bfOffBits, SEEK_SET);
    fwrite(imageData, 1, infoHeader->biSizeImage, fp);

    fclose(fp);
    return 1;
}

// ========== LSB 隐写：将字符串写入像素最低位 ==========
void hideMessage(Pixel *pixels, int width, int height, const char *message) {
    int totalBits = width * height * 3;  // 每个像素有 3 个通道（R,G,B）
    int msgLen = strlen(message);
    int neededBits = (msgLen + 1) * 8;   // +1 是字符串结束符 '\0'

    if (neededBits > totalBits) {
        printf(" 图片容量不足！需要 %d 位，只有 %d 位。\n", neededBits, totalBits);
        return;
    }

    // 先写入消息长度（2 字节，16 位）
    for (int bit = 0; bit < 16; bit++) {
        int pixelIdx = bit / 3;
        int channelIdx = bit % 3;
        uint8_t bitVal = (msgLen >> bit) & 1;
        if (channelIdx == 0) pixels[pixelIdx].r = setLSB(pixels[pixelIdx].r, bitVal);
        else if (channelIdx == 1) pixels[pixelIdx].g = setLSB(pixels[pixelIdx].g, bitVal);
        else pixels[pixelIdx].b = setLSB(pixels[pixelIdx].b, bitVal);
    }

    // 再写入消息内容（每个字符 8 位）
    for (int i = 0; i <= msgLen; i++) {  // 包含 '\0'
        char ch = (i == msgLen) ? '\0' : message[i];
        int baseBit = 16 + i * 8;
        for (int bit = 0; bit < 8; bit++) {
            int pixelIdx = (baseBit + bit) / 3;
            int channelIdx = (baseBit + bit) % 3;
            uint8_t bitVal = (ch >> bit) & 1;
            if (channelIdx == 0) pixels[pixelIdx].r = setLSB(pixels[pixelIdx].r, bitVal);
            else if (channelIdx == 1) pixels[pixelIdx].g = setLSB(pixels[pixelIdx].g, bitVal);
            else pixels[pixelIdx].b = setLSB(pixels[pixelIdx].b, bitVal);
        }
    }

    printf(" 已隐藏 %d 个字符 (含结束符)\n", msgLen);
}

// ========== LSB 提取：从像素最低位读取字符串 ==========
char* extractMessage(Pixel *pixels, int width, int height) {
    int totalBits = width * height * 3;

    // 先读取消息长度（2 字节，16 位）
    int msgLen = 0;
    for (int bit = 0; bit < 16; bit++) {
        int pixelIdx = bit / 3;
        int channelIdx = bit % 3;
        uint8_t bitVal;
        if (channelIdx == 0) bitVal = getLSB(pixels[pixelIdx].r);
        else if (channelIdx == 1) bitVal = getLSB(pixels[pixelIdx].g);
        else bitVal = getLSB(pixels[pixelIdx].b);
        if (bitVal) msgLen |= (1 << bit);
    }

    if (msgLen < 0 || msgLen > 1000) {
        printf(" 消息长度异常: %d\n", msgLen);
        return NULL;
    }

    // 再读取消息内容
    char *message = (char*)malloc(msgLen + 1);
    if (!message) {
        printf(" 内存分配失败\n");
        return NULL;
    }

    for (int i = 0; i <= msgLen; i++) {
        char ch = 0;
        int baseBit = 16 + i * 8;
        for (int bit = 0; bit < 8; bit++) {
            int pixelIdx = (baseBit + bit) / 3;
            int channelIdx = (baseBit + bit) % 3;
            uint8_t bitVal;
            if (channelIdx == 0) bitVal = getLSB(pixels[pixelIdx].r);
            else if (channelIdx == 1) bitVal = getLSB(pixels[pixelIdx].g);
            else bitVal = getLSB(pixels[pixelIdx].b);
            if (bitVal) ch |= (1 << bit);
        }
        message[i] = ch;
        if (ch == '\0') break;
    }

    return message;
}

// ========== 打印 BMP 信息 ==========
void printBMPInfo(BMPFileHeader *fileHeader, BMPInfoHeader *infoHeader) {
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║              BMP 文件信息                      ║\n");
    printf("╠════════════════════════════════════════════════╣\n");
    printf("║ 文件大小    : %-10d 字节                  ║\n", fileHeader->bfSize);
    printf("║ 宽度        : %-10d 像素                  ║\n", infoHeader->biWidth);
    printf("║ 高度        : %-10d 像素                  ║\n", abs(infoHeader->biHeight));
    printf("║ 位深        : %-10d 位/像素               ║\n", infoHeader->biBitCount);
    printf("║ 数据偏移    : %-10d 字节                  ║\n", fileHeader->bfOffBits);
    printf("║ 图像数据大小: %-10d 字节                  ║\n", infoHeader->biSizeImage);
    printf("╚════════════════════════════════════════════════╝\n");
}

// ========== 主程序 ==========
int main() {
    SetConsoleOutputCP(CP_UTF8);
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║       BMP 图片 LSB 隐写工具 v1.0               ║\n");
    printf("║       网安专用 - 二进制位操作实战              ║\n");
    printf("╚════════════════════════════════════════════════╝\n\n");

    const char *inputFile = "input.bmp";
    const char *outputFile = "output_with_secret.bmp";

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    Pixel *pixels = NULL;
    uint8_t *imageData = NULL;

    // 1. 读取 BMP 文件
    printf(" 读取 BMP 文件: %s\n", inputFile);
    if (!readBMP(inputFile, &fileHeader, &infoHeader, &pixels, &imageData)) {
        printf(" 读取失败，请确保 %s 存在且是 24 位 BMP 文件。\n", inputFile);
        return 1;
    }
    printBMPInfo(&fileHeader, &infoHeader);

    // 2. 用户输入要隐藏的信息
    char secret[256];
    printf("\n 请输入要隐藏的密文: ");
    fgets(secret, sizeof(secret), stdin);
    secret[strcspn(secret, "\n")] = '\0';  // 去掉换行符

    if (strlen(secret) == 0) {
        strcpy(secret, "Hello, 网络安全!");
    }

    // 3. 隐写：把密文藏进图片
    printf("\n 正在隐藏信息...\n");
    hideMessage(pixels, infoHeader.biWidth, abs(infoHeader.biHeight), secret);

    // 4. 保存为新的 BMP
    printf(" 保存图片: %s\n", outputFile);
    if (!writeBMP(outputFile, &fileHeader, &infoHeader, pixels, imageData)) {
        printf(" 写入失败\n");
        free(pixels);
        free(imageData);
        return 1;
    }

    // 5. 提取验证：从新图片中读取隐写信息
    printf("\n 正在提取验证...\n");
    Pixel *pixels2 = NULL;
    uint8_t *imageData2 = NULL;
    if (readBMP(outputFile, &fileHeader, &infoHeader, &pixels2, &imageData2)) {
        char *extracted = extractMessage(pixels2, infoHeader.biWidth, abs(infoHeader.biHeight));
        if (extracted) {
            printf("\n 提取出的密文: %s\n", extracted);
            if (strcmp(secret, extracted) == 0) {
                printf(" 验证成功！隐写和提取完全一致！\n");
            } else {
                printf(" 警告：提取结果与原文不符！\n");
            }
            free(extracted);
        }
        free(pixels2);
        free(imageData2);
    }

    // 清理
    free(pixels);
    free(imageData);

    printf("\n按任意键退出...");
    getchar();
    return 0;
}