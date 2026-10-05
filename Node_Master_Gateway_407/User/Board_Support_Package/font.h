#ifndef __FONT_H__
#define __FONT_H__
#include <stdint.h>

//注意此字符库全部用的是utf-8取模   阴码 + 逐行式 + 顺向


typedef struct {
    char Index[4];     // 存储汉字字符串 (UTF-8通常占3个字节，加上结束符'\0'需要4个字节)
    uint8_t Msk[32];   // 存储 16x16 的图像数据 (占用32字节)
} ChineseFont_t;

// 【修改重点】：.h 文件中只能有声明 (extern)，绝对不能有定义
extern const uint8_t ascii_font[95][16];




// 【修改重点】：将 RAM 变量改为宏定义常量。
// 以后加了几个字，就把这里的数字改成几。不占用单片机 RAM，且查找速度最快。
#define CH_FONT_COUNT 9
extern const ChineseFont_t CH_Font[CH_FONT_COUNT];




//图片外部接口
extern const uint8_t gImage_image[32256];
//extern const uint8_t image_data[113280];
#endif