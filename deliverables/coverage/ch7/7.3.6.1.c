/*
 * 测试 C99 7.3.6.1 —— cacosh / cacoshf / cacoshl 复数反双曲余弦函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 cacosh/cacoshf/cacoshl，验证返回值语义
 *             （分支切割在实轴 <1 处；返回值实部非负，虚部在 [-pi, +pi]）。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），期望编译器报错。
 *
 * 编译：gcc -std=c99 -Wall -lm test.c
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* 判断两个 double 近似相等 */
static int deq(double a, double b)
{
    return fabs(a - b) < 1e-9;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：三个函数均可用，返回类型分别为 double complex /
     *     float complex / long double complex。 */
    {
        double complex        z  = 2.0 + 0.0 * I;
        float complex         zf = 2.0f + 0.0f * I;
        long double complex   zl = 2.0L + 0.0L * I;

        double complex        r  = cacosh(z);
        float complex         rf = cacoshf(zf);
        long double complex   rl = cacoshl(zl);

        /* 使用返回值，避免未使用警告 */
        assert(!isnan(creal(r)));
        assert(!isnan(crealf(rf)));
        assert(!isnan(creall(rl)));
    }

    /* [2][3] 基本恒等式：cosh(cacosh(z)) == z （在分支范围内） */
    {
        double complex z = 1.5 + 0.5 * I;
        double complex w = cacosh(z);
        double complex back = ccosh(w);
        assert(deq(creal(back), creal(z)));
        assert(deq(cimag(back), cimag(z)));
    }

    /* [3] 返回值实部非负（half-strip of non-negative values along real axis） */
    {
        double complex zs[] = {
            2.0 + 0.0 * I,
            1.5 + 1.0 * I,
            1.5 - 1.0 * I,
            0.5 + 2.0 * I,
            0.5 - 2.0 * I,
            -3.0 + 0.0 * I,
            10.0 + 5.0 * I
        };
        int i;
        for (i = 0; i < (int)(sizeof(zs)/sizeof(zs[0])); i++) {
            double complex w = cacosh(zs[i]);
            assert(creal(w) >= 0.0);          /* 实部非负 */
            assert(cimag(w) >= -M_PI - 1e-9); /* 虚部下界 */
            assert(cimag(w) <=  M_PI + 1e-9); /* 虚部上界 */
        }
    }

    /* [3] 实轴上 z >= 1 时，cacosh(z) 为实数，虚部为 0 */
    {
        double complex w = cacosh(1.0 + 0.0 * I);
        assert(deq(creal(w), 0.0));   /* acosh(1) = 0 */
        assert(deq(cimag(w), 0.0));

        w = cacosh(2.0 + 0.0 * I);
        assert(deq(creal(w), acosh(2.0)));
        assert(deq(cimag(w), 0.0));
    }

    /* [2] 分支切割在实轴 <1 处：从上方/下方趋近时虚部符号相反，
     *     且实部相同（连续跨越切割线时虚部跳变）。 */
    {
        double complex w_up   = cacosh(0.0 + 1e-12 * I);
        double complex w_down = cacosh(0.0 - 1e-12 * I);
        /* 实部应相同（都趋近 acosh(0) 的实部 0） */
        assert(deq(creal(w_up), creal(w_down)));
        /* 虚部符号相反 */
        assert(cimag(w_up) > 0.0);
        assert(cimag(w_down) < 0.0);
    }

    /* [3] 边界：z = -1 时，cacosh(-1) = i*pi（虚部为 +pi，实部 0） */
    {
        double complex w = cacosh(-1.0 + 0.0 * I);
        assert(deq(creal(w), 0.0));
        assert(deq(fabs(cimag(w)), M_PI));
    }

    /* [1] float 版本与 double 版本结果一致（在精度范围内） */
    {
        float complex wf = cacoshf(2.0f + 0.0f * I);
        assert(fabsf(crealf(wf) - (float)acosh(2.0)) < 1e-5f);
        assert(fabsf(cimagf(wf)) < 1e-5f);
    }

    /* [1] long double 版本 */
    {
        long double complex wl = cacoshl(2.0L + 0.0L * I);
        assert(fabsl(creall(wl) - acoshl(2.0L)) < 1e-15L);
        assert(fabsl(cimagl(wl)) < 1e-15L);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「cacosh 的参数类型必须为 double complex」：
     * 传入结构体类型，gcc -std=c99 应报错（incompatible type / 参数类型不匹配）。 */
    struct S { int x; } s;
    cacosh(s);

    /* 违反约束「cacosh 的参数类型必须为 double complex」：
     * 传入指针类型，应报错。 */
    double *p = 0;
    cacosh(p);

    /* 违反约束「cacoshf 的参数类型必须为 float complex」：
     * 传入 double complex，应报错（类型不兼容）。 */
    double complex zd = 1.0 + 0.0 * I;
    cacoshf(zd);

    /* 违反约束「cacoshl 的参数类型必须为 long double complex」：
     * 传入 double complex，应报错。 */
    cacoshl(zd);

    /* 违反约束「cacosh 返回 double complex，不能作为结构体使用」：
     * 对返回值做非法成员访问，应报错。 */
    cacosh(zd).re;

    /* 违反约束「函数返回值不是左值」：
     * 对 cacosh 的返回值赋值，应报错（lvalue required）。 */
    cacosh(zd) = zd;

    /* 违反约束「未声明标识符」：
     * 未包含 <complex.h> 时使用 complex 关键字/类型，应报错。
     * （此处假设未包含头文件的情形，编译器应报 unknown type name 'complex'） */
    /* complex double q; */

#endif

    return 0;
}