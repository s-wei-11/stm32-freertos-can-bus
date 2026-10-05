#pragma once

#include <stdint.h>
#include "main.h"


//屏幕尺寸
#define st7789_width    240
#define st7789_high     240

#define WHITE   0xFFFF
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0

//0-239
//后续高度还有增加的话 需要
//16高度
typedef enum{
    line0  = 0,    line1  = 16,  line2  = 32,
    line3  = 48,   line4  = 64,  line5  = 80,    
    line6  = 96,   line7  = 112, line8  = 128,
    line9  = 144,  line10 = 160, line11 = 176,
    line12 = 192,  line13 = 208, line14 = 224
}tft_line_y;


typedef enum{
    normal=0,
    display_90,
    display_180,
    display_270  
}lcdxx_orientation;
//定义回调函数 方便调用soft spi与 硬件spi
//这里用void * spi_dev 为了方便后面介入不同屏幕
//spi_dev 是协议对象 ，这里的lcd_spi_swap只是 抽象一个变量 这个变量会用到 void 类型的spi——dev
typedef uint8_t (*lcd_spi_swap)(void *spix_t,uint8_t data);

typedef struct
{
    //协议封装
    void       *   spix;      //用于接收spi协议 （这样可以通用）
    lcd_spi_swap   spi_swap;  //存放函数调用  

    //具体所用引脚封装  
    GPIO_TypeDef * res_port;
    uint16_t       res_pin;
    GPIO_TypeDef * dc_port;
    uint16_t       dc_pin;
    GPIO_TypeDef * cs_port;     //设备应答线
    uint16_t       cs_pin;
    GPIO_TypeDef * blk_port;
    uint16_t       blk_pin;


// 1. 运行时动态参数 (供绘图/开窗/DMA实时读取)
    uint16_t       lcd_width;     // 当前逻辑宽
    uint16_t       lcd_height;    // 当前逻辑高
    uint16_t       x_offset;      // 当前运行 X 偏移    //预防屏幕尺寸ram有偏移
    uint16_t       y_offset;      // 当前运行 Y 偏移


// 2. 硬件出厂固定参数 (初始化传入后不再修改)
    uint16_t       phys_w;        // 屏幕 0° 物理宽度
    uint16_t       phys_h;        // 屏幕 0° 物理高度
    uint16_t       gram_w;        // 芯片最大显存宽
    uint16_t       gram_h;        // 芯片最大显存高
    uint16_t       init_x;        // 0° 状态下厂家出厂 X 偏移
    uint16_t       init_y;        // 0° 状态下厂家出厂 Y 偏移
} lcdxx_dev;


void lcd_conx_sleep(lcdxx_dev *dev ,uint8_t state);
void lcdxx_display_con(lcdxx_dev *dev,lcdxx_orientation ori);
void lcdxx_dev_init(lcdxx_dev *dev, void *spix, lcd_spi_swap spi_swap,
                    uint16_t phys_w, uint16_t phys_h,
                    uint16_t gram_w, uint16_t gram_h,
                    uint16_t init_x, uint16_t init_y,
                    GPIO_TypeDef *res_port, uint16_t res_pin,
                    GPIO_TypeDef *dc_port,  uint16_t dc_pin,
                    GPIO_TypeDef *cs_port,  uint16_t cs_pin,
                    GPIO_TypeDef *blk_port, uint16_t blk_pin);
void st7789_init(lcdxx_dev *dev);       // 1.14寸屏幕时初始化时要加偏移量 x轴52 y轴40   1.54寸不用加偏移量
void st7735_init(lcdxx_dev *dev);       //1.8寸初始化时要加偏移量 x轴2 y轴1



void lcdxx_write_cmd(lcdxx_dev *dev,uint8_t cmd);
void lcdxx_write_data(lcdxx_dev *dev,uint8_t data);
void lcdxx_draw_pixel(lcdxx_dev*dev,uint16_t x,uint16_t y,uint16_t color);
void lcdxx_draw_line(lcdxx_dev *dev, uint16_t x_start, uint16_t x_end, uint16_t y_start, uint16_t y_end, uint16_t color);                


void lcdxx_fill(lcdxx_dev *dev,uint16_t color);
void lcdxx_show_char(lcdxx_dev *dev,uint16_t x_start,tft_line_y line,char ch,uint16_t fcolor,uint16_t bcolor);
void lcdxx_show_text(lcdxx_dev *dev, uint16_t x_start, tft_line_y line, const char *str, uint16_t fcolor, uint16_t bcolor);
void lcdxx_show_chinese(lcdxx_dev *dev,uint16_t x_start,tft_line_y line,const char *ch,uint16_t fcolor,uint16_t bcolor);
void lcdxx_show_string(lcdxx_dev *dev,uint16_t x_start,tft_line_y line,const char *ch,uint16_t fcolor,uint16_t bcolor);
 void lcdxx_show_image(lcdxx_dev *dev,uint16_t width,uint16_t heigth,const uint8_t * image_data);
//共用体解决cs线






// ==================== 【DMA 状态与阻塞/非阻塞 API 声明】 ====================

/**
 * @brief 查询 DMA 是否正在忙碌
 * @return 0: 空闲可以下发新任务, 1: 正在忙碌
 */
uint8_t lcd_dma_is_busy(void);

/**
 * @brief 阻塞式填充接口 (CPU等候发完才返回，适合开机初始化使用)
 */
void lcdxx_dma_sync_area_fill(lcdxx_dev *dev, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);
void lcdxx_sync_full_fill(lcdxx_dev *dev, uint16_t color);
void lcdxx_dma_sync_show_image(lcdxx_dev *dev, uint16_t width, uint16_t heigth, const uint8_t *image_data);


/**
 * @brief 异步非阻塞 DMA 填充接口 (瞬间返回，耗时 < 1us)
 * @return 0: 成功启动, 1: 硬件正忙被拒绝, 2: 坐标参数非法
 */
uint8_t lcdxx_dma_async_area_fill(lcdxx_dev *dev, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color);
uint8_t lcdxx_dma_async_fill(lcdxx_dev *dev, uint16_t color);
uint8_t lcdxx_dma_async_show_image(lcdxx_dev *dev, uint16_t width, uint16_t heigth, const uint8_t *image_data);

/**
 * @brief DMA 传输完成中断处理机 (必须在 main.c 的 HAL_SPI_TxCpltCallback 中调用)
 */
void lcd_dma_tx_cplt_handler(void);