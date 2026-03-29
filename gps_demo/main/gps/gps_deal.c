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
#include "gps_deal.h"
#include "string.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t gps_binary_semaphore;

GPSFRAME_CTRL gpsframe_ctrl={0};

GGAFRAME_CTRL ggaframe_ctrl={0};
RMCFRAME_CTRL rmcframe_ctrl={0};
VTGFRAME_CTRL vtgframe_ctrl={0};

int segment_index = 0;

void clear_gpsframe_ctrl(void){
    gpsframe_ctrl.head_flag = 0;
    gpsframe_ctrl.index = 0;
    for(int i=0;i<8;i++){
        gpsframe_ctrl.type[i] = 0;
    }  
}

void gps_deal(unsigned char data){
    if(gpsframe_ctrl.head_flag!=1){
        if(data==GPS_FRAME_HEAD){
            gpsframe_ctrl.head_flag = 1;
            gpsframe_ctrl.type[gpsframe_ctrl.index++] = data;
        }
    }
    else{
        if(data==GPS_FRAME_TAIL){
            clear_gpsframe_ctrl();
            segment_index = 0;
            ggaframe_ctrl.index = 0;
            rmcframe_ctrl.index = 0;
            vtgframe_ctrl.index = 0;
        }
        else if(data==GPS_FRAME_PART){
            segment_index++;
            ggaframe_ctrl.index = 0;
            rmcframe_ctrl.index = 0;
            vtgframe_ctrl.index = 0;

            if((gpsframe_ctrl.type[3]=='G')&&(gpsframe_ctrl.type[4]=='G')&&(gpsframe_ctrl.type[5]=='A')){
                switch(segment_index){
                    case 1:{
                        memset(ggaframe_ctrl.utctime,0,16);
                    }break;
                    case 2:{
                        memset(ggaframe_ctrl.latitude,0,16);
                    }break;
                    case 3:{
                        memset(ggaframe_ctrl.latitude_d,0,16);
                    }break;
                    case 4:{
                        memset(ggaframe_ctrl.longitude,0,16);
                    }break;
                    case 5:{
                        memset(ggaframe_ctrl.longitude_d,0,16);
                    }break;
                    case 6:{
                        memset(ggaframe_ctrl.status,0,16);
                    }break;
                    case 7:{
                        memset(ggaframe_ctrl.satenum,0,16);
                    }break;
                    case 8:{
                        memset(ggaframe_ctrl.hdop,0,16);
                    }break;
                    case 9:{
                        memset(ggaframe_ctrl.altitude,0,16);
                    }break;
                    case 10:{
                        memset(ggaframe_ctrl.altitude_u,0,16);
                    }break;
                    case 11:{
                        memset(ggaframe_ctrl.galtitude,0,16);
                    }break;
                    case 12:{
                        memset(ggaframe_ctrl.galtitude_u,0,16);
                    }break;
                    case 13:{
                        memset(ggaframe_ctrl.difftime,0,16);
                    }break;
                    case 14:{
                        memset(ggaframe_ctrl.bstation,0,16);
                    }break;
                    default:break;
                }
            }
            else if((gpsframe_ctrl.type[3]=='R')&&(gpsframe_ctrl.type[4]=='M')&&(gpsframe_ctrl.type[5]=='C')){
                switch(segment_index){
                    case 1:{
                        memset(rmcframe_ctrl.utctime,0,16);
                    }break;
                    case 2:{
                        memset(rmcframe_ctrl.status,0,16);
                    }break;
                    case 3:{
                        memset(rmcframe_ctrl.latitude,0,16);
                    }break;
                    case 4:{
                        memset(rmcframe_ctrl.latitude_d,0,16);
                    }break;
                    case 5:{
                        memset(rmcframe_ctrl.longitude,0,16);
                    }break;
                    case 6:{
                        memset(rmcframe_ctrl.longitude_d,0,16);
                    }break;
                    case 7:{
                        memset(rmcframe_ctrl.kspeed,0,16);
                    }break;
                    case 8:{
                        memset(rmcframe_ctrl.kspeed_h,0,16);
                    }break;
                    case 9:{
                        memset(rmcframe_ctrl.utcdate,0,16);
                    }break;
                    case 10:{
                        memset(rmcframe_ctrl.magcdec,0,16);
                    }break;
                    case 11:{
                        memset(rmcframe_ctrl.magcdec_d,0,16);
                    }break;
                    case 12:{
                        memset(rmcframe_ctrl.mode,0,16);
                    }break;
                    default:break;
                }
            }
            else if((gpsframe_ctrl.type[3]=='V')&&(gpsframe_ctrl.type[4]=='T')&&(gpsframe_ctrl.type[5]=='G')){
                switch(segment_index){
                    case 1:{
                        memset(vtgframe_ctrl.realdir,0,16);
                    }break;
                    case 2:{
                        memset(vtgframe_ctrl.realdir_u,0,16);
                    }break;
                    case 3:{
                        memset(vtgframe_ctrl.mgcdir,0,16);
                    }break;
                    case 4:{
                        memset(vtgframe_ctrl.mgcdir_u,0,16);
                    }break;
                    case 5:{
                        memset(vtgframe_ctrl.kspeed,0,16);
                    }break;
                    case 6:{
                        memset(vtgframe_ctrl.kspeed_u,0,16);
                    }break;
                    case 7:{
                        memset(vtgframe_ctrl.gspeed,0,16);
                    }break;
                    case 8:{
                        memset(vtgframe_ctrl.gspeed_u,0,16);
                    }break;
                    case 9:{
                        memset(vtgframe_ctrl.mode,0,16);
                    }break;
                    default:break;
                }
            }
        }
        else{
            if(segment_index==0){
                gpsframe_ctrl.type[gpsframe_ctrl.index++] = data;
                if(gpsframe_ctrl.index>=8){
                    clear_gpsframe_ctrl();
                    return;
                }
            }
            else if((gpsframe_ctrl.type[3]=='G')&&(gpsframe_ctrl.type[4]=='G')&&(gpsframe_ctrl.type[5]=='A')){
                switch(segment_index){
                    case 1:{
                        ggaframe_ctrl.utctime[ggaframe_ctrl.index++] = data;
                    }break;
                    case 2:{
                        ggaframe_ctrl.latitude[ggaframe_ctrl.index++] = data;
                    }break;
                    case 3:{
                        ggaframe_ctrl.latitude_d[ggaframe_ctrl.index++] = data;
                    }break;
                    case 4:{
                        ggaframe_ctrl.longitude[ggaframe_ctrl.index++] = data;
                    }break;
                    case 5:{
                        ggaframe_ctrl.longitude_d[ggaframe_ctrl.index++] = data;
                    }break;
                    case 6:{
                        ggaframe_ctrl.status[ggaframe_ctrl.index++] = data;
                    }break;
                    case 7:{
                        ggaframe_ctrl.satenum[ggaframe_ctrl.index++] = data;
                    }break;
                    case 8:{
                        ggaframe_ctrl.hdop[ggaframe_ctrl.index++] = data;
                    }break;
                    case 9:{
                        ggaframe_ctrl.altitude[ggaframe_ctrl.index++] = data;
                    }break;
                    case 10:{
                        ggaframe_ctrl.altitude_u[ggaframe_ctrl.index++] = data;
                    }break;
                    case 11:{
                        ggaframe_ctrl.galtitude[ggaframe_ctrl.index++] = data;
                    }break;
                    case 12:{
                        ggaframe_ctrl.galtitude_u[ggaframe_ctrl.index++] = data;
                    }break;
                    case 13:{
                        ggaframe_ctrl.difftime[ggaframe_ctrl.index++] = data;
                    }break;
                    case 14:{
                        ggaframe_ctrl.bstation[ggaframe_ctrl.index++] = data;
                    }break;
                    default:break;
                }
            }
            else if((gpsframe_ctrl.type[3]=='R')&&(gpsframe_ctrl.type[4]=='M')&&(gpsframe_ctrl.type[5]=='C')){
                switch(segment_index){
                    case 1:{
                        rmcframe_ctrl.utctime[rmcframe_ctrl.index++] = data;
                    }break;
                    case 2:{
                        rmcframe_ctrl.status[rmcframe_ctrl.index++] = data;
                    }break;
                    case 3:{
                        rmcframe_ctrl.latitude[rmcframe_ctrl.index++] = data;
                    }break;
                    case 4:{
                        rmcframe_ctrl.latitude_d[rmcframe_ctrl.index++] = data;
                    }break;
                    case 5:{
                        rmcframe_ctrl.longitude[rmcframe_ctrl.index++] = data;
                    }break;
                    case 6:{
                        rmcframe_ctrl.longitude_d[rmcframe_ctrl.index++] = data;
                    }break;
                    case 7:{
                        rmcframe_ctrl.kspeed[rmcframe_ctrl.index++] = data;
                    }break;
                    case 8:{
                        rmcframe_ctrl.kspeed_h[rmcframe_ctrl.index++] = data;
                    }break;
                    case 9:{
                        rmcframe_ctrl.utcdate[rmcframe_ctrl.index++] = data;
                    }break;
                    case 10:{
                        rmcframe_ctrl.magcdec[rmcframe_ctrl.index++] = data;
                    }break;
                    case 11:{
                        rmcframe_ctrl.magcdec_d[rmcframe_ctrl.index++] = data;
                    }break;
                    case 12:{
                        rmcframe_ctrl.mode[rmcframe_ctrl.index++] = data;
                    }break;
                    default:break;
                }
            }
            else if((gpsframe_ctrl.type[3]=='V')&&(gpsframe_ctrl.type[4]=='T')&&(gpsframe_ctrl.type[5]=='G')){
                switch(segment_index){
                    case 1:{
                        vtgframe_ctrl.realdir[vtgframe_ctrl.index++] = data;
                    }break;
                    case 2:{
                        vtgframe_ctrl.realdir_u[vtgframe_ctrl.index++] = data;
                    }break;
                    case 3:{
                        vtgframe_ctrl.mgcdir[vtgframe_ctrl.index++] = data;
                    }break;
                    case 4:{
                        vtgframe_ctrl.mgcdir_u[vtgframe_ctrl.index++] = data;
                    }break;
                    case 5:{
                        vtgframe_ctrl.kspeed[vtgframe_ctrl.index++] = data;
                    }break;
                    case 6:{
                        vtgframe_ctrl.kspeed_u[vtgframe_ctrl.index++] = data;
                    }break;
                    case 7:{
                        vtgframe_ctrl.gspeed[vtgframe_ctrl.index++] = data;
                    }break;
                    case 8:{
                        vtgframe_ctrl.gspeed_u[vtgframe_ctrl.index++] = data;
                    }break;
                    case 9:{
                        vtgframe_ctrl.mode[vtgframe_ctrl.index++] = data;
                    }break;
                    default:break;
                }
            }
            else{
                ;
            }
        }
    }
}

void gps_info_get(char *index,char *msg){
    if(xSemaphoreTake(gps_binary_semaphore, portMAX_DELAY)==pdTRUE){
        if(!strcmp(index,"valid")){
            memcpy(msg,ggaframe_ctrl.status,sizeof(ggaframe_ctrl.status));
        }
        else if(!strcmp(index,"time_buf")){
            memcpy(msg,rmcframe_ctrl.utctime,sizeof(rmcframe_ctrl.utctime));
        }
        else if(!strcmp(index,"date_buf")){
            memcpy(msg,rmcframe_ctrl.utcdate,sizeof(rmcframe_ctrl.utcdate));
        }
        else if(!strcmp(index,"lat_buf")){
            memcpy(msg,rmcframe_ctrl.latitude,sizeof(rmcframe_ctrl.latitude));
        }
        else if(!strcmp(index,"lng_buf")){
            memcpy(msg,rmcframe_ctrl.longitude,sizeof(rmcframe_ctrl.longitude));
        }
        else if(!strcmp(index,"alt_unit")){
            memcpy(msg,ggaframe_ctrl.altitude_u,sizeof(ggaframe_ctrl.altitude_u));
        }
        else if(!strcmp(index,"altitude")){
            memcpy(msg,ggaframe_ctrl.altitude,sizeof(ggaframe_ctrl.altitude));
        }
        else if(!strcmp(index,"speed")){
            memcpy(msg,vtgframe_ctrl.gspeed,sizeof(vtgframe_ctrl.gspeed));
        }
        xSemaphoreGive(gps_binary_semaphore);
    }
}
