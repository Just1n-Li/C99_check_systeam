/*
 * 测试条款：C99 7.12.7.2  The fabs functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，fabs / fabsf / fabsl 三个函数可用，
 *             分别接受 double / float / long double 实参，
 *             返回对应类型的 |x|（绝对值），程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数个数错误、对非算术类型调用等）
 *             应导致编译报错；这些片段放在 #if 0 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型
 *   [2] Description：计算浮点数的绝对值
 *   [3] Returns：返回 |x|
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证三个函数原型存在且返回类型正确 */
static void test_synopsis(void)
{
    /* 通过函数指针类型检查返回类型与参数类型 */
    double      (*pf_d)(double)            = fabs;
    float       (*pf_f)(float)             = fabsf;
    long double (*pf_l)(long double)       = fabsl;

    /* 调用一次，确保链接可用 */
    assert(pf_d(-1.0) == 1.0);
    assert(pf_f(-1.0f) == 1.0f);
    assert(pf_l(-1.0L) == 1.0L);

    /* 返回类型大小检查 */
    assert(sizeof(fabs(-1.0))   == sizeof(double));
    assert(sizeof(fabsf(-1.0f)) == sizeof(float));
    assert(sizeof(fabsl(-1.0L)) == sizeof(long double));
}

/* [2][3] Description + Returns：fabs 计算并返回 |x|（double） */
static void test_fabs_double(void)
{
    /* 正数：|x| == x */
    assert(fabs(3.5) == 3.5);
    /* 负数：|x| == -x */
    assert(fabs(-3.5) == 3.5);
    /* 零：|0| == 0，且符号为正 */
    assert(fabs(0.0) == 0.0);
    assert(signbit(fabs(-0.0)) == 0);   /* fabs(-0.0) 应为 +0.0 */
    /* 结果非负 */
    assert(fabs(-123.456) >= 0.0);
    /* 大数 */
    assert(fabs(-DBL_MAX) == DBL_MAX);
    /* 小数 */
    assert(fabs(-DBL_MIN) == DBL_MIN);
    /* 无穷：|±inf| == +inf */
    assert(isinf(fabs(-INFINITY)) && fabs(-INFINITY) > 0);
    /* NaN：|NaN| 仍为 NaN */
    assert(isnan(fabs(NAN)));
}

/* [2][3] fabsf：float 版本 */
static void test_fabsf(void)
{
    assert(fabsf(2.5f) == 2.5f);
    assert(fabsf(-2.5f) == 2.5f);
    assert(fabsf(0.0f) == 0.0f);
    assert(signbit(fabsf(-0.0f)) == 0);
    assert(fabsf(-FLT_MAX) == FLT_MAX);
    assert(isinf(fabsf(-INFINITY)) && fabsf(-INFINITY) > 0);
    assert(isnan(fabsf(NAN)));
}

/* [2][3] fabsl：long double 版本 */
static void test_fabsl(void)
{
    assert(fabsl(7.25L) == 7.25L);
    assert(fabsl(-7.25L) == 7.25L);
    assert(fabsl(0.0L) == 0.0L);
    assert(signbit(fabsl(-0.0L)) == 0);
    assert(fabsl(-LDBL_MAX) == LDBL_MAX);
    assert(isinf(fabsl(-INFINITY)) && fabsl(-INFINITY) > 0);
    assert(isnan(fabsl(NAN)));
}

/* [2] 语义：|x| 满足 |x| == x 当 x>=0，|x| == -x 当 x<0 */
static void test_absolute_value_semantics(void)
{
    double vals[] = { -10.0, -1.0, -0.5, 0.0, 0.5, 1.0, 10.0 };
    size_t i;
    for (i = 0; i < sizeof(vals)/sizeof(vals[0]); ++i) {
        double x = vals[i];
        double r = fabs(x);
        if (x >= 0.0)
            assert(r == x);
        else
            assert(r == -x);
        assert(r >= 0.0);
    }
}

int main(void)
{
    test_synopsis();
    test_fabs_double();
    test_fabsf();
    test_fabsl();
    test_absolute_value_semantics();

    printf("C99 7.12.7.2 fabs/fabsf/fabsl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数必须与原型一致」：
 * fabs 原型为 double fabs(double)，此处传 0 个实参，
 * gcc -std=c99 应报错：too few arguments to function 'fabs'。 */
double bad1 = fabs();

/* 违反约束「函数调用实参个数必须与原型一致」：
 * 传 2 个实参，gcc -std=c99 应报错：too many arguments to function 'fabs'。 */
double bad2 = fabs(1.0, 2.0);

/* 违反约束「实参必须可转换为参数类型」：
 * 结构体类型无法隐式转换为 double，
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fabs'。 */
struct S { int x; } s;
double bad3 = fabs(s);

/* 违反约束「实参必须可转换为参数类型」：
 * 指针类型无法隐式转换为 double，
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fabs'。 */
double bad4 = fabs("hello");

/* 违反约束「函数名不可作为赋值目标」：
 * fabs 是函数指示符，不是左值，
 * gcc -std=c99 应报错：lvalue required as left operand of assignment。 */
void bad5(void) { fabs = 0; }

/* 违反约束「fabsf 参数类型为 float，实参须可转换」：
 * 结构体无法转换为 float，
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fabsf'。 */
float bad6 = fabsf(s);

/* 违反约束「fabsl 参数类型为 long double，实参须可转换」：
 * 结构体无法转换为 long double，
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fabsl'。 */
long double bad7 = fabsl(s);

#endif /* 负向测试结束 */