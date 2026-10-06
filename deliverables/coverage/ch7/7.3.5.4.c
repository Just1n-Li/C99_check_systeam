/*
 * 测试条款：C99 7.3.5.4 —— The ccos functions
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 ccos / ccosf / ccosl，
 *             验证其返回类型与数学语义（复余弦），程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如缺少 <complex.h> 声明、参数类型错误、
 *             对返回的非左值赋值等）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型与返回类型
 *   [2] Description：计算复数 z 的复余弦
 *   [3] Returns：返回复余弦值
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* 辅助：判断两个 double 是否近似相等 */
static int dbl_eq(double a, double b)
{
    return fabs(a - b) < 1e-9;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：验证三个函数原型可用，且返回类型为对应的复数类型。
     * 通过赋值给正确类型的变量来静态检查返回类型。 */
    double complex        z  = 1.0 + 2.0 * I;
    float  complex        zf = 1.0f + 2.0f * I;
    long double complex   zl = 1.0L + 2.0L * I;

    double complex        r  = ccos(z);    /* [1] double complex ccos(double complex) */
    float  complex        rf = ccosf(zf);  /* [1] float complex ccosf(float complex)  */
    long double complex   rl = ccosl(zl);  /* [1] long double complex ccosl(long double complex) */

    /* [2][3] 语义：ccos(z) = cos(x)cosh(y) - i sin(x)sinh(y)，其中 z = x + i y。
     * 用实部/虚部分量验证返回值。 */
    double x = 1.0, y = 2.0;
    double complex expected = cos(x) * cosh(y) - I * sin(x) * sinh(y);

    assert(dbl_eq(creal(r), creal(expected)));
    assert(dbl_eq(cimag(r), cimag(expected)));

    /* [2][3] 特例：z = 0 时 ccos(0) = 1 + 0i */
    double complex zero = 0.0 + 0.0 * I;
    double complex r0 = ccos(zero);
    assert(dbl_eq(creal(r0), 1.0));
    assert(dbl_eq(cimag(r0), 0.0));

    /* [2][3] 纯虚数 z = i*y 时 ccos(i y) = cosh(y)（实部），虚部为 0 */
    double complex iy = 0.0 + 3.0 * I;
    double complex riy = ccos(iy);
    assert(dbl_eq(creal(riy), cosh(3.0)));
    assert(dbl_eq(cimag(riy), 0.0));

    /* [1][3] ccosf 与 ccosl 的返回值类型与数值一致性检查 */
    assert(dbl_eq((double)crealf(rf), cos(1.0) * cosh(2.0)));
    assert(dbl_eq((double)cimagf(rf), -sin(1.0) * sinh(2.0)));
    assert(dbl_eq((double)creall(rl), cos(1.0L) * cosh(2.0L)));
    assert(dbl_eq((double)cimagl(rl), -sin(1.0L) * sinh(2.0L)));

    /* [3] 返回值可参与进一步复数运算（说明返回的是值而非左值） */
    double complex sum = ccos(z) + ccos(zero);
    assert(dbl_eq(creal(sum), creal(expected) + 1.0));

    printf("ccos  tests passed: ccos(1+2i) = %g %+gi\n",
           creal(r), cimag(r));
    printf("ccosf tests passed: ccosf(1+2i) = %g %+gi\n",
           (double)crealf(rf), (double)cimagf(rf));
    printf("ccosl tests passed: ccosl(1+2i) = %Lg %+Lgi\n",
           creall(rl), cimagl(rl));

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「函数调用参数类型必须与原型兼容」：
     * ccos 的原型为 double complex ccos(double complex)，
     * 传入不兼容的指针类型，gcc -std=c99 应报错。 */
    {
        int *p = 0;
        double complex bad = ccos(p);   /* 期望：类型不兼容错误 */
        (void)bad;
    }

    /* 违反约束「函数返回类型为 double complex，不能赋给不兼容类型」：
     * 将复数返回值赋给整型变量，应报错（或至少产生诊断）。 */
    {
        int n = ccos(1.0 + 1.0 * I);    /* 期望：类型不兼容错误 */
        (void)n;
    }

    /* 违反约束「函数返回值不是左值，不能赋值」：
     * ccos(...) 是函数调用结果，非左值，对其赋值应报错。 */
    {
        ccos(1.0 + 1.0 * I) = 0.0;      /* 期望：lvalue required 错误 */
    }

    /* 违反约束「函数返回值不是左值，不能取地址」：
     * 对函数调用结果取地址应报错。 */
    {
        double complex *q = &ccos(1.0 + 1.0 * I);  /* 期望：lvalue required 错误 */
        (void)q;
    }

    /* 违反约束「ccosf 参数为 float complex」：
     * 传入不兼容的指针类型，应报错。 */
    {
        double *d = 0;
        float complex badf = ccosf(d);  /* 期望：类型不兼容错误 */
        (void)badf;
    }

    /* 违反约束「ccosl 参数为 long double complex」：
     * 传入不兼容的指针类型，应报错。 */
    {
        float *f = 0;
        long double complex badl = ccosl(f);  /* 期望：类型不兼容错误 */
        (void)badl;
    }

#endif /* 负向测试结束 */

    return 0;
}