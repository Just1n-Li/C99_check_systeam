/*
 * 测试 C99 7.3.5.1 —— cacos / cacosf / cacosl 复数反余弦函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 cacos/cacosf/cacosl，验证返回值
 *             落在 [0, pi] 实部条带内，且满足 cos(cacos(z)) == z。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中），期望编译器报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数声明与类型
 *   [2] Description：分支切割在实轴 [-1,+1] 之外
 *   [3] Returns：返回值实部在 [0, pi] 区间
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

/* 辅助：判断两个 double 近似相等 */
static int dbl_eq(double a, double b, double eps)
{
    return fabs(a - b) <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：三个函数均可用，返回类型分别为 double complex /
     *     float complex / long double complex。 */
    double complex        z  = 0.5 + 0.5 * I;
    float complex         zf = 0.5f + 0.5f * I;
    long double complex   zl = 0.5L + 0.5L * I;

    double complex        r  = cacos(z);
    float complex         rf = cacosf(zf);
    long double complex   rl = cacosl(zl);

    /* [3] Returns：实部在 [0, pi] 区间内 */
    assert(creal(r)  >= 0.0  && creal(r)  <= acos(-1.0));
    assert(crealf(rf) >= 0.0f && crealf(rf) <= (float)acos(-1.0));
    assert(creall(rl) >= 0.0L && creall(rl) <= (long double)acos(-1.0));

    /* [2][3] 语义验证：cos(cacos(z)) == z（在分支切割之外） */
    {
        double complex back = ccos(r);
        assert(dbl_eq(creal(back), creal(z), 1e-9));
        assert(dbl_eq(cimag(back), cimag(z), 1e-9));
    }
    {
        float complex backf = ccosf(rf);
        assert(fabsf(crealf(backf) - crealf(zf)) <= 1e-5f);
        assert(fabsf(cimagf(backf) - cimagf(zf)) <= 1e-5f);
    }
    {
        long double complex backl = ccosl(rl);
        assert(fabsl(creall(backl) - creall(zl)) <= 1e-15L);
        assert(fabsl(cimagl(backl) - cimagl(zl)) <= 1e-15L);
    }

    /* [3] 实部边界：cacos(1) 的实部应为 0，cacos(-1) 的实部应为 pi */
    {
        double complex r1 = cacos(1.0 + 0.0 * I);
        double complex rm = cacos(-1.0 + 0.0 * I);
        assert(dbl_eq(creal(r1), 0.0, 1e-9));
        assert(dbl_eq(creal(rm), acos(-1.0), 1e-9));
    }

    /* [2] 分支切割之外：实部落在 [-1,+1] 之外的实数输入，虚部非零 */
    {
        double complex r2 = cacos(2.0 + 0.0 * I);
        /* 实部仍应在 [0, pi] 内 */
        assert(creal(r2) >= 0.0 && creal(r2) <= acos(-1.0));
        /* 2 在分支切割之外，结果虚部应为非零（纯虚方向） */
        assert(fabs(cimag(r2)) > 1e-9);
    }

    /* [2] 分支切割之内：实部在 [-1,+1] 内的实数输入，虚部应为 0 */
    {
        double complex r3 = cacos(0.0 + 0.0 * I);
        assert(dbl_eq(creal(r3), acos(0.0), 1e-9));
        assert(dbl_eq(cimag(r3), 0.0, 1e-9));
    }

    printf("cacos  tests passed: cacos(0.5+0.5i) = %g + %gi\n",
           creal(r), cimag(r));
    printf("cacosf tests passed: cacosf(0.5+0.5i) = %g + %gi\n",
           (double)crealf(rf), (double)cimagf(rf));
    printf("cacosl tests passed: cacosl(0.5+0.5i) = %Lg + %Lgi\n",
           creall(rl), cimagl(rl));

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 违反约束「cacos 的参数必须为复数类型」：
     * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
    struct S { int x; } s;
    cacos(s);

    /* 违反约束「cacos 的参数必须为复数类型」：
     * 传入指针类型，gcc -std=c99 应报错。 */
    double *p = 0;
    cacos(p);

    /* 违反约束「cacos 返回复数，不能直接赋给整数」：
     * 隐式转换复数到 int 非法，gcc -std=c99 应报错。 */
    int i = cacos(0.5 + 0.5 * I);

    /* 违反约束「cacos 需要恰好一个参数」：
     * 参数个数不匹配，gcc -std=c99 应报错。 */
    cacos();

    /* 违反约束「cacos 需要恰好一个参数」：
     * 参数过多，gcc -std=c99 应报错。 */
    cacos(1.0 + 0.0 * I, 2.0 + 0.0 * I);

    /* 违反约束「cacos 返回非左值，不能赋值」：
     * 对函数返回值赋值非法，gcc -std=c99 应报错。 */
    cacos(0.5 + 0.5 * I) = 1.0 + 1.0 * I;

    /* 违反约束「cacos 返回非左值，不能取地址」：
     * 对函数返回值取地址非法，gcc -std=c99 应报错。 */
    double complex *pc = &cacos(0.5 + 0.5 * I);
#endif

    return 0;
}