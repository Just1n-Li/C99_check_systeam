/*
 * 测试 C99 7.15 <stdarg.h> 可变参数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] <stdarg.h> 声明一个类型和四个宏
 *   [2] 函数可被可变数量、可变类型实参调用；最右参数 parmN 特殊
 *   [3] va_list 类型；ap 可传给另一函数；被调用方用 va_arg 后
 *       调用方 ap 值不确定，须先 va_end 再引用
 *   Footnote 221: 允许创建指向 va_list 的指针并传给另一函数
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <stdarg.h> 声明类型 va_list 与四个宏 va_start/va_arg/va_end/va_copy。
 *     这里通过实际使用它们来验证其存在与可用性。 */

/* [2] 最右参数 parmN 特殊；函数以可变数量、可变类型实参调用。
 *     经典求和：parmN = n 表示后续 int 实参个数。 */
static int sum_ints(int n, ...)
{
    va_list ap;
    int total = 0;
    int i;

    va_start(ap, n);            /* [1][2] 用 parmN=n 初始化 */
    for (i = 0; i < n; ++i) {
        total += va_arg(ap, int);   /* [1] 逐个取 int */
    }
    va_end(ap);                 /* [1] 结束访问 */
    return total;
}

/* [2] 可变类型：混合 int / double / const char*，用格式串驱动。
 *     注意：float 实参在无原型/可变参数中会提升为 double（6.5.2.2），
 *     所以这里用 double 取。 */
static double sum_mixed(const char *fmt, ...)
{
    va_list ap;
    double total = 0.0;
    const char *p;

    va_start(ap, fmt);
    for (p = fmt; *p; ++p) {
        if (*p == 'i') {
            total += va_arg(ap, int);
        } else if (*p == 'd') {
            total += va_arg(ap, double);
        }
    }
    va_end(ap);
    return total;
}

/* [3] 把 ap 传给另一函数：被调用方用 va_arg 后，调用方 ap 值不确定，
 *     须先 va_end 再引用。这里演示“传值”用法：被调用方消费后，
 *     调用方不再引用 ap（直接 va_end）。 */
static int consume_two(va_list ap)
{
    int a = va_arg(ap, int);
    int b = va_arg(ap, int);
    return a + b;
}

static int caller_passes_ap(int n, ...)
{
    va_list ap;
    int r;

    va_start(ap, n);
    r = consume_two(ap);   /* [3] ap 作为实参传给另一函数 */
    va_end(ap);            /* [3] 之后不再引用 ap */
    return r;
}

/* Footnote 221: 允许创建指向 va_list 的指针并传给另一函数，
 * 原函数在该函数返回后仍可继续使用原列表。 */
static void consume_one_via_ptr(va_list *pap)
{
    (void)va_arg(*pap, int);   /* 通过指针消费一个 int */
}

static int caller_passes_va_list_ptr(int n, ...)
{
    va_list ap;
    int first, second;

    va_start(ap, n);
    first = va_arg(ap, int);          /* 先自己取一个 */
    consume_one_via_ptr(&ap);         /* Footnote 221: 传指针给另一函数 */
    second = va_arg(ap, int);         /* 返回后继续使用原列表 */
    va_end(ap);
    return first + second;
}

/* [1] va_copy 宏：复制 va_list，两个副本可独立遍历。 */
static int test_va_copy(int n, ...)
{
    va_list ap, ap2;
    int s1 = 0, s2 = 0;
    int i;

    va_start(ap, n);
    va_copy(ap2, ap);                 /* [1] 复制 */
    for (i = 0; i < n; ++i) s1 += va_arg(ap, int);
    for (i = 0; i < n; ++i) s2 += va_arg(ap2, int);
    va_end(ap2);
    va_end(ap);
    return (s1 == s2) ? s1 : -1;
}

/* [3] va_list 是对象类型：可以声明对象、可以取地址、可以 sizeof。 */
static void test_va_list_is_object_type(void)
{
    va_list ap;
    va_list *pap = &ap;               /* 对象类型可取地址 */
    assert(pap == &ap);
    assert(sizeof(va_list) > 0);      /* 对象类型有大小 */
}

int main(void)
{
    /* [1][2] 基本可变参数求和 */
    assert(sum_ints(0) == 0);
    assert(sum_ints(1, 42) == 42);
    assert(sum_ints(3, 1, 2, 3) == 6);
    assert(sum_ints(5, 10, 20, 30, 40, 50) == 150);

    /* [2] 可变类型实参 */
    assert(sum_mixed("") == 0.0);
    assert(sum_mixed("i", 7) == 7.0);
    assert(sum_mixed("id", 3, 2.5) == 5.5);
    assert(sum_mixed("did", 1.5, 2, 3.25) == 6.75);

    /* [3] ap 传给另一函数 */
    assert(caller_passes_ap(2, 11, 22) == 33);

    /* Footnote 221: 指向 va_list 的指针传给另一函数 */
    assert(caller_passes_va_list_ptr(3, 100, 200, 300) == 300);

    /* [1] va_copy */
    assert(test_va_copy(4, 1, 2, 3, 4) == 10);

    /* [3] va_list 是对象类型 */
    test_va_list_is_object_type();

    printf("C99 7.15 <stdarg.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「va_start 的第二个实参必须是 parmN（最右具名参数）」：
 * 传入一个非参数标识符，gcc -std=c99 应报错。 */
static int bad_va_start(int n, ...)
{
    va_list ap;
    int x = 0;
    va_start(ap, x);   /* 错误：parmN 必须是 n，不是局部变量 x */
    return va_arg(ap, int);
}

/* 违反约束「va_arg 的第二个实参必须是类型名」：
 * 传入一个表达式而非类型，gcc -std=c99 应报错。 */
static int bad_va_arg(int n, ...)
{
    va_list ap;
    int y = 0;
    va_start(ap, n);
    y = va_arg(ap, y);   /* 错误：第二个实参应为类型名，如 int */
    va_end(ap);
    return y;
}

/* 违反约束「va_start/va_arg/va_end/va_copy 的实参类型必须正确」：
 * va_end 需要一个 va_list 对象，传入 int 应报错。 */
static void bad_va_end(void)
{
    int not_a_va_list = 0;
    va_end(not_a_va_list);   /* 错误：实参类型不是 va_list */
}

/* 违反约束「va_copy 的两个实参都必须是 va_list 类型」：
 * 第二个实参传 int 应报错。 */
static void bad_va_copy(void)
{
    va_list ap;
    int not_a_va_list = 0;
    va_copy(ap, not_a_va_list);   /* 错误：第二个实参类型不是 va_list */
}

/* 违反约束「va_start 只能在可变参数函数中使用」：
 * 在无 ... 的函数里调用 va_start，gcc -std=c99 应报错。 */
static void bad_va_start_no_ellipsis(int n)
{
    va_list ap;
    va_start(ap, n);   /* 错误：函数没有可变参数 */
    va_end(ap);
}

#endif /* 负向测试结束 */