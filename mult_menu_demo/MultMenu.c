#include "menuDriver.h"

// -------------------- 外部变量 --------------------
extern KEYCTRL key[4];

// -------------------- 菜单状态 --------------------
static int menu_level = 0;  // 当前层级：主=0，子=1
static int main_index = 1;  // 主菜单索引位置
static int main_cursor = 1; // 主菜单光标位置
static int sub_index = 1;   // 子菜单索引位置
static int sub_cursor = 1;  // 子菜单光标位置

// ====================================================
// 按键处理
// ====================================================
void KeyProcess(void)
{
    if (key[0].single_flag == 1)
    {
        if (menu_level != MENU_LEVEL_FUNC)
        {
            MenuMoveUp();
        }
        key[0].single_flag = 0;
    }
    if (key[1].single_flag == 1)
    {
        if (menu_level != MENU_LEVEL_FUNC)
        {
            MenuMoveDown();
        }
        key[1].single_flag = 0;
    }
    if (key[2].single_flag == 1)
    {
        if (menu_level != MENU_LEVEL_FUNC)
        {
            MenuConfirm();
        }
        key[2].single_flag = 0;
    }
    if (key[3].single_flag == 1)
    {
        MenuBack();
        key[3].single_flag = 0;
    }
}

// -------------------- 工具函数声明 --------------------
static void CursorToggle(int row, const char *text, int enable);
static void ScreenRefreshMain(void);
static void ScreenRefreshFull(void);
static void MenuDisplay(void);

// -------------------- 回调函数示例 --------------------
static void StaticHandler(int main_idx, int sub_idx)
{
}

static void DynamicHandler(int main_idx, int sub_idx)
{
}

// -------------------- 菜单定义 ------------------------
static MENU_TABLE menu[MM_NUM] = {
    {"LEVEL-1", {"SUB-11", "SUB-12", "SUB-13", "SUB-14", "SUB-15"}, 5, StaticHandler, DynamicHandler},
    {"LEVEL-2", {"SUB-21", "SUB-22", "SUB-23", "SUB-24", "SUB-25", "SUB-26"}, 6, StaticHandler, DynamicHandler},
    {"LEVEL-3", {"SUB-31", "SUB-32", "SUB-33", "SUB-34", "SUB-35", "SUB-36", "SUB-37"}, 7, StaticHandler, DynamicHandler},
    {"LEVEL-4", {"SUB-41", "SUB-42", "SUB-43", "SUB-44", "SUB-45", "SUB-46", "SUB-47", "SUB-48"}, 8, StaticHandler, DynamicHandler},
    {"LEVEL-5", {"SUB-51", "SUB-52", "SUB-53", "SUB-54", "SUB-55", "SUB-56", "SUB-57", "SUB-58", "SUB-59"}, 9, StaticHandler, DynamicHandler},
};

// ====================================================
// 动态显示更新(放在while(1)中)
// ====================================================
void FuncUpdate(void)
{
    if (menu_level == MENU_LEVEL_FUNC)
    {
        if (menu[main_index - 1].dcallback)
        {
            menu[main_index - 1].dcallback(main_index, sub_index);
        }
    }
}

// ====================================================
// 工具函数
// ====================================================
static void CursorToggle(int row, const char *text, int enable)
{
    if (text == NULL)
        return;
    int width = strlen(text) * 8;
    OLED_invert_area(8, row * 16, 16, width); // invert 切换模式
    (void)enable;                             // 仅保持接口一致
}

static void ScreenRefreshMain(void)
{
    OLED_UpdateScreen(0, 16, 48, 128);
}

static void ScreenRefreshFull(void)
{
    OLED_UpdateScreen(0, 0, 64, 128);
}

// ====================================================
// 菜单显示
// ====================================================
static void MenuDisplay(void)
{
    int main_step = 0;
    int sub_step = 0;

    switch (menu_level)
    {
    case MENU_LEVEL_MAIN: // 主菜单
        OLED_erase_area_anywhere(0, 0, 16, 128);
        OLED_ShowString(0, 0, (uint8_t *)"MainMenu", 16);

        if (MM_NUM == 0)
        {
            main_index = main_cursor = 1;
        }
        else if (MM_NUM <= main_index)
        {
            main_index = main_cursor = MM_NUM;
            if (main_cursor > CUR_AY)
            {
                main_cursor = CUR_AY;
            }
        }
        main_step = main_index - main_cursor;

        OLED_erase_area_anywhere(0, 16, 48, 128);
        for (int i = 0; i < CUR_AY && (main_step + i) < MM_NUM; i++)
        {
            OLED_ShowString(8, 16 * (i + 1), (uint8_t *)menu[main_step + i].main, 16);
        }
        break;

    case MENU_LEVEL_SUB: // 子菜单
        OLED_erase_area_anywhere(0, 0, 16, 128);
        OLED_ShowString(0, 0, (uint8_t *)menu[main_index - 1].main, 16);

        if (menu[main_index - 1].sub_count == 0)
        {
            sub_index = sub_cursor = 1;
        }
        else if (menu[main_index - 1].sub_count <= sub_index)
        {
            sub_index = sub_cursor = menu[main_index - 1].sub_count;
            if (sub_cursor > CUR_AY)
            {
                sub_cursor = CUR_AY;
            }
        }
        sub_step = sub_index - sub_cursor;

        OLED_erase_area_anywhere(0, 16, 48, 128);
        for (int i = 0; i < CUR_AY && (sub_step + i) < menu[main_index - 1].sub_count; i++)
        {
            OLED_ShowString(8, 16 * (i + 1), (uint8_t *)menu[main_index - 1].sub[sub_step + i], 16);
        }
        break;

    default:
        break;
    }
}

// ====================================================
// 菜单初始化
// ====================================================
void MenuInit(void)
{
    MenuDisplay();
    CursorToggle(1, menu[0].main, 1);
    ScreenRefreshFull();

    menu_level = 0;
    main_index = 1;
    main_cursor = 1;
    sub_index = 1;
    sub_cursor = 1;
}

// ====================================================
// 菜单操作
// ====================================================
void MenuMoveDown(void)
{
    if (menu_level == MENU_LEVEL_MAIN && main_index < MM_NUM)
    {
        CursorToggle(main_cursor, menu[main_index - 1].main, 0);
        main_index++;
        if (++main_cursor > (MM_NUM < CUR_AY ? MM_NUM : CUR_AY))
        {
            main_cursor = (MM_NUM < CUR_AY ? MM_NUM : CUR_AY);
        }
        MenuDisplay();
        CursorToggle(main_cursor, menu[main_index - 1].main, 1);
        ScreenRefreshMain();
    }
    else if (menu_level == MENU_LEVEL_SUB && sub_index < menu[main_index - 1].sub_count)
    {
        CursorToggle(sub_cursor, menu[main_index - 1].sub[sub_index - 1], 0);
        sub_index++;
        if (++sub_cursor > (menu[main_index - 1].sub_count < CUR_AY ? menu[main_index - 1].sub_count : CUR_AY))
        {
            sub_cursor = (menu[main_index - 1].sub_count < CUR_AY ? menu[main_index - 1].sub_count : CUR_AY);
        }
        MenuDisplay();
        CursorToggle(sub_cursor, menu[main_index - 1].sub[sub_index - 1], 1);
        ScreenRefreshMain();
    }
}

void MenuMoveUp(void)
{
    if (menu_level == MENU_LEVEL_MAIN && main_index > 1)
    {
        CursorToggle(main_cursor, menu[main_index - 1].main, 0);
        main_index--;
        if (--main_cursor < 1)
            main_cursor = 1;
        MenuDisplay();
        CursorToggle(main_cursor, menu[main_index - 1].main, 1);
        ScreenRefreshMain();
    }
    else if (menu_level == MENU_LEVEL_SUB && sub_index > 1)
    {
        CursorToggle(sub_cursor, menu[main_index - 1].sub[sub_index - 1], 0);
        sub_index--;
        if (--sub_cursor < 1)
            sub_cursor = 1;
        MenuDisplay();
        CursorToggle(sub_cursor, menu[main_index - 1].sub[sub_index - 1], 1);
        ScreenRefreshMain();
    }
}

void MenuConfirm(void)
{
    MENU_TABLE *m = &menu[main_index - 1];

    if (menu_level == MENU_LEVEL_MAIN)
    {
        // 进入子菜单
        CursorToggle(main_cursor, m->main, 0);
        menu_level = MENU_LEVEL_SUB;
        MenuDisplay();
        if (m->sub_count > 0)
        {
            CursorToggle(sub_cursor, m->sub[sub_index - 1], 1);
        }
        else if (m->scallback)
        {
            // 无子菜单，直接进入功能界面
            m->scallback(main_index, 0);
            menu_level = MENU_LEVEL_FUNC;
        }
        ScreenRefreshFull();
    }
    else if (menu_level == MENU_LEVEL_SUB)
    {
        // 执行子菜单
        if (m->scallback)
        {
            m->scallback(main_index, sub_index);
            menu_level = MENU_LEVEL_FUNC; // 进入功能层
        }
        else
        {
            printf("Main=%d, Sub=%d\r\n", main_index, sub_index);
        }
    }
    else if (menu_level == MENU_LEVEL_FUNC)
    {
        // 如果功能层里还要继续嵌套，可以在这里扩展
        // 目前保持不动：功能层内再次确认键可忽略或交给回调自己处理
        printf("Already in FUNC level, ignoring extra confirm.\r\n");
    }
}

void MenuBack(void)
{
    if (menu_level == MENU_LEVEL_FUNC)
    {
        if (menu[main_index - 1].sub_count == 0)
        {
            menu_level = MENU_LEVEL_MAIN;
            MenuDisplay();
            CursorToggle(main_cursor, menu[main_index - 1].main, 1);
        }
        else
        {
            menu_level = MENU_LEVEL_SUB;
            MenuDisplay();
            CursorToggle(sub_cursor, menu[main_index - 1].sub[sub_index - 1], 1);
        }
        ScreenRefreshFull();
    }
    else if (menu_level == MENU_LEVEL_SUB)
    {
        if (menu[main_index - 1].sub_count > 0)
        {
            CursorToggle(sub_cursor, menu[main_index - 1].sub[sub_index - 1], 0);
        }
        menu_level = MENU_LEVEL_MAIN;
        MenuDisplay();
        CursorToggle(main_cursor, menu[main_index - 1].main, 1);
        ScreenRefreshFull();
    }
    else if (menu_level == MENU_LEVEL_MAIN)
    {
        printf("Already at MAIN menu, cannot go back further.\r\n");
    }
}

void Menu_deal(void)
{
    FuncUpdate();
    KeyProcess();
}
