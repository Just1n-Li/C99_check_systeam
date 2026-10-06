/*
 * 测试 C99 7.12.11.1 —— copysign / copysignf / copysignl
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，不影响本文件正常编译。
 *
 * 条款要点：
 *   [1] 原型：double copysign(double,double);
 *             float  copysignf(float,float);
 *             long double copysignl(long double,long double);
 *   [2] 语义：结果幅值取自 x，符号取自 y；x 为 NaN 时产生带 y 符号的 NaN；
 *             对带符号零的实现，copysign 把零的符号视为正。
 *   [3] 返回值：幅值 = |x|，符号 = y 的符号。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <float.h>

/* 判断一个 double 是否为负（含负零） */
static int is_neg_d(double v)
{
    return signbit(v) != 0;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三个函数都能被调用，返回类型正确 */
    {
        double d = copysign(3.0, -1.0);
        float  f = copysignf(3.0f, -1.0f);
        long double ld = copysignl(3.0L, -1.0L);
        assert(d == -3.0);
        assert(f == -3.0f);
        assert(ld == -3.0L);
    }

    /* [2][3] 基本语义：幅值取自 x，符号取自 y */
    {
        /* 正 x，负 y -> 负结果，幅值 = |x| */
        double r1 = copysign(5.0, -2.0);
        assert(r1 == -5.0);
        assert(is_neg_d(r1));

        /* 负 x，正 y -> 正结果，幅值 = |x| */
        double r2 = copysign(-5.0, 2.0);
        assert(r2 == 5.0);
        assert(!is_neg_d(r2));

        /* 负 x，负 y -> 负结果 */
        double r3 = copysign(-5.0, -2.0);
        assert(r3 == -5.0);
        assert(is_neg_d(r3));

        /* 正 x，正 y -> 正结果 */
        double r4 = copysign(5.0, 2.0);
        assert(r4 == 5.0);
        assert(!is_neg_d(r4));
    }

    /* [2][3] 幅值严格等于 |x|（用 fabs 对照） */
    {
        double xs[] = { 0.0, 1.5, -1.5, 123.456, -123.456, DBL_MAX, -DBL_MAX };
        double ys[] = { 1.0, -1.0, 0.0, -0.0, 7.0, -7.0 };
        size_t i, j;
        for (i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
            for (j = 0; j < sizeof(ys)/sizeof(ys[0]); ++j) {
                double r = copysign(xs[i], ys[j]);
                /* 幅值必须等于 |x| */
                assert(fabs(r) == fabs(xs[i]));
                /* 符号必须与 y 一致（y 非零时） */
                if (ys[j] != 0.0) {
                    assert(is_neg_d(r) == is_neg_d(ys[j]));
                }
            }
        }
    }

    /* [2] 带符号零：copysign 把零的符号视为正
     *     即 copysign(+0.0, -0.0) 与 copysign(-0.0, -0.0) 的幅值都视为 +0，
     *     结果符号由 y 决定。这里验证结果符号跟随 y。 */
    {
        double r1 = copysign(0.0, -1.0);
        assert(r1 == 0.0);
        assert(is_neg_d(r1));          /* 符号取自 y = -1.0 */

        double r2 = copysign(-0.0, 1.0);
        assert(r2 == 0.0);
        assert(!is_neg_d(r2));         /* 符号取自 y = +1.0 */

        /* 幅值为零（无论 x 是 +0 还是 -0），符号由 y 决定 */
        double r3 = copysign(0.0, -0.0);
        assert(r3 == 0.0);
        assert(is_neg_d(r3));          /* y 是负零，符号为负 */

        double r4 = copysign(-0.0, 0.0);
        assert(r4 == 0.0);
        assert(!is_neg_d(r4));         /* y 是正零，符号为正 */
    }

    /* [2] x 为 NaN 时，产生带 y 符号的 NaN */
    {
        double nan_val = NAN;
        double r1 = copysign(nan_val, -1.0);
        assert(isnan(r1));
        assert(is_neg_d(r1));          /* NaN 带 y 的负号 */

        double r2 = copysign(nan_val, 1.0);
        assert(isnan(r2));
        assert(!is_neg_d(r2));         /* NaN 带 y 的正号 */

        /* 负 NaN 输入同样处理 */
        double neg_nan = -NAN;
        double r3 = copysign(neg_nan, -1.0);
        assert(isnan(r3));
        assert(is_neg_d(r3));

        double r4 = copysign(neg_nan, 1.0);
        assert(isnan(r4));
        assert(!is_neg_d(r4));
    }

    /* [2] y 为 NaN 时，符号取自 y 的符号位 */
    {
        double r1 = copysign(5.0, NAN);
        assert(r1 == 5.0 || r1 == -5.0);   /* 幅值仍为 |x| */
        assert(fabs(r1) == 5.0);

        double r2 = copysign(5.0, -NAN);
        assert(fabs(r2) == 5.0);
    }

    /* [1][3] float 版本 copysignf */
    {
        float r1 = copysignf(3.5f, -1.0f);
        assert(r1 == -3.5f);
        assert(signbit(r1));

        float r2 = copysignf(-3.5f, 1.0f);
        assert(r2 == 3.5f);
        assert(!signbit(r2));

        float r3 = copysignf(0.0f, -1.0f);
        assert(r3 == 0.0f);
        assert(signbit(r3));

        float r4 = copysignf(NAN, -1.0f);
        assert(isnan(r4));
        assert(signbit(r4));
    }

    /* [1][3] long double 版本 copysignl */
    {
        long double r1 = copysignl(3.5L, -1.0L);
        assert(r1 == -3.5L);
        assert(signbit(r1));

        long double r2 = copysignl(-3.5L, 1.0L);
        assert(r2 == 3.5L);
        assert(!signbit(r2));

        long double r3 = copysignl(0.0L, -1.0L);
        assert(r3 == 0.0L);
        assert(signbit(r3));

        long double r4 = copysignl(NAN, -1.0L);
        assert(isnan(r4));
        assert(signbit(r4));
    }

    /* [2][3] 无穷大：幅值取自 x，符号取自 y */
    {
        double r1 = copysign(INFINITY, -1.0);
        assert(isinf(r1));
        assert(r1 < 0);

        double r2 = copysign(-INFINITY, 1.0);
        assert(isinf(r2));
        assert(r2 > 0);

        double r3 = copysign(INFINITY, 1.0);
        assert(isinf(r3));
        assert(r3 > 0);

        double r4 = copysign(-INFINITY, -1.0);
        assert(isinf(r4));
        assert(r4 < 0);
    }

    /* [2][3] 幂等性：copysign(copysign(x,y), y) == copysign(x,y) */
    {
        double x = 42.0, y = -1.0;
        double a = copysign(x, y);
        double b = copysign(a, y);
        assert(a == b);
    }

    /* [2][3] 符号翻转：copysign(x, y) == -copysign(x, -y)（y 非零时） */
    {
        double x = 7.25, y = 3.0;
        assert(copysign(x, y) == -copysign(x, -y));
        assert(copysign(-x, y) == -copysign(-x, -y));
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * copysign 原型要求 2 个实参，只给 1 个，gcc -std=c99 应报错
     * （error: too few arguments to function 'copysign'）。 */
    {
        double r = copysign(1.0);
        (void)r;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * copysign 给 3 个实参，gcc -std=c99 应报错
     * （error: too many arguments to function 'copysign'）。 */
    {
        double r = copysign(1.0, 2.0, 3.0);
        (void)r;
    }

    /* 违反约束「实参类型必须可转换为形参类型」：
     * 传入结构体，无法转换为 double，gcc -std=c99 应报错。 */
    {
        struct S { int x; } s;
        double r = copysign(s, 1.0);
        (void)r;
    }

    /* 违反约束「实参类型必须可转换为形参类型」：
     * 传入指针，无法隐式转换为 double，gcc -std=c99 应报错。 */
    {
        int *p = 0;
        double r = copysign(p, 1.0);
        (void)r;
    }

    /* 违反约束「copysignf 的实参必须可转换为 float」：
     * 传入结构体，gcc -std=c99 应报错。 */
    {
        struct S { int x; } s;
        float r = copysignf(s, 1.0f);
        (void)r;
    }

    /* 违反约束「copysignl 的实参必须可转换为 long double」：
     * 传入结构体，gcc -std=c99 应报错。 */
    {
        struct S { int x; } s;
        long double r = copysignl(s, 1.0L);
        (void)r;
    }

    /* 违反约束「函数返回值不可作为左值赋值」：
     * copysign 返回非左值，对其赋值应编译报错
     * （error: lvalue required as left operand of assignment）。 */
    {
        copysign(1.0, 2.0) = 3.0;
    }

    /* 违反约束「函数返回值不可取地址」：
     * copysign 返回非左值，&copysign(...) 应编译报错
     * （error: lvalue required as unary '&' operand）。 */
    {
        double *p = &copysign(1.0, 2.0);
        (void)p;
    }

    /* 违反约束「函数返回值不可自增」：
     * copysign 返回非左值，++copysign(...) 应编译报错。 */
    {
        ++copysign(1.0, 2.0);
    }

#endif /* 负向测试结束 */

    return 0;
}