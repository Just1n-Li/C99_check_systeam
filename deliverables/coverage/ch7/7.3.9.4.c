/*
 * 测试 C99 7.3.9.4 —— cproj / cprojf / cprojl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：double complex cproj(double complex);
 *             float  complex cprojf(float complex);
 *             long double complex cprojl(long double complex);
 *   [2] 语义：把 z 投影到 Riemann 球面；有限 z 投影为 z 本身；
 *             任何含无穷部分的复数（即使另一部分是 NaN）都投影为
 *             实轴正无穷 INFINITY + I * copysign(0.0, cimag(z))。
 *   [3] 返回值：投影结果。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三个函数都能被调用，返回类型正确 */
    {
        double complex        zd = 1.0 + 2.0 * I;
        float  complex        zf = 1.0f + 2.0f * I;
        long double complex   zl = 1.0L + 2.0L * I;

        double complex        rd = cproj(zd);
        float  complex        rf = cprojf(zf);
        long double complex   rl = cprojl(zl);

        /* 仅验证可调用并得到有限结果 */
        assert(isfinite(creal(rd)) && isfinite(cimag(rd)));
        assert(isfinite(crealf(rf)) && isfinite(cimagf(rf)));
        assert(isfinite(creall(rl)) && isfinite(cimagl(rl)));
    }

    /* [2] 有限复数：投影就是它本身（实部、虚部都保持不变） */
    {
        double complex z = 3.5 - 4.25 * I;
        double complex r = cproj(z);
        assert(creal(r) == 3.5);
        assert(cimag(r) == -4.25);
    }

    /* [2] 有限复数（float 版本）：投影就是它本身 */
    {
        float complex z = 2.5f + 1.25f * I;
        float complex r = cprojf(z);
        assert(crealf(r) == 2.5f);
        assert(cimagf(r) == 1.25f);
    }

    /* [2] 有限复数（long double 版本）：投影就是它本身 */
    {
        long double complex z = -7.5L + 0.5L * I;
        long double complex r = cprojl(z);
        assert(creall(r) == -7.5L);
        assert(cimagl(r) == 0.5L);
    }

    /* [2] 实部为 +INFINITY：投影为 INFINITY + I*copysign(0.0, cimag(z)) */
    {
        double complex z = INFINITY + 5.0 * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, 5.0));   /* +0.0 */
    }

    /* [2] 实部为 -INFINITY：投影仍为实轴正无穷 */
    {
        double complex z = -INFINITY + 5.0 * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, 5.0));   /* +0.0 */
    }

    /* [2] 虚部为 +INFINITY：投影为 INFINITY + I*copysign(0.0, +inf) = +0.0 */
    {
        double complex z = 3.0 + INFINITY * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, INFINITY));  /* +0.0 */
    }

    /* [2] 虚部为 -INFINITY：投影为 INFINITY + I*copysign(0.0, -inf) = -0.0 */
    {
        double complex z = 3.0 - INFINITY * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, -INFINITY)); /* -0.0 */
        /* 验证符号位：-0.0 的符号位为 1 */
        assert(signbit(cimag(r)));
    }

    /* [2] 一个无穷部分 + 一个 NaN 部分：仍投影为实轴正无穷
     *     例如实部 +INFINITY，虚部 NaN */
    {
        double complex z = INFINITY + NAN * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        /* 虚部为 copysign(0.0, NaN)，符号取决于 NaN 的符号位，
         * 这里只要求虚部是零（+0.0 或 -0.0） */
        assert(cimag(r) == 0.0);
    }

    /* [2] 实部 NaN，虚部 +INFINITY：投影为实轴正无穷 */
    {
        double complex z = NAN + INFINITY * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, INFINITY)); /* +0.0 */
    }

    /* [2] 实部 NaN，虚部 -INFINITY：投影为实轴正无穷，虚部 -0.0 */
    {
        double complex z = NAN - INFINITY * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, -INFINITY)); /* -0.0 */
        assert(signbit(cimag(r)));
    }

    /* [2] 两个部分都是无穷：投影为实轴正无穷 */
    {
        double complex z = INFINITY + INFINITY * I;
        double complex r = cproj(z);
        assert(isinf(creal(r)) && creal(r) > 0.0);
        assert(cimag(r) == copysign(0.0, INFINITY)); /* +0.0 */
    }

    /* [2] float 版本：含无穷部分 */
    {
        float complex z = INFINITY + 1.0f * I;
        float complex r = cprojf(z);
        assert(isinf(crealf(r)) && crealf(r) > 0.0f);
        assert(cimagf(r) == copysignf(0.0f, 1.0f));
    }

    /* [2] long double 版本：含无穷部分 */
    {
        long double complex z = -INFINITY + 1.0L * I;
        long double complex r = cprojl(z);
        assert(isinf(creall(r)) && creall(r) > 0.0L);
        assert(cimagl(r) == copysignl(0.0L, 1.0L));
    }

    /* [3] 返回值语义：投影结果与直接构造的等价表达式一致 */
    {
        double complex z = INFINITY - 2.0 * I;
        double complex r = cproj(z);
        double complex expected = INFINITY + I * copysign(0.0, cimag(z));
        assert(creal(r) == creal(expected));
        assert(cimag(r) == cimag(expected));
    }

    printf("All positive tests for C99 7.3.9.4 (cproj) passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「cproj 的参数必须为 double complex 类型」：
     * 传入不兼容的指针类型，gcc -std=c99 应报错
     * （incompatible type for argument 1 of 'cproj'） */
    {
        double *p = 0;
        cproj(p);
    }

    /* 违反约束「cprojf 的参数必须为 float complex 类型」：
     * 传入 double complex，gcc -std=c99 应报错 */
    {
        double complex z = 1.0 + 2.0 * I;
        cprojf(z);
    }

    /* 违反约束「cprojl 的参数必须为 long double complex 类型」：
     * 传入 int，gcc -std=c99 应报错 */
    {
        int n = 0;
        cprojl(n);
    }

    /* 违反约束「cproj 的实参个数必须为 1」：
     * 多传一个实参，gcc -std=c99 应报错（too many arguments） */
    {
        double complex z = 1.0 + 2.0 * I;
        cproj(z, z);
    }

    /* 违反约束「cproj 的实参个数必须为 1」：
     * 不传实参，gcc -std=c99 应报错（too few arguments） */
    {
        cproj();
    }

    /* 违反约束「cproj 的返回值不能作为左值被赋值」：
     * 函数调用结果不是左值，赋值应编译报错 */
    {
        double complex z = 1.0 + 2.0 * I;
        cproj(z) = z;
    }

    /* 违反约束「cproj 的返回值不能取地址」：
     * 函数调用结果不是左值，& 应编译报错 */
    {
        double complex z = 1.0 + 2.0 * I;
        double complex *p = &cproj(z);
        (void)p;
    }

#endif
}