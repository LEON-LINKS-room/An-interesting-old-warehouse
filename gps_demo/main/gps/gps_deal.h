/*******************************************************************************
MIT License

Copyright (c) 2023 LEON-LINKS-room

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#ifndef __GPS_DEAL_H__
#define __GPS_DEAL_H__

#define GPS_FRAME_HEAD  '$'         //帧头
#define GPS_FRAME_TAIL  '*'         //帧尾
#define GPS_FRAME_PART  ','         //帧内容间隔符

/*帧控制结构体*/
typedef struct GPSFRAME_CTRL{
    unsigned char head_flag;        //帧头收到标志,1
    unsigned char index;            //索引
    unsigned char type[8];          //帧类型
}GPSFRAME_CTRL;

/*GNGGA帧结构体*/
typedef struct GGAFRAME_CTRL{
    unsigned char index;
    unsigned char utctime[16];      //UTC时间
    unsigned char latitude[16];     //纬度
    unsigned char latitude_d[16];   //纬度
    unsigned char longitude[16];    //经度
    unsigned char longitude_d[16];  //经度
    unsigned char status[16];       //GPS状态
    unsigned char satenum[16];      //正在使用的用于定位的卫星数量
    unsigned char hdop[16];         //HDOP水平精确度因子
    unsigned char altitude[16];     //海拔高度
    unsigned char altitude_u[16];   //海拔高度单位
    unsigned char galtitude[16];    //大地水准面高度
    unsigned char galtitude_u[16];  //大地水准面高度单位
    unsigned char difftime[16];     //差分时间
    unsigned char bstation[16];     //差分参考基站标号
    unsigned char end[16];
}GGAFRAME_CTRL;

/*GNRMC帧结构体*/
typedef struct RMCFRAME_CTRL{
    unsigned char index;            //索引
    unsigned char utctime[16];      //UTC时间
    unsigned char status[16];       //状态
    unsigned char latitude[16];     //纬度
    unsigned char latitude_d[16];   //纬度
    unsigned char longitude[16];    //经度
    unsigned char longitude_d[16];  //经度
    unsigned char kspeed[16];       //速度
    unsigned char kspeed_h[16];     //方位角
    unsigned char utcdate[16];      //UTC日期
    unsigned char magcdec[16];      //磁偏角
    unsigned char magcdec_d[16];    //磁偏角方向
    unsigned char mode[16];         //模式
	unsigned char end[16];
}RMCFRAME_CTRL;

/*GNVTG帧结构体*/
typedef struct VTGFRAME_CTRL{
    unsigned char index;
    unsigned char realdir[16];      //以真北为参考基准的地面航向
    unsigned char realdir_u[16];    //航向标志位
    unsigned char mgcdir[16];       //以磁北为参考基准的地面航向
    unsigned char mgcdir_u[16];     //航向标志位
    unsigned char kspeed[16];       //地面速率
    unsigned char kspeed_u[16];     //地面速率单位
    unsigned char gspeed[16];       //地面速率
    unsigned char gspeed_u[16];     //地面速率单位
    unsigned char mode[16];         //模式指示
    unsigned char end[16];
}VTGFRAME_CTRL;

void gps_deal(unsigned char data);
void gps_info_get(char *index,char *msg);

#endif
