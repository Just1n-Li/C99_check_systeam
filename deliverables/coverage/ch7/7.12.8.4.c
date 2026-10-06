/*
 * 验证 C99 条款 7.12.8.4 —— tgamma 函数族
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 tgamma / tgammaf / tgammal，
 *             验证返回类型、原型可见性、Gamma(x) 的数值语义，
 *             以及定义域/值域错误时返回 HUGE_VAL 并置 errno（实现允许）。
 *   负向测试：违反约束的代码（如参数个数错误、对非函数名调用等）
 *             应导致编译报错；这些片段放在 #if 0 中，不影响本文件编译。
 *
 * 编译：gcc -std=c99 -Wall -Wextra tgamma_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <assert.h>

/* 判断浮点近似相等 */
static int approx(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可见性：三个函数均声明于 <math.h>，返回类型分别为
     *     double / float / long double。用赋值给对应类型变量来验证。 */
    double      (*pd)(double)          = tgamma;
    float       (*pf)(float)           = tgammaf;
    long double (*pl)(long double)     = tgammal;
    assert(pd != NULL && pf != NULL && pl != NULL);

    /* [1] 通过原型调用，检查返回类型大小与预期一致 */
    {
        double      rd = tgamma(5.0);
        float       rf = tgammaf(5.0f);
        long double rl = tgammal(5.0L);
        assert(sizeof(rd) == sizeof(double));
        assert(sizeof(rf) == sizeof(float));
        assert(sizeof(rl) == sizeof(long double));
        /* 三个函数对同一输入应给出一致结果（在各自精度内） */
        assert(approx((double)rf, rd, 1e-4));
        assert(approx((double)rl, rd, 1e-12));
    }

    /* [2][3] 语义：Gamma(n) = (n-1)! 对正整数 n。
     *        Gamma(1)=1, Gamma(2)=1, Gamma(3)=2, Gamma(4)=6, Gamma(5)=24 */
    assert(approx(tgamma(1.0), 1.0, 1e-12));
    assert(approx(tgamma(2.0), 1.0, 1e-12));
    assert(approx(tgamma(3.0), 2.0, 1e-12));
    assert(approx(tgamma(4.0), 6.0, 1e-12));
    assert(approx(tgamma(5.0), 24.0, 1e-12));

    /* [2][3] 语义：Gamma(1/2) = sqrt(pi) */
    assert(approx(tgamma(0.5), sqrt(3.14159265358979323846), 1e-12));

    /* [2][3] 语义：递推关系 Gamma(x+1) = x * Gamma(x) */
    {
        double x = 3.7;
        assert(approx(tgamma(x + 1.0), x * tgamma(x), 1e-10));
    }

    /* [2] 定义域错误：x 为负整数或零时，可能发生 domain error。
     *     实现通常返回 HUGE_VAL 并置 errno = EDOM。此处只验证
     *     "返回非有限值或 HUGE_VAL" 这一可观察行为，不强制 errno。 */
    {
        errno = 0;
        double r0 = tgamma(0.0);
        /* 允许实现返回 HUGE_VAL（可能带符号）或 NaN */
        assert(isinf(r0) || isnan(r0));

        errno = 0;
        double rn = tgamma(-1.0);
        assert(isinf(rn) || isnan(rn));

        errno = 0;
        double rn2 = tgamma(-5.0);
        assert(isinf(rn2) || isnan(rn2));
    }

    /* [2] 值域错误：|x| 过大时可能发生 range error（上溢）。
     *     例如 tgamma(200.0) 远超 DBL_MAX，应返回 HUGE_VAL 或 inf。 */
    {
        errno = 0;
        double big = tgamma(200.0);
        assert(isinf(big) || big == HUGE_VAL);
    }

    /* [2] 值域错误：|x| 过小（接近 0 的正数）时 Gamma(x) 趋于 +inf，
     *     可能发生 range error。 */
    {
        errno = 0;
        double tiny = tgamma(1e-300);
        assert(isinf(tiny) || tiny > 1e300);
    }

    /* [1] float 版本：tgammaf 对 float 输入返回 float */
    assert(approx((double)tgammaf(4.0f), 6.0, 1e-4));

    /* [1] long double 版本：tgammal 对 long double 输入返回 long double */
    assert(approx((double)tgammal(4.0L), 6.0, 1e-12));

    /* [2][3] 负非整数参数：Gamma 有定义，例如 Gamma(-0.5) = -2*sqrt(pi) */
    assert(approx(tgamma(-0.5), -2.0 * sqrt(3.14159265358979323846), 1e-10));

    printf("tgamma 正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * tgamma 原型为 double tgamma(double)，调用时给 0 个或 2 个实参
     * 应报错：too few / too many arguments to function 'tgamma' */
    tgamma();
    tgamma(1.0, 2.0);

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * tgammaf 原型为 float tgammaf(float)，给 2 个实参应报错 */
    tgammaf(1.0f, 2.0f);

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * tgammal 原型为 long double tgammal(long double)，给 0 个实参应报错 */
    tgammal();

    /* 违反约束「被调用者必须是函数或函数指针」：
     * 对非函数标识符使用函数调用语法应报错 */
    int not_a_function = 0;
    not_a_function(1.0);

    /* 违反约束「函数调用结果不是左值，不能赋值」：
     * tgamma(3.0) 是右值，对其赋值应报错：
     * lvalue required as left operand of assignment */
    tgamma(3.0) = 1.0;

    /* 违反约束「函数调用结果不是左值，不能取地址」：
     * 对右值取地址应报错：lvalue required as unary '&' operand */
    double *pbad = &tgamma(3.0);

    /* 违反约束「函数调用结果不是左值，不能自增」：
     * 对右值使用 ++ 应报错：lvalue required as increment operand */
    tgamma(3.0)++;

#endif

    return 0;
}