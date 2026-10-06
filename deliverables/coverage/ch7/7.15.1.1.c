/*
 * 测试目标：C99 7.15.1.1 —— va_arg 宏
 *
 * 预期行为：
 *   正向测试：以下使用 va_arg 的代码应能编译并正确运行（assert 全部通过）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 概要：<stdarg.h> 中声明 type va_arg(va_list ap, type);
 *   [2] 描述：ap 须由 va_start/va_copy 初始化；每次调用修改 ap；type 须为可加 * 得到对象指针的类型名；
 *       类型不匹配/无下一实参为 UB（此处只测合法情形与约束）；
 *       例外：有符号/对应无符号且值可表示；void* 与字符指针。
 *   [3] 返回：va_start 后第一次调用返回 parmN 之后的实参，依次返回剩余实参。
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1][2][3] 基本用法：依次取回 int 实参 */
static int sum_ints(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);                 /* [2] 用 va_start 初始化 ap */
    for (i = 0; i < n; i++) {
        total += va_arg(ap, int);    /* [1][3] 依次返回剩余实参 */
    }
    va_end(ap);
    return total;
}

/* [2][3] 混合类型：int, double, char* —— 注意默认实参提升 */
static void mixed(int dummy, ...)
{
    va_list ap;
    int    i;
    double d;
    char  *s;

    (void)dummy;
    va_start(ap, dummy);

    i = va_arg(ap, int);             /* [3] 第一个：parmN 之后的实参 */
    d = va_arg(ap, double);          /* [3] 第二个 */
    s = va_arg(ap, char *);          /* [3] 第三个 */

    assert(i == 42);
    assert(d == 3.5);
    assert(strcmp(s, "hello") == 0);

    va_end(ap);
}

/* [2] 例外一：有符号/对应无符号，值在两者中均可表示 */
static void signed_unsigned_exception(void)
{
    va_list ap;
    int v;

    va_start(ap, 0);                 /* 无 parmN 语义要求，仅需初始化 */
    /* 实参以 unsigned int 传入，但值 7 在 int 与 unsigned int 中均可表示，
       因此用 int 取回是条款允许的例外情形 */
    v = va_arg(ap, int);
    assert(v == 7);
    va_end(ap);
}

/* [2] 例外二：void* 与字符指针（char*）互换 */
static void void_char_ptr_exception(void)
{
    va_list ap;
    char *cp;
    void *vp;

    va_start(ap, 0);
    /* 实参为 char*，用 void* 取回 */
    vp = va_arg(ap, void *);
    assert(vp != NULL);
    assert(strcmp((char *)vp, "abc") == 0);
    va_end(ap);

    va_start(ap, 0);
    /* 实参为 void*，用 char* 取回 */
    cp = va_arg(ap, char *);
    assert(cp != NULL);
    assert(strcmp(cp, "xyz") == 0);
    va_end(ap);
}

/* [2] va_copy 初始化 ap 后使用 va_arg（验证 va_copy 也可作为初始化来源） */
static int copy_then_arg(int n, ...)
{
    va_list ap, ap2;
    int total = 0;
    int i;

    va_start(ap, n);
    va_copy(ap2, ap);                /* [2] ap2 由 va_copy 初始化 */
    for (i = 0; i < n; i++) {
        total += va_arg(ap2, int);   /* [2] 对 ap2 使用 va_arg */
    }
    va_end(ap2);
    va_end(ap);
    return total;
}

/* [2] 每次调用 va_arg 修改 ap：连续取回不同值 */
static void successive(void)
{
    va_list ap;
    int a, b, c;

    va_start(ap, 0);
    a = va_arg(ap, int);
    b = va_arg(ap, int);
    c = va_arg(ap, int);
    assert(a == 1 && b == 2 && c == 3);
    va_end(ap);
}

/* [2] type 为指针类型名（可加 * 得到对象指针）：取回 int* */
static void pointer_type(void)
{
    va_list ap;
    int x = 99;
    int *p;

    va_start(ap, 0);
    p = va_arg(ap, int *);           /* type = int*，合法 */
    assert(p == &x);
    assert(*p == 99);
    va_end(ap);
}

int main(void)
{
    /* [1][2][3] */
    assert(sum_ints(3, 10, 20, 30) == 60);
    assert(sum_ints(0) == 0);

    /* [2][3] 混合类型 */
    mixed(0, 42, 3.5, "hello");

    /* [2] 例外：有符号/无符号 */
    signed_unsigned_exception();

    /* [2] 例外：void* 与字符指针 */
    void_char_ptr_exception();

    /* [2] va_copy 初始化 */
    assert(copy_then_arg(4, 1, 2, 3, 4) == 10);

    /* [2] 连续调用修改 ap */
    successive();

    /* [2] type 为指针类型名 */
    pointer_type();

    printf("C99 7.15.1.1 va_arg: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「type 须为可加 * 得到对象指针的类型名」：
   va_arg 的第二个参数必须是类型名，不能是表达式/变量。
   gcc -std=c99 应报错：expected type name / 语法错误。 */
void bad_type_arg(va_list ap)
{
    int t = 0;
    (void)va_arg(ap, t);   /* 错误：t 不是类型名 */
}

/* 违反约束「type 须为可加 * 得到对象指针的类型名」：
   函数类型不能通过后缀 * 得到对象指针（函数指针需 (*) 形式），
   因此不能作为 va_arg 的 type。gcc -std=c99 应报错。 */
void bad_func_type(va_list ap)
{
    (void)va_arg(ap, int (int));   /* 错误：函数类型不能直接加 * 得对象指针 */
}

/* 违反约束「type 须为可加 * 得到对象指针的类型名」：
   数组类型不能通过后缀 * 得到对象指针（int[3] * 非法）。
   gcc -std=c99 应报错。 */
void bad_array_type(va_list ap)
{
    (void)va_arg(ap, int[3]);      /* 错误：数组类型不能直接加 * */
}

/* 违反约束「va_arg 的第一个参数须为 va_list 类型」：
   传入 int 而非 va_list。gcc -std=c99 应报错（类型不匹配）。 */
void bad_first_arg(void)
{
    int not_ap = 0;
    (void)va_arg(not_ap, int);     /* 错误：第一个实参不是 va_list */
}

/* 违反约束「va_arg 是宏，需 <stdarg.h> 声明」：
   未包含 <stdarg.h> 时使用 va_arg/va_list，应报错（未声明标识符）。
   注：本片段单独编译时才会触发；此处仅示意。 */
/*
void no_header(void)
{
    va_list ap;
    (void)va_arg(ap, int);
}
*/

#endif /* 负向测试结束 */