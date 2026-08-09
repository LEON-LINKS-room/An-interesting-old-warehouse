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
