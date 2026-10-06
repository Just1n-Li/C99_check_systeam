/*
 * 测试 C99 7.12.3.1 —— fpclassify 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，fpclassify(x) 能对实浮点参数分类，
 *             返回 FP_NAN / FP_INFINITE / FP_NORMAL / FP_SUBNORMAL / FP_ZERO
 *             之一（或实现定义类别），且分类基于参数的语义类型。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如对整型/指针调用 fpclassify 而不满足
 *             real-floating 要求、缺少 <math.h> 声明等）应导致编译报错。
 *             这些片段放在 #if 0 中，保证本文件仍可正常编译。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <math.h> 提供 fpclassify 宏；此处已包含，直接使用 */
static void test_synopsis_and_header(void)
{
    /* [1] 能作为宏调用，返回 int 类型的分类值 */
    double d = 1.0;
    int c = fpclassify(d);
    assert(c == FP_NORMAL);
}

/* [2] 分类：NaN */
static void test_nan(void)
{
    double nan_val = NAN;                 /* <math.h> 提供 NAN */
    assert(fpclassify(nan_val) == FP_NAN);

    float fnan = nanf("");
    assert(fpclassify(fnan) == FP_NAN);

    long double lnan = nanl("");
    assert(fpclassify(lnan) == FP_NAN);

    /* 0.0/0.0 产生 NaN（运行时） */
    volatile double zero = 0.0;
    double q = zero / zero;
    assert(fpclassify(q) == FP_NAN);
}

/* [2] 分类：无穷 */
static void test_infinite(void)
{
    double inf_val = INFINITY;            /* <math.h> 提供 INFINITY */
    assert(fpclassify(inf_val) == FP_INFINITE);

    float finf = HUGE_VALF;
    assert(fpclassify(finf) == FP_INFINITE);

    long double linf = HUGE_VALL;
    assert(fpclassify(linf) == FP_INFINITE);

    /* 负无穷 */
    assert(fpclassify(-INFINITY) == FP_INFINITE);

    /* 1.0/0.0 产生无穷 */
    volatile double one = 1.0, zero = 0.0;
    double r = one / zero;
    assert(fpclassify(r) == FP_INFINITE);
}

/* [2] 分类：正规数 */
static void test_normal(void)
{
    assert(fpclassify(1.0) == FP_NORMAL);
    assert(fpclassify(-1.0) == FP_NORMAL);
    assert(fpclassify(3.14159) == FP_NORMAL);
    assert(fpclassify(1.0f) == FP_NORMAL);
    assert(fpclassify(1.0L) == FP_NORMAL);

    /* DBL_MIN 是正规数 */
    assert(fpclassify(DBL_MIN) == FP_NORMAL);
    assert(fpclassify(FLT_MIN) == FP_NORMAL);
}

/* [2] 分类：次正规数（subnormal） */
static void test_subnormal(void)
{
    /* DBL_MIN 的一半是次正规数（若实现支持次正规数） */
    double sub = DBL_MIN / 2.0;
    int c = fpclassify(sub);
    /* 若实现支持次正规数，应为 FP_SUBNORMAL；否则可能为 FP_ZERO。
       标准允许实现定义类别，这里只要求返回值是合法分类之一。 */
    assert(c == FP_SUBNORMAL || c == FP_ZERO);

    float fsub = FLT_MIN / 2.0f;
    int fc = fpclassify(fsub);
    assert(fc == FP_SUBNORMAL || fc == FP_ZERO);
}

/* [2] 分类：零 */
static void test_zero(void)
{
    assert(fpclassify(0.0) == FP_ZERO);
    assert(fpclassify(-0.0) == FP_ZERO);
    assert(fpclassify(0.0f) == FP_ZERO);
    assert(fpclassify(-0.0f) == FP_ZERO);
    assert(fpclassify(0.0L) == FP_ZERO);
}

/* [2] 分类基于参数的语义类型：
 *     一个在 long double 中为正规数的值，转换为 double 后可能变为次正规数，
 *     转换为 float 后可能变为零。这里验证 fpclassify 依据实参的语义类型分类。
 *     由于表达式可能以更宽范围/精度求值（脚注 205），我们使用 volatile 强制
 *     存储到具体类型，确保分类基于该类型。 */
static void test_semantic_type(void)
{
    /* 构造一个在 double 中为次正规数、在 float 中为 0 的值 */
    volatile double dsub = DBL_MIN / 2.0;   /* double 次正规 */
    int cd = fpclassify(dsub);
    assert(cd == FP_SUBNORMAL || cd == FP_ZERO);

    /* 将 double 次正规数转换为 float，通常变为 0 */
    volatile float f_from_d = (float)dsub;
    int cf = fpclassify(f_from_d);
    assert(cf == FP_ZERO || cf == FP_SUBNORMAL);

    /* 同一数值在不同语义类型下分类可能不同：验证 fpclassify 使用实参类型 */
    volatile long double ld = 1.0L;
    assert(fpclassify(ld) == FP_NORMAL);
    volatile double dd = (double)ld;
    assert(fpclassify(dd) == FP_NORMAL);
    volatile float ff = (float)ld;
    assert(fpclassify(ff) == FP_NORMAL);
}

/* [3] 返回值是「适当的数字分类宏」的值：
 *     验证返回值确实等于某个分类宏，且分类宏之间互不相同。 */
static void test_return_values(void)
{
    /* 分类宏应互不相同 */
    assert(FP_NAN != FP_INFINITE);
    assert(FP_NAN != FP_NORMAL);
    assert(FP_NAN != FP_SUBNORMAL);
    assert(FP_NAN != FP_ZERO);
    assert(FP_INFINITE != FP_NORMAL);
    assert(FP_INFINITE != FP_SUBNORMAL);
    assert(FP_INFINITE != FP_ZERO);
    assert(FP_NORMAL != FP_SUBNORMAL);
    assert(FP_NORMAL != FP_ZERO);
    assert(FP_SUBNORMAL != FP_ZERO);

    /* 返回值类型为 int */
    int c = fpclassify(1.0);
    assert(c == FP_NORMAL);
}

/* [4] EXAMPLE：fpclassify 可基于 sizeof 实现。
 *     这里不测试具体实现，只验证宏可对 float/double/long double 三种类型工作，
 *     与 EXAMPLE 中按 sizeof 分派的思路一致。 */
static void test_example_three_types(void)
{
    float f = 1.0f;
    double d = 1.0;
    long double ld = 1.0L;

    assert(fpclassify(f) == FP_NORMAL);
    assert(fpclassify(d) == FP_NORMAL);
    assert(fpclassify(ld) == FP_NORMAL);

    /* 三种类型都能正确识别 NaN */
    assert(fpclassify(nanf("")) == FP_NAN);
    assert(fpclassify(nan("")) == FP_NAN);
    assert(fpclassify(nanl("")) == FP_NAN);
}

/* 脚注 205：表达式可能以更宽范围/精度求值，分类基于其类型。
 * 使用强制转换/volatile 确保类型确定。 */
static void test_footnote_205(void)
{
    /* 一个 long double 正规数转换为 double 可能变为次正规数 */
    volatile long double ld_normal = 1.0L;
    assert(fpclassify(ld_normal) == FP_NORMAL);

    /* 强制转换到 double 后仍为正规数（1.0 在 double 中正规） */
    volatile double d = (double)ld_normal;
    assert(fpclassify(d) == FP_NORMAL);

    /* 强制转换到 float 后仍为正规数 */
    volatile float f = (float)ld_normal;
    assert(fpclassify(f) == FP_NORMAL);
}

int main(void)
{
    test_synopsis_and_header();
    test_nan();
    test_infinite();
    test_normal();
    test_subnormal();
    test_zero();
    test_semantic_type();
    test_return_values();
    test_example_three_types();
    test_footnote_205();

    printf("C99 7.12.3.1 fpclassify: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「参数必须为 real-floating 类型」：
 * fpclassify 的参数 x 必须是实浮点类型（float/double/long double）。
 * 传入 int 应导致编译错误（宏展开后类型不匹配或约束违反）。 */
#include <math.h>
void neg_int_arg(void)
{
    int i = 42;
    int c = fpclassify(i);   /* 错误：int 不是 real-floating */
    (void)c;
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入指针类型应编译报错。 */
void neg_pointer_arg(void)
{
    double d = 1.0;
    double *p = &d;
    int c = fpclassify(p);   /* 错误：指针不是 real-floating */
    (void)c;
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入复数类型应编译报错（复数不是 real-floating）。 */
#include <complex.h>
void neg_complex_arg(void)
{
    double complex z = 1.0 + 2.0 * I;
    int c = fpclassify(z);   /* 错误：复数不是 real-floating */
    (void)c;
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入结构体类型应编译报错。 */
struct S { double x; };
void neg_struct_arg(void)
{
    struct S s = { 1.0 };
    int c = fpclassify(s);   /* 错误：结构体不是 real-floating */
    (void)c;
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入 void 表达式应编译报错。 */
void neg_void_arg(void)
{
    int c = fpclassify((void)0);   /* 错误：void 不是 real-floating */
    (void)c;
}

/* 违反约束「参数必须为 real-floating 类型」：
 * 传入字符串字面量（char*）应编译报错。 */
void neg_string_arg(void)
{
    int c = fpclassify("hello");   /* 错误：char* 不是 real-floating */
    (void)c;
}

#endif /* 负向测试结束 */