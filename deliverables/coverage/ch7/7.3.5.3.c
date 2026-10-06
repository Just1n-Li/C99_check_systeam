/*
 * 测试条款：C99 7.3.5.3 —— The catan functions
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 catan / catanf / catanl，
 *             验证返回类型、分支切割位置、值域（实部在 [-pi/2, +pi/2]），
 *             以及 catan(z) 与 atan 在实轴上的关系、与 tan 的互逆关系。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），
 *             期望编译器在 -std=c99 下报错。
 *
 * 说明：本测试仅使用 C99 标准库 <complex.h> 与 <math.h> 中规定的接口。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* 用于浮点比较的容差 */
#define EPS 1e-9

int main(void)
{
    /* ============================================================
     * 正向测试：以下代码应能编译并运行通过
     * ============================================================ */

    /* [1] Synopsis：三个函数均可声明/调用，返回类型分别为
     *     double complex / float complex / long double complex。
     *     这里通过赋值给对应类型来验证返回类型。 */
    {
        double complex        z  = 0.5 + 0.5 * I;
        float  complex        zf = 0.5f + 0.5f * I;
        long double complex   zl = 0.5L + 0.5L * I;

        double complex        r  = catan(z);
        float  complex        rf = catanf(zf);
        long double complex   rl = catanl(zl);

        /* 三个函数对同一（近似）输入应给出近似相同的结果 */
        assert(fabs(creal(r)  - (double)crealf(rf)) < 1e-5);
        assert(fabs(cimag(r)  - (double)cimagf(rf)) < 1e-5);
        assert(fabs(creal(r)  - (double)creall(rl)) < 1e-5);
        assert(fabs(cimag(r)  - (double)cimagl(rl)) < 1e-5);
    }

    /* [2] Description：catan 计算复数反正切。
     *     在实轴上（虚部为 0），catan(x) 应等于 atan(x)（实部），虚部为 0。 */
    {
        double xs[] = { 0.0, 0.5, -0.5, 1.0, -1.0, 2.0, -3.0 };
        size_t i;
        for (i = 0; i < sizeof(xs) / sizeof(xs[0]); ++i) {
            double complex w = catan(xs[i] + 0.0 * I);
            assert(fabs(creal(w) - atan(xs[i])) < EPS);
            assert(fabs(cimag(w) - 0.0) < EPS);
        }
    }

    /* [2] 分支切割：位于虚轴上区间 [-i, +i] 之外（即 |Im| > 1 的纯虚数）。
     *     对 z = i*y (y > 1)，catan(z) 的实部应为 +pi/2（取主值分支），
     *     虚部为 atanh(1/y) 的正值。这里只验证实部落在 ±pi/2 上。 */
    {
        double complex w = catan(0.0 + 2.0 * I);   /* y = 2 > 1 */
        assert(fabs(creal(w) - (M_PI / 2.0)) < 1e-9);

        double complex w2 = catan(0.0 - 2.0 * I);  /* y = -2 < -1 */
        assert(fabs(creal(w2) - (-M_PI / 2.0)) < 1e-9);
    }

    /* [3] Returns：返回值实部在区间 [-pi/2, +pi/2] 内。
     *     对一批覆盖各象限的输入验证该值域约束。 */
    {
        double complex zs[] = {
            0.0 + 0.0 * I,
            1.0 + 1.0 * I,
           -1.0 + 1.0 * I,
            1.0 - 1.0 * I,
           -1.0 - 1.0 * I,
            3.0 + 0.5 * I,
           -3.0 - 0.5 * I,
            0.0 + 5.0 * I,
            0.0 - 5.0 * I,
            10.0 + 0.0 * I
        };
        size_t i;
        for (i = 0; i < sizeof(zs) / sizeof(zs[0]); ++i) {
            double complex w = catan(zs[i]);
            double re = creal(w);
            assert(re >= -M_PI / 2.0 - EPS);
            assert(re <=  M_PI / 2.0 + EPS);
        }
    }

    /* [3] 与 tan 的互逆关系：tan(catan(z)) == z（在分支切割之外）。
     *     使用 ctan 验证。 */
    {
        double complex zs[] = {
            0.5 + 0.5 * I,
           -0.5 + 0.5 * I,
            0.5 - 0.5 * I,
           -0.5 - 0.5 * I,
            1.0 + 0.0 * I,
            0.0 + 0.5 * I
        };
        size_t i;
        for (i = 0; i < sizeof(zs) / sizeof(zs[0]); ++i) {
            double complex w = ctan(catan(zs[i]));
            assert(fabs(creal(w) - creal(zs[i])) < 1e-9);
            assert(fabs(cimag(w) - cimag(zs[i])) < 1e-9);
        }
    }

    /* [3] 特殊值：catan(0) == 0 */
    {
        double complex w = catan(0.0 + 0.0 * I);
        assert(creal(w) == 0.0);
        assert(cimag(w) == 0.0);
    }

    printf("All positive tests for C99 7.3.5.3 (catan) passed.\n");

    /* ============================================================
     * 负向测试：以下代码违反 C99 约束，应编译报错
     * ============================================================ */
#if 0

    /* 违反约束「catan 的参数必须为 double complex 类型」：
     * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
    struct S { int x; } s;
    catan(s);

    /* 违反约束「catan 的参数必须为 double complex 类型」：
     * 传入指针类型，gcc -std=c99 应报错。 */
    double *p = 0;
    catan(p);

    /* 违反约束「catan 的实参个数必须为 1」：
     * 传入两个实参，gcc -std=c99 应报错（too many arguments）。 */
    catan(1.0 + 0.0 * I, 2.0 + 0.0 * I);

    /* 违反约束「catan 的实参个数必须为 1」：
     * 不传实参，gcc -std=c99 应报错（too few arguments）。 */
    catan();

    /* 违反约束「catanf 的参数必须为 float complex 类型」：
     * 传入 double complex，gcc -std=c99 应报错（类型不兼容）。 */
    double complex dz = 1.0 + 1.0 * I;
    catanf(dz);

    /* 违反约束「catanl 的参数必须为 long double complex 类型」：
     * 传入 double complex，gcc -std=c99 应报错（类型不兼容）。 */
    catanl(dz);

    /* 违反约束「catan 的返回值不能被赋值（非左值）」：
     * 对函数调用结果赋值，gcc -std=c99 应报错（lvalue required）。 */
    catan(1.0 + 1.0 * I) = 0.0 + 0.0 * I;

    /* 违反约束「catan 的返回值不能被取地址（非左值）」：
     * 对函数调用结果取地址，gcc -std=c99 应报错（lvalue required）。 */
    double complex *pc = &catan(1.0 + 1.0 * I);

#endif /* 负向测试结束 */

    return 0;
}