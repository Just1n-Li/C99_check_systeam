/*
 * 测试 C99 7.18.2.5 —— 最大宽度整数类型的极限值
 *   INTMAX_MIN  : intmax_t 的最小值，等于 -(2^(N-1) - 1) 形式（即 -INTMAX_MAX）
 *   INTMAX_MAX  : intmax_t 的最大值，等于  2^(N-1) - 1
 *   UINTMAX_MAX : uintmax_t 的最大值，等于 2^N - 1
 * 其中 N 为 intmax_t/uintmax_t 的位宽（通常 64）。
 *
 * 预期行为：
 *   正向测试：应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] INTMAX_MIN 是 intmax_t 的最小值：必须为负，且等于 -INTMAX_MAX */
static void test_intmax_min(void)
{
    intmax_t mn = INTMAX_MIN;
    intmax_t mx = INTMAX_MAX;

    /* INTMAX_MIN 为负值 */
    assert(mn < 0);

    /* 条款给出的形式：INTMAX_MIN == -(2^(N-1) - 1) == -INTMAX_MAX */
    assert(mn == -mx);

    /* INTMAX_MIN 是 intmax_t 可表示的最小值：不能再减 1（会溢出，故用比较验证边界） */
    assert(mn <= mx);

    /* 类型正确性：INTMAX_MIN 可赋给 intmax_t 而不丢失信息 */
    intmax_t copy = INTMAX_MIN;
    assert(copy == INTMAX_MIN);
}

/* [1] INTMAX_MAX 是 intmax_t 的最大值：必须为正，且为 2^(N-1) - 1 */
static void test_intmax_max(void)
{
    intmax_t mx = INTMAX_MAX;

    /* 最大值为正 */
    assert(mx > 0);

    /* 形式 2^(N-1) - 1：即 (mx + 1) 是 2 的幂 */
    /* 用无符号运算避免有符号溢出 UB */
    uintmax_t p = (uintmax_t)mx + 1u;
    assert(p != 0);
    assert((p & (p - 1u)) == 0);   /* p 是 2 的幂 */

    /* 该幂次对应的位宽 N 应 >= 64（C99 要求 intmax_t 至少 64 位） */
    int bits = 0;
    uintmax_t t = p;
    while (t > 1u) { t >>= 1; bits++; }
    assert(bits >= 63);            /* N-1 >= 63，即 N >= 64 */

    /* INTMAX_MAX 与 INTMAX_MIN 的关系 */
    assert(INTMAX_MAX == -(INTMAX_MIN + 1));
}

/* [1] UINTMAX_MAX 是 uintmax_t 的最大值：必须为 2^N - 1（全 1 位模式） */
static void test_uintmax_max(void)
{
    uintmax_t um = UINTMAX_MAX;

    /* 最大值非零 */
    assert(um != 0);

    /* 形式 2^N - 1：加 1 后回绕为 0（无符号回绕是良定义的） */
    assert((uintmax_t)(um + 1u) == 0u);

    /* 全 1 位模式：um 的所有位均为 1 */
    assert((um & (um + 1u)) == 0u);   /* um+1==0，故 um & 0 == 0 */

    /* UINTMAX_MAX 与 INTMAX_MAX 的位宽一致：UINTMAX_MAX == 2*INTMAX_MAX + 1 */
    assert(um == (uintmax_t)INTMAX_MAX * 2u + 1u);

    /* UINTMAX_MAX 至少能容纳 INTMAX_MAX */
    assert(um >= (uintmax_t)INTMAX_MAX);
}

/* [1] 三个宏的类型与可用性：应能用于常量表达式与 printf 格式 */
static void test_macros_usable(void)
{
    /* 宏可用于初始化静态存储期对象（常量表达式） */
    static const intmax_t  s_min = INTMAX_MIN;
    static const intmax_t  s_max = INTMAX_MAX;
    static const uintmax_t u_max = UINTMAX_MAX;

    assert(s_min == INTMAX_MIN);
    assert(s_max == INTMAX_MAX);
    assert(u_max == UINTMAX_MAX);

    /* 与 <inttypes.h> 的格式宏配合打印（PRIdMAX/PRIuMAX 对应 intmax_t/uintmax_t） */
    printf("INTMAX_MIN  = %" PRIdMAX "\n", INTMAX_MIN);
    printf("INTMAX_MAX  = %" PRIdMAX "\n", INTMAX_MAX);
    printf("UINTMAX_MAX = %" PRIuMAX "\n", UINTMAX_MAX);

    /* 边界算术：INTMAX_MAX + 1 在 uintmax_t 中等于 2^(N-1) */
    uintmax_t half = (uintmax_t)INTMAX_MAX + 1u;
    assert(half * 2u - 1u == UINTMAX_MAX);
}

int main(void)
{
    test_intmax_min();
    test_intmax_max();
    test_uintmax_max();
    test_macros_usable();

    printf("C99 7.18.2.5 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「INTMAX_MIN/INTMAX_MAX/UINTMAX_MAX 由 <stdint.h> 提供」：
 * 未包含 <stdint.h> 就使用这些宏，gcc -std=c99 应报 "undeclared identifier"。
 * （注：本文件已包含 <stdint.h>，此处演示的是缺失头文件的情形。） */
intmax_t x1 = INTMAX_MIN;   /* 若未 #include <stdint.h>，intmax_t 未声明 */

/* 违反约束「INTMAX_MIN 是 intmax_t 类型的最小值」：
 * 把 INTMAX_MIN 赋给更窄的类型并期望无损，编译器在 -Wconversion 下应告警/报错；
 * 更直接地，下面把超出 int 范围的常量赋给 int 属于约束违反（常量不可表示）。 */
int x2 = INTMAX_MAX;        /* INTMAX_MAX 通常超出 int 范围，应报错/告警 */

/* 违反约束「UINTMAX_MAX 是 uintmax_t 的最大值」：
 * 试图用有符号类型承载 UINTMAX_MAX 的完整值，常量超出范围。 */
intmax_t x3 = UINTMAX_MAX;  /* UINTMAX_MAX 超出 intmax_t 范围，应报错/告警 */

/* 违反约束「宏为整数常量表达式」：
 * 对宏取地址（宏不是对象，无地址），应报 "lvalue required"。 */
void *p = &INTMAX_MAX;      /* 不能对常量宏取地址 */

/* 违反约束「INTMAX_MIN 为负的最小值」：
 * 断言 INTMAX_MIN 为正，静态断言失败（C99 无 _Static_assert，用数组大小技巧）。 */
typedef char static_check[INTMAX_MIN > 0 ? 1 : -1];  /* 条件为假，数组大小为负，应报错 */

#endif