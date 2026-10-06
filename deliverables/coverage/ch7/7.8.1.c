/*
 * 测试 C99 7.8.1 —— Macros for format specifiers (<inttypes.h>)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，gcc -std=c99 应报错。
 *
 * 覆盖段落：[1] 宏的一般形式与用途；[2] fprintf 有符号宏；
 *           [3] fprintf 无符号宏；[4] fscanf 有符号宏；
 *           [5] fscanf 无符号宏；[6] 与 <stdint.h> 类型对应；
 *           [7] EXAMPLE。
 */

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 宏展开为字符串字面量，可拼接进格式串 */
static void test_string_literal_concat(void)
{
    /* 字符串字面量拼接：PRIxMAX 是字符串字面量，可与前后字面量拼接 */
    const char *fmt = "%" PRIxMAX;
    assert(strcmp(fmt, "%lx") == 0 || strcmp(fmt, "%llx") == 0 ||
           strcmp(fmt, "%x") == 0);   /* 具体长度修饰符依实现而定 */
}

/* [2] fprintf 有符号整数宏：PRIdN / PRIiN 系列 */
static void test_fprintf_signed(void)
{
    char buf[128];

    int8_t   d8  = -8;
    int16_t  d16 = -16;
    int32_t  d32 = -32;
    int64_t  d64 = -64;
    int_least8_t   dl8  = -1;
    int_least16_t  dl16 = -2;
    int_least32_t  dl32 = -3;
    int_least64_t  dl64 = -4;
    int_fast8_t    df8  = -5;
    int_fast16_t   df16 = -6;
    int_fast32_t   df32 = -7;
    int_fast64_t   df64 = -8;
    intmax_t       dm   = -99;
    intptr_t       dp   = -1;

    /* PRIdN */
    sprintf(buf, "%" PRId8,  d8);  assert(strcmp(buf, "-8")  == 0);
    sprintf(buf, "%" PRId16, d16); assert(strcmp(buf, "-16") == 0);
    sprintf(buf, "%" PRId32, d32); assert(strcmp(buf, "-32") == 0);
    sprintf(buf, "%" PRId64, d64); assert(strcmp(buf, "-64") == 0);
    /* PRIdLEASTN */
    sprintf(buf, "%" PRIdLEAST8,  dl8);  assert(strcmp(buf, "-1") == 0);
    sprintf(buf, "%" PRIdLEAST16, dl16); assert(strcmp(buf, "-2") == 0);
    sprintf(buf, "%" PRIdLEAST32, dl32); assert(strcmp(buf, "-3") == 0);
    sprintf(buf, "%" PRIdLEAST64, dl64); assert(strcmp(buf, "-4") == 0);
    /* PRIdFASTN */
    sprintf(buf, "%" PRIdFAST8,  df8);  assert(strcmp(buf, "-5") == 0);
    sprintf(buf, "%" PRIdFAST16, df16); assert(strcmp(buf, "-6") == 0);
    sprintf(buf, "%" PRIdFAST32, df32); assert(strcmp(buf, "-7") == 0);
    sprintf(buf, "%" PRIdFAST64, df64); assert(strcmp(buf, "-8") == 0);
    /* PRIdMAX / PRIdPTR */
    sprintf(buf, "%" PRIdMAX, dm); assert(strcmp(buf, "-99") == 0);
    sprintf(buf, "%" PRIdPTR, dp); assert(strcmp(buf, "-1")  == 0);

    /* PRIiN 系列（i 转换说明符，与 d 等价） */
    sprintf(buf, "%" PRIi8,  d8);  assert(strcmp(buf, "-8")  == 0);
    sprintf(buf, "%" PRIi16, d16); assert(strcmp(buf, "-16") == 0);
    sprintf(buf, "%" PRIi32, d32); assert(strcmp(buf, "-32") == 0);
    sprintf(buf, "%" PRIi64, d64); assert(strcmp(buf, "-64") == 0);
    sprintf(buf, "%" PRIiLEAST8,  dl8);  assert(strcmp(buf, "-1") == 0);
    sprintf(buf, "%" PRIiLEAST16, dl16); assert(strcmp(buf, "-2") == 0);
    sprintf(buf, "%" PRIiLEAST32, dl32); assert(strcmp(buf, "-3") == 0);
    sprintf(buf, "%" PRIiLEAST64, dl64); assert(strcmp(buf, "-4") == 0);
    sprintf(buf, "%" PRIiFAST8,  df8);  assert(strcmp(buf, "-5") == 0);
    sprintf(buf, "%" PRIiFAST16, df16); assert(strcmp(buf, "-6") == 0);
    sprintf(buf, "%" PRIiFAST32, df32); assert(strcmp(buf, "-7") == 0);
    sprintf(buf, "%" PRIiFAST64, df64); assert(strcmp(buf, "-8") == 0);
    sprintf(buf, "%" PRIiMAX, dm); assert(strcmp(buf, "-99") == 0);
    sprintf(buf, "%" PRIiPTR, dp); assert(strcmp(buf, "-1")  == 0);
}

/* [3] fprintf 无符号整数宏：PRIoN / PRIuN / PRIxN / PRIXN 系列 */
static void test_fprintf_unsigned(void)
{
    char buf[128];

    uint8_t   u8  = 255;
    uint16_t  u16 = 65535;
    uint32_t  u32 = 4294967295u;
    uint64_t  u64 = 18446744073709551615ull;
    uint_least8_t  ul8  = 1;
    uint_least16_t ul16 = 2;
    uint_least32_t ul32 = 3;
    uint_least64_t ul64 = 4;
    uint_fast8_t   uf8  = 5;
    uint_fast16_t  uf16 = 6;
    uint_fast32_t  uf32 = 7;
    uint_fast64_t  uf64 = 8;
    uintmax_t      um   = 99;
    uintptr_t      up   = 1;

    /* PRIoN（八进制） */
    sprintf(buf, "%" PRIo8,  u8);  assert(strcmp(buf, "377") == 0);
    sprintf(buf, "%" PRIo16, u16); assert(strcmp(buf, "177777") == 0);
    sprintf(buf, "%" PRIo32, u32); assert(strcmp(buf, "37777777777") == 0);
    sprintf(buf, "%" PRIo64, u64); assert(strcmp(buf, "1777777777777777777777") == 0);
    sprintf(buf, "%" PRIoLEAST8,  ul8);  assert(strcmp(buf, "1") == 0);
    sprintf(buf, "%" PRIoLEAST16, ul16); assert(strcmp(buf, "2") == 0);
    sprintf(buf, "%" PRIoLEAST32, ul32); assert(strcmp(buf, "3") == 0);
    sprintf(buf, "%" PRIoLEAST64, ul64); assert(strcmp(buf, "4") == 0);
    sprintf(buf, "%" PRIoFAST8,  uf8);  assert(strcmp(buf, "5") == 0);
    sprintf(buf, "%" PRIoFAST16, uf16); assert(strcmp(buf, "6") == 0);
    sprintf(buf, "%" PRIoFAST32, uf32); assert(strcmp(buf, "7") == 0);
    sprintf(buf, "%" PRIoFAST64, uf64); assert(strcmp(buf, "10") == 0);
    sprintf(buf, "%" PRIoMAX, um); assert(strcmp(buf, "143") == 0);
    sprintf(buf, "%" PRIoPTR, up); assert(strcmp(buf, "1") == 0);

    /* PRIuN（十进制） */
    sprintf(buf, "%" PRIu8,  u8);  assert(strcmp(buf, "255") == 0);
    sprintf(buf, "%" PRIu16, u16); assert(strcmp(buf, "65535") == 0);
    sprintf(buf, "%" PRIu32, u32); assert(strcmp(buf, "4294967295") == 0);
    sprintf(buf, "%" PRIu64, u64); assert(strcmp(buf, "18446744073709551615") == 0);
    sprintf(buf, "%" PRIuLEAST8,  ul8);  assert(strcmp(buf, "1") == 0);
    sprintf(buf, "%" PRIuLEAST16, ul16); assert(strcmp(buf, "2") == 0);
    sprintf(buf, "%" PRIuLEAST32, ul32); assert(strcmp(buf, "3") == 0);
    sprintf(buf, "%" PRIuLEAST64, ul64); assert(strcmp(buf, "4") == 0);
    sprintf(buf, "%" PRIuFAST8,  uf8);  assert(strcmp(buf, "5") == 0);
    sprintf(buf, "%" PRIuFAST16, uf16); assert(strcmp(buf, "6") == 0);
    sprintf(buf, "%" PRIuFAST32, uf32); assert(strcmp(buf, "7") == 0);
    sprintf(buf, "%" PRIuFAST64, uf64); assert(strcmp(buf, "8") == 0);
    sprintf(buf, "%" PRIuMAX, um); assert(strcmp(buf, "99") == 0);
    sprintf(buf, "%" PRIuPTR, up); assert(strcmp(buf, "1") == 0);

    /* PRIxN（小写十六进制） */
    sprintf(buf, "%" PRIx8,  u8);  assert(strcmp(buf, "ff") == 0);
    sprintf(buf, "%" PRIx16, u16); assert(strcmp(buf, "ffff") == 0);
    sprintf(buf, "%" PRIx32, u32); assert(strcmp(buf, "ffffffff") == 0);
    sprintf(buf, "%" PRIx64, u64); assert(strcmp(buf, "ffffffffffffffff") == 0);
    sprintf(buf, "%" PRIxLEAST8,  ul8);  assert(strcmp(buf, "1") == 0);
    sprintf(buf, "%" PRIxLEAST16, ul16); assert(strcmp(buf, "2") == 0);
    sprintf(buf, "%" PRIxLEAST32, ul32); assert(strcmp(buf, "3") == 0);
    sprintf(buf, "%" PRIxLEAST64, ul64); assert(strcmp(buf, "4") == 0);
    sprintf(buf, "%" PRIxFAST8,  uf8);  assert(strcmp(buf, "5") == 0);
    sprintf(buf, "%" PRIxFAST16, uf16); assert(strcmp(buf, "6") == 0);
    sprintf(buf, "%" PRIxFAST32, uf32); assert(strcmp(buf, "7") == 0);
    sprintf(buf, "%" PRIxFAST64, uf64); assert(strcmp(buf, "8") == 0);
    sprintf(buf, "%" PRIxMAX, um); assert(strcmp(buf, "63") == 0);
    sprintf(buf, "%" PRIxPTR, up); assert(strcmp(buf, "1") == 0);

    /* PRIXN（大写十六进制） */
    sprintf(buf, "%" PRIX8,  u8);  assert(strcmp(buf, "FF") == 0);
    sprintf(buf, "%" PRIX16, u16); assert(strcmp(buf, "FFFF") == 0);
    sprintf(buf, "%" PRIX32, u32); assert(strcmp(buf, "FFFFFFFF") == 0);
    sprintf(buf, "%" PRIX64, u64); assert(strcmp(buf, "FFFFFFFFFFFFFFFF") == 0);
    sprintf(buf, "%" PRIXLEAST8,  ul8);  assert(strcmp(buf, "1") == 0);
    sprintf(buf, "%" PRIXLEAST16, ul16); assert(strcmp(buf, "2") == 0);
    sprintf(buf, "%" PRIXLEAST32, ul32); assert(strcmp(buf, "3") == 0);
    sprintf(buf, "%" PRIXLEAST64, ul64); assert(strcmp(buf, "4") == 0);
    sprintf(buf, "%" PRIXFAST8,  uf8);  assert(strcmp(buf, "5") == 0);
    sprintf(buf, "%" PRIXFAST16, uf16); assert(strcmp(buf, "6") == 0);
    sprintf(buf, "%" PRIXFAST32, uf32); assert(strcmp(buf, "7") == 0);
    sprintf(buf, "%" PRIXFAST64, uf64); assert(strcmp(buf, "8") == 0);
    sprintf(buf, "%" PRIXMAX, um); assert(strcmp(buf, "63") == 0);
    sprintf(buf, "%" PRIXPTR, up); assert(strcmp(buf, "1") == 0);
}

/* [4] fscanf 有符号整数宏：SCNdN / SCNiN 系列 */
static void test_fscanf_signed(void)
{
    int8_t   d8  = 0;
    int16_t  d16 = 0;
    int32_t  d32 = 0;
    int64_t  d64 = 0;
    int_least8_t   dl8  = 0;
    int_least16_t  dl16 = 0;
    int_least32_t  dl32 = 0;
    int_least64_t  dl64 = 0;
    int_fast8_t    df8  = 0;
    int_fast16_t   df16 = 0;
    int_fast32_t   df32 = 0;
    int_fast64_t   df64 = 0;
    intmax_t       dm   = 0;
    intptr_t       dp   = 0;

    /* SCNdN */
    assert(sscanf("-8",  "%" SCNd8,  &d8)  == 1 && d8  == -8);
    assert(sscanf("-16", "%" SCNd16, &d16) == 1 && d16 == -16);
    assert(sscanf("-32", "%" SCNd32, &d32) == 1 && d32 == -32);
    assert(sscanf("-64", "%" SCNd64, &d64) == 1 && d64 == -64);
    assert(sscanf("-1",  "%" SCNdLEAST8,  &dl8)  == 1 && dl8  == -1);
    assert(sscanf("-2",  "%" SCNdLEAST16, &dl16) == 1 && dl16 == -2);
    assert(sscanf("-3",  "%" SCNdLEAST32, &dl32) == 1 && dl32 == -3);
    assert(sscanf("-4",  "%" SCNdLEAST64, &dl64) == 1 && dl64 == -4);
    assert(sscanf("-5",  "%" SCNdFAST8,  &df8)  == 1 && df8  == -5);
    assert(sscanf("-6",  "%" SCNdFAST16, &df16) == 1 && df16 == -6);
    assert(sscanf("-7",  "%" SCNdFAST32, &df32) == 1 && df32 == -7);
    assert(sscanf("-8",  "%" SCNdFAST64, &df64) == 1 && df64 == -8);
    assert(sscanf("-99", "%" SCNdMAX, &dm) == 1 && dm == -99);
    assert(sscanf("-1",  "%" SCNdPTR, &dp) == 1 && dp == -1);

    /* SCNiN（i 转换说明符，可识别 0x/0 前缀） */
    assert(sscanf("-8",  "%" SCNi8,  &d8)  == 1 && d8  == -8);
    assert(sscanf("-16", "%" SCNi16, &d16) == 1 && d16 == -16);
    assert(sscanf("-32", "%" SCNi32, &d32) == 1 && d32 == -32);
    assert(sscanf("-64", "%" SCNi64, &d64) == 1 && d64 == -64);
    assert(sscanf("-1",  "%" SCNiLEAST8,  &dl8)  == 1 && dl8  == -1);
    assert(sscanf("-2",  "%" SCNiLEAST16, &dl16) == 1 && dl16 == -2);
    assert(sscanf("-3",  "%" SCNiLEAST32, &dl32) == 1 && dl32 == -3);
    assert(sscanf("-4",  "%" SCNiLEAST64, &dl64) == 1 && dl64 == -4);
    assert(sscanf("-5",  "%" SCNiFAST8,  &df8)  == 1 && df8  == -5);
    assert(sscanf("-6",  "%" SCNiFAST16, &df16) == 1 && df16 == -6);
    assert(sscanf("-7",  "%" SCNiFAST32, &df32) == 1 && df32 == -7);
    assert(sscanf("-8",  "%" SCNiFAST64, &df64) == 1 && df64 == -8);
    assert(sscanf("-99", "%" SCNiMAX, &dm) == 1 && dm == -99);
    assert(sscanf("-1",  "%" SCNiPTR, &dp) == 1 && dp == -1);
}

/* [5] fscanf 无符号整数宏：SCNoN / SCNuN / SCNxN 系列 */
static void test_fscanf_unsigned(void)
{
    uint8_t   u8  = 0;
    uint16_t  u16 = 0;
    uint32_t  u32 = 0;
    uint64_t  u64 = 0;
    uint_least8_t  ul8  = 0;
    uint_least16_t ul16 = 0;
    uint_least32_t ul32 = 0;
    uint_least64_t ul64 = 0;
    uint_fast8_t   uf8  = 0;
    uint_fast16_t  uf16 = 0;
    uint_fast32_t  uf32 = 0;
    uint_fast64_t  uf64 = 0;
    uintmax_t      um   = 0;
    uintptr_t      up   = 0;

    /* SCNoN（八进制） */
    assert(sscanf("377", "%" SCNo8,  &u8)  == 1 && u8  == 255);
    assert(sscanf("177777", "%" SCNo16, &u16) == 1 && u16 == 65535);
    assert(sscanf("37777777777", "%" SCNo32, &u32) == 1 && u32 == 4294967295u);
    assert(sscanf("1777777777777777777777", "%" SCNo64, &u64) == 1 &&
           u64 == 18446744073709551615ull);
    assert(sscanf("1", "%" SCNoLEAST8,  &ul8)  == 1 && ul8  == 1);
    assert(sscanf("2", "%" SCNoLEAST16, &ul16) == 1 && ul16 == 2);
    assert(sscanf("3", "%" SCNoLEAST32, &ul32) == 1 && ul32 == 3);
    assert(sscanf("4", "%" SCNoLEAST64, &ul64) == 1 && ul64 == 4);
    assert(sscanf("5", "%" SCNoFAST8,  &uf8)  == 1 && uf8  == 5);
    assert(sscanf("6", "%" SCNoFAST16, &uf16) == 1 && uf16 == 6);
    assert(sscanf("7", "%" SCNoFAST32, &uf32) == 1 && uf32 == 7);
    assert(sscanf("10", "%" SCNoFAST64, &uf64) == 1 && uf64 == 8);
    assert(sscanf("143", "%" SCNoMAX, &um) == 1 && um == 99);
    assert(sscanf("1", "%" SCNoPTR, &up) == 1 && up == 1);

    /* SCNuN（十进制） */
    assert(sscanf("255", "%" SCNu8,  &u8)  == 1 && u8  == 255);
    assert(sscanf("65535", "%" SCNu16, &u16) == 1 && u16 == 65535);
    assert(sscanf("4294967295", "%" SCNu32, &u32) == 1 && u32 == 4294967295u);
    assert(sscanf("18446744073709551615", "%" SCNu64, &u64) == 1 &&
           u64 == 18446744073709551615ull);
    assert(sscanf("1", "%" SCNuLEAST8,  &ul8)  == 1 && ul8  == 1);
    assert(sscanf("2", "%" SCNuLEAST16, &ul16) == 1 && ul16 == 2);
    assert(sscanf("3", "%" SCNuLEAST32, &ul32) == 1 && ul32 == 3);
    assert(sscanf("4", "%" SCNuLEAST64, &ul64) == 1 && ul64 == 4);
    assert(sscanf("5", "%" SCNuFAST8,  &uf8)  == 1 && uf8  == 5);
    assert(sscanf("6", "%" SCNuFAST16, &uf16) == 1 && uf16 == 6);
    assert(sscanf("7", "%" SCNuFAST32, &uf32) == 1 && uf32 == 7);
    assert(sscanf("8", "%" SCNuFAST64, &uf64) == 1 && uf64 == 8);
    assert(sscanf("99", "%" SCNuMAX, &um) == 1 && um == 99);
    assert(sscanf("1", "%" SCNuPTR, &up) == 1 && up == 1);

    /* SCNxN（十六进制） */
    assert(sscanf("ff", "%" SCNx8,  &u8)  == 1 && u8  == 255);
    assert(sscanf("ffff", "%" SCNx16, &u16) == 1 && u16 == 65535);
    assert(sscanf("ffffffff", "%" SCNx32, &u32) == 1 && u32 == 4294967295u);
    assert(sscanf("ffffffffffffffff", "%" SCNx64, &u64) == 1 &&
           u64 == 18446744073709551615ull);
    assert(sscanf("1", "%" SCNxLEAST8,  &ul8)  == 1 && ul8  == 1);
    assert(sscanf("2", "%" SCNxLEAST16, &ul16) == 1 && ul16 == 2);
    assert(sscanf("3", "%" SCNxLEAST32, &ul32) == 1 && ul32 == 3);
    assert(sscanf("4", "%" SCNxLEAST64, &ul64) == 1 && ul64 == 4);
    assert(sscanf("5", "%" SCNxFAST8,  &uf8)  == 1 && uf8  == 5);
    assert(sscanf("6", "%" SCNxFAST16, &uf16) == 1 && uf16 == 6);
    assert(sscanf("7", "%" SCNxFAST32, &uf32) == 1 && uf32 == 7);
    assert(sscanf("8", "%" SCNxFAST64, &uf64) == 1 && uf64 == 8);
    assert(sscanf("63", "%" SCNxMAX, &um) == 1 && um == 99);
    assert(sscanf("1", "%" SCNxPTR, &up) == 1 && up == 1);
}

/* [6] 与 <stdint.h> 类型对应：每个实现提供的类型都有对应宏。
 *     这里通过“宏存在且可拼接”来验证（编译期即可确认）。 */
static void test_macros_exist(void)
{
    /* 若宏未定义，下面的字符串拼接会编译失败 */
    const char *fmts[] = {
        "%" PRId8,  "%" PRId16, "%" PRId32, "%" PRId64,
        "%" PRIdLEAST8, "%" PRIdLEAST16, "%" PRIdLEAST32, "%" PRIdLEAST64,
        "%" PRIdFAST8,  "%" PRIdFAST16,  "%" PRIdFAST32,  "%" PRIdFAST64,
        "%" PRIdMAX, "%" PRIdPTR,
        "%" PRIi8,  "%" PRIi16, "%" PRIi32, "%" PRIi64,
        "%" PRIiLEAST8, "%" PRIiLEAST16, "%" PRIiLEAST32, "%" PRIiLEAST64,
        "%" PRIiFAST8,  "%" PRIiFAST16,  "%" PRIiFAST32,  "%" PRIiFAST64,
        "%" PRIiMAX, "%" PRIiPTR,
        "%" PRIo8,  "%" PRIo16, "%" PRIo32, "%" PRIo64,
        "%" PRIoLEAST8, "%" PRIoLEAST16, "%" PRIoLEAST32, "%" PRIoLEAST64,
        "%" PRIoFAST8,  "%" PRIoFAST16,  "%" PRIoFAST32,  "%" PRIoFAST64,
        "%" PRIoMAX, "%" PRIoPTR,
        "%" PRIu8,  "%" PRIu16, "%" PRIu32, "%" PRIu64,
        "%" PRIuLEAST8, "%" PRIuLEAST16, "%" PRIuLEAST32, "%" PRIuLEAST64,
        "%" PRIuFAST8,  "%" PRIuFAST16,  "%" PRIuFAST32,  "%" PRIuFAST64,
        "%" PRIuMAX, "%" PRIuPTR,
        "%" PRIx8,  "%" PRIx16, "%" PRIx32, "%" PRIx64,
        "%" PRIxLEAST8, "%" PRIxLEAST16, "%" PRIxLEAST32, "%" PRIxLEAST64,
        "%" PRIxFAST8,  "%" PRIxFAST16,  "%" PRIxFAST32,  "%" PRIxFAST64,
        "%" PRIxMAX, "%" PRIxPTR,
        "%" PRIX8,  "%" PRIX16, "%" PRIX32, "%" PRIX64,
        "%" PRIXLEAST8, "%" PRIXLEAST16, "%" PRIXLEAST32, "%" PRIXLEAST64,
        "%" PRIXFAST8,  "%" PRIXFAST16,  "%" PRIXFAST32,  "%" PRIXFAST64,
        "%" PRIXMAX, "%" PRIXPTR,
        "%" SCNd8,  "%" SCNd16, "%" SCNd32, "%" SCNd64,
        "%" SCNdLEAST8, "%" SCNdLEAST16, "%" SCNdLEAST32, "%" SCNdLEAST64,
        "%" SCNdFAST8,  "%" SCNdFAST16,  "%" SCNdFAST32,  "%" SCNdFAST64,
        "%" SCNdMAX, "%" SCNdPTR,
        "%" SCNi8,  "%" SCNi16, "%" SCNi32, "%" SCNi64,
        "%" SCNiLEAST8, "%" SCNiLEAST16, "%" SCNiLEAST32, "%" SCNiLEAST64,
        "%" SCNiFAST8,  "%" SCNiFAST16,  "%" SCNiFAST32,  "%" SCNiFAST64,
        "%" SCNiMAX, "%" SCNiPTR,
        "%" SCNo8,  "%" SCNo16, "%" SCNo32, "%" SCNo64,
        "%" SCNoLEAST8, "%" SCNoLEAST16, "%" SCNoLEAST32, "%" SCNoLEAST64,
        "%" SCNoFAST8,  "%" SCNoFAST16,  "%" SCNoFAST32,  "%" SCNoFAST64,
        "%" SCNoMAX, "%" SCNoPTR,
        "%" SCNu8,  "%" SCNu16, "%" SCNu32, "%" SCNu64,
        "%" SCNuLEAST8, "%" SCNuLEAST16, "%" SCNuLEAST32, "%" SCNuLEAST64,
        "%" SCNuFAST8,  "%" SCNuFAST16,  "%" SCNuFAST32,  "%" SCNuFAST64,
        "%" SCNuMAX, "%" SCNuPTR,
        "%" SCNx8,  "%" SCNx16, "%" SCNx32, "%" SCNx64,
        "%" SCNxLEAST8, "%" SCNxLEAST16, "%" SCNxLEAST32, "%" SCNxLEAST64,
        "%" SCNxFAST8,  "%" SCNxFAST16,  "%" SCNxFAST32,  "%" SCNxFAST64,
        "%" SCNxMAX, "%" SCNxPTR
    };
    size_t i;
    for (i = 0; i < sizeof fmts / sizeof fmts[0]; i++) {
        assert(fmts[i] != NULL && fmts[i][0] == '%');
    }
}

/* [7] EXAMPLE：条款给出的示例（改为窄字符版本以便 assert 检查） */
static void test_example(void)
{
    uintmax_t i = UINTMAX_MAX;   /* 该类型总是存在 */
    char buf[128];
    int n = sprintf(buf, "The largest integer value is %020" PRIxMAX, i);
    assert(n > 0);
    /* 结果应为 20 位十六进制（大写/小写依实现，PRIxMAX 为小写） */
    assert(strlen(buf) == strlen("The largest integer value is ") + 20);
    assert(strncmp(buf, "The largest integer value is ", 29) == 0);
}

int main(void)
{
    test_string_literal_concat();
    test_fprintf_signed();
    test_fprintf_unsigned();
    test_fscanf_signed();
    test_fscanf_unsigned();
    test_macros_exist();
    test_example();

    printf("All C99 7.8.1 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「PRI/SCN 宏展开为字符串字面量，只能用于字符串拼接」：
 * 把 PRIxMAX 当作整数使用（如算术运算），gcc -std=c99 应报错。 */
int bad1 = PRIxMAX + 1;

/* 违反约束「PRI/SCN 宏是对象式宏，展开为字符串字面量」：
 * 试图把宏当作函数调用，gcc -std=c99 应报错。 */
const char *bad2 = PRId32(5);

/* 违反约束「SCN 宏用于 fscanf 的格式串，其对应实参应为指针」：
 * 传入非指针实参，gcc -std=c99 应报错（格式串与实参类型不匹配）。 */
void bad3(void)
{
    int32_t v;
    sscanf("42", "%" SCNd32, v);   /* 应为 &v */
}

/* 违反约束「PRI 宏用于 fprintf 的格式串，其对应实参应为对应整数类型」：
 * 传入指针实参，gcc -std=c99 应报错（格式串与实参类型不匹配）。 */
void bad4(void)
{
    int32_t v = 0;
    printf("%" PRId32, &v);        /* 应为 v */
}

/* 违反约束「宏名必须来自 <inttypes.h>」：
 * 未包含 <inttypes.h> 时使用 PRIxMAX，gcc -std=c99 应报错（未声明标识符）。 */
void bad5(void)
{
    /* 假设此处未包含 <inttypes.h> */
    const char *s = "%" PRIxMAX;   /* PRIxMAX 未定义 */
    (void)s;
}

#endif