/*
 * 测试条款：C99 7.18.1.2 Minimum-width integer types
 *
 * 预期行为：
 *   正向测试：包含 <stdint.h> 后，条款 [3] 要求的 8 个最小宽度整数类型必须存在，
 *             且满足 [1][2] 的语义——宽度至少为 N，且不存在更小 size 的类型能达到该宽度。
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：本条款本身没有显式的 "Constraints" 段落，其约束性内容体现在
 *       [3] 中“以下类型是必需的”——若实现未提供这些 typedef，则违反条款。
 *       负向测试用“使用未定义类型名”来模拟“实现未提供必需类型”的编译错误。
 */

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] 条款要求的 8 个最小宽度整数类型必须存在。
 *     通过声明这些类型的对象来验证 typedef 名字可用。 */
static int_least8_t   v_i8;
static uint_least8_t  v_u8;
static int_least16_t  v_i16;
static uint_least16_t v_u16;
static int_least32_t  v_i32;
static uint_least32_t v_u32;
static int_least64_t  v_i64;
static uint_least64_t v_u64;

/* [1] int_leastN_t 是有符号整数类型，宽度至少 N 位。
 *     用 sizeof 与 CHAR_BIT 计算实际宽度，验证 >= N。 */
static void test_signed_min_width(void)
{
    /* [1] int_least8_t 宽度至少 8 位 */
    assert(sizeof(int_least8_t) * CHAR_BIT >= 8);
    /* [1] int_least16_t 宽度至少 16 位 */
    assert(sizeof(int_least16_t) * CHAR_BIT >= 16);
    /* [1] int_least32_t 宽度至少 32 位 */
    assert(sizeof(int_least32_t) * CHAR_BIT >= 32);
    /* [1] int_least64_t 宽度至少 64 位 */
    assert(sizeof(int_least64_t) * CHAR_BIT >= 64);

    /* [1] 这些类型必须是有符号类型：能表示负值 */
    int_least8_t  a = -1;
    int_least16_t b = -1;
    int_least32_t c = -1;
    int_least64_t d = -1;
    assert(a < 0);
    assert(b < 0);
    assert(c < 0);
    assert(d < 0);
}

/* [2] uint_leastN_t 是无符号整数类型，宽度至少 N 位。 */
static void test_unsigned_min_width(void)
{
    /* [2] uint_least8_t 宽度至少 8 位 */
    assert(sizeof(uint_least8_t) * CHAR_BIT >= 8);
    /* [2] uint_least16_t 宽度至少 16 位 */
    assert(sizeof(uint_least16_t) * CHAR_BIT >= 16);
    /* [2] uint_least32_t 宽度至少 32 位 */
    assert(sizeof(uint_least32_t) * CHAR_BIT >= 32);
    /* [2] uint_least64_t 宽度至少 64 位 */
    assert(sizeof(uint_least64_t) * CHAR_BIT >= 64);

    /* [2] 这些类型必须是无符号类型：所有值非负，且 -1 转换后为最大值 */
    uint_least8_t  a = (uint_least8_t)-1;
    uint_least16_t b = (uint_least16_t)-1;
    uint_least32_t c = (uint_least32_t)-1;
    uint_least64_t d = (uint_least64_t)-1;
    assert(a > 0);
    assert(b > 0);
    assert(c > 0);
    assert(d > 0);
}

/* [1][2] “不存在更小 size 的有符号/无符号类型能达到该宽度”：
 *        即 int_leastN_t 的 size 不应大于任何同样满足宽度 N 的整数类型的 size。
 *        这里用“最小可用类型”的语义做可观察验证：
 *        int_leastN_t 的宽度应等于或小于同宽度族中更大的类型（单调不减）。
 *        更直接的验证：int_leastN_t 的 size 不超过 intN_t（若存在）的 size，
 *        但 intN_t 是可选类型，故此处只验证单调性。 */
static void test_minimality_monotonic(void)
{
    /* 宽度要求越大，类型 size 不应变小（保证“最小”语义的一致性） */
    assert(sizeof(int_least8_t)  <= sizeof(int_least16_t));
    assert(sizeof(int_least16_t) <= sizeof(int_least32_t));
    assert(sizeof(int_least32_t) <= sizeof(int_least64_t));

    assert(sizeof(uint_least8_t)  <= sizeof(uint_least16_t));
    assert(sizeof(uint_least16_t) <= sizeof(uint_least32_t));
    assert(sizeof(uint_least32_t) <= sizeof(uint_least64_t));
}

/* [1][2] 验证这些类型可用于常规算术运算，且能容纳对应宽度的值。 */
static void test_arithmetic(void)
{
    /* 能容纳 8 位有符号范围 [-128, 127] */
    int_least8_t  i8  = 127;
    int_least8_t  i8n = -128;
    assert(i8 == 127);
    assert(i8n == -128);

    /* 能容纳 16 位无符号范围 [0, 65535] */
    uint_least16_t u16 = 65535u;
    assert(u16 == 65535u);

    /* 能容纳 32 位有符号范围边界附近的值 */
    int_least32_t i32 = 2147483647;
    assert(i32 == 2147483647);

    /* 能容纳 64 位无符号范围边界附近的值 */
    uint_least64_t u64 = 18446744073709551615ull;
    assert(u64 == 18446744073709551615ull);

    /* 算术运算 */
    int_least32_t sum = (int_least32_t)100 + (int_least32_t)200;
    assert(sum == 300);
}

int main(void)
{
    test_signed_min_width();      /* [1] */
    test_unsigned_min_width();    /* [2] */
    test_minimality_monotonic();  /* [1][2] 最小性 */
    test_arithmetic();            /* [1][2] 可用性 */

    printf("C99 7.18.1.2 minimum-width integer types: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 违反约束「[3] 以下类型是必需的」：
 * 若实现未提供 int_least8_t，则使用该名字应编译报错。
 * 期望：gcc -std=c99 报 "unknown type name 'int_least8_t'" 或类似错误。
 * 这里通过“未包含 <stdint.h> 就使用该类型名”来模拟实现未提供必需类型的情形。
 */
int_least8_t missing_type_use;   /* 期望报错：类型名未定义 */

/*
 * 违反约束「[3] 以下类型是必需的」：
 * 同理，uint_least64_t 是必需类型，未定义时使用应报错。
 */
uint_least64_t missing_type_use2; /* 期望报错：类型名未定义 */

/*
 * 违反约束「[1] int_leastN_t 是有符号整数类型」：
 * 若把 int_least32_t 当作结构体类型使用（例如取不存在的成员），应报错。
 * 期望：gcc -std=c99 报 "request for member 'x' in something not a structure or union"。
 */
int_least32_t s;
s.x = 1;   /* 期望报错：int_least32_t 不是结构体/联合体 */

/*
 * 违反约束「[2] uint_leastN_t 是无符号整数类型」：
 * 若把 uint_least16_t 当作指针解引用（非指针类型），应报错。
 * 期望：gcc -std=c99 报 "invalid type argument of unary '*'"。
 */
uint_least16_t u;
*u = 0;    /* 期望报错：uint_least16_t 不是指针类型 */

#endif /* 负向测试结束 */