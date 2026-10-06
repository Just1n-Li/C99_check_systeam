/*
 * 测试 C99 7.12.5.3 —— atanh / atanhf / atanhl 函数
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 atanh/atanhf/atanhl，验证返回值语义
 *             （artanh x），并验证定义域 [-1,+1] 之外触发 domain error。
 *   负向测试：违反约束的代码应导致编译报错（见 #if 0 块）。
 *
 * 编译：gcc -std=c99 -Wall -lm test.c
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <errno.h>

/* 判断浮点近似相等 */
static int close_enough(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，返回类型分别为 double/float/long double */
    {
        double  (*pd)(double)       = atanh;
        float   (*pf)(float)        = atanhf;
        long double (*pl)(long double) = atanhl;
        assert(pd != NULL && pf != NULL && pl != NULL);
    }

    /* [2][3] 语义：atanh 计算 arc hyperbolic tangent（反双曲正切） */
    /* 已知值：artanh(0) = 0 */
    assert(atanh(0.0) == 0.0);
    assert(atanhf(0.0f) == 0.0f);
    assert(atanhl(0.0L) == 0.0L);

    /* artanh(0.5) = 0.5 * ln(3) ≈ 0.5493061443340548 */
    {
        double expected = 0.54930614433405484569762261846126285232374527891137;
        assert(close_enough(atanh(0.5), expected, 1e-15));
        assert(close_enough((double)atanhf(0.5f), expected, 1e-6));
        assert(close_enough((double)atanhl(0.5L), expected, 1e-15));
    }

    /* artanh(-0.5) = -artanh(0.5)（奇函数） */
    assert(close_enough(atanh(-0.5), -atanh(0.5), 1e-15));

    /* 与 tanh 互逆：atanh(tanh(y)) == y（y 在合理范围内） */
    {
        double y = 0.7;
        assert(close_enough(atanh(tanh(y)), y, 1e-14));
    }

    /* [2] 定义域：参数不在 [-1, +1] 内时发生 domain error。
     *     标准要求实现报告 domain error（通常设置 errno = EDOM 并返回 NaN）。
     *     这里只验证“确实报告了错误”，不强制具体返回值形式。 */
    {
        errno = 0;
        double r = atanh(2.0);          /* 2.0 不在 [-1,+1] */
        assert(errno == EDOM || isnan(r));

        errno = 0;
        r = atanh(-2.0);                /* -2.0 不在 [-1,+1] */
        assert(errno == EDOM || isnan(r));

        errno = 0;
        float rf = atanhf(1.5f);
        assert(errno == EDOM || isnan(rf));

        errno = 0;
        long double rl = atanhl(-1.5L);
        assert(errno == EDOM || isnan(rl));
    }

    /* [2] 边界 ±1：可能发生 range error（pole error）。
     *     标准用 “may occur”，因此不强制；仅验证调用不崩溃。 */
    {
        errno = 0;
        volatile double r1 = atanh(1.0);
        volatile double r2 = atanh(-1.0);
        (void)r1; (void)r2;
    }

    /* [3] 返回值类型检查：atanhf 返回 float，atanhl 返回 long double */
    {
        float f = atanhf(0.25f);
        long double l = atanhl(0.25L);
        assert(close_enough((double)f, atanh(0.25), 1e-6));
        assert(close_enough((double)l, atanh(0.25), 1e-15));
    }

    printf("C99 7.12.5.3 atanh/atanhf/atanhl: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「[1] 原型：参数必须为 double/float/long double 类型」：
     * 传入结构体类型，gcc -std=c99 应报 incompatible type 错误。 */
    struct S { int x; } s;
    atanh(s);

    /* 违反约束「[1] 参数个数必须为 1」：
     * 少传参数，gcc -std=c99 应报 too few arguments 错误。 */
    atanh();

    /* 违反约束「[1] 参数个数必须为 1」：
     * 多传参数，gcc -std=c99 应报 too many arguments 错误。 */
    atanh(0.5, 0.5);

    /* 违反约束「[1] 参数必须为算术类型」：
     * 传入指针，gcc -std=c99 应报 incompatible type 错误。 */
    double d = 0.5;
    atanh(&d);

    /* 违反约束「[1] 函数返回类型不可被赋值」：
     * 对函数调用结果赋值，gcc -std=c99 应报 lvalue required 错误。 */
    atanh(0.5) = 1.0;

    /* 违反约束「[1] 函数返回类型不可取地址」：
     * 对函数调用结果取地址，gcc -std=c99 应报 lvalue required 错误。 */
    double *p = &atanh(0.5);

#endif

    return 0;
}