/*
 * 测试 C99 7.12.7.1 —— cbrt / cbrtf / cbrtl 函数
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后调用 cbrt/cbrtf/cbrtl，程序应能编译并运行通过，
 *             验证它们计算实数立方根（[2]）并返回结果（[3]）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 条款段落：
 *   [1] 概要：三个函数原型 double cbrt(double); float cbrtf(float);
 *             long double cbrtl(long double);
 *   [2] 描述：计算 x 的实数立方根。
 *   [3] 返回：返回 x 的立方根。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：验证三个函数原型可用，且返回类型分别为 double/float/long double */
static void test_synopsis_types(void)
{
    double      (*pd)(double)          = cbrt;
    float       (*pf)(float)           = cbrtf;
    long double (*pl)(long double)     = cbrtl;

    /* 通过函数指针调用，确认签名匹配 */
    double      rd = pd(8.0);
    float       rf = pf(8.0f);
    long double rl = pl(8.0L);

    (void)rd; (void)rf; (void)rl;
    printf("[1] 原型/返回类型检查通过\n");
}

/* [2][3] 描述与返回：cbrt 计算实数立方根并返回之 */
static void test_cbrt_double(void)
{
    /* 正数：cbrt(8) == 2 */
    double r = cbrt(8.0);
    assert(fabs(r - 2.0) < 1e-12);

    /* 正数：cbrt(27) == 3 */
    r = cbrt(27.0);
    assert(fabs(r - 3.0) < 1e-12);

    /* 零：cbrt(0) == 0 */
    r = cbrt(0.0);
    assert(r == 0.0);

    /* 负数：实数立方根，cbrt(-8) == -2（区别于 sqrt 的定义域限制） */
    r = cbrt(-8.0);
    assert(fabs(r - (-2.0)) < 1e-12);

    r = cbrt(-27.0);
    assert(fabs(r - (-3.0)) < 1e-12);

    /* 立方根的自反性：cbrt(x^3) == x */
    double x = 1.5;
    r = cbrt(x * x * x);
    assert(fabs(r - x) < 1e-12);

    printf("[2][3] cbrt(double) 语义检查通过\n");
}

/* [2][3] cbrtf：float 版本 */
static void test_cbrtf(void)
{
    float r = cbrtf(8.0f);
    assert(fabsf(r - 2.0f) < 1e-5f);

    r = cbrtf(-8.0f);
    assert(fabsf(r - (-2.0f)) < 1e-5f);

    r = cbrtf(0.0f);
    assert(r == 0.0f);

    printf("[2][3] cbrtf(float) 语义检查通过\n");
}

/* [2][3] cbrtl：long double 版本 */
static void test_cbrtl(void)
{
    long double r = cbrtl(8.0L);
    assert(fabsl(r - 2.0L) < 1e-15L);

    r = cbrtl(-8.0L);
    assert(fabsl(r - (-2.0L)) < 1e-15L);

    r = cbrtl(0.0L);
    assert(r == 0.0L);

    printf("[2][3] cbrtl(long double) 语义检查通过\n");
}

/* [2] 与 pow(x, 1.0/3.0) 的数值一致性（对正数） */
static void test_consistency_with_pow(void)
{
    double x = 64.0;
    double a = cbrt(x);
    double b = pow(x, 1.0 / 3.0);
    assert(fabs(a - b) < 1e-9);
    printf("[2] cbrt 与 pow(x,1/3) 一致性检查通过\n");
}

int main(void)
{
    test_synopsis_types();
    test_cbrt_double();
    test_cbrtf();
    test_cbrtl();
    test_consistency_with_pow();

    printf("所有正向测试通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「cbrt 的参数必须为算术类型（可转换为 double）」：
 * 传入结构体类型，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'cbrt' */
struct S { int x; } s;
double bad1 = cbrt(s);

/* 违反约束「cbrt 的参数必须为算术类型」：
 * 传入指针类型，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'cbrt' */
double bad2 = cbrt("hello");

/* 违反约束「cbrt 只接受一个参数」：
 * 参数个数不匹配，gcc -std=c99 应报错：
 *   error: too many arguments to function 'cbrt' */
double bad3 = cbrt(1.0, 2.0);

/* 违反约束「cbrt 需要恰好一个参数」：
 * 参数个数不足，gcc -std=c99 应报错：
 *   error: too few arguments to function 'cbrt' */
double bad4 = cbrt();

/* 违反约束「cbrtf 的参数必须为算术类型」：
 * 传入结构体，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'cbrtf' */
float bad5 = cbrtf(s);

/* 违反约束「cbrtl 的参数必须为算术类型」：
 * 传入指针，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'cbrtl' */
long double bad6 = cbrtl(&s);

/* 违反约束「cbrt 的返回值不可作为左值赋值」：
 * 函数调用结果不是左值，gcc -std=c99 应报错：
 *   error: lvalue required as left operand of assignment */
cbrt(8.0) = 2.0;

/* 违反约束「cbrtf 的返回值不可作为左值赋值」：
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
cbrtf(8.0f) = 2.0f;

/* 违反约束「cbrtl 的返回值不可作为左值赋值」：
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
cbrtl(8.0L) = 2.0L;

#endif