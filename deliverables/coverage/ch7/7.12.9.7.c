/*
 * 测试条款：C99 7.12.9.7  The lround and llround functions
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型（lround/lroundf/lroundl/llround/llroundf/llroundl）
 *   [2] 语义：四舍五入到最近整数，恰好一半时远离零方向舍入，
 *       不受当前舍入方向影响；结果超出返回类型范围时数值结果未指定，
 *       可能发生 domain error 或 range error。
 *   [3] 返回值：返回舍入后的整数值。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：通过取函数指针验证各函数签名与返回类型 */
static void test_prototypes(void)
{
    long int      (*p1)(double)      = lround;
    long int      (*p2)(float)       = lroundf;
    long int      (*p3)(long double) = lroundl;
    long long int (*p4)(double)      = llround;
    long long int (*p5)(float)       = llroundf;
    long long int (*p6)(long double) = llroundl;

    assert(p1 != NULL && p2 != NULL && p3 != NULL);
    assert(p4 != NULL && p5 != NULL && p6 != NULL);
}

/* [2][3] 基本四舍五入：非半值情况 */
static void test_basic_rounding(void)
{
    /* 2.3 -> 2, 2.7 -> 3 */
    assert(lround(2.3) == 2L);
    assert(lround(2.7) == 3L);
    assert(lround(-2.3) == -2L);
    assert(lround(-2.7) == -3L);

    /* 整数本身不变 */
    assert(lround(5.0) == 5L);
    assert(lround(-5.0) == -5L);
    assert(lround(0.0) == 0L);

    /* float / long double 版本 */
    assert(lroundf(2.3f) == 2L);
    assert(lroundf(2.7f) == 3L);
    assert(lroundl(2.3L) == 2L);
    assert(lroundl(2.7L) == 3L);

    /* llround 系列 */
    assert(llround(2.3) == 2LL);
    assert(llround(2.7) == 3LL);
    assert(llroundf(2.3f) == 2LL);
    assert(llroundf(2.7f) == 3LL);
    assert(llroundl(2.3L) == 2LL);
    assert(llroundl(2.7L) == 3LL);
}

/* [2] 恰好一半时远离零方向舍入（round half away from zero） */
static void test_halfway_away_from_zero(void)
{
    /* 0.5 -> 1, -0.5 -> -1 */
    assert(lround(0.5) == 1L);
    assert(lround(-0.5) == -1L);

    /* 1.5 -> 2, -1.5 -> -2 */
    assert(lround(1.5) == 2L);
    assert(lround(-1.5) == -2L);

    /* 2.5 -> 3, -2.5 -> -3 */
    assert(lround(2.5) == 3L);
    assert(lround(-2.5) == -3L);

    /* 3.5 -> 4, -3.5 -> -4 */
    assert(lround(3.5) == 4L);
    assert(lround(-3.5) == -4L);

    /* float / long double 版本 */
    assert(lroundf(0.5f) == 1L);
    assert(lroundf(-0.5f) == -1L);
    assert(lroundl(0.5L) == 1L);
    assert(lroundl(-0.5L) == -1L);

    /* llround 系列 */
    assert(llround(0.5) == 1LL);
    assert(llround(-0.5) == -1LL);
    assert(llround(2.5) == 3LL);
    assert(llround(-2.5) == -3LL);
    assert(llroundf(1.5f) == 2LL);
    assert(llroundf(-1.5f) == -2LL);
    assert(llroundl(1.5L) == 2LL);
    assert(llroundl(-1.5L) == -2LL);
}

/* [2] 不受当前舍入方向影响：设置 FE_DOWNWARD / FE_UPWARD / FE_TOWARDZERO
 *     后，lround 的结果仍应为「远离零」的舍入。 */
static void test_independent_of_rounding_direction(void)
{
    int saved = fegetround();

    if (fesetround(FE_DOWNWARD) == 0) {
        /* 向下舍入模式下，2.5 仍应得到 3（远离零），而非 2 */
        assert(lround(2.5) == 3L);
        assert(lround(-2.5) == -3L);
        assert(lround(0.5) == 1L);
        assert(lround(-0.5) == -1L);
    }

    if (fesetround(FE_UPWARD) == 0) {
        /* 向上舍入模式下，-2.5 仍应得到 -3（远离零），而非 -2 */
        assert(lround(2.5) == 3L);
        assert(lround(-2.5) == -3L);
        assert(lround(0.5) == 1L);
        assert(lround(-0.5) == -1L);
    }

    if (fesetround(FE_TOWARDZERO) == 0) {
        /* 向零舍入模式下，2.5 仍应得到 3（远离零），而非 2 */
        assert(lround(2.5) == 3L);
        assert(lround(-2.5) == -3L);
        assert(lround(0.5) == 1L);
        assert(lround(-0.5) == -1L);
    }

    fesetround(saved);
}

/* [2][3] 大数值：仍在返回类型范围内时应正确返回 */
static void test_large_values(void)
{
    /* 在 long 范围内的大值 */
    double big = 1e15;
    assert(lround(big) == 1000000000000000L);
    assert(lround(-big) == -1000000000000000L);

    /* llround 可处理更大范围 */
    double bigger = 1e18;
    assert(llround(bigger) == 1000000000000000000LL);
    assert(llround(-bigger) == -1000000000000000000LL);
}

/* [3] 返回值类型检查：lround 返回 long，llround 返回 long long */
static void test_return_types(void)
{
    long lv = lround(3.7);
    long long llv = llround(3.7);
    assert(lv == 4L);
    assert(llv == 4LL);

    /* 类型宽度关系（C99 保证 long long 至少和 long 一样宽） */
    assert(sizeof(long long) >= sizeof(long));
}

int main(void)
{
    test_prototypes();
    test_basic_rounding();
    test_halfway_away_from_zero();
    test_independent_of_rounding_direction();
    test_large_values();
    test_return_types();

    printf("C99 7.12.9.7 lround/llround: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数/类型必须匹配原型」：
 * lround 原型为 long lround(double)，传入两个实参应报错。
 * 期望：gcc -std=c99 报 "too many arguments to function 'lround'"。 */
void bad_arg_count(void)
{
    long r = lround(1.5, 2.5);
    (void)r;
}

/* 违反约束「函数调用实参个数必须匹配原型」：
 * llround 原型为 long long llround(double)，不传实参应报错。
 * 期望：gcc -std=c99 报 "too few arguments to function 'llround'"。 */
void bad_arg_missing(void)
{
    long long r = llround();
    (void)r;
}

/* 违反约束「赋值目标必须是可修改的左值」：
 * lround 的返回值是右值（非左值），不能赋值。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"。 */
void bad_assign_to_return(void)
{
    lround(1.5) = 7L;
}

/* 违反约束「取地址操作数必须是左值」：
 * 函数返回值不是左值，不能取地址。
 * 期望：gcc -std=c99 报 "lvalue required as unary '&' operand"。 */
void bad_address_of_return(void)
{
    long *p = &lround(1.5);
    (void)p;
}

/* 违反约束「函数名在调用中必须已声明」：
 * 未包含 <math.h> 且未声明 lround 时调用，C99 不允许隐式函数声明。
 * 期望：gcc -std=c99 报 "implicit declaration of function 'lround'"。 */
void bad_implicit_decl(void)
{
    /* 注意：本片段假设 <math.h> 未提供声明；实际测试时需单独编译。 */
    long r = lround(1.5);
    (void)r;
}

#endif /* 负向测试结束 */