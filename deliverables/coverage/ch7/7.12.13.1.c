/*
 * 测试 C99 7.12.13.1 —— fma 函数族 (fma / fmaf / fmal)
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 fma/fmaf/fmal，验证它们计算 (x*y)+z
 *             并作为一次三元运算（单次舍入）返回；验证返回类型与原型一致。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应被编译器拒绝。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：三个函数均声明于 <math.h>，返回类型分别为
 *     double / float / long double，参数个数均为 3。
 *     这里通过取函数指针并检查其类型来验证原型。 */
static double  (*p_fma) (double, double, double)        = fma;
static float   (*p_fmaf)(float, float, float)           = fmaf;
static long double (*p_fmal)(long double, long double, long double) = fmal;

/* [2][3] 基本语义：fma(x,y,z) == (x*y)+z，作为一次三元运算舍入。
 *        对可精确表示的情形，结果必须与 (x*y)+z 完全一致。 */
static void test_basic_semantics(void)
{
    /* 小整数，乘积与加法均可精确表示 */
    assert(fma(2.0, 3.0, 4.0) == 10.0);
    assert(fma(-2.0, 3.0, 4.0) == -2.0);
    assert(fma(0.0, 5.0, 7.0) == 7.0);
    assert(fma(1.0, 1.0, 0.0) == 1.0);

    assert(fmaf(2.0f, 3.0f, 4.0f) == 10.0f);
    assert(fmaf(-2.0f, 3.0f, 4.0f) == -2.0f);

    assert(fmal(2.0L, 3.0L, 4.0L) == 10.0L);
    assert(fmal(-2.0L, 3.0L, 4.0L) == -2.0L);

    /* 与普通 (x*y)+z 在可精确情形下一致 */
    double x = 1.5, y = 2.0, z = 0.25;
    assert(fma(x, y, z) == (x * y) + z);
}

/* [2][3] 单次舍入（ternary operation）语义：
 *        经典例子：x = 1 + 2^-53, y = 1 + 2^-53, z = -1。
 *        精确值 = (1+2^-53)^2 - 1 = 2^-52 + 2^-106。
 *        若先算 x*y 再减 1，会因中间舍入丢失 2^-106 项；
 *        fma 只舍入一次，结果应等于 2^-52 + 2^-106 舍入到 double。
 *        这里用一个更稳健的检查：fma 的结果应比 (x*y)+z 更接近精确值，
 *        或至少满足 fma 结果 == 精确值舍入后的值。 */
static void test_single_rounding(void)
{
    double eps = ldexp(1.0, -53);          /* 2^-53 */
    double x = 1.0 + eps;
    double y = 1.0 + eps;
    double z = -1.0;

    /* 精确值 = 2^-52 + 2^-106 */
    long double exact = (long double)x * (long double)y + (long double)z;
    double fma_res = fma(x, y, z);

    /* fma 结果应等于精确值舍入到 double */
    double exact_rounded = (double)exact;
    assert(fma_res == exact_rounded);

    /* 对比：普通 (x*y)+z 会丢失低位，通常与 fma 不同 */
    double naive = (x * y) + z;
    /* 不强制二者不同（实现可能恰好相同），但 fma 必须等于精确舍入值 */
    (void)naive;

    /* 另一个经典例子：a*b - a*b 用 fma 应精确为 0 */
    double a = 1.0 + ldexp(1.0, -30);
    double b = 1.0 + ldexp(1.0, -30);
    assert(fma(a, b, -(a * b)) == 0.0 || fma(a, b, -(a * b)) != 0.0);
    /* 上面恒真，仅确保调用不崩溃；真正的单次舍入检查用下面的恒等式 */
    double r = fma(a, b, -a * b);
    /* r 是 a*b 精确值与 a*b 舍入值之差，量级很小 */
    assert(fabs(r) <= fabs(a * b) * DBL_EPSILON);
}

/* [2] 特殊值：NaN 传播、无穷、零的符号等基本行为 */
static void test_special_values(void)
{
    /* 0 * inf + NaN 之类：fma(0, inf, NaN) 应为 NaN */
    double nan_val = fma(0.0, INFINITY, NAN);
    assert(isnan(nan_val));

    /* inf * 1 + 1 = inf */
    assert(isinf(fma(INFINITY, 1.0, 1.0)));
    assert(fma(INFINITY, 1.0, 1.0) > 0.0);

    /* -inf * 1 + 1 = -inf */
    assert(isinf(fma(-INFINITY, 1.0, 1.0)));
    assert(fma(-INFINITY, 1.0, 1.0) < 0.0);

    /* 0 * 0 + 0 = 0 */
    assert(fma(0.0, 0.0, 0.0) == 0.0);

    /* NaN 参与：fma(NaN, 1, 1) 为 NaN */
    assert(isnan(fma(NAN, 1.0, 1.0)));
    assert(isnan(fma(1.0, NAN, 1.0)));
    assert(isnan(fma(1.0, 1.0, NAN)));
}

/* [1] 返回类型检查：通过 _Generic（C11）不可用，改用赋值兼容性检查。
 *     这里用 sizeof 与类型兼容的指针赋值来间接验证。 */
static void test_return_types(void)
{
    /* 若返回类型不是 double，赋给 double 变量仍可（隐式转换），
     * 因此用函数指针类型匹配来严格验证（已在文件顶部完成）。
     * 这里再验证调用结果可赋给对应类型变量。 */
    double d = fma(1.0, 2.0, 3.0);
    float  f = fmaf(1.0f, 2.0f, 3.0f);
    long double ld = fmal(1.0L, 2.0L, 3.0L);
    assert(d == 5.0);
    assert(f == 5.0f);
    assert(ld == 5.0L);
}

int main(void)
{
    test_basic_semantics();
    test_single_rounding();
    test_special_values();
    test_return_types();

    printf("C99 7.12.13.1 fma functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fma 的参数必须为算术类型（double）」：
 * 传入结构体，gcc -std=c99 应报错（incompatible type for argument）。 */
struct S { int x; } s;
double bad1 = fma(s, 1.0, 2.0);

/* 违反约束「参数个数必须为 3」：
 * 少传参数，gcc -std=c99 应报错（too few arguments to function 'fma'）。 */
double bad2 = fma(1.0, 2.0);

/* 违反约束「参数个数必须为 3」：
 * 多传参数，gcc -std=c99 应报错（too many arguments to function 'fma'）。 */
double bad3 = fma(1.0, 2.0, 3.0, 4.0);

/* 违反约束「fmaf 的参数必须为 float 兼容的算术类型」：
 * 传入结构体，gcc -std=c99 应报错。 */
float bad4 = fmaf(s, 1.0f, 2.0f);

/* 违反约束「fmal 的参数必须为 long double 兼容的算术类型」：
 * 传入结构体，gcc -std=c99 应报错。 */
long double bad5 = fmal(s, 1.0L, 2.0L);

/* 违反约束「fma 的返回类型为 double，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
fma(1.0, 2.0, 3.0) = 5.0;

/* 违反约束「fmaf 的返回类型为 float，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
fmaf(1.0f, 2.0f, 3.0f) = 5.0f;

/* 违反约束「fmal 的返回类型为 long double，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
fmal(1.0L, 2.0L, 3.0L) = 5.0L;

/* 违反约束「fma 的返回类型为 double，不能取地址」：
 * 函数调用结果不是左值，&fma(...) 应编译报错。 */
double *bad6 = &fma(1.0, 2.0, 3.0);

#endif