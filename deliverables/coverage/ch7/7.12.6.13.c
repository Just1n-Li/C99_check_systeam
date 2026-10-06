/*
 * 测试条款：C99 7.12.6.13  scalbn / scalbln 函数族
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 scalbn/scalbnf/scalbnl/scalbln/scalblnf/scalblnl，
 *             验证返回值等于 x * FLT_RADIX^n（在可表示范围内精确相等）。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），期望编译器报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：六个函数声明、参数类型（int n 与 long int n）
 *   [2] Description：计算 x * FLT_RADIX^n；范围错误可能发生
 *   [3] Returns：返回 x * FLT_RADIX^n
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证六个函数均可被调用，且参数类型正确。
 *     scalbn 族第二参数为 int；scalbln 族第二参数为 long int。 */
static void test_synopsis(void)
{
    double      d  = scalbn(1.0, 3);          /* double scalbn(double, int) */
    float       f  = scalbnf(1.0f, 3);        /* float  scalbnf(float, int) */
    long double ld = scalbnl(1.0L, 3);        /* long double scalbnl(long double, int) */

    double      d2 = scalbln(1.0, 3L);        /* double scalbln(double, long int) */
    float       f2 = scalblnf(1.0f, 3L);      /* float  scalblnf(float, long int) */
    long double ld2= scalblnl(1.0L, 3L);      /* long double scalblnl(long double, long int) */

    /* 仅验证调用成功、结果有限 */
    assert(isfinite(d) && isfinite(f) && isfinite(ld));
    assert(isfinite(d2) && isfinite(f2) && isfinite(ld2));
}

/* [2][3] 核心语义：scalbn(x, n) == x * FLT_RADIX^n
 *        在结果可精确表示时，应严格相等。 */
static void test_semantics_exact(void)
{
    /* FLT_RADIX 通常为 2；用 ldexp 作为等价参照（ldexp 也是 x*2^n，
     * 但这里我们直接按 FLT_RADIX 计算，避免依赖 ldexp 的实现细节）。 */
    int i;

    /* 对若干整数 n，验证 scalbn(1.0, n) == FLT_RADIX^n（用循环累乘得到期望值） */
    for (i = -10; i <= 10; ++i) {
        double expected = 1.0;
        int k;
        if (i >= 0) {
            for (k = 0; k < i; ++k)  expected *= (double)FLT_RADIX;
        } else {
            for (k = 0; k < -i; ++k) expected /= (double)FLT_RADIX;
        }
        double got = scalbn(1.0, i);
        assert(got == expected);   /* 精确相等（2 的幂可精确表示） */
    }

    /* 一般 x：scalbn(x, n) == x * FLT_RADIX^n，取可精确表示的组合 */
    {
        double x = 3.0;            /* 3.0 可精确表示 */
        int n = 4;
        double expected = x;
        int k;
        for (k = 0; k < n; ++k) expected *= (double)FLT_RADIX;
        assert(scalbn(x, n) == expected);
    }

    /* n == 0 时返回 x 本身 */
    assert(scalbn(1.5, 0) == 1.5);
    assert(scalbn(-2.25, 0) == -2.25);

    /* 负 n：scalbn(x, -n) == x / FLT_RADIX^n */
    {
        double x = 8.0;
        int n = -3;
        double expected = x;
        int k;
        for (k = 0; k < -n; ++k) expected /= (double)FLT_RADIX;
        assert(scalbn(x, n) == expected);   /* 8 / 2^3 = 1.0 */
    }
}

/* [2][3] 各类型版本语义一致 */
static void test_semantics_types(void)
{
    /* float 版本 */
    {
        float x = 1.0f;
        int n = 5;
        float expected = x;
        int k;
        for (k = 0; k < n; ++k) expected *= (float)FLT_RADIX;
        assert(scalbnf(x, n) == expected);
    }
    /* long double 版本 */
    {
        long double x = 1.0L;
        int n = 5;
        long double expected = x;
        int k;
        for (k = 0; k < n; ++k) expected *= (long double)FLT_RADIX;
        assert(scalbnl(x, n) == expected);
    }
    /* scalbln 族：第二参数为 long int，语义相同 */
    {
        double x = 1.0;
        long int n = 6L;
        double expected = x;
        long int k;
        for (k = 0; k < n; ++k) expected *= (double)FLT_RADIX;
        assert(scalbln(x, n) == expected);
    }
    {
        float x = 1.0f;
        long int n = 6L;
        float expected = x;
        long int k;
        for (k = 0; k < n; ++k) expected *= (float)FLT_RADIX;
        assert(scalblnf(x, n) == expected);
    }
    {
        long double x = 1.0L;
        long int n = 6L;
        long double expected = x;
        long int k;
        for (k = 0; k < n; ++k) expected *= (long double)FLT_RADIX;
        assert(scalblnl(x, n) == expected);
    }
}

/* [2] 范围错误：极大 n 可能触发范围错误（返回 HUGE_VAL 或 0）。
 *     这里只验证函数被调用且结果符合「溢出/下溢」的合理形态，
 *     不把 UB 当作测试目标。 */
static void test_range(void)
{
    double big = scalbn(1.0, 100000);   /* 极大正指数，通常溢出 */
    double small = scalbn(1.0, -100000);/* 极大负指数，通常下溢为 0 */

    /* 溢出时返回 HUGE_VAL（正无穷或极大值），下溢时返回 0 或极小值。
     * 只做宽松检查：结果非 NaN，且 big 不小于 small。 */
    assert(!isnan(big));
    assert(!isnan(small));
    assert(big >= small);
}

int main(void)
{
    test_synopsis();
    test_semantics_exact();
    test_semantics_types();
    test_range();

    printf("C99 7.12.6.13 scalbn/scalbln: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参类型必须与原型兼容」：
 * scalbn 原型为 double scalbn(double, int)，
 * 第二参数必须是 int 兼容类型；传入结构体应报错。 */
struct S { int x; } s;
double r1 = scalbn(1.0, s);          /* 期望：error: incompatible type for argument 2 */

/* 违反约束「实参个数必须与原型一致」：
 * scalbn 需要 2 个实参，只给 1 个应报错。 */
double r2 = scalbn(1.0);             /* 期望：error: too few arguments to function 'scalbn' */

/* 违反约束「实参个数必须与原型一致」：
 * scalbn 需要 2 个实参，给 3 个应报错。 */
double r3 = scalbn(1.0, 2, 3);       /* 期望：error: too many arguments to function 'scalbn' */

/* 违反约束「函数必须已声明」：
 * 未包含 <math.h> 且未声明 scalbn 时调用，C99 禁止隐式函数声明。 */
double r4 = scalbn_undeclared(1.0, 2); /* 期望：error: implicit declaration of function */

/* 违反约束「赋值目标必须是可修改左值」：
 * 函数调用结果不是左值，不能赋值。 */
scalbn(1.0, 2) = 5.0;                /* 期望：error: lvalue required as left operand of assignment */

/* 违反约束「取地址操作数必须是左值」：
 * 函数调用结果不是左值，不能取地址。 */
double *p = &scalbn(1.0, 2);         /* 期望：error: lvalue required as unary '&' operand */

#endif