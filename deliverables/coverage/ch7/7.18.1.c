/*
 * 测试 C99 7.18.1 <stdint.h> Integer types
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错。
 *
 * 条款要点：
 *   [1] 仅相差开头 u 的 typedef 名，必须分别表示对应的有符号/无符号类型；
 *       实现提供其中一个，就必须提供另一个。
 *   [2] 描述中的 N 表示无前导零的无符号十进制整数（如 8、24，而非 04、048）。
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 对应的有符号/无符号类型必须成对存在。
 *     这里逐一检查标准要求成对提供的类型。
 *     使用 _Generic 在编译期确认类型对应关系（C99 无 _Generic，
 *     故改用 sizeof 与取值范围检查，保证可移植到 C99）。 */

/* [1] 精确宽度类型 intN_t / uintN_t 成对存在 */
static void test_exact_width_pairs(void)
{
    /* 8 位 */
    assert(sizeof(int8_t)  == 1);
    assert(sizeof(uint8_t) == 1);
    assert((int8_t)-1 < 0);                 /* 有符号 */
    assert((uint8_t)-1 > 0);                /* 无符号 */
    assert(INT8_MAX  == 127);
    assert(UINT8_MAX == 255);

    /* 16 位 */
    assert(sizeof(int16_t)  == 2);
    assert(sizeof(uint16_t) == 2);
    assert((int16_t)-1 < 0);
    assert((uint16_t)-1 > 0);
    assert(INT16_MAX  == 32767);
    assert(UINT16_MAX == 65535);

    /* 32 位 */
    assert(sizeof(int32_t)  == 4);
    assert(sizeof(uint32_t) == 4);
    assert((int32_t)-1 < 0);
    assert((uint32_t)-1 > 0);
    assert(INT32_MAX  == 2147483647);
    assert(UINT32_MAX == 4294967295u);

    /* 64 位 */
    assert(sizeof(int64_t)  == 8);
    assert(sizeof(uint64_t) == 8);
    assert((int64_t)-1 < 0);
    assert((uint64_t)-1 > 0);
    assert(INT64_MAX  == 9223372036854775807LL);
    assert(UINT64_MAX == 18446744073709551615ULL);
}

/* [1] 最小宽度类型 int_leastN_t / uint_leastN_t 成对存在 */
static void test_least_width_pairs(void)
{
    assert(sizeof(int_least8_t)  >= 1);
    assert(sizeof(uint_least8_t) >= 1);
    assert((int_least8_t)-1 < 0);
    assert((uint_least8_t)-1 > 0);

    assert(sizeof(int_least16_t)  >= 2);
    assert(sizeof(uint_least16_t) >= 2);
    assert((int_least16_t)-1 < 0);
    assert((uint_least16_t)-1 > 0);

    assert(sizeof(int_least32_t)  >= 4);
    assert(sizeof(uint_least32_t) >= 4);
    assert((int_least32_t)-1 < 0);
    assert((uint_least32_t)-1 > 0);

    assert(sizeof(int_least64_t)  >= 8);
    assert(sizeof(uint_least64_t) >= 8);
    assert((int_least64_t)-1 < 0);
    assert((uint_least64_t)-1 > 0);
}

/* [1] 最快最小宽度类型 int_fastN_t / uint_fastN_t 成对存在 */
static void test_fast_width_pairs(void)
{
    assert(sizeof(int_fast8_t)  >= 1);
    assert(sizeof(uint_fast8_t) >= 1);
    assert((int_fast8_t)-1 < 0);
    assert((uint_fast8_t)-1 > 0);

    assert(sizeof(int_fast16_t)  >= 2);
    assert(sizeof(uint_fast16_t) >= 2);
    assert((int_fast16_t)-1 < 0);
    assert((uint_fast16_t)-1 > 0);

    assert(sizeof(int_fast32_t)  >= 4);
    assert(sizeof(uint_fast32_t) >= 4);
    assert((int_fast32_t)-1 < 0);
    assert((uint_fast32_t)-1 > 0);

    assert(sizeof(int_fast64_t)  >= 8);
    assert(sizeof(uint_fast64_t) >= 8);
    assert((int_fast64_t)-1 < 0);
    assert((uint_fast64_t)-1 > 0);
}

/* [1] 指针宽度类型 intptr_t / uintptr_t 成对存在 */
static void test_intptr_pairs(void)
{
    assert(sizeof(intptr_t)  == sizeof(void *));
    assert(sizeof(uintptr_t) == sizeof(void *));
    assert((intptr_t)-1 < 0);
    assert((uintptr_t)-1 > 0);
}

/* [1] 最大宽度类型 intmax_t / uintmax_t 成对存在 */
static void test_intmax_pairs(void)
{
    assert(sizeof(intmax_t)  >= sizeof(long long));
    assert(sizeof(uintmax_t) >= sizeof(unsigned long long));
    assert((intmax_t)-1 < 0);
    assert((uintmax_t)-1 > 0);
    assert(INTMAX_MAX  == 9223372036854775807LL);
    assert(UINTMAX_MAX == 18446744073709551615ULL);
}

/* [1] 对应的有符号/无符号类型必须具有相同的表示宽度（sizeof 相等） */
static void test_corresponding_same_size(void)
{
    assert(sizeof(int8_t)  == sizeof(uint8_t));
    assert(sizeof(int16_t) == sizeof(uint16_t));
    assert(sizeof(int32_t) == sizeof(uint32_t));
    assert(sizeof(int64_t) == sizeof(uint64_t));

    assert(sizeof(int_least8_t)  == sizeof(uint_least8_t));
    assert(sizeof(int_least16_t) == sizeof(uint_least16_t));
    assert(sizeof(int_least32_t) == sizeof(uint_least32_t));
    assert(sizeof(int_least64_t) == sizeof(uint_least64_t));

    assert(sizeof(int_fast8_t)  == sizeof(uint_fast8_t));
    assert(sizeof(int_fast16_t) == sizeof(uint_fast16_t));
    assert(sizeof(int_fast32_t) == sizeof(uint_fast32_t));
    assert(sizeof(int_fast64_t) == sizeof(uint_fast64_t));

    assert(sizeof(intptr_t)  == sizeof(uintptr_t));
    assert(sizeof(intmax_t)  == sizeof(uintmax_t));
}

/* [1] 有符号类型的最小值/最大值与无符号类型的最大值满足 6.2.5 的对应关系：
 *     有符号类型最大值 == 无符号类型最大值 / 2
 *     有符号类型最小值 == -有符号类型最大值 - 1
 */
static void test_signed_unsigned_correspondence(void)
{
    assert(INT8_MAX  == UINT8_MAX  / 2);
    assert(INT8_MIN  == -INT8_MAX  - 1);
    assert(INT16_MAX == UINT16_MAX / 2);
    assert(INT16_MIN == -INT16_MAX - 1);
    assert(INT32_MAX == UINT32_MAX / 2);
    assert(INT32_MIN == -INT32_MAX - 1);
    assert(INT64_MAX == UINT64_MAX / 2);
    assert(INT64_MIN == -INT64_MAX - 1);

    assert(INTMAX_MAX == UINTMAX_MAX / 2);
    assert(INTMAX_MIN == -INTMAX_MAX - 1);
}

/* [2] N 表示无前导零的无符号十进制整数。
 *     标准中出现的 N 取值包括 8、16、32、64（精确/最小/最快宽度），
 *     以及 intptr_t、intmax_t 等无 N 的类型。
 *     这里验证标准中所有 N 对应的类型名都存在且可用。 */
static void test_N_values(void)
{
    /* N = 8 */
    int8_t   a8  = 0;  uint8_t   u8  = 0;
    int_least8_t  al8 = 0;  uint_least8_t  ul8 = 0;
    int_fast8_t   af8 = 0;  uint_fast8_t   uf8 = 0;
    assert(a8 == 0 && u8 == 0 && al8 == 0 && ul8 == 0 && af8 == 0 && uf8 == 0);

    /* N = 16 */
    int16_t  a16 = 0;  uint16_t  u16 = 0;
    int_least16_t al16 = 0;  uint_least16_t ul16 = 0;
    int_fast16_t  af16 = 0;  uint_fast16_t  uf16 = 0;
    assert(a16 == 0 && u16 == 0 && al16 == 0 && ul16 == 0 && af16 == 0 && uf16 == 0);

    /* N = 32 */
    int32_t  a32 = 0;  uint32_t  u32 = 0;
    int_least32_t al32 = 0;  uint_least32_t ul32 = 0;
    int_fast32_t  af32 = 0;  uint_fast32_t  uf32 = 0;
    assert(a32 == 0 && u32 == 0 && al32 == 0 && ul32 == 0 && af32 == 0 && uf32 == 0);

    /* N = 64 */
    int64_t  a64 = 0;  uint64_t  u64 = 0;
    int_least64_t al64 = 0;  uint_least64_t ul64 = 0;
    int_fast64_t  af64 = 0;  uint_fast64_t  uf64 = 0;
    assert(a64 == 0 && u64 == 0 && al64 == 0 && ul64 == 0 && af64 == 0 && uf64 == 0);

    /* 无 N 的类型 */
    intptr_t  ip  = 0;  uintptr_t  up  = 0;
    intmax_t  im  = 0;  uintmax_t  um  = 0;
    assert(ip == 0 && up == 0 && im == 0 && um == 0);
}

/* [1] 对应的有符号/无符号类型可以互相转换，且转换结果符合 6.2.5 的模运算规则 */
static void test_conversion(void)
{
    int8_t  s8  = -1;
    uint8_t u8  = (uint8_t)s8;
    assert(u8 == 255);

    int16_t s16 = -1;
    uint16_t u16 = (uint16_t)s16;
    assert(u16 == 65535);

    int32_t s32 = -1;
    uint32_t u32 = (uint32_t)s32;
    assert(u32 == 4294967295u);

    int64_t s64 = -1;
    uint64_t u64 = (uint64_t)s64;
    assert(u64 == 18446744073709551615ULL);

    /* 反向转换：无符号最大值转有符号，按模运算得到 -1 */
    assert((int8_t)UINT8_MAX  == -1);
    assert((int16_t)UINT16_MAX == -1);
    assert((int32_t)UINT32_MAX == -1);
    assert((int64_t)UINT64_MAX == -1);
}

int main(void)
{
    test_exact_width_pairs();
    test_least_width_pairs();
    test_fast_width_pairs();
    test_intptr_pairs();
    test_intmax_pairs();
    test_corresponding_same_size();
    test_signed_unsigned_correspondence();
    test_N_values();
    test_conversion();

    printf("C99 7.18.1 Integer types: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「N 表示无前导零的无符号十进制整数」：
 * 标准中不存在 int08_t、uint08_t 等带前导零的类型名。
 * gcc -std=c99 应报错：'int08_t' undeclared / unknown type name。 */
int08_t  bad_a;
uint08_t bad_b;

/* 违反约束「N 表示无前导零的无符号十进制整数」：
 * 标准中不存在 int048_t 等带前导零的类型名。 */
int048_t bad_c;

/* 违反约束「仅相差开头 u 的 typedef 名必须成对提供」：
 * 标准要求提供 int8_t 就必须提供 uint8_t，反之亦然。
 * 这里故意只声明一个方向，若实现只提供其中一个则违反 [1]。
 * 注意：这是对实现的约束，不是对用户代码的约束；
 * 用户代码无法直接触发该约束的编译错误，故此处仅作说明性示例。 */

/* 违反约束「N 必须是无符号十进制整数」：
 * 标准中不存在 int_8_t（带下划线分隔）这样的类型名。
 * gcc -std=c99 应报错：'int_8_t' undeclared。 */
int_8_t bad_d;

/* 违反约束「N 必须是无符号十进制整数」：
 * 标准中不存在 int8_t_ 这样的类型名。 */
int8_t_ bad_e;

/* 违反约束「N 必须是无前导零」：
 * 标准中不存在 int0x8_t 这样的十六进制形式类型名。 */
int0x8_t bad_f;

#endif