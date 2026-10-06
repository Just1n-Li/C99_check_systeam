/*
 * 测试条款：C99 7.12.5.2 —— asinh 函数族
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，asinh / asinhf / asinhl 三个函数应能编译、
 *             链接并运行，返回 x 的反双曲正弦 arsinh x。
 *   负向测试：违反约束的代码（如参数个数错误、缺少 <math.h> 声明等）
 *             应导致编译报错。
 *
 * 说明：本条款只规定 Synopsis[1]、Description[2]、Returns[3]，
 *       没有额外的 Constraints 段落，因此负向测试针对的是
 *       “函数原型/调用形式”这类由 Synopsis 隐含的约束。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：三个函数均声明于 <math.h>，返回类型分别为
 *     double / float / long double，参数类型分别为
 *     double / float / long double。这里通过取函数指针来静态验证原型。 */
static void test_synopsis_prototypes(void)
{
    double (*pd)(double)          = asinh;
    float  (*pf)(float)           = asinhf;
    long double (*pl)(long double) = asinhl;

    assert(pd != NULL);
    assert(pf != NULL);
    assert(pl != NULL);
}

/* [2][3] Description/Returns：asinh(x) 返回 arsinh x。
 *        用已知精确值验证：arsinh(0) = 0，arsinh(sinh(y)) = y。 */
static void test_asinh_double(void)
{
    /* arsinh(0) == 0 */
    assert(asinh(0.0) == 0.0);

    /* arsinh 是 sinh 的反函数：asinh(sinh(y)) == y */
    double y = 1.0;
    double r = asinh(sinh(y));
    assert(fabs(r - y) < 1e-12);

    y = -2.5;
    r = asinh(sinh(y));
    assert(fabs(r - y) < 1e-12);

    /* 奇函数性质：asinh(-x) == -asinh(x) */
    double x = 3.0;
    assert(fabs(asinh(-x) + asinh(x)) < 1e-12);

    /* 已知值：arsinh(1) = ln(1 + sqrt(2)) ≈ 0.881373587019543 */
    assert(fabs(asinh(1.0) - 0.88137358701954302523) < 1e-12);
}

/* [1][2][3] float 版本 asinhf */
static void test_asinhf_float(void)
{
    float x = 0.0f;
    assert(asinhf(x) == 0.0f);

    float y = 1.5f;
    float r = asinhf(sinhf(y));
    assert(fabsf(r - y) < 1e-5f);

    /* 奇函数性质 */
    float a = 2.0f;
    assert(fabsf(asinhf(-a) + asinhf(a)) < 1e-5f);
}

/* [1][2][3] long double 版本 asinhl */
static void test_asinhl_long_double(void)
{
    long double x = 0.0L;
    assert(asinhl(x) == 0.0L);

    long double y = 1.25L;
    long double r = asinhl(sinhl(y));
    assert(fabsl(r - y) < 1e-15L);

    /* 奇函数性质 */
    long double a = 4.0L;
    assert(fabsl(asinhl(-a) + asinhl(a)) < 1e-15L);
}

/* [2][3] 大值/小值行为：asinh 在 |x| 很大时近似 ln(2|x|)，
 *        在 |x| 很小时近似 x。仅做宽松的合理性检查。 */
static void test_asinh_extremes(void)
{
    /* 小值：asinh(x) ≈ x */
    double small = 1e-8;
    assert(fabs(asinh(small) - small) < 1e-15);

    /* 大值：asinh(x) ≈ ln(2x)，x 很大 */
    double big = 1e10;
    double approx = log(2.0 * big);
    assert(fabs(asinh(big) - approx) < 1e-6);

    /* 负大值 */
    assert(fabs(asinh(-big) + approx) < 1e-6);
}

int main(void)
{
    test_synopsis_prototypes();
    test_asinh_double();
    test_asinhf_float();
    test_asinhl_long_double();
    test_asinh_extremes();

    printf("C99 7.12.5.2 asinh/asinhf/asinhl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「Synopsis 规定 asinh 接受 1 个 double 参数」：
 * 参数个数错误，gcc -std=c99 应报错
 *   error: too many arguments to function 'asinh' */
double bad1 = asinh(1.0, 2.0);

/* 违反约束「Synopsis 规定 asinhf 接受 1 个 float 参数」：
 * 参数个数错误，应报错 */
float bad2 = asinhf(1.0f, 2.0f);

/* 违反约束「Synopsis 规定 asinhl 接受 1 个 long double 参数」：
 * 参数个数错误，应报错 */
long double bad3 = asinhl(1.0L, 2.0L);

/* 违反约束「Synopsis 规定 asinh 返回 double」：
 * 把返回值赋给结构体类型，类型不兼容，应报错 */
struct S { int x; };
struct S bad4 = asinh(1.0);

/* 违反约束「Synopsis 规定 asinh 返回 double」：
 * 对函数调用结果取成员（非结构体），应报错 */
int bad5 = asinh(1.0).x;

/* 违反约束「Synopsis 规定 asinh 返回 double」：
 * 对函数调用结果赋值（非左值），应报错
 *   error: lvalue required as left operand of assignment */
asinh(1.0) = 2.0;

/* 违反约束「Synopsis 规定 asinhf 返回 float」：
 * 对函数调用结果赋值（非左值），应报错 */
asinhf(1.0f) = 2.0f;

/* 违反约束「Synopsis 规定 asinhl 返回 long double」：
 * 对函数调用结果赋值（非左值），应报错 */
asinhl(1.0L) = 2.0L;

/* 违反约束「Synopsis 规定函数声明于 <math.h>」：
 * 若未包含 <math.h>，则 asinh 无原型，C99 下隐式声明被禁止，
 * 应报错（或至少警告）。此处演示缺少声明的调用。 */
/* 注意：本文件已包含 <math.h>，故下面这行在 #if 0 中仅作示意。 */
double bad6 = asinh_undeclared(1.0);

#endif /* 负向测试结束 */