/*
 * 测试条款：C99 7.3.6.6 —— ctanh 函数族
 *
 * 预期行为：
 *   正向测试：<complex.h> 提供 ctanh / ctanhf / ctanhl 三个函数，
 *             分别对 double complex / float complex / long double complex
 *             计算复双曲正切，结果应能编译并运行通过（用 assert 验证）。
 *   负向测试：违反约束的代码（如参数类型错误、缺少头文件声明等）
 *             应导致编译报错，统一放在 #if 0 ... #endif 中。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型
 *   [2] Description：计算复双曲正切
 *   [3] Returns：返回复双曲正切值
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* 辅助：比较两个 double 是否近似相等 */
static int deq(double a, double b)
{
    return fabs(a - b) < 1e-9;
}

int main(void)
{
    /* ---------------------------------------------------------
     * [1] Synopsis：验证三个函数原型可用，且返回类型正确
     * --------------------------------------------------------- */
    {
        double complex        (*pf)(double complex)        = ctanh;
        float  complex        (*pff)(float complex)        = ctanhf;
        long double complex   (*pfl)(long double complex)  = ctanhl;
        assert(pf  != NULL);
        assert(pff != NULL);
        assert(pfl != NULL);
    }

    /* ---------------------------------------------------------
     * [2] Description + [3] Returns：
     *     ctanh(z) = sinh(z)/cosh(z)，验证基本恒等式
     * --------------------------------------------------------- */
    {
        double complex z = 1.0 + 0.5 * I;
        double complex w = ctanh(z);

        /* 用 sinh/cosh 交叉验证 */
        double complex expected = csinh(z) / ccosh(z);
        assert(deq(creal(w), creal(expected)));
        assert(deq(cimag(w), cimag(expected)));
    }

    /* ---------------------------------------------------------
     * [2][3] 特殊值：ctanh(0) = 0
     * --------------------------------------------------------- */
    {
        double complex z = 0.0 + 0.0 * I;
        double complex w = ctanh(z);
        assert(deq(creal(w), 0.0));
        assert(deq(cimag(w), 0.0));
    }

    /* ---------------------------------------------------------
     * [2][3] 纯虚数：ctanh(i*y) = i*tan(y)
     * --------------------------------------------------------- */
    {
        double y = 0.7;
        double complex z = 0.0 + y * I;
        double complex w = ctanh(z);
        assert(deq(creal(w), 0.0));
        assert(deq(cimag(w), tan(y)));
    }

    /* ---------------------------------------------------------
     * [2][3] 实数：ctanh(x) = tanh(x)
     * --------------------------------------------------------- */
    {
        double x = 0.3;
        double complex z = x + 0.0 * I;
        double complex w = ctanh(z);
        assert(deq(creal(w), tanh(x)));
        assert(deq(cimag(w), 0.0));
    }

    /* ---------------------------------------------------------
     * [1][2][3] ctanhf：float complex 版本
     * --------------------------------------------------------- */
    {
        float complex z = 0.5f + 0.25f * I;
        float complex w = ctanhf(z);
        /* 与 double 版本比较（放宽精度） */
        double complex wd = ctanh((double)crealf(z) + (double)cimagf(z) * I);
        assert(fabsf(crealf(w) - (float)creal(wd)) < 1e-5f);
        assert(fabsf(cimagf(w) - (float)cimag(wd)) < 1e-5f);
    }

    /* ---------------------------------------------------------
     * [1][2][3] ctanhl：long double complex 版本
     * --------------------------------------------------------- */
    {
        long double complex z = 0.5L + 0.25L * I;
        long double complex w = ctanhl(z);
        double complex wd = ctanh((double)creall(z) + (double)cimagl(z) * I);
        assert(fabsl(creall(w) - (long double)creal(wd)) < 1e-9L);
        assert(fabsl(cimagl(w) - (long double)cimag(wd)) < 1e-9L);
    }

    /* ---------------------------------------------------------
     * [2][3] 奇函数性质：ctanh(-z) = -ctanh(z)
     * --------------------------------------------------------- */
    {
        double complex z = 0.8 + 0.4 * I;
        double complex a = ctanh(z);
        double complex b = ctanh(-z);
        assert(deq(creal(a), -creal(b)));
        assert(deq(cimag(a), -cimag(b)));
    }

    printf("All positive tests for C99 7.3.6.6 (ctanh) passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「[1] 参数类型必须为 double complex」：
 * 传入 double（非复数）——在严格 C99 下，double 可隐式转换为
 * double complex，因此这一条其实合法，不作为负向测试。
 * 改为测试真正违反约束的情形： */

/* 违反约束「[1] 函数名必须已声明」：
 * 未包含 <complex.h> 时使用 ctanh，编译器应报
 * "implicit declaration of function 'ctanh'" 或类似错误。
 * （此处假设未包含头文件） */
double complex bad1(double complex z)
{
    return ctanh(z);   /* 期望：error: implicit declaration of 'ctanh' */
}

/* 违反约束「[1] 参数个数必须为 1」：
 * 调用 ctanh 时传入两个实参，编译器应报
 * "too many arguments to function 'ctanh'"。 */
double complex bad2(void)
{
    double complex z = 1.0 + 1.0 * I;
    return ctanh(z, z);   /* 期望：error: too many arguments */
}

/* 违反约束「[1] 参数个数必须为 1」：
 * 调用 ctanh 时传入零个实参，编译器应报
 * "too few arguments to function 'ctanh'"。 */
double complex bad3(void)
{
    return ctanh();   /* 期望：error: too few arguments */
}

/* 违反约束「[1] 返回类型为 double complex」：
 * 将 ctanh 的返回值赋给不兼容的指针类型，编译器应报
 * "incompatible types" 或 "assignment from incompatible pointer type"。 */
void bad4(void)
{
    int *p = ctanh;   /* 期望：error: incompatible types in initialization */
    (void)p;
}

/* 违反约束「[1] 函数名 ctanh 不可被重新定义为非函数对象」：
 * 与标准库函数同名定义对象，编译器应报
 * "redefinition of 'ctanh'" 或 "conflicting types"。 */
int ctanh = 0;   /* 期望：error: conflicting types for 'ctanh' */

#endif