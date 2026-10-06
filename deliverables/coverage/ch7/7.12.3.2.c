/*
 * 测试条款：C99 7.12.3.2  The isfinite macro
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后使用 isfinite 宏，对有限值（零、次正规、正规）
 *             返回非零；对无穷大和 NaN 返回 0。程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数不是 real-floating 类型）应导致编译报错，
 *             这些片段放在 #if 0 中，保证本文件仍可正常编译。
 *
 * 覆盖段落：
 *   [1] Synopsis: #include <math.h>  int isfinite(real-floating x);
 *   [2] Description: 判断参数是否为有限值；宽格式先转换为语义类型再判断。
 *   [3] Returns: 当且仅当参数为有限值时返回非零。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：包含 <math.h> 后 isfinite 可用，接受 real-floating 实参 */
static void test_synopsis(void)
{
    /* [1] 对 float / double / long double 三种 real-floating 类型均可调用 */
    float       f = 1.0f;
    double      d = 1.0;
    long double ld = 1.0L;

    assert(isfinite(f)  != 0);
    assert(isfinite(d)  != 0);
    assert(isfinite(ld) != 0);
}

/* [2][3] 有限值：零、次正规、正规 —— 应返回非零 */
static void test_finite_values(void)
{
    /* 零（+0.0 与 -0.0 都是有限值） */
    assert(isfinite(0.0)  != 0);
    assert(isfinite(-0.0) != 0);

    /* 正规数 */
    assert(isfinite(1.0)   != 0);
    assert(isfinite(-1.0)  != 0);
    assert(isfinite(3.14)  != 0);
    assert(isfinite(DBL_MAX) != 0);
    assert(isfinite(-DBL_MAX) != 0);

    /* 次正规数（subnormal）：小于 DBL_MIN 但大于 0 的正数 */
    {
        double sub = DBL_MIN / 2.0;   /* 典型次正规值 */
        assert(sub > 0.0);
        assert(sub < DBL_MIN);
        assert(isfinite(sub) != 0);
    }

    /* float 与 long double 的有限值 */
    assert(isfinite(FLT_MIN)  != 0);
    assert(isfinite(LDBL_MAX) != 0);
}

/* [2][3] 非有限值：无穷大与 NaN —— 应返回 0 */
static void test_non_finite_values(void)
{
    double inf  = INFINITY;
    double ninf = -INFINITY;
    double nan  = NAN;

    /* 无穷大不是有限值 */
    assert(isfinite(inf)  == 0);
    assert(isfinite(ninf) == 0);

    /* NaN 不是有限值 */
    assert(isfinite(nan)  == 0);

    /* 由运算产生的无穷大与 NaN */
    assert(isfinite(1.0 / 0.0)  == 0);   /* +inf */
    assert(isfinite(-1.0 / 0.0) == 0);   /* -inf */
    assert(isfinite(0.0 / 0.0)  == 0);   /* NaN */

    /* float / long double 版本 */
    assert(isfinite((float)INFINITY)  == 0);
    assert(isfinite((long double)NAN) == 0);
}

/* [2] 宽格式先转换为语义类型再判断：
 *     例如 long double 的有限值转换为 double 后仍有限；
 *     这里验证对 long double 实参的判断结果与语义类型一致。 */
static void test_semantic_type_conversion(void)
{
    long double ld_finite = 1.0L;
    long double ld_inf    = (long double)INFINITY;

    assert(isfinite(ld_finite) != 0);
    assert(isfinite(ld_inf)    == 0);
}

/* [3] 返回值语义：非零当且仅当有限 */
static void test_iff_semantics(void)
{
    double vals[] = { 0.0, -0.0, 1.0, -1.0, DBL_MIN, DBL_MAX,
                      INFINITY, -INFINITY, NAN };
    size_t i;
    for (i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i) {
        double v = vals[i];
        int finite = isfinite(v);
        if (v == INFINITY || v == -INFINITY || v != v) {
            /* 无穷或 NaN：必须返回 0 */
            assert(finite == 0);
        } else {
            /* 有限：必须返回非零 */
            assert(finite != 0);
        }
    }
}

int main(void)
{
    test_synopsis();
    test_finite_values();
    test_non_finite_values();
    test_semantic_type_conversion();
    test_iff_semantics();

    printf("C99 7.12.3.2 isfinite: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「参数必须为 real-floating 类型」：
 * isfinite 的参数应为 real-floating（float/double/long double）。
 * 传入整数类型，gcc -std=c99 应报错（类型不匹配 / 宏展开后类型错误）。 */
#include <math.h>
int bad_int_arg(void)
{
    int i = 1;
    return isfinite(i);   /* 期望：编译报错，参数不是 real-floating */
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入复数类型，应编译报错。 */
#include <complex.h>
int bad_complex_arg(void)
{
    double complex z = 1.0 + 2.0 * I;
    return isfinite(z);   /* 期望：编译报错，复数不是 real-floating */
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入指针类型，应编译报错。 */
int bad_pointer_arg(void)
{
    double d = 1.0;
    double *p = &d;
    return isfinite(p);   /* 期望：编译报错，指针不是 real-floating */
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入结构体类型，应编译报错。 */
struct S { double x; };
int bad_struct_arg(void)
{
    struct S s = { 1.0 };
    return isfinite(s);   /* 期望：编译报错，结构体不是 real-floating */
}

#endif /* 负向测试结束 */