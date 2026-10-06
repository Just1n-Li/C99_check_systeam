/*
 * 验证 C99 6.3.2.2 void
 * 正向测试：应能编译并运行通过（验证 void 表达式求值、副作用、值/指示符丢弃）
 * 负向测试：违反约束应编译报错（void 值被使用、void 被隐式/显式转换为非 void 类型）
 */

#include <stdio.h>
#include <assert.h>

/* 辅助 void 函数，有副作用 */
static void f_void(int *p) {
    (*p)++;
}

/* 返回 int 的函数 */
static int f_int(void) {
    return 42;
}

int main(void) {
    int n = 0;

    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] void 表达式为其副作用而求值——调用 void 函数 */
    f_void(&n);
    assert(n == 1);

    /* [1] void 表达式作为表达式语句，值被丢弃 */
    (void)f_int();  /* 显式转换为 void，丢弃返回值 */

    /* [1] 任何其他类型的表达式被作为 void 表达式求值时，其值或指示符被丢弃 */
    (void)(n + 1);   /* int 表达式转换为 void，值丢弃 */
    (void)n;         /* 左值表达式转换为 void，指示符丢弃 */
    (void)f_int();   /* 函数返回值丢弃 */

    /* [1] void 表达式可显式转换为 void（唯一允许的转换） */
    (void)(void)0;

    /* [1] void 表达式求值产生副作用 */
    n = 0;
    f_void(&n);      /* 副作用：n 变为 1 */
    assert(n == 1);

    /* [1] 逗号表达式中 void 表达式作为左操作数，求值后丢弃 */
    n = 0;
    int result = (f_void(&n), n);  /* 左操作数为 void 表达式，逗号结果为 n */
    assert(n == 1);
    assert(result == 1);

    /* [1] 条件表达式中两个分支都是 void 表达式，结果为 void 类型 */
    int cond = 1;
    n = 0;
    (cond ? f_void(&n) : f_void(&n));  /* void 条件表达式，求值副作用 */
    assert(n == 1);

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* [1] 违反约束「void 表达式的值不能以任何方式使用」：
           void 函数返回值赋值给 int 变量 */
    {
        int x = f_void(&n);  /* gcc -std=c99 应报错：void 值不能赋给 int */
    }

    /* [1] 违反约束「不能对 void 表达式应用隐式转换（除转换为 void 外）」：
           void 表达式隐式转换为 int */
    {
        int x;
        x = f_void(&n);  /* gcc -std=c99 应报错：不能从 void 隐式转换为 int */
    }

    /* [1] 违反约束「不能对 void 表达式应用显式转换（除转换为 void 外）」：
           void 表达式显式转换为 int */
    {
        int x = (int)f_void(&n);  /* gcc -std=c99 应报错：不能从 void 显式转换为 int */
    }

    /* [1] 违反约束「void 表达式的值不能以任何方式使用」：
           void 表达式作为算术操作数 */
    {
        int x = 1 + f_void(&n);  /* gcc -std=c99 应报错：void 不能用于算术运算 */
    }

    /* [1] 违反约束「void 表达式的值不能以任何方式使用」：
           void 表达式作为条件判断 */
    {
        if (f_void(&n)) {  /* gcc -std=c99 应报错：void 不能用作条件 */
        }
    }

    /* [1] 违反约束「void 表达式的值不能以任何方式使用」：
           void 表达式作为函数实参（参数类型非 void） */
    {
        printf("%d\n", f_void(&n));  /* gcc -std=c99 应报错：void 不能作为实参 */
    }

    /* [1] 违反约束「不能对 void 表达式应用显式转换（除转换为 void 外）」：
           void 表达式显式转换为指针类型 */
    {
        int *p = (int *)f_void(&n);  /* gcc -std=c99 应报错：不能从 void 转换为指针 */
    }
#endif

    return 0;
}