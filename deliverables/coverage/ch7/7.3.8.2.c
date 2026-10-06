/*
 * 测试 C99 7.3.8.2 —— cpow / cpowf / cpowl 复数幂函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 cpow/cpowf/cpowl，验证返回类型与
 *             基本幂运算语义（x^y），程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数类型错误、缺少头文件声明等）
 *             应导致编译报错，统一放在 #if 0 ... #endif 中。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型与返回类型
 *   [2] Description：计算 x^y，第一参数沿负实轴有分支切割
 *   [3] Returns：返回复数幂值
 */

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>

/* 辅助：复数近似相等比较 */
static int dcplx_eq(double complex a, double complex b, double eps)
{
    return cabs(a - b) < eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：验证三个函数原型可用，返回类型正确 */
    {
        double complex        z1 = 1.0 + 0.0 * I;
        float  complex        z2 = 1.0f + 0.0f * I;
        long double complex   z3 = 1.0L + 0.0L * I;

        double complex        r1 = cpow(z1, z1);
        float  complex        r2 = cpowf(z2, z2);
        long double complex   r3 = cpowl(z3, z3);

        /* 返回类型应为对应的复数类型 */
        assert(sizeof(r1) == sizeof(double complex));
        assert(sizeof(r2) == sizeof(float complex));
        assert(sizeof(r3) == sizeof(long double complex));

        /* 1^1 == 1 */
        assert(dcplx_eq(r1, 1.0 + 0.0 * I, 1e-12));
        assert(cabsf(r2 - (1.0f + 0.0f * I)) < 1e-5f);
        assert(cabsl(r3 - (1.0L + 0.0L * I)) < 1e-15L);
    }

    /* [2][3] Description/Returns：验证 x^y 的幂运算语义
     * 取 x = 2 + 0i, y = 3 + 0i，期望 2^3 = 8
     */
    {
        double complex x = 2.0 + 0.0 * I;
        double complex y = 3.0 + 0.0 * I;
        double complex r = cpow(x, y);
        assert(dcplx_eq(r, 8.0 + 0.0 * I, 1e-10));
    }

    /* [2][3] 验证 x^0 == 1（任意非零 x） */
    {
        double complex x = 3.0 + 4.0 * I;
        double complex y = 0.0 + 0.0 * I;
        double complex r = cpow(x, y);
        assert(dcplx_eq(r, 1.0 + 0.0 * I, 1e-10));
    }

    /* [2][3] 验证 x^1 == x */
    {
        double complex x = 3.0 + 4.0 * I;
        double complex y = 1.0 + 0.0 * I;
        double complex r = cpow(x, y);
        assert(dcplx_eq(r, x, 1e-10));
    }

    /* [2][3] 验证 i^2 == -1（纯虚数幂） */
    {
        double complex x = 0.0 + 1.0 * I;
        double complex y = 2.0 + 0.0 * I;
        double complex r = cpow(x, y);
        assert(dcplx_eq(r, -1.0 + 0.0 * I, 1e-10));
    }

    /* [2] 分支切割：第一参数沿负实轴。取 x = -1 + 0i, y = 0.5 + 0i
     * 期望 sqrt(-1) = i（主值，分支切割在负实轴）
     */
    {
        double complex x = -1.0 + 0.0 * I;
        double complex y = 0.5 + 0.0 * I;
        double complex r = cpow(x, y);
        /* 主值应为 +i */
        assert(dcplx_eq(r, 0.0 + 1.0 * I, 1e-10));
    }

    /* [2][3] 验证 cpowf 与 cpowl 的幂运算语义 */
    {
        float complex xf = 2.0f + 0.0f * I;
        float complex yf = 3.0f + 0.0f * I;
        float complex rf = cpowf(xf, yf);
        assert(cabsf(rf - (8.0f + 0.0f * I)) < 1e-4f);

        long double complex xl = 2.0L + 0.0L * I;
        long double complex yl = 3.0L + 0.0L * I;
        long double complex rl = cpowl(xl, yl);
        assert(cabsl(rl - (8.0L + 0.0L * I)) < 1e-14L);
    }

    /* [2][3] 验证一般复数幂：x = 1 + i, y = 2 + 0i，期望 (1+i)^2 = 2i */
    {
        double complex x = 1.0 + 1.0 * I;
        double complex y = 2.0 + 0.0 * I;
        double complex r = cpow(x, y);
        assert(dcplx_eq(r, 0.0 + 2.0 * I, 1e-10));
    }

    printf("All positive tests for C99 7.3.8.2 (cpow) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「参数必须为复数类型」：传入整数给 cpow，gcc -std=c99 应报错
     * （隐式转换可能允许，但若用不兼容类型如指针则必报错）
     */
    {
        int *p = 0;
        cpow(p, p);   /* 期望报错：参数类型不兼容 */
    }

    /* 违反约束「函数调用参数个数必须匹配原型」：cpow 需要 2 个参数 */
    {
        double complex x = 1.0 + 0.0 * I;
        cpow(x);      /* 期望报错：参数太少 */
    }

    /* 违反约束「函数调用参数个数必须匹配原型」：cpow 只接受 2 个参数 */
    {
        double complex x = 1.0 + 0.0 * I;
        cpow(x, x, x); /* 期望报错：参数太多 */
    }

    /* 违反约束「返回类型为复数，不能赋给不兼容类型」：
     * 将 double complex 赋给结构体指针，类型不兼容
     */
    {
        struct S { int a; } s;
        s = cpow(1.0 + 0.0 * I, 1.0 + 0.0 * I); /* 期望报错：类型不兼容 */
    }

    /* 违反约束「cpowf 参数必须为 float complex」：传入 double complex 指针 */
    {
        double complex *pd = 0;
        cpowf(pd, pd); /* 期望报错：参数类型不兼容 */
    }

#endif

    return 0;
}