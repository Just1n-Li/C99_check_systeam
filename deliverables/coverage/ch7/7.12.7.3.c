/*
 * 测试 C99 7.12.7.3 —— hypot 函数族
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错。
 *
 * 覆盖段落：
 *   [1] 原型声明：double hypot(double,double);
 *                  float  hypotf(float,float);
 *                  long double hypotl(long double,long double);
 *   [2] 语义：计算 sqrt(x^2 + y^2)，避免不必要的上溢/下溢；可能发生范围错误。
 *   [4] 返回值：返回 sqrt(x^2 + y^2)。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证三个原型均已在 <math.h> 中声明 */
static double (*p_hypot)(double, double)              = hypot;
static float  (*p_hypotf)(float, float)               = hypotf;
static long double (*p_hypotl)(long double, long double) = hypotl;

/* [4] 基本返回值：hypot(3,4) == 5 */
static void test_basic_return(void)
{
    double r = hypot(3.0, 4.0);
    assert(fabs(r - 5.0) < 1e-12);

    /* 对称性：hypot(x,y) == hypot(y,x) */
    assert(fabs(hypot(3.0, 4.0) - hypot(4.0, 3.0)) < 1e-12);

    /* 与 sqrt(x*x+y*y) 一致（在无上溢/下溢的范围内） */
    double x = 1.5, y = 2.5;
    assert(fabs(hypot(x, y) - sqrt(x * x + y * y)) < 1e-12);

    /* 零参数 */
    assert(hypot(0.0, 0.0) == 0.0);
    assert(hypot(5.0, 0.0) == 5.0);
    assert(hypot(0.0, 5.0) == 5.0);
}

/* [2] 避免不必要的上溢：x,y 很大时，x*x 会溢出，但 hypot 不应溢出 */
static void test_no_undue_overflow(void)
{
    double big = DBL_MAX / 4.0;   /* big*big 会溢出为 inf */
    double r = hypot(big, big);
    /* 结果应为 big*sqrt(2)，有限且非 inf */
    assert(!isinf(r));
    assert(r > 0.0);
    assert(fabs(r - big * sqrt(2.0)) / (big * sqrt(2.0)) < 1e-12);
}

/* [2] 避免不必要的下溢：x,y 很小时，x*x 会下溢为 0，但 hypot 应保留精度 */
static void test_no_undue_underflow(void)
{
    double tiny = DBL_MIN;        /* tiny*tiny 下溢为 0 */
    double r = hypot(tiny, tiny);
    /* 结果应为 tiny*sqrt(2)，非零 */
    assert(r > 0.0);
    assert(fabs(r - tiny * sqrt(2.0)) / (tiny * sqrt(2.0)) < 1e-12);
}

/* [1][4] float 版本 hypotf */
static void test_hypotf(void)
{
    float r = hypotf(3.0f, 4.0f);
    assert(fabsf(r - 5.0f) < 1e-5f);

    /* 避免上溢 */
    float big = FLT_MAX / 4.0f;
    float rb = hypotf(big, big);
    assert(!isinf(rb));
    assert(rb > 0.0f);
}

/* [1][4] long double 版本 hypotl */
static void test_hypotl(void)
{
    long double r = hypotl(3.0L, 4.0L);
    assert(fabsl(r - 5.0L) < 1e-12L);

    long double big = LDBL_MAX / 4.0L;
    long double rb = hypotl(big, big);
    assert(!isinf(rb));
    assert(rb > 0.0L);
}

/* [2] 范围错误：hypot(inf, nan) 等特殊值行为（C99 F.9 规定 hypot(±inf, y) = +inf） */
static void test_special_values(void)
{
    /* hypot(inf, y) = +inf */
    assert(isinf(hypot(INFINITY, 1.0)));
    assert(isinf(hypot(1.0, INFINITY)));
    assert(isinf(hypot(-INFINITY, 1.0)));

    /* hypot(nan, inf) = +inf（F.9.4.3） */
    assert(isinf(hypot(NAN, INFINITY)));

    /* hypot(nan, y) = nan */
    assert(isnan(hypot(NAN, 1.0)));
    assert(isnan(hypot(1.0, NAN)));
}

int main(void)
{
    test_basic_return();
    test_no_undue_overflow();
    test_no_undue_underflow();
    test_hypotf();
    test_hypotl();
    test_special_values();

    printf("C99 7.12.7.3 hypot: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型参数类型」：hypot 需要两个 double 实参，
 * 传入结构体类型不满足算术类型要求，gcc -std=c99 应报错。 */
struct S { int x; } s1, s2;
double bad1 = hypot(s1, s2);

/* 违反约束「[1] 参数个数」：hypot 需要 2 个实参，只给 1 个应报错。 */
double bad2 = hypot(1.0);

/* 违反约束「[1] 参数个数」：hypot 需要 2 个实参，给 3 个应报错。 */
double bad3 = hypot(1.0, 2.0, 3.0);

/* 违反约束「[1] 返回类型不可赋值」：hypot 返回 double，
 * 不能把函数调用结果当作左值赋值，应报错。 */
hypot(1.0, 2.0) = 5.0;

/* 违反约束「[1] 函数名不可被赋值」：函数指示符不是可修改左值。 */
hypot = 0;

/* 违反约束「[1] 参数类型」：hypotf 需要 float 实参，
 * 传入结构体类型应报错。 */
float bad4 = hypotf(s1, s2);

/* 违反约束「[1] 参数类型」：hypotl 需要 long double 实参，
 * 传入结构体类型应报错。 */
long double bad5 = hypotl(s1, s2);

#endif