#include "lcdxx_driver.h"
#include "font.h"

#include <float.h>
#include <stdint.h>
#include <string.h>
#include <sys/_intsup.h>

/**
 * @brief 只是给设备初始化引脚 以及其需要的协议
 * 
 * @param dev 
 * @param spix          这里spix传入的是具体的 spi对象
 *                      @note 如软件定义的spi_one/亦或者硬件的hspi1
 * @param res_port 
 * @param res_pin 
 * @param dc_port 
 * @param dc_pin 
 * @param cs_port 
 * @param cs_pin 
 * @param blk_port 
 * @param blk_pin 
 */


void lcdxx_dev_init( lcdxx_dev *dev ,void  *spix,lcd_spi_swap spi_swap,
                    uint16_t phys_w,uint16_t phys_h,uint16_t gram_w,uint16_t gram_h,
                    uint16_t init_x, uint16_t init_y,
                    GPIO_TypeDef * res_port,uint16_t res_pin,
                    GPIO_TypeDef * dc_port,uint16_t dc_pin,
                    GPIO_TypeDef * cs_port,uint16_t cs_pin,
                    GPIO_TypeDef * blk_port,uint16_t blk_pin)
{   //这里不用解引用 spix 本身就是一个void，现在是接收协议对象地址 不需要调用
    dev->spix      = spix;       dev->spi_swap   = spi_swap;       /*spi_swap只是一个函数的变量指针 后面存的是地址 这里其实就是用来绑定设备的业务处理函数 只是这里的业务是发送字节*/
    
    //原始的参考标准    —— 0° 下的状态
    dev->phys_w   = phys_w;      dev->phys_h   = phys_h;        //0°时的宽和高分别多少  
    dev->gram_w   = gram_w;      dev->gram_h   = gram_h;        //显存gram大小
    dev->init_x   = init_x;      dev->init_y   = init_y;        //0°时的偏移量
    
    //引脚绑定
    dev->res_port  = res_port;   dev->res_pin    = res_pin;
    dev->dc_port   = dc_port;    dev->dc_pin     = dc_pin;
    dev->cs_port   = cs_port;    dev->cs_pin     = cs_pin;
    dev->blk_port  = blk_port;   dev->blk_pin    = blk_pin;

    // 默认初始方向设定为 0°    —— normal状态
    dev->lcd_width = phys_w;     dev->lcd_height = phys_h;
    dev->x_offset   = init_x;    dev->y_offset   = init_y;
    
}

#define res_high(x)  ((x->res_port)->BSRR = (x->res_pin)) 
#define res_low(x)   ((x->res_port)->BSRR = (uint32_t)(x->res_pin) << 16u)
#define dc_high(x)   ((x->dc_port)->BSRR = (x->dc_pin)) 
#define dc_low(x)    ((x->dc_port)->BSRR = (uint32_t)(x->dc_pin) << 16u)
#define cs_high(x)   ((x->cs_port)->BSRR = (x->cs_pin)) 
#define cs_low(x)    ((x->cs_port)->BSRR = (uint32_t)(x->cs_pin) << 16u)
#define blk_high(x)  ((x->blk_port)->BSRR = (x->blk_pin)) 
#define blk_low(x)   ((x->blk_port)->BSRR = (uint32_t)(x->blk_pin) << 16u)


static inline void spi_swap(lcdxx_dev *dev,uint8_t data)
{
    if(dev->spi_swap!=NULL)
    {
        //这里的dev->spix 就是上面的 void类型 具体的spi地址  在dev->spi_swap的具体函数中才需要解引用
        dev->spi_swap(dev->spix,data);
    }
}

void lcdxx_write_cmd(lcdxx_dev *dev,uint8_t cmd)
{
    dc_low(dev);    //dc拉低为command
    spi_swap(dev,cmd);
}

void lcdxx_write_data(lcdxx_dev *dev,uint8_t data)
{
    dc_high(dev);    //dc拉高为数据
    spi_swap(dev,data);
}


void lcdxx_reset(lcdxx_dev *dev)
{
    cs_high(dev);       // 这里非常关键   ⭐就是这里
    res_high(dev);
    res_low(dev);
    HAL_Delay(50);
    res_high(dev);
    HAL_Delay(120);
}



void st7789_init(lcdxx_dev *dev)
{

    lcdxx_reset(dev);
    cs_low(dev);
    lcdxx_write_cmd(dev,0x11); 
    HAL_Delay(120);                                        //Delay 120ms 

        //魔法参数
    lcdxx_write_cmd(dev,0x36); 
    lcdxx_write_data(dev ,0x00); 
    lcdxx_write_cmd(dev,0x3a); 
    lcdxx_write_data(dev ,0x05); 
    lcdxx_write_cmd(dev,0xb2); 
    lcdxx_write_data(dev ,0x0c); 
    lcdxx_write_data(dev ,0x0c); 
    lcdxx_write_data(dev ,0x00); 
    lcdxx_write_data(dev ,0x33); 
    lcdxx_write_data(dev ,0x33); 
    lcdxx_write_cmd(dev,0xb7); 
    lcdxx_write_data(dev ,0x35); 
    lcdxx_write_cmd(dev,0xbb); 
    lcdxx_write_data(dev ,0x19); 
    lcdxx_write_cmd(dev,0xc0); 
    lcdxx_write_data(dev ,0x2c); 
    lcdxx_write_cmd(dev,0xc2);
    lcdxx_write_data(dev ,0x01); 
    lcdxx_write_cmd(dev,0xc3); 
    lcdxx_write_data(dev ,0x0d); //GVDD=4.2V
    lcdxx_write_cmd(dev,0xc4); 
    lcdxx_write_data(dev ,0x20); 
    lcdxx_write_cmd(dev,0xc6); 
    lcdxx_write_data(dev ,0x0f); //Frame=60Hz
    lcdxx_write_cmd(dev,0xd0); 
    lcdxx_write_data(dev ,0xa4); 
    lcdxx_write_data(dev ,0xa1); 
    lcdxx_write_cmd(dev,0xe0); 
    lcdxx_write_data(dev ,0xd0); 
    lcdxx_write_data(dev ,0x00); 
    lcdxx_write_data(dev ,0x05); 
    lcdxx_write_data(dev ,0x0f); 
    lcdxx_write_data(dev ,0x10); 
    lcdxx_write_data(dev ,0x28); 
    lcdxx_write_data(dev ,0x34); 
    lcdxx_write_data(dev ,0x50); 
    lcdxx_write_data(dev ,0x44); 
    lcdxx_write_data(dev ,0x3a); 
    lcdxx_write_data(dev ,0x0b); 
    lcdxx_write_data(dev ,0x06); 
    lcdxx_write_data(dev ,0x11); 
    lcdxx_write_data(dev ,0x20); 
    lcdxx_write_cmd(dev,0xe1); 
    lcdxx_write_data(dev ,0xd0); 
    lcdxx_write_data(dev ,0x00); 
    lcdxx_write_data(dev ,0x05); 
    lcdxx_write_data(dev ,0x0a); 
    lcdxx_write_data(dev ,0x0b); 
    lcdxx_write_data(dev ,0x16); 
    lcdxx_write_data(dev ,0x32); 
    lcdxx_write_data(dev ,0x40); 
    lcdxx_write_data(dev ,0x4a); 
    lcdxx_write_data(dev ,0x2b); 
    lcdxx_write_data(dev ,0x1b); 
    lcdxx_write_data(dev ,0x1c); 
    lcdxx_write_data(dev ,0x22); 
    lcdxx_write_data(dev ,0x1f); 

    lcdxx_write_cmd(dev, 0x21); // IPS 反色
    lcdxx_write_cmd(dev,0x29); 


    cs_high(dev);
    blk_high(dev);  //打开背光
}




void st7735_init(lcdxx_dev *dev)
{
    lcdxx_reset(dev); // 先物理复位
    cs_low(dev);

    // 1. 退出睡眠
    lcdxx_write_cmd(dev, 0x11); // Sleep Out
    HAL_Delay(120);             // 必须等待 120ms 供电稳定

    // 2. 帧率与电源参数配置 (ST7735S 常用配置)
    lcdxx_write_cmd(dev, 0xB1); 
    lcdxx_write_data(dev, 0x05); lcdxx_write_data(dev, 0x3C); lcdxx_write_data(dev, 0x3C);
    lcdxx_write_cmd(dev, 0xB2); 
    lcdxx_write_data(dev, 0x05); lcdxx_write_data(dev, 0x3C); lcdxx_write_data(dev, 0x3C);
    lcdxx_write_cmd(dev, 0xB3); 
    lcdxx_write_data(dev, 0x05); lcdxx_write_data(dev, 0x3C); lcdxx_write_data(dev, 0x3C); 
    lcdxx_write_data(dev, 0x05); lcdxx_write_data(dev, 0x3C); lcdxx_write_data(dev, 0x3C);

    lcdxx_write_cmd(dev, 0xB4); // Display Inversion Control
    lcdxx_write_data(dev, 0x03);

    lcdxx_write_cmd(dev, 0xC0); // Power Control 1
    lcdxx_write_data(dev, 0x28); lcdxx_write_data(dev, 0x08); lcdxx_write_data(dev, 0x04);
    lcdxx_write_cmd(dev, 0xC1); // Power Control 2
    lcdxx_write_data(dev, 0xC0);
    lcdxx_write_cmd(dev, 0xC2); // Power Control 3
    lcdxx_write_data(dev, 0x0D); lcdxx_write_data(dev, 0x00);
    lcdxx_write_cmd(dev, 0xC3); // Power Control 4
    lcdxx_write_data(dev, 0x8D); lcdxx_write_data(dev, 0x2A);
    lcdxx_write_cmd(dev, 0xC4); // Power Control 5
    lcdxx_write_data(dev, 0x8D); lcdxx_write_data(dev, 0xEE);
    lcdxx_write_cmd(dev, 0xC5); // VCOM
    lcdxx_write_data(dev, 0x1A);

    // 3. 显示模式与色彩设置
    lcdxx_write_cmd(dev, 0x36);  // MADCTL: 显示方向与 RGB/BGR 控制
    lcdxx_write_data(dev, 0x00); // 若红蓝反色，在这里修改（0x00 或 0x08）

    lcdxx_write_cmd(dev, 0x3A);  // COLMOD: 颜色格式
    lcdxx_write_data(dev, 0x05); // 16-bit/pixel (RGB565)

    // 4. Gamma 曲线校正（关键：修正偏色和灰阶）
    lcdxx_write_cmd(dev, 0xE0);
    lcdxx_write_data(dev, 0x02); lcdxx_write_data(dev, 0x1C); lcdxx_write_data(dev, 0x07); 
    lcdxx_write_data(dev, 0x12); lcdxx_write_data(dev, 0x37); lcdxx_write_data(dev, 0x32); 
    lcdxx_write_data(dev, 0x29); lcdxx_write_data(dev, 0x2D); lcdxx_write_data(dev, 0x29); 
    lcdxx_write_data(dev, 0x2C); lcdxx_write_data(dev, 0x39); lcdxx_write_data(dev, 0x00); 
    lcdxx_write_data(dev, 0x01); lcdxx_write_data(dev, 0x03); lcdxx_write_data(dev, 0x10);

    lcdxx_write_cmd(dev, 0xE1);
    lcdxx_write_data(dev, 0x03); lcdxx_write_data(dev, 0x1D); lcdxx_write_data(dev, 0x07); 
    lcdxx_write_data(dev, 0x06); lcdxx_write_data(dev, 0x2E); lcdxx_write_data(dev, 0x2C); 
    lcdxx_write_data(dev, 0x29); lcdxx_write_data(dev, 0x2D); lcdxx_write_data(dev, 0x2E); 
    lcdxx_write_data(dev, 0x2E); lcdxx_write_data(dev, 0x37); lcdxx_write_data(dev, 0x00); 
    lcdxx_write_data(dev, 0x00); lcdxx_write_data(dev, 0x02); lcdxx_write_data(dev, 0x10);

    // 5. 反色与开显示
    // 若颜色依然像底片，请将 0x21 改为 0x20 (INVOFF)
    lcdxx_write_cmd(dev, 0x20); 

    lcdxx_write_cmd(dev, 0x29);  // Display ON
    HAL_Delay(100);

    cs_high(dev);
    blk_high(dev); // 打开背光
}

/**
 * @brief 屏幕休眠控制函数
 * 
 * @param dev   具体设备
 * @param state     
        @arg  0为休眠
        @arg  !0为开启
 */
void lcd_conx_sleep(lcdxx_dev *dev ,uint8_t state)
{
    cs_low(dev);
    dc_low(dev);
    if(state==0)
    {   //关闭时要先关显存再开睡眠
        //注意关显存里面的数据不会丢失，再开还在
        spi_swap(dev, 0x28);
        HAL_Delay(20);
        spi_swap(dev, 0x10);
        HAL_Delay(120);
        blk_low(dev);
    }
    else {
        //开启时先关睡眠 再开显存
        spi_swap(dev, 0x11);
        HAL_Delay(120);
        spi_swap(dev, 0x29);
        HAL_Delay(20);
        blk_high(dev);
    }
    cs_high(dev);
}


/**
 * @brief 框定区域准备填入色块
 * 
 * @param dev           具体要操作的设备
    * @note  四个参数为框定区域的范围
    * @note  传入的是 总宽度/总高度（尺寸） 时 且 起点为 0 时必须 两个end需要减1
    * @param x_start       
    * @param x_end         
    * @param y_start 
    * @param y_end 
 */
void lcdxx_setwindows(lcdxx_dev*dev,uint16_t x_start,uint16_t x_end,uint16_t y_start,uint16_t y_end)
{   
    //如果有偏移 需要整体偏移
    x_start += dev->x_offset; x_end += dev->x_offset;
    y_start += dev->y_offset; y_end += dev->y_offset;           
    cs_low(dev);
    lcdxx_write_cmd(dev, 0x2a);//设置列宽
    lcdxx_write_data(dev, x_start>>8);lcdxx_write_data(dev, x_start&0xff);   //清除高8位
    lcdxx_write_data(dev, x_end>>8);lcdxx_write_data(dev, x_end&0xff);   //清除高8位

    lcdxx_write_cmd(dev, 0x2b);//设置行宽
    lcdxx_write_data(dev, y_start>>8);lcdxx_write_data(dev, y_start&0xff);
    lcdxx_write_data(dev, y_end>>8);lcdxx_write_data(dev, y_end&0xff);   //清除高8位

    lcdxx_write_cmd(dev, 0x2c);   //写入显存
    //cs_high(dev);         //这个函数用于所有的函数模块内部 不独立使用    不用关
}

/**
 * @brief 全屏填充
 * 
 * @param dev     要操作的设备
 * @param color   要填充的颜色
 */
void lcdxx_fill(lcdxx_dev *dev,uint16_t color)
{
    lcdxx_setwindows(dev, 0,(dev->lcd_width)-1, 0,(dev->lcd_height)-1);
    dc_high(dev);    //dc拉高为数据
    cs_low(dev);
    for(uint16_t i=0;i<(dev->lcd_width);i++)
    {
        for(uint16_t j=0;j<(dev->lcd_height);j++)
        {
            spi_swap(dev,color>>8);
            spi_swap(dev,color&0xff);
        }
    }
    cs_high(dev);
}

/**
 * @brief   在指定坐标画一个像素点
 * @note    要注意当前设备的尺寸
 * @param dev   指定设备
 * @param x     位置
 * @param y 
 * @param color     颜色
 */
void lcdxx_draw_pixel(lcdxx_dev*dev,uint16_t x,uint16_t y,uint16_t color)
{
    lcdxx_setwindows(dev, x, x, y, y);
    cs_low(dev);
    lcdxx_write_data(dev, color>>8);
    lcdxx_write_data(dev, color&0xff);
    cs_high(dev);
}

/**
 * @brief   画线函数
 * 
 * @param dev 
 * @param x_start 
 * @param x_end 
 * @param y_start 
 * @param y_end 
 * @param color 
 */
void lcdxx_draw_line(lcdxx_dev *dev, uint16_t x_start, uint16_t x_end, uint16_t y_start, uint16_t y_end, uint16_t color)
{
    // 1. 计算 X 与 Y 轴的绝对差值
    int dx = (x_end > x_start) ? (x_end - x_start) : (x_start - x_end);
    int dy = (y_end > y_start) ? (y_end - y_start) : (y_start - y_end);
    // 2. 确定 X 和 Y 轴的步进方向（+1 或 -1）
    int sx = (x_start < x_end) ? 1 : -1;
    int sy = (y_start < y_end) ? 1 : -1;
    // 3. 初始化误差值与累加器
    int err = dx - dy;
    int e2;
    while (1)
    {
        // 绘制当前坐标点
        lcdxx_draw_pixel(dev, x_start, y_start, color);
        // 到达终点，退出循环
        if (x_start == x_end && y_start == y_end)
        {
            break;
        }
        e2 = 2 * err;
        // 根据误差调整下一个 X 坐标
        if (e2 > -dy)
        {
            err -= dy;
            x_start += sx;
        }
        // 根据误差调整下一个 Y 坐标
        if (e2 < dx)
        {
            err += dx;
            y_start += sy;
        }
    }
}

/**
 * @brief    单个字符显示函数
 * 
 * @param dev 
 * @param x_start 
 * @param line 
 * @param ch 
 * @param fcolor        文字颜色
 * @param bcolor        文字底色
 */
void lcdxx_show_char(lcdxx_dev *dev,uint16_t x_start,tft_line_y line,char ch,uint16_t fcolor,uint16_t bcolor)
{
    uint8_t index=ch-32;
    lcdxx_setwindows(dev,x_start, x_start+7, line, line+15); //字符  划分小块区域填充
    dc_high(dev);
    cs_low(dev);
    //因为字库为16个8字节   所以发16个 byte
    //每个字节按照像素点的模式刷新
    for (uint8_t st=0; st<16; st++) 
    {
        uint8_t data=ascii_font[index][st]; //提取出这个字符的每一个byte
        for (uint8_t yt=0;yt<8; yt++) 
        {
            if(data&(0x80>>yt)) //这里是1显示文字 还是0显示文字 根据汉字取模来的阳码                
            {
                spi_swap(dev, fcolor>>8);
                spi_swap(dev, fcolor&0xff);
            }
            else 
            {
                spi_swap(dev, bcolor>>8);
                spi_swap(dev, bcolor&0xff);
            }
        }
    }
    cs_high(dev);
}


/**
 * @brief 汉字显示函数
 * 
 * @param dev 
 * @param x_start 
 * @param line 
 * @param ch 
 * @param fcolor        文字颜色
 * @param bcolor        文字底色
 */
void lcdxx_show_chinese(lcdxx_dev *dev,uint16_t x_start,tft_line_y line,const char *ch,uint16_t fcolor,uint16_t bcolor)
{
    uint16_t x=x_start,y=line;       //要写入多个字符这里必须有载体 算偏移量
    //utf—8 汉字为三字节  单独一个ch存不了
    while(*ch!='\0')
    {
        int judge=-1;   //存了对应的汉字结构体 索引
        for (uint16_t k=0; k<CH_FONT_COUNT; k++) 
        {
            if(strncmp(ch,CH_Font[k].Index,3)==0)   //等于0是匹配成功
            {
                judge=k;
                break;
            }
        }
        if(judge!=-1)
        {
            if (x + 16 > dev->lcd_width) { x = 0; y += 16; } // 越界换行保护
            lcdxx_setwindows(dev, x, x+15,y, y+15);    //选定区域 开始写入颜色
            dc_high(dev);//数据
            cs_low(dev);//响应  spi开始
            for (uint8_t i=0; i<32; i++) 
            {
                uint8_t data=CH_Font[judge].Msk[i]; //取出每一个汉字的8位字节
                for(uint8_t j=0;j<8;j++)            //发送字节
                {
                    if(data&(0x80>>j)) //这里是1显示文字 还是0显示文字 根据汉字取模来的阳码                
                    {
                        spi_swap(dev, fcolor>>8);
                        spi_swap(dev, fcolor&0xff);
                    }
                    else 
                    {
                        spi_swap(dev, bcolor>>8);
                        spi_swap(dev, bcolor&0xff);
                    }
                }
            }
            cs_high(dev);
            x+=16;//下一次开始的位置 且只有匹配到写了在加偏移量 //这里x加就好了
            ch+=3;   //指针移动 这里一定要是加3！！
        }
        else {
            ch++;   //后来改的
        }
        
    }
}

/**
 * @brief 字符串显示函数
 * 
 * @param dev 
 * @param x_start 
 * @param line 
 * @param ch 
 * @param fcolor        文字颜色
 * @param bcolor        文字底色
 */
void lcdxx_show_string(lcdxx_dev *dev,uint16_t x_start,tft_line_y line,const char *ch,uint16_t fcolor,uint16_t bcolor)
{
    uint16_t x=x_start,y=line;
    while(*ch!='\0')
    {
        if(x+8>dev->lcd_width)x=0,y+=16;   //超出则重新设定下一行位置
        lcdxx_show_char(dev, x, y, *ch, fcolor, bcolor);
        x+=8;
        ch++;
    }
}

/**
 * @brief 字符汉字综合文本显示
 * 
 * @param dev 
 * @param x_start 
 * @param line 
 * @param str 
 * @param fcolor        文字颜色
 * @param bcolor        文字底色
 */
void lcdxx_show_text(lcdxx_dev *dev, uint16_t x_start, tft_line_y line, const char *str, uint16_t fcolor, uint16_t bcolor)
{
    uint16_t x = x_start;
    uint16_t y = line;
    while (*str != '\0')        //这个只用于字符串
    {
        //ASCII 字符处理
        if ((uint8_t)*str >= 32 && (uint8_t)*str <= 126)
        {
            if (x + 8 > dev->lcd_width) { x = 0; y += 16; } // 边界换行
            
            lcdxx_show_char(dev, x, y, *str, fcolor, bcolor);
            
            x += 8;
            str += 1; // ASCII 占 1 字节
        }
        //汉字处理
        else
        {
            if (x + 16 > dev->lcd_width) { x = 0; y += 16; } // 边界换行

            // 截取当前 UTF-8 汉字的 3 个字节，拼成临时单字字符串
            char hz_buf[4] = { *str, *(str+1), *(str+2), '\0' };

            lcdxx_show_chinese(dev, x, y, hz_buf, fcolor, bcolor);
            x += 16;
            str += 3; // UTF-8 汉字跨越 3 字节
        }
    }
}



void lcdxx_show_image(lcdxx_dev *dev, uint16_t width, uint16_t heigth, const uint8_t *image_data)
{
    uint32_t total_pixels = (uint32_t)width * heigth;
    lcdxx_setwindows(dev, 0, width - 1, 0, heigth - 1);
    dc_high(dev);
    cs_low(dev);
    for (uint32_t i = 0; i < total_pixels * 2; i++)
    {
        spi_swap(dev, image_data[i]);
    }
    cs_high(dev);
}
/**
 * @brief 屏幕方向配置函数 (完美解决镜像、宽高与偏移量问题)
 * 
 * @param dev 屏幕设备句柄
 * @param ori 旋转方向
 */
void lcdxx_display_con(lcdxx_dev *dev, lcdxx_orientation ori)
{
    //记录当前的宽 高 偏移量 以及这个屏幕的显存大小
    uint16_t pw = dev->phys_w;
    uint16_t ph = dev->phys_h;
    uint16_t gw = dev->gram_w;
    uint16_t gh = dev->gram_h;
    //从左上角刷新时的偏移量
    uint16_t ix = dev->init_x;
    uint16_t iy = dev->init_y;

    // 几何互补边距计算
    // 算出右x与 y轴的 真实显示 与 gram的边距
    uint16_t rx = gw - pw - ix;     
    uint16_t ry = gh - ph - iy;

    uint8_t madctl = 0x00;

    switch (ori)
    {
        //默认情况下 偏移量宽度都是初始默认 从左上角0，0开始算偏移

        case normal: // 0° 竖屏
            madctl          = 0x00;
            dev->lcd_width  = pw;
            dev->lcd_height = ph;
            dev->x_offset   = ix;
            dev->y_offset   = iy;
            break;

        //显存宽度和高度对调    偏移量 x轴与y轴对调         
        //变成原来的右上角算偏移          
        case display_90: // 90° 横屏
            madctl          = 0x60;
            dev->lcd_width  = ph;
            dev->lcd_height = pw;
            dev->x_offset   = iy;   //因为还是从上向下刷新所以 x偏移量只对掉不用重新算
            dev->y_offset   = rx;
            break;
        //宽度高度不用对调  偏移量
        //原来的右下角算偏移
        case display_180: // 180° 倒竖屏
            madctl          = 0xC0;
            dev->lcd_width  = pw;
            dev->lcd_height = ph;
            dev->x_offset   = rx;
            dev->y_offset   = ry;
            break;

        //原来的左下角算偏移
        case display_270: // 270° 倒横屏
            madctl          = 0xA0;
            dev->lcd_width  = ph;
            dev->lcd_height = pw;
            dev->x_offset   = ry;
            dev->y_offset   = ix;
            break;

        default:
            return;
    }

    // 发送 0x36 寄存器指令
    dc_low(dev);
    cs_low(dev);
    lcdxx_write_cmd(dev, 0x36);

    dc_high(dev);
    lcdxx_write_data(dev, madctl);
    cs_high(dev);
}



/*阻塞式函数 变量定义*/


#define lcd_max_width 240   //240像素宽度
#define lcd_fill_line 40    //20行
//由于color刷屏但是dma开了自增 所以建立区块刷新数组
//思想就是把屏幕按照自己要的行分成 n块区域 然后n--通过中断连续刷新
//这样可以节省空间
static uint8_t lcd_fill_block_buf[lcd_max_width*lcd_fill_line*2];       
typedef enum{dma_state_idle = 0,dma_state_image,dma_state_fill}dma_state_t;
static volatile dma_state_t s_dma_state        = dma_state_idle; // DMA状态锁
static volatile uint16_t    s_remaining_blocks = 0;              // 颜色填充剩余切片块数
static uint32_t             s_block_bytes      = 0;              // 颜色填充单块字节数
static uint16_t s_tail_bytes  = 0; // ⭐ 最后一块不足lcd_fill_line 行的尾巴字节数 (0 表示没有尾巴)
static lcdxx_dev          * s_current_dev      = NULL;      // 记录当前操作的屏幕指针
static const uint8_t      * s_image_next_ptr   = NULL;      // ⭐ 图片传输：下一次发送的内存起始地址
static volatile uint32_t    s_image_rem_bytes  = 0;         // ⭐ 图片传输：剩余未发总字节数

uint8_t lcd_dma_is_busy(void)
{
    return (s_dma_state != dma_state_idle); //不等于0就返回1，阻塞
}




/**
 * @brief 真正的 DMA 阻塞式区域填充 (利用硬件 DMA 切片高速搬运，发完才退出)
 * 
 * @param dev      屏幕对象指针
 * @param x_start  起始 X 坐标
 * @param y_start  起始 Y 坐标
 * @param x_end    结束 X 坐标
 * @param y_end    结束 Y 坐标
 * @param color    填充颜色 (RGB565)
 */
void lcdxx_dma_sync_area_fill(lcdxx_dev *dev, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
    // 1. 如果当前有其他 DMA 正在传输，先等待其结束 (总线保护)
    while (lcd_dma_is_busy());

    // 2. 下发 DMA 异步区域填充任务
    // (内部会自动做参数校验、开窗、切片计算，并启动第 1 块传输)
    if (lcdxx_dma_async_area_fill(dev, x_start, y_start, x_end, y_end, color) == 0)
    {
        // 3. ⭐ CPU 在此处阻塞等待，直到后台中断把所有切片/尾巴全部发完并释放总线
        while (lcd_dma_is_busy());
    }
}

/**
 * @brief 真正的 DMA 阻塞式全屏填充
 */
void lcdxx_sync_full_fill(lcdxx_dev *dev, uint16_t color)
{
    lcdxx_dma_sync_area_fill(dev, 0, 0, dev->lcd_width - 1, dev->lcd_height - 1, color);
}


/**
 * @brief DMA 阻塞式显示图片 (发完全部数据包才返回)
 */
void lcdxx_dma_sync_show_image(lcdxx_dev *dev, uint16_t width, uint16_t heigth, const uint8_t *image_data)
{
    while (lcd_dma_is_busy());

    if (lcdxx_dma_async_show_image(dev, width, heigth, image_data) == 0)
    {
        while (lcd_dma_is_busy());
    }
}
// ---------------- 4. 异步非阻塞 DMA 接口 ----------------
/**
 * @brief 异步区域填充（全屏填充也是基于它实现）
 * @return 0: 成功下发任务, 1: 硬件正忙被拒绝, 2: 坐标参数非法
 * @note   这里加边界判断是因为开启动态缓冲区
 */
 uint8_t lcdxx_dma_async_area_fill(lcdxx_dev *dev,uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end,uint16_t color)
 {
    if(lcd_dma_is_busy())return 1;  
    if (x_start > x_end || y_start > y_end || x_end >= dev->lcd_width || y_end >= dev->lcd_height)
    {
        return 2;
    }
    uint16_t width  = x_end - x_start + 1;
    uint16_t height = y_end - y_start + 1;  //要写入的高度
   
    uint32_t line_per_block = (height>lcd_fill_line) ? lcd_fill_line : height;      //用来判断当前要填入的高度是否高于设定的框架，避免多填
    uint32_t total_bytes = (uint32_t)width * line_per_block;    //这里没乘2是因为等下减少循环，但是一次写两个数据
    
    uint8_t hb = color >> 8, lb = color & 0xFF;
    for (uint32_t k=0; k<total_bytes; k++) 
    {
        lcd_fill_block_buf[k*2]   = hb;//(color>>8);    //如果用color 他每次都要动态计算
        lcd_fill_block_buf[k*2+1] = lb;//(color&0xff);
    }

    // ---------------- ⭐ 计算满块与尾巴 ----------------
    uint16_t full_blocks = height / lcd_fill_line; // 满 x 行的块数
    uint16_t rem_lines   = height % lcd_fill_line; // 剩下的余数行 (比如 50%20 = 10行)      //用于避免如240 - 30的情况

    //这个参数要传到全局变量然后用于后续填充用
    s_block_bytes = width * lcd_fill_line * 2;     // 满 lcd_fill_line  行的标准字节数 //也是前面建造的全局变量数组空间

    s_dma_state   = dma_state_fill;    //填充状态
    s_current_dev = dev;               //记录当前设备---这点很重要，因为非阻塞cs线不在当前函数内拉高 要在中断里面收尾
    
    uint16_t first_send_bytes;
    if (full_blocks == 0) // 情况 A：总高度不足 20 行（如高 8 行，1 次直接发完）
    {
        first_send_bytes   = (uint16_t)(width * height * 2);
        s_remaining_blocks = 0;
        s_tail_bytes       = 0;     //没有余数 
    }
    else // 情况 B：总高度 >= 20 行（如 30、50、240 行）
    {
        first_send_bytes   = (uint16_t)s_block_bytes;       //第一次要发的是数据
        s_tail_bytes       = (rem_lines > 0) ? (uint16_t)(width * rem_lines * 2) : 0; // 算出尾巴字节   //没有余数就为0 有的话就算出尾巴的字节数    //因为填充颜色尾巴肯定是一次发完的 因为尾巴的数据量小于 建造的ram
        s_remaining_blocks = full_blocks + (rem_lines > 0 ? 1 : 0) - 1;                // 总块数 - 1    //（full——block 算的是总块数 但是没算尾巴 所以这里判断有没有尾巴 有的话加一次 没有不加）但是这次函数的调用也会发送一次 所以也还要减一次
    }
    lcdxx_setwindows(dev, x_start, x_end, y_start, y_end);
    dc_high(dev);
    cs_low(dev);
    //他们的数据发送结束都会跳转一个 spi发送完的中断 
   HAL_SPI_Transmit_DMA((SPI_HandleTypeDef *)dev->spix, lcd_fill_block_buf, (uint16_t)first_send_bytes);  //传输第一个区块        //size就是他的计数器自减计数器
    return 0;
}

uint8_t lcdxx_dma_async_fill(lcdxx_dev *dev, uint16_t color)
{
    return lcdxx_dma_async_area_fill(dev, 0, 0, dev->lcd_width - 1, dev->lcd_height - 1, color);
}

/**
 * @brief 
 * 
 * @param dev 
 * @param width             图片宽度
 * @param heigth            图片高度
 * @param image_data 
 * @return uint8_t 
 */
uint8_t lcdxx_dma_async_show_image(lcdxx_dev *dev, uint16_t width, uint16_t heigth, const uint8_t *image_data)
{
    if(lcd_dma_is_busy())return 1;   //空闲判断
    uint32_t total_bytes = (uint32_t)width * heigth * 2;

    // 1. 计算第一包发送的大小（最大取 65534 偶数，防止 RGB565 颜色错位）
    uint16_t first_send_bytes = (total_bytes > 65534) ? 65534 : (uint16_t)total_bytes;

    // 2. 记录剩余字节数和下一包的起始地址
    //这里如果上一步 1。哪里 total不大于first 那么这里的s_imagexxx这个变量就是0
    s_image_rem_bytes = total_bytes - first_send_bytes; // 剩余字节数       
    //其实就是起始地址加上第一次发送的数据量 ⭐这里的数据是指针也就是 直接解引用他就能得到对应数据，而不是单纯的数字
    s_image_next_ptr  = image_data + first_send_bytes;  // 下一包起始指针   
    s_dma_state   = dma_state_image;   // 给一个图片填充状态
    s_current_dev = dev;               // 存地址
   
    lcdxx_setwindows(dev, 0, width -1 , 0, heigth -1);
    dc_high(dev);
    cs_low(dev);
    //他们的数据发送结束都会跳转一个 spi发送完的中断 
    HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)dev->spix, (uint8_t *)image_data, first_send_bytes);       
    return 0;   // 任务下发成功
}

// ---------------- 5. 驱动内部中断处理机 ----------------
void lcd_dma_tx_cplt_handler(void)
{
    if (s_current_dev == NULL) return; // 防止找不到对应当前要处理的设备
    //将spix声明一下类型 供后面使用
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)s_current_dev->spix;
    if (s_dma_state == dma_state_image)
    {
        // 只要还有剩余字节（不管剩下 1 包还是多包），中断就继续接力发送
        if (s_image_rem_bytes > 0)
        {
            // 本次发送大小：若剩余大于 65534 则发 65534，否则发完剩余全部
            uint16_t send_size = (s_image_rem_bytes > 65534) ? 65534 : (uint16_t)s_image_rem_bytes;
            //在这里留作应对下一次中断来袭  s_image_next_ptr是一波的 数组地址 传给send_ptr
            const uint8_t *send_ptr = s_image_next_ptr;     
            // 更新状态
            //减去刚才发送的字节，如果刚才 一次发完了这里剩余字节就会变为0   
            //在这里他全局变量s_image_byte已经变为0了
            s_image_rem_bytes -= send_size;       
            //继续加上刚才移动的指针，也就是发送的字节数，万一这次没法送完，留待下次发送锚定位置
            s_image_next_ptr  += send_size;        
            //发送 这样发送后 会再次进入发送完中断，再进如这个函数 ，如果没法送完 就会再次重复刚才的操作
            HAL_SPI_Transmit_DMA(hspi, (uint8_t *)send_ptr, send_size); 
        }
        else
        {
            // 整张图片所有包全部发送完毕，硬件收尾
            //⭐ SR 是整个寄存器的值，而 SPI_SR_BSY 是告诉你“我要看 SR 里的哪一位”。  
            /*SPI_SR_BSY 本质上只是： 0000 0000 1000 0000 用来把 SR 里的 BSY 那一位“筛出来” */
            //sr是spi的状态寄存器       SPI_SR_BSY是状态寄存器中的busy标志位 0空闲 1在忙 
            while (hspi->Instance->SR & SPI_SR_BSY);                   
            cs_high(s_current_dev);             // 自动拉高当前屏幕的 CS
            s_dma_state      = dma_state_idle;  // 释放总线
            s_current_dev    = NULL;
            s_image_next_ptr = NULL;        //指针清空要用null
        }
    }
    else if (s_dma_state == dma_state_fill)
    {
        if (s_remaining_blocks > 0) //总区块大于0
        {
            s_remaining_blocks--;   //进一次减一次
            // ⭐ 如果这是最后一块 (自减后等于0) 且有尾巴，只发尾巴大小；否则发整块 20 行
            //在此函数中再次定义局部变量来判断是否有尾巴，尾巴最后发
            //不是最后 或者没有尾巴 每次都发一整个ram空间 也就是自定义的width * lcd——line
            uint16_t send_size = (s_remaining_blocks == 0 && s_tail_bytes > 0) 
                                 ? s_tail_bytes 
                                 : (uint16_t)s_block_bytes;
            //循环发送数据 直到结束                      
            HAL_SPI_Transmit_DMA(hspi, lcd_fill_block_buf, send_size);  
        }
        else    //结束后处理收尾 也将dma 恢复空闲
        {
            // 全部切片发送完毕，收尾
            while (hspi->Instance->SR & SPI_SR_BSY);
            cs_high(s_current_dev);             // 自动拉高当前屏幕的 CS
            s_dma_state   = dma_state_idle;    // 恢复空闲
            s_current_dev = NULL;
        }
    }
}


