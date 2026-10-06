/*
 * 测试 C99 7.18.1.1 —— Exact-width integer types（精确宽度整数类型）
 *
 * 预期行为：
 *   正向测试：若实现提供了宽度为 8/16/32/64 位、无填充位、有符号类型为
 *             二进制补码表示的整数类型，则 <stdint.h> 必须定义对应的
 *             intN_t / uintN_t typedef 名；程序应能编译并运行通过。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 覆盖段落：
 *   [1] intN_t 为有符号、宽度恰为 N、无填充位、二进制补码
 *   [2] uintN_t 为无符号、宽度恰为 N
 *   [3] 这些类型是可选的；但若实现提供了满足条件的 8/16/32/64 位类型，
 *       则必须定义相应的 typedef 名
 */

#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] 若实现提供 8/16/32/64 位且无填充位、有符号为补码的类型，
 *     则必须定义对应的 typedef 名。用条件编译探测实现是否提供，
 *     若提供则验证其性质。 */

/* ---------- [1] intN_t：有符号、宽度恰为 N、无填充位、二进制补码 ---------- */

#ifdef INT8_MAX
/* 实现提供了 int8_t */
static void test_int8(void)
{
    /* [1] 宽度恰为 8 位 */
    assert(sizeof(int8_t) * CHAR_BIT == 8);
    /* [1] 有符号类型 */
    assert((int8_t)-1 < 0);
    /* [1] 无填充位：宽度 == 8，故取值范围为 -128..127 */
    assert(INT8_MIN == -128);
    assert(INT8_MAX == 127);
    /* [1] 二进制补码表示：-1 的所有位为 1，即 (uint8_t)-1 == 255 */
    assert((uint8_t)(int8_t)-1 == 255);
    /* [1] 补码：INT8_MIN 的位模式为 0x80 */
    assert((uint8_t)INT8_MIN == 0x80);
}
#endif

#ifdef INT16_MAX
static void test_int16(void)
{
    /* [1] 宽度恰为 16 位 */
    assert(sizeof(int16_t) * CHAR_BIT == 16);
    /* [1] 有符号类型 */
    assert((int16_t)-1 < 0);
    /* [1] 无填充位 */
    assert(INT16_MIN == -32768);
    assert(INT16_MAX == 32767);
    /* [1] 二进制补码 */
    assert((uint16_t)(int16_t)-1 == 65535);
    assert((uint16_t)INT16_MIN == 0x8000);
}
#endif

#ifdef INT32_MAX
static void test_int32(void)
{
    /* [1] 宽度恰为 32 位 */
    assert(sizeof(int32_t) * CHAR_BIT == 32);
    /* [1] 有符号类型 */
    assert((int32_t)-1 < 0);
    /* [1] 无填充位 */
    assert(INT32_MIN == -2147483647 - 1);
    assert(INT32_MAX == 2147483647);
    /* [1] 二进制补码 */
    assert((uint32_t)(int32_t)-1 == 4294967295u);
    assert((uint32_t)INT32_MIN == 0x80000000u);
}
#endif

#ifdef INT64_MAX
static void test_int64(void)
{
    /* [1] 宽度恰为 64 位 */
    assert(sizeof(int64_t) * CHAR_BIT == 64);
    /* [1] 有符号类型 */
    assert((int64_t)-1 < 0);
    /* [1] 无填充位 */
    assert(INT64_MIN == -9223372036854775807LL - 1);
    assert(INT64_MAX == 9223372036854775807LL);
    /* [1] 二进制补码 */
    assert((uint64_t)(int64_t)-1 == 18446744073709551615ULL);
    assert((uint64_t)INT64_MIN == 0x8000000000000000ULL);
}
#endif

/* ---------- [2] uintN_t：无符号、宽度恰为 N ---------- */

#ifdef UINT8_MAX
static void test_uint8(void)
{
    /* [2] 宽度恰为 8 位 */
    assert(sizeof(uint8_t) * CHAR_BIT == 8);
    /* [2] 无符号类型 */
    assert((uint8_t)-1 > 0);
    /* [2] 取值范围 0..255 */
    assert(UINT8_MAX == 255);
    assert((uint8_t)0 == 0);
}
#endif

#ifdef UINT16_MAX
static void test_uint16(void)
{
    /* [2] 宽度恰为 16 位 */
    assert(sizeof(uint16_t) * CHAR_BIT == 16);
    /* [2] 无符号类型 */
    assert((uint16_t)-1 > 0);
    /* [2] 取值范围 0..65535 */
    assert(UINT16_MAX == 65535);
}
#endif

#ifdef UINT32_MAX
static void test_uint32(void)
{
    /* [2] 宽度恰为 32 位 */
    assert(sizeof(uint32_t) * CHAR_BIT == 32);
    /* [2] 无符号类型 */
    assert((uint32_t)-1 > 0);
    /* [2] 取值范围 0..4294967295 */
    assert(UINT32_MAX == 4294967295u);
}
#endif

#ifdef UINT64_MAX
static void test_uint64(void)
{
    /* [2] 宽度恰为 64 位 */
    assert(sizeof(uint64_t) * CHAR_BIT == 64);
    /* [2] 无符号类型 */
    assert((uint64_t)-1 > 0);
    /* [2] 取值范围 0..18446744073709551615 */
    assert(UINT64_MAX == 18446744073709551615ULL);
}
#endif

/* ---------- [3] 可选性：至少应能探测到实现是否提供这些类型 ---------- */

static void test_optionality(void)
{
    /* [3] 这些类型是可选的。实现要么定义 typedef 名（并定义对应的
     *     INTN_MAX/UINTN_MAX 宏），要么不定义。这里验证：若定义了
     *     INTN_MAX，则对应的 intN_t 一定可用且宽度正确。 */
#ifdef INT8_MAX
    assert(sizeof(int8_t) * CHAR_BIT == 8);
#endif
#ifdef INT16_MAX
    assert(sizeof(int16_t) * CHAR_BIT == 16);
#endif
#ifdef INT32_MAX
    assert(sizeof(int32_t) * CHAR_BIT == 32);
#endif
#ifdef INT64_MAX
    assert(sizeof(int64_t) * CHAR_BIT == 64);
#endif
    /* 若实现未提供任何精确宽度类型，本函数仍能编译通过（可选性） */
}

/* ---------- 综合：用精确宽度类型做一次实际运算 ---------- */

static void test_usage(void)
{
#ifdef INT32_MAX
    int32_t a = 100000;
    int32_t b = 200000;
    int32_t c = a + b;
    assert(c == 300000);
#endif
#ifdef UINT16_MAX
    uint16_t u = 65535;
    uint16_t v = (uint16_t)(u + 1); /* 回绕到 0 */
    assert(v == 0);
#endif
}

int main(void)
{
#ifdef INT8_MAX
    test_int8();
    printf("int8_t  : OK (width=8, two's complement)\n");
#else
    printf("int8_t  : not provided (optional)\n");
#endif

#ifdef INT16_MAX
    test_int16();
    printf("int16_t : OK (width=16, two's complement)\n");
#else
    printf("int16_t : not provided (optional)\n");
#endif

#ifdef INT32_MAX
    test_int32();
    printf("int32_t : OK (width=32, two's complement)\n");
#else
    printf("int32_t : not provided (optional)\n");
#endif

#ifdef INT64_MAX
    test_int64();
    printf("int64_t : OK (width=64, two's complement)\n");
#else
    printf("int64_t : not provided (optional)\n");
#endif

#ifdef UINT8_MAX
    test_uint8();
    printf("uint8_t : OK (width=8, unsigned)\n");
#endif
#ifdef UINT16_MAX
    test_uint16();
    printf("uint16_t: OK (width=16, unsigned)\n");
#endif
#ifdef UINT32_MAX
    test_uint32();
    printf("uint32_t: OK (width=32, unsigned)\n");
#endif
#ifdef UINT64_MAX
    test_uint64();
    printf("uint64_t: OK (width=64, unsigned)\n");
#endif

    test_optionality();
    test_usage();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 说明：7.18.1.1 本身是「类型定义」条款，其约束主要体现在
 * <stdint.h> 中这些 typedef 名所代表的类型性质上。以下片段
 * 故意违反由该条款语义所隐含的约束，期望编译器报错。
 */

/* 违反约束「intN_t 为有符号类型」：把 int8_t 当作无符号使用，
 * 通过静态断言（C11 _Static_assert 或编译期数组大小技巧）验证
 * 其有符号性。若实现把 int8_t 定义为无符号，则下列断言失败，
 * gcc -std=c99 应报错（数组大小为负）。 */
#include <stdint.h>
typedef char assert_int8_signed[(int8_t)-1 < 0 ? 1 : -1];

/* 违反约束「uintN_t 为无符号类型」：若 uint8_t 被定义为有符号，
 * 则下列断言失败，编译报错。 */
typedef char assert_uint8_unsigned[(uint8_t)-1 > 0 ? 1 : -1];

/* 违反约束「宽度恰为 N 位」：若 int32_t 宽度不是 32 位，
 * 则下列断言失败，编译报错。 */
typedef char assert_int32_width[(sizeof(int32_t) * 8 == 32) ? 1 : -1];

/* 违反约束「无填充位」：若 int16_t 有填充位，则其取值范围
 * 不会恰好是 -32768..32767，下列断言失败，编译报错。 */
typedef char assert_int16_no_padding[(INT16_MIN == -32768 && INT16_MAX == 32767) ? 1 : -1];

/* 违反约束「二进制补码表示」：若 int8_t 不是补码，则 (uint8_t)INT8_MIN
 * 不会等于 0x80，下列断言失败，编译报错。 */
typedef char assert_int8_twos_complement[((uint8_t)INT8_MIN == 0x80) ? 1 : -1];

#endif