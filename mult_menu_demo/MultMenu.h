/*******************************************************************************
MIT License

Copyright (c) 2021 LEON-LINKS-room

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

#ifndef __MENUDRIVER_H_
#define __MENUDRIVER_H_

#include "main.h"
#include "keyDriver.h"
#include "oledDriver.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "stdlib.h"

#define CUR_AY 3  // 光标范围 只有3行
#define MM_NUM 5  // 主菜单数量 要与定义的菜单menu[]成员数量一致
#define SU_NUM 10 // 次菜单数量 可修改最大支持数量

#define MENU_LEVEL_MAIN 0 // 主菜单
#define MENU_LEVEL_SUB 1  // 子菜单列表
#define MENU_LEVEL_FUNC 2 // 功能界面（回调执行）

// -------------------- 回调函数类型 --------------------
typedef void (*DynamicCallback)(int main_idx, int sub_idx); // 周期刷新
typedef void (*StaticCallback)(int main_idx, int sub_idx);  // 静态显示

// -------------------- 菜单结构 --------------------
typedef struct
{
    const char *main;          // 主菜单标题
    const char *sub[SU_NUM];   // 子菜单标题（最多10个，可改宏）
    int sub_count;             // 子菜单数量
    StaticCallback scallback;  // 功能页面动态显示（静态）
    DynamicCallback dcallback; // 功能页面动态显示（动态）
} MENU_TABLE;

void KeyProcess(void);
void FuncUpdate(void);
void MenuInit(void);
void MenuMoveUp(void);
void MenuMoveDown(void);
void MenuConfirm(void);
void MenuBack(void);
void Menu_deal(void);

#endif
