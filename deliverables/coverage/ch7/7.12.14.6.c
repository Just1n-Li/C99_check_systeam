/*
 * 测试 C99 7.12.14.6 —— isunordered 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，isunordered(x, y) 可编译并运行；
 *             当 x 与 y 无序（即任一为 NaN）时返回 1，否则返回 0。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 条款结构：
 *   [1] 概要：#include <math.h>  int isunordered(real-floating x, real-floating y);
 *   [2] 描述：判断实参是否无序（unordered）。
 *   [3] 返回值：无序返回 1，否则返回 0。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：isunordered 是 <math.h> 中声明的宏，接受两个 real-floating 实参 */
static int test_synopsis(void)
{
    /* 能取地址/调用形式：作为宏使用，接受两个浮点实参 */
    double x = 1.0, y = 2.0;
    int r = isunordered(x, y);
    (void)r;
    return 1;
}

/* [2][3] 描述与返回值：有序（ordered）时返回 0 */
static void test_ordered(void)
{
    double a = 1.0, b = 2.0;
    float  c = 3.0f, d = 4.0f;
    long double e = 5.0L, f = 6.0L;

    /* 普通有限值：有序 */
    assert(isunordered(a, b) == 0);
    assert(isunordered(c, d) == 0);
    assert(isunordered(e, f) == 0);

    /* 相等值：有序 */
    assert(isunordered(a, a) == 0);

    /* 无穷大与有限值：有序（+inf 与 -inf 也是有序的） */
    assert(isunordered(INFINITY, a) == 0);
    assert(isunordered(-INFINITY, INFINITY) == 0);
    assert(isunordered(INFINITY, INFINITY) == 0);
}

/* [2][3] 描述与返回值：无序（unordered）时返回 1 —— 任一实参为 NaN */
static void test_unordered(void)
{
    double nan_d = NAN;
    float  nan_f = NAN;
    long double nan_l = NAN;
    double a = 1.0;

    /* 一个 NaN 与一个普通值：无序 */
    assert(isunordered(nan_d, a) == 1);
    assert(isunordered(a, nan_d) == 1);

    /* 两个 NaN：无序 */
    assert(isunordered(nan_d, nan_d) == 1);
    assert(isunordered(nan_f, nan_f) == 1);
    assert(isunordered(nan_l, nan_l) == 1);

    /* NaN 与无穷大：无序 */
    assert(isunordered(nan_d, INFINITY) == 1);
    assert(isunordered(INFINITY, nan_d) == 1);

    /* 通过 0.0/0.0 生成 NaN 的常见方式（若实现支持） */
    {
        volatile double zero = 0.0;
        double nan2 = zero / zero;
        assert(isunordered(nan2, a) == 1);
    }
}

/* [3] 返回值语义：结果恰为 0 或 1（不是任意非零值） */
static void test_return_value_is_0_or_1(void)
{
    double nan_d = NAN;
    double a = 1.0;

    int r1 = isunordered(a, a);
    int r2 = isunordered(nan_d, a);

    assert(r1 == 0 || r1 == 1);
    assert(r2 == 0 || r2 == 1);
    assert(r1 == 0);
    assert(r2 == 1);
}

/* [2] 描述：isunordered 与 isless/isgreater 等的关系 —— 无序时比较宏均为假 */
static void test_relation_with_comparison_macros(void)
{
    double nan_d = NAN;
    double a = 1.0, b = 2.0;

    /* 无序时：isless/isgreater/islessequal/isgreaterequal 均为 0 */
    assert(isunordered(nan_d, a) == 1);
    assert(isless(nan_d, a) == 0);
    assert(isgreater(nan_d, a) == 0);
    assert(islessequal(nan_d, a) == 0);
    assert(isgreaterequal(nan_d, a) == 0);

    /* 有序时：isunordered 为 0 */
    assert(isunordered(a, b) == 0);
    assert(isless(a, b) == 1);
}

/* [1] 概要：宏可用于任意 real-floating 类型（float/double/long double） */
static void test_all_real_floating_types(void)
{
    float  f1 = NAN, f2 = 1.0f;
    double d1 = NAN, d2 = 1.0;
    long double l1 = NAN, l2 = 1.0L;

    assert(isunordered(f1, f2) == 1);
    assert(isunordered(d1, d2) == 1);
    assert(isunordered(l1, l2) == 1);

    assert(isunordered(f2, f2) == 0);
    assert(isunordered(d2, d2) == 0);
    assert(isunordered(l2, l2) == 0);
}

int main(void)
{
    test_synopsis();
    test_ordered();
    test_unordered();
    test_return_value_is_0_or_1();
    test_relation_with_comparison_macros();
    test_all_real_floating_types();

    printf("C99 7.12.14.6 isunordered: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isunordered 的实参必须为 real-floating 类型」：
 * 传入整数实参，gcc -std=c99 应报错（类型不匹配 / 宏展开后类型错误）。 */
#include <math.h>
void bad_int_args(void)
{
    int i = 1, j = 2;
    int r = isunordered(i, j);   /* 期望：编译报错，实参非 real-floating */
    (void)r;
}

/* 违反约束「isunordered 需要两个实参」：
 * 只传一个实参，gcc -std=c99 应报错（宏参数个数不匹配）。 */
void bad_one_arg(void)
{
    double x = 1.0;
    int r = isunordered(x);      /* 期望：编译报错，参数个数不足 */
    (void)r;
}

/* 违反约束「isunordered 需要两个实参」：
 * 传三个实参，gcc -std=c99 应报错（宏参数个数不匹配）。 */
void bad_three_args(void)
{
    double x = 1.0, y = 2.0, z = 3.0;
    int r = isunordered(x, y, z); /* 期望：编译报错，参数个数过多 */
    (void)r;
}

/* 违反约束「isunordered 的实参必须为 real-floating 类型」：
 * 传入指针实参，gcc -std=c99 应报错。 */
void bad_pointer_args(void)
{
    double x = 1.0, y = 2.0;
    int r = isunordered(&x, &y);  /* 期望：编译报错，实参为指针而非浮点 */
    (void)r;
}

/* 违反约束「isunordered 的实参必须为 real-floating 类型」：
 * 传入结构体实参，gcc -std=c99 应报错。 */
struct S { double v; };
void bad_struct_args(void)
{
    struct S s1 = { 1.0 }, s2 = { 2.0 };
    int r = isunordered(s1, s2);  /* 期望：编译报错，实参为结构体 */
    (void)r;
}

/* 违反约束「isunordered 的实参必须为 real-floating 类型」：
 * 传入复数实参（C99 复数类型），gcc -std=c99 应报错。 */
void bad_complex_args(void)
{
    double _Complex c1 = 1.0 + 2.0 * I;
    double _Complex c2 = 3.0 + 4.0 * I;
    int r = isunordered(c1, c2);  /* 期望：编译报错，实参为复数而非实浮点 */
    (void)r;
}

#endif /* 负向测试结束 */