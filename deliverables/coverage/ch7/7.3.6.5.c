/*
 * 测试 C99 7.3.6.5 —— csinh 函数族（csinh / csinhf / csinhl）
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 csinh/csinhf/csinhl 计算复数双曲正弦，
 *             结果与数学定义 sinh(z) = (e^z - e^-z)/2 一致，程序编译并运行通过。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型、参数类型、返回类型
 *   [2] Description：计算复数双曲正弦
 *   [3] Returns：返回复数双曲正弦值
 */

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <assert.h>

/* 复数近似比较辅助函数 */
static int dcplx_close(double complex a, double complex b, double eps)
{
    double dr = creal(a) - creal(b);
    double di = cimag(a) - cimag(b);
    return (fabs(dr) < eps) && (fabs(di) < eps);
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证三个函数原型可用，参数/返回类型符合声明 */
static void test_synopsis(void)
{
    double complex        zd = 1.0 + 2.0 * I;
    float  complex        zf = 1.0f + 2.0f * I;
    long double complex   zl = 1.0L + 2.0L * I;

    double complex        rd;
    float  complex        rf;
    long double complex   rl;

    /* 调用三个函数，检查返回类型可赋值给对应复数类型 */
    rd = csinh(zd);
    rf = csinhf(zf);
    rl = csinhl(zl);

    /* 使用结果，避免未使用变量警告 */
    assert(isfinite(creal(rd)) && isfinite(cimag(rd)));
    assert(isfinite(crealf(rf)) && isfinite(cimagf(rf)));
    assert(isfinite(creall(rl)) && isfinite(cimagl(rl)));
}

/* [2][3] Description/Returns：csinh(z) 应等于 (e^z - e^-z)/2 */
static void test_definition(void)
{
    double complex z = 0.5 + 0.75 * I;
    double complex expected = (cexp(z) - cexp(-z)) / 2.0;
    double complex got = csinh(z);

    assert(dcplx_close(got, expected, 1e-12));
}

/* [2][3] 特殊值：csinh(0) = 0 */
static void test_zero(void)
{
    double complex z = 0.0 + 0.0 * I;
    double complex r = csinh(z);
    assert(dcplx_close(r, 0.0 + 0.0 * I, 1e-15));
}

/* [2][3] 纯虚数：sinh(i*y) = i*sin(y) */
static void test_pure_imaginary(void)
{
    double y = 0.9;
    double complex z = 0.0 + y * I;
    double complex r = csinh(z);
    double complex expected = 0.0 + sin(y) * I;
    assert(dcplx_close(r, expected, 1e-12));
}

/* [2][3] 纯实数：sinh(x) 为实数 */
static void test_pure_real(void)
{
    double x = 1.3;
    double complex z = x + 0.0 * I;
    double complex r = csinh(z);
    double complex expected = sinh(x) + 0.0 * I;
    assert(dcplx_close(r, expected, 1e-12));
}

/* [2][3] 奇函数性质：csinh(-z) = -csinh(z) */
static void test_odd(void)
{
    double complex z = 0.4 - 1.1 * I;
    double complex a = csinh(-z);
    double complex b = -csinh(z);
    assert(dcplx_close(a, b, 1e-12));
}

/* [1][2][3] float 版本与 double 版本一致性 */
static void test_float_consistency(void)
{
    float complex zf = 0.3f + 0.6f * I;
    float complex rf = csinhf(zf);
    double complex rd = csinh((double)crealf(zf) + (double)cimagf(zf) * I);
    assert(fabsf(crealf(rf) - (float)creal(rd)) < 1e-5f);
    assert(fabsf(cimagf(rf) - (float)cimag(rd)) < 1e-5f);
}

/* [1][2][3] long double 版本与 double 版本一致性 */
static void test_longdouble_consistency(void)
{
    long double complex zl = 0.3L + 0.6L * I;
    long double complex rl = csinhl(zl);
    double complex rd = csinh(0.3 + 0.6 * I);
    assert(fabsl(creall(rl) - (long double)creal(rd)) < 1e-12L);
    assert(fabsl(cimagl(rl) - (long double)cimag(rd)) < 1e-12L);
}

int main(void)
{
    test_synopsis();
    test_definition();
    test_zero();
    test_pure_imaginary();
    test_pure_real();
    test_odd();
    test_float_consistency();
    test_longdouble_consistency();

    printf("C99 7.3.6.5 csinh functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 参数类型必须为 double complex」：
 * 传入结构体类型，无法隐式转换为 double complex，gcc -std=c99 应报错。 */
struct NotComplex { double re, im; };
struct NotComplex nc = { 1.0, 2.0 };
double complex bad1 = csinh(nc);

/* 违反约束「[1] 参数类型必须为 float complex」：
 * 传入指针类型，无法隐式转换为 float complex，应报错。 */
float f = 1.0f;
float complex bad2 = csinhf(&f);

/* 违反约束「[1] 参数类型必须为 long double complex」：
 * 传入字符串字面量（char*），无法转换为 long double complex，应报错。 */
long double complex bad3 = csinhl("hello");

/* 违反约束「[1] 函数必须已声明」：
 * 未包含 <complex.h> 且未声明 csinh 时调用，C99 中隐式声明返回 int，
 * 但赋值给 double complex 且参数类型不匹配，应报错（或至少诊断）。
 * 这里显式用错误原型调用以触发约束违反。 */
double complex bad4 = csinh(1, 2);   /* 参数个数与原型不符 */

/* 违反约束「[1] 返回类型为 double complex，不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
csinh(1.0 + 0.0 * I) = 0.0 + 0.0 * I;

#endif