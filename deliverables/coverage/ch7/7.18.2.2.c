/*
 * 测试条款：C99 7.18.2.2 —— Limits of minimum-width integer types
 *
 * 条款要求 <stdint.h> 定义以下宏：
 *   [1] INT_LEASTN_MIN  = -(2^(N-1) - 1)  （最小宽度有符号类型的最小值）
 *       INT_LEASTN_MAX  =   2^(N-1) - 1   （最小宽度有符号类型的最大值）
 *       UINT_LEASTN_MAX =   2^N - 1       （最小宽度无符号类型的最大值）
 *   其中 N ∈ {8,16,32,64}。
 *
 * 预期行为：
 *   正向测试：这些宏存在、类型正确、取值符合公式，程序编译并运行通过。
 *   负向测试：违反约束的用法（如把宏当左值赋值、缺少 <stdint.h> 时使用）
 *             应导致编译报错。
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 宏必须存在（若未定义，下面 #ifndef 会触发 #error） */
#ifndef INT_LEAST8_MIN
#error "INT_LEAST8_MIN 未定义"
#endif
#ifndef INT_LEAST8_MAX
#error "INT_LEAST8_MAX 未定义"
#endif
#ifndef UINT_LEAST8_MAX
#error "UINT_LEAST8_MAX 未定义"
#endif
#ifndef INT_LEAST16_MIN
#error "INT_LEAST16_MIN 未定义"
#endif
#ifndef INT_LEAST16_MAX
#error "INT_LEAST16_MAX 未定义"
#endif
#ifndef UINT_LEAST16_MAX
#error "UINT_LEAST16_MAX 未定义"
#endif
#ifndef INT_LEAST32_MIN
#error "INT_LEAST32_MIN 未定义"
#endif
#ifndef INT_LEAST32_MAX
#error "INT_LEAST32_MAX 未定义"
#endif
#ifndef UINT_LEAST32_MAX
#error "UINT_LEAST32_MAX 未定义"
#endif
#ifndef INT_LEAST64_MIN
#error "INT_LEAST64_MIN 未定义"
#endif
#ifndef INT_LEAST64_MAX
#error "INT_LEAST64_MAX 未定义"
#endif
#ifndef UINT_LEAST64_MAX
#error "UINT_LEAST64_MAX 未定义"
#endif

/* [1] 取值公式验证：INT_LEASTN_MIN == -(2^(N-1) - 1) */
static void test_least8(void)
{
    /* INT_LEAST8_MIN = -(2^7 - 1) = -127 */
    assert(INT_LEAST8_MIN == -127);
    /* INT_LEAST8_MAX = 2^7 - 1 = 127 */
    assert(INT_LEAST8_MAX == 127);
    /* UINT_LEAST8_MAX = 2^8 - 1 = 255 */
    assert(UINT_LEAST8_MAX == 255);

    /* 类型必须能容纳这些值 */
    int_least8_t  s = INT_LEAST8_MIN;
    uint_least8_t u = UINT_LEAST8_MAX;
    assert(s == -127);
    assert(u == 255);
}

static void test_least16(void)
{
    /* INT_LEAST16_MIN = -(2^15 - 1) = -32767 */
    assert(INT_LEAST16_MIN == -32767);
    /* INT_LEAST16_MAX = 2^15 - 1 = 32767 */
    assert(INT_LEAST16_MAX == 32767);
    /* UINT_LEAST16_MAX = 2^16 - 1 = 65535 */
    assert(UINT_LEAST16_MAX == 65535);

    int_least16_t  s = INT_LEAST16_MIN;
    uint_least16_t u = UINT_LEAST16_MAX;
    assert(s == -32767);
    assert(u == 65535);
}

static void test_least32(void)
{
    /* INT_LEAST32_MIN = -(2^31 - 1) = -2147483647 */
    assert(INT_LEAST32_MIN == -2147483647);
    /* INT_LEAST32_MAX = 2^31 - 1 = 2147483647 */
    assert(INT_LEAST32_MAX == 2147483647);
    /* UINT_LEAST32_MAX = 2^32 - 1 = 4294967295 */
    assert(UINT_LEAST32_MAX == 4294967295U);

    int_least32_t  s = INT_LEAST32_MIN;
    uint_least32_t u = UINT_LEAST32_MAX;
    assert(s == -2147483647);
    assert(u == 4294967295U);
}

static void test_least64(void)
{
    /* INT_LEAST64_MIN = -(2^63 - 1) = -9223372036854775807 */
    assert(INT_LEAST64_MIN == -9223372036854775807LL);
    /* INT_LEAST64_MAX = 2^63 - 1 = 9223372036854775807 */
    assert(INT_LEAST64_MAX == 9223372036854775807LL);
    /* UINT_LEAST64_MAX = 2^64 - 1 = 18446744073709551615 */
    assert(UINT_LEAST64_MAX == 18446744073709551615ULL);

    int_least64_t  s = INT_LEAST64_MIN;
    uint_least64_t u = UINT_LEAST64_MAX;
    assert(s == -9223372036854775807LL);
    assert(u == 18446744073709551615ULL);
}

/* [1] 关系验证：MIN < 0 < MAX，且 UINT_MAX == 2*MAX + 1（对最小宽度类型成立） */
static void test_relations(void)
{
    assert(INT_LEAST8_MIN  < 0 && INT_LEAST8_MAX  > 0);
    assert(INT_LEAST16_MIN < 0 && INT_LEAST16_MAX > 0);
    assert(INT_LEAST32_MIN < 0 && INT_LEAST32_MAX > 0);
    assert(INT_LEAST64_MIN < 0 && INT_LEAST64_MAX > 0);

    /* UINT_LEASTN_MAX = 2^N - 1 = 2*(2^(N-1) - 1) + 1 = 2*INT_LEASTN_MAX + 1 */
    assert((uint_least8_t)UINT_LEAST8_MAX  == (uint_least8_t)(2 * INT_LEAST8_MAX  + 1));
    assert((uint_least16_t)UINT_LEAST16_MAX == (uint_least16_t)(2 * INT_LEAST16_MAX + 1));
    assert((uint_least32_t)UINT_LEAST32_MAX == (uint_least32_t)(2 * INT_LEAST32_MAX + 1));
    assert((uint_least64_t)UINT_LEAST64_MAX == (uint_least64_t)(2 * INT_LEAST64_MAX + 1));
}

/* [1] 宏可用于 #if 预处理表达式（整数常量表达式） */
#if INT_LEAST8_MAX != 127
#error "INT_LEAST8_MAX 在预处理期不等于 127"
#endif
#if UINT_LEAST16_MAX != 65535
#error "UINT_LEAST16_MAX 在预处理期不等于 65535"
#endif

int main(void)
{
    test_least8();
    test_least16();
    test_least32();
    test_least64();
    test_relations();

    printf("C99 7.18.2.2 正向测试全部通过\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「宏展开为整数常量表达式，不是可修改左值」：
 * 对 INT_LEAST8_MAX 赋值，gcc -std=c99 应报错
 *   error: lvalue required as left operand of assignment
 */
INT_LEAST8_MAX = 0;

/* 违反约束「同上」：对 UINT_LEAST32_MAX 赋值应报错 */
UINT_LEAST32_MAX = 1;

/* 违反约束「取地址操作数必须是左值」：
 * &INT_LEAST16_MIN 应报错
 *   error: lvalue required as unary '&' operand
 */
int *p = &INT_LEAST16_MIN;

/* 违反约束「自增/自减操作数必须是可修改左值」：
 * INT_LEAST64_MAX++ 应报错
 *   error: lvalue required as increment operand
 */
INT_LEAST64_MAX++;

/* 违反约束「未包含 <stdint.h> 时这些宏不可用」：
 * 在未包含头文件的作用域内使用 INT_LEAST8_MAX 应报错
 *   error: 'INT_LEAST8_MAX' undeclared
 * （注：本文件顶部已包含 <stdint.h>，此处仅示意约束语义）
 */

#endif /* 负向测试结束 */