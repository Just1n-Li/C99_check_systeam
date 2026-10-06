/*
 * 测试条款：C99 7.3.7.1 —— cexp 函数族（cexp / cexpf / cexpl）
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 cexp/cexpf/cexpl，验证其计算复底 e 的指数
 *             （即 e^z），并检查返回类型与数值结果，程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数类型错误、缺少原型声明、返回值类型误用等）
 *             应导致编译报错；这些片段放在 #if 0 中，保证本文件仍可编译运行。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型与返回类型
 *   [2] Description：计算复底 e 的指数 e^z
 *   [3] Returns：返回复指数值
 */

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>

/* 浮点近似比较 */
static int close_enough(double a, double b, double eps)
{
    return fabs(a - b) <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：验证三个函数原型可用，返回类型分别为
     *     double complex / float complex / long double complex
     *     通过赋值给对应类型变量来静态验证返回类型兼容。 */
    double complex        z  = 1.0 + 0.0 * I;
    float complex         zf = 1.0f + 0.0f * I;
    long double complex   zl = 1.0L + 0.0L * I;

    double complex        r  = cexp(z);
    float complex         rf = cexpf(zf);
    long double complex   rl = cexpl(zl);

    /* [2][3] cexp(1 + 0i) = e^1 = e ≈ 2.718281828...，虚部为 0 */
    assert(close_enough(creal(r), 2.718281828459045, 1e-12));
    assert(close_enough(cimag(r), 0.0, 1e-12));

    /* [2][3] cexp(0 + 0i) = e^0 = 1 */
    {
        double complex z0 = 0.0 + 0.0 * I;
        double complex r0 = cexp(z0);
        assert(close_enough(creal(r0), 1.0, 1e-12));
        assert(close_enough(cimag(r0), 0.0, 1e-12));
    }

    /* [2][3] 纯虚数：cexp(0 + i*pi) = cos(pi) + i*sin(pi) = -1 + 0i */
    {
        double complex zi = 0.0 + 3.14159265358979323846 * I;
        double complex ri = cexp(zi);
        assert(close_enough(creal(ri), -1.0, 1e-12));
        assert(close_enough(cimag(ri),  0.0, 1e-12));
    }

    /* [2][3] 一般复数：cexp(a + bi) = e^a * (cos b + i sin b) */
    {
        double a = 0.5, b = 1.0;
        double complex zg = a + b * I;
        double complex rg = cexp(zg);
        double ea = exp(a);
        assert(close_enough(creal(rg), ea * cos(b), 1e-12));
        assert(close_enough(cimag(rg), ea * sin(b), 1e-12));
    }

    /* [1][3] cexpf 返回 float complex，数值与 cexp 一致（在 float 精度内） */
    assert(close_enough((double)crealf(rf), 2.718281828459045, 1e-5));
    assert(close_enough((double)cimagf(rf), 0.0, 1e-5));

    /* [1][3] cexpl 返回 long double complex，数值与 cexp 一致 */
    assert(close_enough((double)creall(rl), 2.718281828459045, 1e-12));
    assert(close_enough((double)cimagl(rl), 0.0, 1e-12));

    /* [2][3] 验证 cexp 与 exp 在实轴上的关系：cexp(x + 0i) = exp(x) */
    {
        double x = 2.0;
        double complex zx = x + 0.0 * I;
        double complex rx = cexp(zx);
        assert(close_enough(creal(rx), exp(x), 1e-12));
        assert(close_enough(cimag(rx), 0.0, 1e-12));
    }

    printf("All positive tests for C99 7.3.7.1 (cexp/cexpf/cexpl) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「实参类型必须与原型参数类型兼容」：
     * cexp 的参数类型为 double complex，传入 int 会触发隐式转换，
     * 但若传入不兼容的指针类型（如 int*），gcc -std=c99 应报错：
     *   error: incompatible type for argument 1 of 'cexp' */
    {
        int *p = 0;
        double complex bad = cexp(p);   /* 期望编译报错 */
        (void)bad;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * cexp 原型只接受 1 个参数，传 2 个应报错：
     *   error: too many arguments to function 'cexp' */
    {
        double complex bad = cexp(1.0 + 0.0 * I, 2.0);  /* 期望编译报错 */
        (void)bad;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * cexp 原型需要 1 个参数，传 0 个应报错：
     *   error: too few arguments to function 'cexp' */
    {
        double complex bad = cexp();    /* 期望编译报错 */
        (void)bad;
    }

    /* 违反约束「返回类型为 double complex，不能赋给不兼容类型」：
     * 将 double complex 直接赋给 struct 类型应报错：
     *   error: incompatible types when assigning to type 'struct S' */
    {
        struct S { int x; } s;
        s = cexp(1.0 + 0.0 * I);        /* 期望编译报错 */
        (void)s;
    }

    /* 违反约束「cexpf 返回 float complex，不能用于需要结构体的上下文」：
     * 对返回的 float complex 使用 . 成员访问（complex 不是结构体）应报错：
     *   error: request for member 'x' in something not a structure or union */
    {
        float complex bad = cexpf(1.0f + 0.0f * I);
        int v = bad.x;                  /* 期望编译报错 */
        (void)v;
    }

    /* 违反约束「cexpl 返回 long double complex，不能作为函数指针调用」：
     * 将 long double complex 当作函数调用应报错：
     *   error: called object is not a function or function pointer */
    {
        long double complex bad = cexpl(1.0L + 0.0L * I);
        bad();                          /* 期望编译报错 */
    }

#endif /* 负向测试结束 */

    return 0;
}