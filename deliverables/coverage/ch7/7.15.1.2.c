/*
 * 测试 C99 7.15.1.2 —— va_copy 宏
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（放在 #if 0 中，不参与编译）。
 *
 * 覆盖段落：
 *   [1] 概要：<stdarg.h> 提供 void va_copy(va_list dest, va_list src);
 *   [2] 描述：va_copy 把 dest 初始化为 src 的副本；dest 在未 va_end 前
 *       不得被 va_copy / va_start 重新初始化。
 *   [3] 返回值：va_copy 不返回值（是宏，不是返回值的函数）。
 */

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* 辅助函数：用 va_copy 复制 va_list，然后分别从原列表和副本中读取     */
/* ------------------------------------------------------------------ */

/* [2] 核心语义：va_copy 之后，dest 与 src 处于相同状态，
 *     从两者读取相同的序列应得到相同结果。 */
static int test_copy_basic(int n, ...)
{
    va_list src;
    va_list dst;
    int i;
    int sum_src = 0;
    int sum_dst = 0;

    va_start(src, n);

    /* 先消耗 src 的一部分，制造“当前状态” */
    for (i = 0; i < n / 2; i++) {
        sum_src += va_arg(src, int);
    }

    /* [1][2] 把 dst 初始化为 src 当前状态的副本 */
    va_copy(dst, src);

    /* 从 src 继续读剩余部分 */
    for (i = n / 2; i < n; i++) {
        sum_src += va_arg(src, int);
    }

    /* 从 dst 读同样的剩余部分，结果必须一致 */
    for (i = n / 2; i < n; i++) {
        sum_dst += va_arg(dst, int);
    }

    va_end(src);
    va_end(dst);

    assert(sum_src == sum_dst);
    return sum_src;
}

/* [2] va_copy 之后两个列表相互独立：继续推进 dst 不影响 src 的状态 */
static int test_copy_independent(int n, ...)
{
    va_list src, dst;
    int first_from_src, first_from_dst;
    int second_from_src;

    va_start(src, n);

    va_copy(dst, src);

    /* 从 dst 读一个，再从 src 读一个，应得到同一个值 */
    first_from_dst = va_arg(dst, int);
    first_from_src = va_arg(src, int);
    assert(first_from_dst == first_from_src);

    /* dst 已前进一格，src 也已前进一格；再各读一个仍应相同 */
    assert(va_arg(dst, int) == va_arg(src, int));

    /* 再验证 src 还能继续读（dst 的推进没有破坏 src） */
    second_from_src = va_arg(src, int);
    (void)second_from_src;

    va_end(src);
    va_end(dst);
    return 0;
}

/* [2] 多次 va_copy：可以从同一个 src 复制出多个副本 */
static int test_multiple_copies(int n, ...)
{
    va_list src, c1, c2, c3;
    int a, b, c;

    va_start(src, n);

    va_copy(c1, src);
    va_copy(c2, src);
    va_copy(c3, src);

    a = va_arg(c1, int);
    b = va_arg(c2, int);
    c = va_arg(c3, int);

    assert(a == b && b == c);

    va_end(c1);
    va_end(c2);
    va_end(c3);
    va_end(src);
    return a;
}

/* [2] 复制后对副本使用 va_arg 读取不同类型（double / 指针），
 *     验证副本完整保留了类型信息与状态。 */
static int test_copy_types(int dummy, ...)
{
    va_list src, dst;
    double d1, d2;
    const char *s1, *s2;
    int i1, i2;

    (void)dummy;
    va_start(src, dummy);

    va_copy(dst, src);

    d1 = va_arg(src, double);
    d2 = va_arg(dst, double);
    assert(d1 == d2);

    s1 = va_arg(src, const char *);
    s2 = va_arg(dst, const char *);
    assert(s1 == s2);
    assert(strcmp(s1, "hello") == 0);

    i1 = va_arg(src, int);
    i2 = va_arg(dst, int);
    assert(i1 == i2);

    va_end(src);
    va_end(dst);
    return 0;
}

/* [2] 合法的“重新初始化”模式：先 va_end(dest)，再 va_copy(dest, ...) */
static int test_reinit_after_va_end(int n, ...)
{
    va_list src, dst;
    int v1, v2;

    va_start(src, n);

    va_copy(dst, src);
    v1 = va_arg(dst, int);
    va_end(dst);                 /* 合法：先结束 dst */

    va_copy(dst, src);           /* 合法：va_end 之后重新初始化 */
    v2 = va_arg(dst, int);
    va_end(dst);

    assert(v1 == v2);

    va_end(src);
    return v1;
}

/* [3] va_copy 不返回值：它只能作为独立语句使用（宏展开为 void 表达式）。
 *     这里演示其“无返回值”的用法——不能出现在需要值的上下文中。 */
static int test_no_return_value(int n, ...)
{
    va_list src, dst;
    int v;

    va_start(src, n);
    va_copy(dst, src);           /* 独立语句，无返回值 */
    v = va_arg(dst, int);
    va_end(dst);
    va_end(src);
    return v;
}

int main(void)
{
    /* [1] 概要：头文件 <stdarg.h> 提供 va_copy */
    /* [2] 基本复制语义 */
    assert(test_copy_basic(6, 1, 2, 3, 4, 5, 6) == 21);

    /* [2] 副本独立性 */
    test_copy_independent(4, 10, 20, 30, 40);

    /* [2] 多次复制 */
    assert(test_multiple_copies(3, 7, 8, 9) == 7);

    /* [2] 复制保留类型信息 */
    test_copy_types(0, 3.14, "hello", 42);

    /* [2] va_end 之后重新初始化是合法的 */
    assert(test_reinit_after_va_end(2, 100, 200) == 100);

    /* [3] va_copy 无返回值 */
    assert(test_no_return_value(1, 55) == 55);

    printf("C99 7.15.1.2 va_copy: all positive tests passed.\n");
    return 0;
}

/* ================================================================== */
/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
/* ================================================================== */
#if 0

#include <stdarg.h>

/* 违反约束「va_copy 是宏，不返回值」[3]：
 * 把 va_copy 用在需要值的表达式中（例如赋值、算术运算），
 * 由于宏展开为 void 表达式，gcc -std=c99 应报错：
 *   error: void value not ignored as it ought to be
 *   （或 "invalid use of void expression"） */
void bad_use_as_value(va_list a, va_list b)
{
    int x = va_copy(a, b);       /* 错误：void 表达式不能赋给 int */
    (void)x;
}

/* 违反约束「va_copy 不返回值」[3]：
 * 在需要操作数的上下文中使用 va_copy 的结果。 */
void bad_use_in_expression(va_list a, va_list b)
{
    int y = 1 + va_copy(a, b);   /* 错误：void 表达式不能参与算术 */
    (void)y;
}

/* 违反约束「va_copy 的实参必须是 va_list 类型」[1]：
 * 传入非 va_list 类型（例如 int），gcc -std=c99 应报错。 */
void bad_argument_type(int a, int b)
{
    va_copy(a, b);               /* 错误：实参类型不是 va_list */
}

/* 违反约束「va_copy 需要两个实参」[1]：
 * 实参个数不足，gcc -std=c99 应报错。 */
void bad_too_few_args(va_list a)
{
    va_copy(a);                  /* 错误：宏参数个数不匹配 */
}

/* 违反约束「va_copy 需要两个实参」[1]：
 * 实参个数过多，gcc -std=c99 应报错。 */
void bad_too_many_args(va_list a, va_list b, va_list c)
{
    va_copy(a, b, c);            /* 错误：宏参数个数不匹配 */
}

#endif /* 负向测试结束 */