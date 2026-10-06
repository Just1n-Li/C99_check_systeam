/*
 * 测试条款：C99 7.12.3.6  The signbit macro
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后可使用 signbit 宏；对负值返回非零，对正值返回 0；
 *             对 ±0、±inf、NaN 也报告符号（脚注 207）。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，不影响本文件正常编译。
 *
 * 段落覆盖：
 *   [1] Synopsis：需要 #include <math.h>，signbit 接受 real-floating 实参。
 *   [2] Description：判定实参值的符号是否为负。
 *   [3] Returns：当且仅当符号为负时返回非零值。
 *   Footnote 207：报告所有值的符号，包括 inf、零、NaN；无符号零视为正。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：signbit 是 <math.h> 中声明的宏，接受 real-floating 实参。
 *     这里用 float / double / long double 三种 real-floating 类型调用。 */
static void test_synopsis_real_floating(void)
{
    float       f = -1.0f;
    double      d = -1.0;
    long double ld = -1.0L;

    /* 三种 real-floating 类型都应可用，且负值返回非零 */
    assert(signbit(f)  != 0);
    assert(signbit(d)  != 0);
    assert(signbit(ld) != 0);
}

/* [2][3] 基本语义：负值 -> 非零；正值 -> 0（当且仅当） */
static void test_basic_sign(void)
{
    /* 负值：返回非零 */
    assert(signbit(-1.0)  != 0);
    assert(signbit(-0.5)  != 0);
    assert(signbit(-1e300) != 0);
    assert(signbit(-1e-300) != 0);

    /* 正值：返回 0 */
    assert(signbit(1.0)   == 0);
    assert(signbit(0.5)   == 0);
    assert(signbit(1e300) == 0);
    assert(signbit(1e-300) == 0);

    /* [3] “当且仅当”：返回值只有“非零 / 零”两种判定，
     *     用逻辑取反验证等价性 */
    double v = -3.25;
    assert((signbit(v) != 0) == (v < 0.0));
    v = 3.25;
    assert((signbit(v) != 0) == (v < 0.0));
}

/* Footnote 207：零的符号。
 * 负零 -0.0 符号为负 -> 非零；正零 +0.0 符号为正 -> 0。
 * 注意：不能用 (x < 0) 判定 -0.0，必须用 signbit。 */
static void test_zero_sign(void)
{
    double pos_zero = 0.0;
    double neg_zero = -0.0;

    /* 正零视为正 */
    assert(signbit(pos_zero) == 0);

    /* 负零符号为负 */
    assert(signbit(neg_zero) != 0);

    /* 用 1.0/(-0.0) 得到 -inf 来确认 -0.0 确实是负零 */
    assert(1.0 / neg_zero < 0.0);

    /* float / long double 的零同样适用 */
    assert(signbit(0.0f)  == 0);
    assert(signbit(-0.0f) != 0);
    assert(signbit(0.0L)  == 0);
    assert(signbit(-0.0L) != 0);
}

/* Footnote 207：无穷的符号 */
static void test_infinity_sign(void)
{
    double pos_inf = INFINITY;
    double neg_inf = -INFINITY;

    assert(signbit(pos_inf) == 0);
    assert(signbit(neg_inf) != 0);

    /* 用 HUGE_VAL 构造无穷（若实现支持） */
    assert(signbit(HUGE_VAL)  == 0);
    assert(signbit(-HUGE_VAL) != 0);
}

/* Footnote 207：NaN 的符号。
 * NaN 的符号位由实现决定，但 signbit 必须报告该符号位：
 * 对 -NAN 返回非零，对 +NAN 返回 0。 */
static void test_nan_sign(void)
{
    double pos_nan = NAN;
    double neg_nan = -NAN;

    /* 确认它们确实是 NaN */
    assert(isnan(pos_nan));
    assert(isnan(neg_nan));

    /* signbit 报告 NaN 的符号位 */
    assert(signbit(pos_nan) == 0);
    assert(signbit(neg_nan) != 0);
}

/* [3] 返回值语义：非零值不要求等于 1，只要求“非零”。
 *     这里验证 signbit 的结果可直接用于条件判断。 */
static void test_nonzero_truthiness(void)
{
    if (signbit(-2.0)) {
        /* 负值分支，正确 */
    } else {
        assert(0 && "signbit(-2.0) 应为非零");
    }

    if (!signbit(2.0)) {
        /* 正值分支，正确 */
    } else {
        assert(0 && "signbit(2.0) 应为 0");
    }
}

int main(void)
{
    test_synopsis_real_floating();  /* [1] */
    test_basic_sign();              /* [2][3] */
    test_zero_sign();               /* Footnote 207 */
    test_infinity_sign();           /* Footnote 207 */
    test_nan_sign();                /* Footnote 207 */
    test_nonzero_truthiness();      /* [3] */

    printf("C99 7.12.3.6 signbit: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] Synopsis：signbit 接受 real-floating 实参」：
 * 传入整数类型实参。signbit 是类型泛型宏，要求 real-floating 类型，
 * 传 int 不满足约束，gcc -std=c99 应报错（如 "invalid argument type" 或
 * "wrong type argument to signbit"）。 */
#include <math.h>
int bad_int_arg(void)
{
    int i = -5;
    return signbit(i);   /* 错误：实参不是 real-floating 类型 */
}

/* 违反约束「[1] Synopsis：signbit 接受 real-floating 实参」：
 * 传入指针类型实参，同样不满足 real-floating 约束，应编译报错。 */
int bad_ptr_arg(void)
{
    double d = -1.0;
    double *p = &d;
    return signbit(p);   /* 错误：实参是指针，不是 real-floating */
}

/* 违反约束「[1] Synopsis：signbit 接受 real-floating 实参」：
 * 传入复数类型实参。复数不是 real-floating，应编译报错。 */
#include <complex.h>
int bad_complex_arg(void)
{
    double complex z = -1.0 + 0.0 * I;
    return signbit(z);   /* 错误：实参是复数，不是 real-floating */
}

/* 违反约束「[1] Synopsis：signbit 接受 real-floating 实参」：
 * 传入结构体类型实参，应编译报错。 */
struct S { double x; };
int bad_struct_arg(void)
{
    struct S s = { -1.0 };
    return signbit(s);   /* 错误：实参是结构体，不是 real-floating */
}

#endif /* 负向测试结束 */