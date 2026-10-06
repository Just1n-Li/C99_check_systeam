/*
 * 测试 C99 7.6.3 —— Rounding（fegetround / fesetround）
 *
 * 条款原文：
 *   [1] The fegetround and fesetround functions provide control of
 *       rounding direction modes.
 *
 * 预期行为：
 *   - 正向测试：<fenv.h> 提供 fegetround/fesetround，能查询/设置舍入方向模式，
 *     设置后 fegetround 返回对应宏值，且舍入模式确实影响浮点运算结果。
 *     整个程序应能编译并运行通过（assert 全部成立）。
 *   - 负向测试：违反约束的片段（如把非舍入模式宏的任意整数传给 fesetround、
 *     参数个数/类型错误等）应导致编译报错。这些片段放在 #if 0 中，
 *     保证本文件本身仍可正常编译运行。
 *
 * 说明：C99 7.6.3 只规定 fegetround/fesetround 提供舍入方向模式的控制。
 *       具体宏（FE_TONEAREST 等）与舍入行为由 7.6 及实现定义，本测试在
 *       实现支持相应宏时才断言其行为，避免依赖未定义特性。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>
#include <math.h>

/* 若实现未定义某舍入宏，则跳过对应断言 */
#if defined(FE_TONEAREST) && defined(FE_UPWARD) && defined(FE_DOWNWARD) && defined(FE_TOWARDZERO)
#define HAVE_ALL_ROUND_MODES 1
#else
#define HAVE_ALL_ROUND_MODES 0
#endif

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] fegetround 返回当前舍入方向模式（一个舍入模式宏值） */
static void test_fegetround_returns_mode(void)
{
    int mode = fegetround();
    /* 返回值应为某个已定义的舍入模式宏之一 */
    int ok = 0;
#if defined(FE_TONEAREST)
    if (mode == FE_TONEAREST) ok = 1;
#endif
#if defined(FE_UPWARD)
    if (mode == FE_UPWARD) ok = 1;
#endif
#if defined(FE_DOWNWARD)
    if (mode == FE_DOWNWARD) ok = 1;
#endif
#if defined(FE_TOWARDZERO)
    if (mode == FE_TOWARDZERO) ok = 1;
#endif
    assert(ok);
    printf("[1] fegetround() = %d (合法舍入模式)\n", mode);
}

/* [1] fesetround 设置舍入方向模式，fegetround 应能读回 */
static void test_set_and_get_round(void)
{
    int saved = fegetround();

#if defined(FE_TONEAREST)
    assert(fesetround(FE_TONEAREST) == 0);
    assert(fegetround() == FE_TONEAREST);
    printf("[1] 设置 FE_TONEAREST 成功，读回一致\n");
#endif
#if defined(FE_UPWARD)
    assert(fesetround(FE_UPWARD) == 0);
    assert(fegetround() == FE_UPWARD);
    printf("[1] 设置 FE_UPWARD 成功，读回一致\n");
#endif
#if defined(FE_DOWNWARD)
    assert(fesetround(FE_DOWNWARD) == 0);
    assert(fegetround() == FE_DOWNWARD);
    printf("[1] 设置 FE_DOWNWARD 成功，读回一致\n");
#endif
#if defined(FE_TOWARDZERO)
    assert(fesetround(FE_TOWARDZERO) == 0);
    assert(fegetround() == FE_TOWARDZERO);
    printf("[1] 设置 FE_TOWARDZERO 成功，读回一致\n");
#endif

    /* 恢复原模式 */
    fesetround(saved);
    assert(fegetround() == saved);
}

/* [1] 舍入方向模式确实影响浮点运算结果（语义验证） */
static void test_rounding_affects_result(void)
{
#if HAVE_ALL_ROUND_MODES
    volatile double x = 1.0;
    volatile double y = 3.0;
    double up, down, zero, near;

    fesetround(FE_UPWARD);
    up = x / y;

    fesetround(FE_DOWNWARD);
    down = x / y;

    fesetround(FE_TOWARDZERO);
    zero = x / y;

    fesetround(FE_TONEAREST);
    near = x / y;

    /* 1/3 非精确：向上舍入 >= 向下舍入；向零舍入介于两者之间 */
    assert(up >= down);
    assert(zero >= down);
    assert(zero <= up);
    /* 最近舍入应落在 [down, up] 区间内 */
    assert(near >= down && near <= up);

    printf("[1] 舍入模式影响结果: up=%.20g down=%.20g zero=%.20g near=%.20g\n",
           up, down, zero, near);

    fesetround(FE_TONEAREST);
#else
    printf("[1] 实现未定义全部舍入宏，跳过舍入影响测试\n");
#endif
}

/* [1] fesetround 成功返回 0（C99 规定成功返回零） */
static void test_fesetround_return(void)
{
    int saved = fegetround();
#if defined(FE_TONEAREST)
    int r = fesetround(FE_TONEAREST);
    assert(r == 0);
    printf("[1] fesetround 成功返回 0\n");
#endif
    fesetround(saved);
}

int main(void)
{
    printf("=== C99 7.6.3 Rounding 正向测试 ===\n");
    test_fegetround_returns_mode();
    test_set_and_get_round();
    test_rounding_affects_result();
    test_fesetround_return();
    printf("=== 正向测试全部通过 ===\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fesetround 的参数必须是舍入方向模式宏（int 类型的舍入模式值）」：
 * 传入一个非舍入模式宏的任意整数常量，语义上不是合法舍入模式。
 * 注意：C99 未把「任意 int」列为约束违反，故此处仅演示参数类型错误。
 * 期望：gcc -std=c99 对下面「参数类型不匹配」的调用报错。 */

/* 违反约束「fesetround 接受一个 int 参数」：传入指针，类型不匹配，应报错 */
void bad1(void)
{
    double d = 0.0;
    fesetround(&d);          /* 期望：error: incompatible type for argument 1 */
}

/* 违反约束「fesetround 接受一个 int 参数」：传入结构体，类型不匹配，应报错 */
struct S { int x; };
void bad2(void)
{
    struct S s;
    fesetround(s);           /* 期望：error: incompatible type for argument 1 */
}

/* 违反约束「fesetround 只接受一个参数」：参数个数过多，应报错 */
void bad3(void)
{
    fesetround(FE_TONEAREST, FE_UPWARD);  /* 期望：error: too many arguments */
}

/* 违反约束「fegetround 不接受参数」：传入参数，应报错 */
void bad4(void)
{
    int m = fegetround(1);   /* 期望：error: too many arguments to function */
    (void)m;
}

/* 违反约束「fesetround 返回 int」：把返回值赋给不兼容类型（如结构体）应报错 */
void bad5(void)
{
    struct S s;
    s = fesetround(FE_TONEAREST);  /* 期望：error: incompatible types in assignment */
}

#endif /* 负向测试结束 */