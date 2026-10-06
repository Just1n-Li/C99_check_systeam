/*
 * 测试 C99 7.12.14 —— Comparison macros（比较宏）
 *
 * 预期行为：
 *   正向测试：以下使用 <math.h> 中 isgreater / isgreaterequal / isless /
 *             islessequal / islessgreater / isunordered 六个宏的代码应能
 *             编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束「real-floating indicates that the argument shall be
 *             an expression of real floating type」的调用应编译报错。
 *
 * 说明：本条款只规定这六个宏的语义与约束，不涉及 UB 测试。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 六个比较宏的语义：对任意有序数值对，less/greater/equal 恰有一个为真；
 *     对 NaN 与数值、或两个 NaN，只有 unordered 关系为真。
 *     这些宏是关系运算符的“安静”版本（不引发 invalid 浮点异常）。 */

static void test_ordered_pairs(void)
{
    /* [1] 普通有序数值对：1.0 < 2.0 */
    double a = 1.0, b = 2.0;

    /* 恰好一个关系为真：less 为真，greater/equal 为假 */
    assert(isless(a, b) != 0);          /* a < b  */
    assert(isgreater(a, b) == 0);       /* a > b  */
    assert(islessgreater(a, b) != 0);   /* a < b 或 a > b（即不相等） */
    assert(isgreaterequal(a, b) == 0);  /* a >= b */
    assert(islessequal(a, b) != 0);     /* a <= b */
    assert(isunordered(a, b) == 0);     /* 有序，非 unordered */

    /* [1] 相等数值对：3.0 == 3.0 */
    double c = 3.0, d = 3.0;
    assert(isless(c, d) == 0);
    assert(isgreater(c, d) == 0);
    assert(islessgreater(c, d) == 0);   /* 相等 => 既不小于也不大于 */
    assert(isgreaterequal(c, d) != 0);
    assert(islessequal(c, d) != 0);
    assert(isunordered(c, d) == 0);

    /* [1] 大于数值对：5.0 > 4.0 */
    double e = 5.0, f = 4.0;
    assert(isless(e, f) == 0);
    assert(isgreater(e, f) != 0);
    assert(islessgreater(e, f) != 0);
    assert(isgreaterequal(e, f) != 0);
    assert(islessequal(e, f) == 0);
    assert(isunordered(e, f) == 0);
}

static void test_nan(void)
{
    /* [1] NaN 与数值：只有 unordered 关系为真 */
    double nan_val = NAN;
    double num = 1.0;

    assert(isunordered(nan_val, num) != 0);
    assert(isunordered(num, nan_val) != 0);

    /* 对 NaN 与数值，less/greater/equal 关系均不成立 */
    assert(isless(nan_val, num) == 0);
    assert(isgreater(nan_val, num) == 0);
    assert(islessgreater(nan_val, num) == 0);
    assert(isgreaterequal(nan_val, num) == 0);
    assert(islessequal(nan_val, num) == 0);

    /* [1] 两个 NaN：只有 unordered 关系为真 */
    double nan2 = NAN;
    assert(isunordered(nan_val, nan2) != 0);
    assert(isless(nan_val, nan2) == 0);
    assert(isgreater(nan_val, nan2) == 0);
    assert(islessgreater(nan_val, nan2) == 0);
    assert(isgreaterequal(nan_val, nan2) == 0);
    assert(islessequal(nan_val, nan2) == 0);
}

static void test_quiet_no_exception(void)
{
    /* [1] 这些宏是“安静”版本：即使参数为 NaN 也不应引发 invalid 浮点异常。
     * 这里通过 feclearexcept / fetestexcept 检查（若实现支持 <fenv.h>）。
     * 注意：C99 7.12.14 只保证宏本身不引发异常；此处仅作可选的运行时检查。 */
#if defined(FE_INVALID) && defined(__STDC_IEC_559__)
    feclearexcept(FE_INVALID);
    volatile double nan_val = NAN;
    volatile double num = 1.0;
    (void)isless(nan_val, num);
    (void)isgreater(nan_val, num);
    (void)isunordered(nan_val, num);
    /* 安静版本不应设置 FE_INVALID */
    assert(fetestexcept(FE_INVALID) == 0);
#endif
}

static void test_real_floating_types(void)
{
    /* [1] 参数应为 real floating 类型：float / double / long double 均可 */
    float  fa = 1.0f, fb = 2.0f;
    double da = 1.0,  db = 2.0;
    long double la = 1.0L, lb = 2.0L;

    assert(isless(fa, fb) != 0);
    assert(isless(da, db) != 0);
    assert(isless(la, lb) != 0);

    assert(isgreater(fb, fa) != 0);
    assert(isgreater(db, da) != 0);
    assert(isgreater(lb, la) != 0);

    /* 混合类型参数也应可用（usual arithmetic conversions 后为 real floating） */
    assert(isless(fa, db) != 0);
    assert(isless(da, lb) != 0);
}

int main(void)
{
    test_ordered_pairs();
    test_nan();
    test_quiet_no_exception();
    test_real_floating_types();

    printf("C99 7.12.14 comparison macros: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束 [1]「real-floating indicates that the argument shall be an
 * expression of real floating type」：
 * 传入整数类型参数，gcc -std=c99 应报错（宏展开后类型不匹配 / 约束违反）。 */
void neg_integer_argument(void)
{
    int i = 1, j = 2;
    (void)isless(i, j);          /* 期望报错：参数不是 real floating 类型 */
    (void)isgreater(i, j);       /* 期望报错 */
    (void)isunordered(i, j);     /* 期望报错 */
}

/* 违反约束 [1]：传入指针类型参数，不是 real floating 类型。 */
void neg_pointer_argument(void)
{
    double x = 1.0, y = 2.0;
    double *px = &x, *py = &y;
    (void)isless(px, py);        /* 期望报错：参数是指针，不是 real floating */
    (void)isgreaterequal(px, py);/* 期望报错 */
}

/* 违反约束 [1]：传入结构体类型参数，不是 real floating 类型。 */
struct S { double v; };
void neg_struct_argument(void)
{
    struct S s1, s2;
    (void)isless(s1, s2);        /* 期望报错：参数是结构体，不是 real floating */
    (void)islessequal(s1, s2);   /* 期望报错 */
}

/* 违反约束 [1]：传入复数类型参数（complex 不是 real floating）。 */
void neg_complex_argument(void)
{
    double _Complex c1 = 1.0 + 0.0 * I;
    double _Complex c2 = 2.0 + 0.0 * I;
    (void)isless(c1, c2);        /* 期望报错：参数是复数，不是 real floating */
    (void)islessgreater(c1, c2); /* 期望报错 */
}

#endif /* 负向测试结束 */