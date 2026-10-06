/*
 * 测试 C99 7.3.6.2 —— casinh / casinhf / casinhl 复数反双曲正弦函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 casinh/casinhf/casinhl，
 *             验证返回值满足 sinh(casinh(z)) == z（在数值误差内），
 *             并验证返回值的虚部落在 [-pi/2, +pi/2] 区间内。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），
 *             期望编译器报错。此处主要验证函数原型约束：
 *             参数必须是复数类型，不能传入不兼容类型（如结构体）。
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* 数值容差 */
#define EPS 1e-9

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：三个函数均可用，返回对应精度的复数类型 */
    double complex        z  = 0.5 + 0.3 * I;
    float complex         zf = 0.5f + 0.3f * I;
    long double complex   zl = 0.5L + 0.3L * I;

    double complex        r;
    float complex         rf;
    long double complex   rl;

    /* [1] 调用 casinh */
    r = casinh(z);
    /* [1] 调用 casinhf */
    rf = casinhf(zf);
    /* [1] 调用 casinhl */
    rl = casinhl(zl);

    /* [3] 返回值类型正确：是复数类型，可提取实部/虚部 */
    double re = creal(r);
    double im = cimag(r);
    (void)re; (void)im;

    /* [3] 验证 casinh 是 sinh 的逆运算：sinh(casinh(z)) == z */
    {
        double complex back = csinh(r);
        assert(fabs(creal(back) - creal(z)) < EPS);
        assert(fabs(cimag(back) - cimag(z)) < EPS);
    }

    /* [3] 验证 casinhf 是 sinhf 的逆运算 */
    {
        float complex back = csinhf(rf);
        assert(fabsf(crealf(back) - crealf(zf)) < 1e-5f);
        assert(fabsf(cimagf(back) - cimagf(zf)) < 1e-5f);
    }

    /* [3] 验证 casinhl 是 sinhl 的逆运算 */
    {
        long double complex back = csinhl(rl);
        assert(fabsl(creall(back) - creall(zl)) < 1e-9L);
        assert(fabsl(cimagl(back) - cimagl(zl)) < 1e-9L);
    }

    /* [3] 验证返回值虚部落在 [-pi/2, +pi/2] 区间内 */
    {
        double pi = acos(-1.0);
        double im_val = cimag(r);
        assert(im_val >= -pi / 2.0 - EPS);
        assert(im_val <=  pi / 2.0 + EPS);
    }

    /* [3] 对多个测试点验证虚部范围约束 */
    {
        double pi = acos(-1.0);
        double complex tests[] = {
            0.0 + 0.0 * I,
            1.0 + 0.0 * I,
            0.0 + 0.5 * I,
            0.0 + 2.0 * I,   /* 虚部 > 1，分支切割外侧 */
           -1.0 + 0.0 * I,
            2.0 - 3.0 * I,
           -2.0 + 3.0 * I
        };
        int n = (int)(sizeof(tests) / sizeof(tests[0]));
        int i;
        for (i = 0; i < n; i++) {
            double complex w = casinh(tests[i]);
            double imw = cimag(w);
            assert(imw >= -pi / 2.0 - EPS);
            assert(imw <=  pi / 2.0 + EPS);
            /* 逆运算验证 */
            double complex b = csinh(w);
            assert(fabs(creal(b) - creal(tests[i])) < 1e-8);
            assert(fabs(cimag(b) - cimag(tests[i])) < 1e-8);
        }
    }

    /* [2] 分支切割在虚轴 [-i, +i] 之外：对纯虚数输入 i*0.5（在切割内）
     *     函数仍应有定义并返回有限值 */
    {
        double complex w = casinh(0.0 + 0.5 * I);
        assert(isfinite(creal(w)));
        assert(isfinite(cimag(w)));
    }

    /* [2] 对纯虚数输入 i*2.0（在切割外）也应有定义 */
    {
        double complex w = casinh(0.0 + 2.0 * I);
        assert(isfinite(creal(w)));
        assert(isfinite(cimag(w)));
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「casinh 的参数必须是 double complex 类型」：
     * 传入 struct 类型，gcc -std=c99 应报错（类型不兼容）。 */
    struct S { int x; } s;
    casinh(s);

    /* 违反约束「casinhf 的参数必须是 float complex 类型」：
     * 传入 int，gcc -std=c99 应报错。 */
    casinhf(42);

    /* 违反约束「casinhl 的参数必须是 long double complex 类型」：
     * 传入 double（非 long double complex），gcc -std=c99 应报错。 */
    casinhl(1.0);

    /* 违反约束「casinh 返回 double complex，不能赋给不兼容类型」：
     * 将 double complex 赋给 struct 类型，gcc -std=c99 应报错。 */
    struct T { int y; } t;
    t = casinh(1.0 + 1.0 * I);

    /* 违反约束「casinh 需要 <complex.h> 中的原型」：
     * 若未包含头文件，隐式声明与复数类型不兼容，gcc -std=c99 应报错。
     * （此处仅作示意，实际测试时需移除 #include <complex.h>） */

#endif

    return 0;
}