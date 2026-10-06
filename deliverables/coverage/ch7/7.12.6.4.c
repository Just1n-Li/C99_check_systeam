/*
 * 测试条款：C99 7.12.6.4  The frexp functions
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 声明/原型：frexp / frexpf / frexpl，返回类型与参数类型。
 *   [2] 语义：把浮点数分解为规格化小数 x 与整数指数 *exp（2 的幂）。
 *   [3] 返回值：x 的绝对值在 [1/2, 1) 或为 0；value == x * 2^(*exp)；value 为 0 时两部分均为 0。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* 辅助：判断 x 是否落在 [1/2, 1) 或为 0 */
static int in_range_or_zero(double x)
{
    double a = fabs(x);
    return (a == 0.0) || (a >= 0.5 && a < 1.0);
}

/* 辅助：用 ldexp 还原，检查 value == x * 2^exp */
static int roundtrip_ok(double value, double x, int exp)
{
    return ldexp(x, exp) == value;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：三个函数都存在，且返回类型/参数类型符合标准。
 *     通过取函数指针并赋值给正确类型的变量来静态验证原型。 */
static void test_prototypes(void)
{
    double (*pf)(double, int *)          = frexp;
    float  (*pff)(float, int *)          = frexpf;
    long double (*pfl)(long double, int *) = frexpl;

    assert(pf  != NULL);
    assert(pff != NULL);
    assert(pfl != NULL);
}

/* [2][3] 基本语义：value = x * 2^exp，x 在 [1/2,1) 或 0 */
static void test_basic_semantics(void)
{
    int exp;
    double x;

    /* 典型正数 */
    x = frexp(8.0, &exp);
    assert(x == 0.5);
    assert(exp == 4);
    assert(in_range_or_zero(x));
    assert(roundtrip_ok(8.0, x, exp));

    /* 典型小数 */
    x = frexp(0.75, &exp);
    assert(x == 0.75);
    assert(exp == 0);
    assert(in_range_or_zero(x));
    assert(roundtrip_ok(0.75, x, exp));

    /* 1.0 -> 0.5 * 2^1 */
    x = frexp(1.0, &exp);
    assert(x == 0.5);
    assert(exp == 1);
    assert(roundtrip_ok(1.0, x, exp));

    /* 负数：x 为负，绝对值仍在 [1/2,1) */
    x = frexp(-8.0, &exp);
    assert(x == -0.5);
    assert(exp == 4);
    assert(in_range_or_zero(x));
    assert(roundtrip_ok(-8.0, x, exp));

    /* 大数 */
    x = frexp(1024.0, &exp);
    assert(x == 0.5);
    assert(exp == 11);
    assert(roundtrip_ok(1024.0, x, exp));
}

/* [3] value 为 0 时，两部分结果均为 0 */
static void test_zero(void)
{
    int exp = 12345; /* 故意预置非零，验证函数会写入 0 */
    double x = frexp(0.0, &exp);
    assert(x == 0.0);
    assert(exp == 0);

    exp = 999;
    x = frexp(-0.0, &exp);
    assert(x == 0.0);
    assert(exp == 0);
}

/* [3] 对任意有限非零值，x 的绝对值必须落在 [1/2, 1) */
static void test_range_property(void)
{
    static const double vals[] = {
        1e-300, 1e-10, 0.1, 0.3, 0.5, 0.9, 1.0, 2.0, 3.14159,
        1e10, 1e100, 1e300, -1e-300, -0.1, -3.14159, -1e100
    };
    size_t i;
    for (i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i) {
        int exp;
        double x = frexp(vals[i], &exp);
        assert(in_range_or_zero(x));
        assert(roundtrip_ok(vals[i], x, exp));
    }
}

/* [1][2][3] frexpf：float 版本 */
static void test_frexpf(void)
{
    int exp;
    float x = frexpf(8.0f, &exp);
    assert(x == 0.5f);
    assert(exp == 4);
    assert(fabsf(x) >= 0.5f && fabsf(x) < 1.0f);
    assert(ldexpf(x, exp) == 8.0f);

    exp = 7;
    x = frexpf(0.0f, &exp);
    assert(x == 0.0f);
    assert(exp == 0);
}

/* [1][2][3] frexpl：long double 版本 */
static void test_frexpl(void)
{
    int exp;
    long double x = frexpl(8.0L, &exp);
    assert(x == 0.5L);
    assert(exp == 4);
    assert(fabsl(x) >= 0.5L && fabsl(x) < 1.0L);
    assert(ldexpl(x, exp) == 8.0L);

    exp = 7;
    x = frexpl(0.0L, &exp);
    assert(x == 0.0L);
    assert(exp == 0);
}

/* [2] exp 指针必须被写入（通过预置哨兵值验证） */
static void test_exp_written(void)
{
    int exp = -12345;
    double x = frexp(6.0, &exp);
    assert(exp != -12345);   /* 已被写入 */
    assert(x == 0.75);
    assert(exp == 3);
    assert(roundtrip_ok(6.0, x, exp));
}

int main(void)
{
    test_prototypes();
    test_basic_semantics();
    test_zero();
    test_range_property();
    test_frexpf();
    test_frexpl();
    test_exp_written();

    printf("C99 7.12.6.4 frexp: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「frexp 的第二个参数类型为 int *」：
 * 传入 double * 而非 int *，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 2 ... from incompatible pointer type）。 */
void bad_arg_type(void)
{
    double e;
    double x = frexp(1.0, &e);   /* 期望报错：&e 是 double*，不是 int* */
    (void)x;
}

/* 违反约束「frexp 的第一个参数类型为 double」：
 * 传入结构体，gcc -std=c99 应报错（incompatible type for argument 1）。 */
struct S { int a; };
void bad_first_arg(void)
{
    struct S s;
    int e;
    double x = frexp(s, &e);     /* 期望报错：结构体不能转换为 double */
    (void)x;
}

/* 违反约束「frexp 返回 double」：
 * 把返回值赋给结构体，gcc -std=c99 应报错（incompatible types in assignment）。 */
void bad_return_use(void)
{
    int e;
    struct S s;
    s = frexp(1.0, &e);          /* 期望报错：double 不能赋给 struct S */
}

/* 违反约束「frexp 需要两个实参」：
 * 实参个数不足，gcc -std=c99 应报错（too few arguments to function 'frexp'）。 */
void bad_arg_count(void)
{
    double x = frexp(1.0);       /* 期望报错：缺少第二个实参 */
    (void)x;
}

/* 违反约束「frexp 的第二个参数必须是指针」：
 * 传入 int 值而非 int*，gcc -std=c99 应报错（incompatible type for argument 2）。 */
void bad_second_arg_not_pointer(void)
{
    int e = 0;
    double x = frexp(1.0, e);    /* 期望报错：int 不能转换为 int* */
    (void)x;
}

#endif /* 负向测试结束 */