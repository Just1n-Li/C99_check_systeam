/*
 * 测试 C99 7.3.8.3 —— csqrt / csqrtf / csqrtl 复数平方根函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 csqrt/csqrtf/csqrtl，验证返回值位于右半平面
 *             （实部 >= 0，含虚轴），且平方后等于原参数（在浮点误差内）。
 *   负向测试：违反约束的代码应导致编译报错（见 #if 0 块）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -lm test.c
 */

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>

/* 浮点近似比较辅助 */
static int close_enough(double a, double b, double eps)
{
    return fabs(a - b) <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，返回类型分别为 double complex /
     *     float complex / long double complex。此处通过实际调用验证原型可用。 */

    /* [2][3] csqrt：基本语义 —— 返回右半平面的复数平方根 */
    {
        double complex z = 4.0 + 0.0 * I;
        double complex r = csqrt(z);
        /* 右半平面：实部 >= 0 */
        assert(creal(r) >= 0.0);
        /* sqrt(4) = 2 */
        assert(close_enough(creal(r), 2.0, 1e-12));
        assert(close_enough(cimag(r), 0.0, 1e-12));
        /* 平方回原值 */
        double complex sq = r * r;
        assert(close_enough(creal(sq), creal(z), 1e-12));
        assert(close_enough(cimag(sq), cimag(z), 1e-12));
    }

    /* [2][3] csqrt：纯虚数输入，结果仍在右半平面 */
    {
        double complex z = 0.0 + 4.0 * I;
        double complex r = csqrt(z);
        assert(creal(r) >= 0.0);
        double complex sq = r * r;
        assert(close_enough(creal(sq), 0.0, 1e-12));
        assert(close_enough(cimag(sq), 4.0, 1e-12));
    }

    /* [2][3] csqrt：一般复数输入，验证平方回原值且实部非负 */
    {
        double complex z = 3.0 + 4.0 * I;
        double complex r = csqrt(z);
        assert(creal(r) >= 0.0);
        double complex sq = r * r;
        assert(close_enough(creal(sq), 3.0, 1e-12));
        assert(close_enough(cimag(sq), 4.0, 1e-12));
    }

    /* [2][3] csqrt：分支切割沿负实轴。负实轴上的点，结果取右半平面
     *     （即虚部取正号一侧，实部为 0，位于虚轴上，属于右半平面闭集）。 */
    {
        double complex z = -4.0 + 0.0 * I;
        double complex r = csqrt(z);
        /* 右半平面（含虚轴）：实部 >= 0 */
        assert(creal(r) >= 0.0);
        /* 结果应为 0 + 2i 或 0 - 2i 之一，但按右半平面约定实部为 0 */
        assert(close_enough(creal(r), 0.0, 1e-12));
        assert(close_enough(fabs(cimag(r)), 2.0, 1e-12));
        double complex sq = r * r;
        assert(close_enough(creal(sq), -4.0, 1e-12));
        assert(close_enough(cimag(sq), 0.0, 1e-12));
    }

    /* [1][2][3] csqrtf：float complex 版本 */
    {
        float complex z = 9.0f + 0.0f * I;
        float complex r = csqrtf(z);
        assert(crealf(r) >= 0.0f);
        assert(fabsf(crealf(r) - 3.0f) <= 1e-5f);
        assert(fabsf(cimagf(r) - 0.0f) <= 1e-5f);
        float complex sq = r * r;
        assert(fabsf(crealf(sq) - 9.0f) <= 1e-5f);
        assert(fabsf(cimagf(sq) - 0.0f) <= 1e-5f);
    }

    /* [1][2][3] csqrtl：long double complex 版本 */
    {
        long double complex z = 16.0L + 0.0L * I;
        long double complex r = csqrtl(z);
        assert(creall(r) >= 0.0L);
        assert(fabsl(creall(r) - 4.0L) <= 1e-12L);
        assert(fabsl(cimagl(r) - 0.0L) <= 1e-12L);
        long double complex sq = r * r;
        assert(fabsl(creall(sq) - 16.0L) <= 1e-12L);
        assert(fabsl(cimagl(sq) - 0.0L) <= 1e-12L);
    }

    /* [3] 右半平面性质：对多个随机样点验证实部 >= 0 */
    {
        double complex samples[5];
        int i;
        samples[0] = 1.0 + 1.0 * I;
        samples[1] = -1.0 + 1.0 * I;
        samples[2] = -1.0 - 1.0 * I;
        samples[3] = 1.0 - 1.0 * I;
        samples[4] = 0.0 - 9.0 * I;
        for (i = 0; i < 5; i++) {
            double complex r = csqrt(samples[i]);
            assert(creal(r) >= 0.0);
            double complex sq = r * r;
            assert(close_enough(creal(sq), creal(samples[i]), 1e-12));
            assert(close_enough(cimag(sq), cimag(samples[i]), 1e-12));
        }
    }

    printf("csqrt/csqrtf/csqrtl: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「csqrt 的参数必须为 double complex 类型（可隐式转换）」：
     * 传入不兼容的指针类型，gcc -std=c99 应报错（参数类型不匹配）。 */
    {
        double *p = 0;
        double complex r = csqrt(p);   /* 期望：error: incompatible type for argument 1 */
        (void)r;
    }

    /* 违反约束「csqrt 返回 double complex，不能直接赋给不兼容的标量类型」：
     * 将复数结果赋给 double 会丢失虚部，标准要求诊断（gcc 报错或警告，
     * 在 -Werror 下为错误）。此处演示类型不兼容的赋值。 */
    {
        double d = csqrt(4.0 + 0.0 * I);  /* 期望：error/warning: incompatible types */
        (void)d;
    }

    /* 违反约束「函数调用参数个数必须匹配原型」：
     * csqrt 原型只接受 1 个参数，传 2 个应报错。 */
    {
        double complex r = csqrt(1.0 + 0.0 * I, 2.0 + 0.0 * I);
        /* 期望：error: too many arguments to function 'csqrt' */
        (void)r;
    }

    /* 违反约束「csqrtf 参数必须为 float complex」：
     * 传入 double complex 且无隐式转换路径时（此处演示类型不匹配），
     * 期望编译器诊断。 */
    {
        double complex z = 1.0 + 0.0 * I;
        float complex r = csqrtf(z);  /* 期望：error/warning: incompatible type */
        (void)r;
    }

#endif /* 负向测试结束 */

    return 0;
}