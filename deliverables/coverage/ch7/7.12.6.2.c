/*
 * 测试条款：C99 7.12.6.2 —— exp2 函数族
 *   [1] 原型：double exp2(double); float exp2f(float); long double exp2l(long double);
 *   [2] 语义：计算以 2 为底的指数 2^x；当 |x| 过大时发生 range error。
 *   [3] 返回值：返回 2^x。
 *
 * 预期行为：
 *   正向测试：应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，不影响本文件正常编译。
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <errno.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 三个函数的原型必须存在，且返回类型分别为 double / float / long double。
 *     用函数指针类型来静态验证原型签名。 */
static double (*p_exp2)(double)          = exp2;
static float  (*p_exp2f)(float)          = exp2f;
static long double (*p_exp2l)(long double) = exp2l;

static void test_prototypes(void)
{
    /* [1] 原型可用性：能取地址即说明声明存在且签名匹配 */
    assert(p_exp2  != NULL);
    assert(p_exp2f != NULL);
    assert(p_exp2l != NULL);
    printf("[1] prototypes OK\n");
}

static void test_semantics_basic(void)
{
    /* [2][3] 2^0 = 1, 2^1 = 2, 2^2 = 4, 2^3 = 8, 2^10 = 1024 */
    assert(exp2(0.0)  == 1.0);
    assert(exp2(1.0)  == 2.0);
    assert(exp2(2.0)  == 4.0);
    assert(exp2(3.0)  == 8.0);
    assert(exp2(10.0) == 1024.0);

    /* [2][3] 负指数：2^-1 = 0.5, 2^-2 = 0.25 */
    assert(exp2(-1.0) == 0.5);
    assert(exp2(-2.0) == 0.25);

    /* [2][3] 与 pow(2,x) 一致（在可精确表示处比较） */
    assert(exp2(4.0) == pow(2.0, 4.0));
    assert(exp2(-3.0) == pow(2.0, -3.0));

    printf("[2][3] basic semantics OK\n");
}

static void test_semantics_float_longdouble(void)
{
    /* [1][2][3] float 版本 */
    float xf = 5.0f;
    float rf = exp2f(xf);
    assert(rf == 32.0f);

    /* [1][2][3] long double 版本 */
    long double xl = 6.0L;
    long double rl = exp2l(xl);
    assert(rl == 64.0L);

    /* [1] 参数类型转换：整型实参经默认实参提升/转换后调用 */
    assert(exp2(3) == 8.0);          /* int -> double */
    assert(exp2f(3) == 8.0f);        /* int -> float  */
    assert(exp2l(3) == 8.0L);        /* int -> long double */

    printf("[1][2][3] float/long double variants OK\n");
}

static void test_semantics_identity(void)
{
    /* [2][3] 恒等式：exp2(x) == exp2(x/2) * exp2(x/2) 在可精确表示时成立 */
    assert(exp2(8.0) == exp2(4.0) * exp2(4.0));
    assert(exp2(1.0) * exp2(1.0) == exp2(2.0));

    /* [2][3] 2^x * 2^-x == 1 */
    assert(exp2(7.0) * exp2(-7.0) == 1.0);

    printf("[2][3] identity OK\n");
}

static void test_range_error(void)
{
    /* [2] 当 |x| 过大时发生 range error。
     * 这里只验证「调用不会崩溃」以及「结果趋于 HUGE_VAL / 0」，
     * 不把 errno 的具体取值当作硬性断言（实现可自由选择是否设置 errno）。 */
    errno = 0;
    double big = exp2(1e9);          /* 远超 double 可表示范围 */
    assert(big == HUGE_VAL || big > DBL_MAX || isinf(big));

    errno = 0;
    double tiny = exp2(-1e9);        /* 下溢到 0 */
    assert(tiny == 0.0 || tiny < DBL_MIN);

    printf("[2] range error handling OK\n");
}

int main(void)
{
    test_prototypes();
    test_semantics_basic();
    test_semantics_float_longdouble();
    test_semantics_identity();
    test_range_error();

    printf("ALL POSITIVE TESTS PASSED\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型参数类型为 double/float/long double」：
 * 用不兼容的函数指针类型接收 exp2，gcc -std=c99 应报 incompatible pointer type。 */
double (*bad1)(int) = exp2;

/* 违反约束「[1] 返回类型为 double」：
 * 用返回 int 的函数指针接收 exp2，应报 incompatible pointer type。 */
int (*bad2)(double) = exp2;

/* 违反约束「[1] exp2f 参数为 float」：
 * 用 double 参数类型的函数指针接收 exp2f，应报 incompatible pointer type。 */
double (*bad3)(double) = exp2f;

/* 违反约束「[1] exp2l 返回 long double」：
 * 用 double 返回类型的函数指针接收 exp2l，应报 incompatible pointer type。 */
double (*bad4)(long double) = exp2l;

/* 违反约束「[1] 函数名不可被赋值」：
 * exp2 是函数指示符，不是可修改左值，赋值应报错。 */
void bad5(void) { exp2 = 0; }

/* 违反约束「[1] 函数名不可作为赋值目标」：
 * 对函数指示符取地址后赋值同样非法。 */
void bad6(void) { &exp2 = 0; }

#endif