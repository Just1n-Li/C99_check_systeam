/*
 * 测试 C99 7.12.3 —— Classification macros（分类宏）
 *
 * 条款要点：
 *   [1] 本子条款各宏原型中，real-floating 表示实参必须是「实浮点类型」的表达式。
 *       即：float / double / long double（不含复数类型 _Complex，不含整型）。
 *
 * 预期行为：
 *   - 正向测试：对 float / double / long double 使用 fpclassify / isfinite /
 *     isinf / isnan / isnormal / signbit，应能编译并运行通过（assert 成立）。
 *   - 负向测试：把整型、复数类型、结构体等非「实浮点类型」的表达式传给这些宏，
 *     违反 [1] 的约束，编译器应报错（诊断信息）。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] real-floating 允许 float / double / long double 三种实浮点类型 */
static void test_real_floating_types(void)
{
    float       f  = 1.0f;
    double      d  = 1.0;
    long double ld = 1.0L;

    /* 每个宏都接受三种实浮点类型 */
    assert(fpclassify(f)  == FP_NORMAL);
    assert(fpclassify(d)  == FP_NORMAL);
    assert(fpclassify(ld) == FP_NORMAL);

    assert(isfinite(f)  != 0);
    assert(isfinite(d)  != 0);
    assert(isfinite(ld) != 0);

    assert(isinf(f)  == 0);
    assert(isinf(d)  == 0);
    assert(isinf(ld) == 0);

    assert(isnan(f)  == 0);
    assert(isnan(d)  == 0);
    assert(isnan(ld) == 0);

    assert(isnormal(f)  != 0);
    assert(isnormal(d)  != 0);
    assert(isnormal(ld) != 0);

    assert(signbit(f)  == 0);
    assert(signbit(d)  == 0);
    assert(signbit(ld) == 0);
}

/* [1] 实参可以是任意「实浮点类型」的表达式（含算术表达式、函数返回值） */
static double make_nan(void) { return 0.0 / 0.0; }   /* 产生 NaN（IEEE 环境） */
static double make_inf(void) { return 1.0 / 0.0; }   /* 产生 Inf（IEEE 环境） */

static void test_expressions(void)
{
    double x = 2.0, y = 3.0;

    /* 算术表达式作为实参 */
    assert(isfinite(x + y) != 0);
    assert(fpclassify(x * y) == FP_NORMAL);

    /* 函数返回值作为实参 */
    assert(isnan(make_nan()) != 0);
    assert(isinf(make_inf()) != 0);

    /* 负零：signbit 为真，但仍是有限、非 NaN */
    double nz = -0.0;
    assert(signbit(nz) != 0);
    assert(isfinite(nz) != 0);
    assert(isnan(nz) == 0);
}

/* [1] 分类结果与 FP_* 常量配合使用 */
static void test_classification_values(void)
{
    double zero = 0.0;
    double inf  = 1.0 / 0.0;
    double nan  = 0.0 / 0.0;

    assert(fpclassify(zero) == FP_ZERO);
    assert(fpclassify(inf)  == FP_INFINITE);
    assert(fpclassify(nan)  == FP_NAN);

    /* 各分类互斥：一个值只属于一个类别 */
    assert(fpclassify(inf) != FP_NAN);
    assert(fpclassify(nan) != FP_INFINITE);
}

int main(void)
{
    test_real_floating_types();
    test_expressions();
    test_classification_values();

    printf("C99 7.12.3 classification macros: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]「real-floating 表示实参必须是实浮点类型」：
 * 整型不是实浮点类型，把 int 传给 fpclassify，gcc -std=c99 应报错
 * （例如 "incompatible type for argument" / "invalid type argument"）。 */
void neg_int_argument(void)
{
    int i = 1;
    int r = fpclassify(i);      /* 错误：int 非实浮点类型 */
    (void)r;
}

/* 违反约束 [1]：整型传给 isfinite，应报错。 */
void neg_int_isfinite(void)
{
    long n = 42L;
    int r = isfinite(n);        /* 错误：long 非实浮点类型 */
    (void)r;
}

/* 违反约束 [1]：复数类型不是「实浮点类型」，传给 isnan 应报错。 */
void neg_complex_argument(void)
{
    double _Complex z = 1.0 + 2.0 * I;
    int r = isnan(z);           /* 错误：复数非实浮点类型 */
    (void)r;
}

/* 违反约束 [1]：结构体不是实浮点类型，传给 signbit 应报错。 */
struct S { double v; };
void neg_struct_argument(void)
{
    struct S s;
    s.v = 1.0;
    int r = signbit(s);         /* 错误：结构体非实浮点类型 */
    (void)r;
}

/* 违反约束 [1]：指针不是实浮点类型，传给 isnormal 应报错。 */
void neg_pointer_argument(void)
{
    double d = 1.0;
    double *p = &d;
    int r = isnormal(p);        /* 错误：指针非实浮点类型 */
    (void)r;
}

#endif /* 负向测试结束 */