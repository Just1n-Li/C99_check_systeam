/*
 * 测试条款：C99 7.18.2.3 —— Limits of fastest minimum-width integer types
 *
 * 预期行为：
 *   正向测试：<stdint.h> 中定义的 INT_FASTN_MIN / INT_FASTN_MAX / UINT_FASTN_MAX
 *             宏必须存在，且满足条款给出的取值范围公式：
 *               INT_FASTN_MIN  == -(2^(N-1) - 1)   （即 -(2^(N-1)-1)）
 *               INT_FASTN_MAX  ==   2^(N-1) - 1
 *               UINT_FASTN_MAX ==   2^N - 1
 *             其中 N = 8,16,32,64。程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（例如把宏当作左值赋值、缺少 <stdint.h> 时使用宏等）
 *             应导致编译报错，统一放在 #if 0 中。
 *
 * 说明：条款原文中公式的排版丢失了指数符号，实际语义为
 *       INT_FASTN_MIN = -(2^(N-1) - 1)，INT_FASTN_MAX = 2^(N-1) - 1，
 *       UINT_FASTN_MAX = 2^N - 1。本测试据此验证。
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* 计算 2 的幂的辅助宏（仅用于测试，避免依赖浮点） */
#define POW2_8   256ULL
#define POW2_16  65536ULL
#define POW2_32  4294967296ULL
#define POW2_64  18446744073709551616ULL

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] INT_FAST8_MIN / INT_FAST8_MAX / UINT_FAST8_MAX 存在且满足公式 */
static void test_fast8(void)
{
    /* INT_FAST8_MIN == -(2^7 - 1) == -127 */
    assert(INT_FAST8_MIN == -(int_fast8_t)((1ULL << 7) - 1));
    /* INT_FAST8_MAX == 2^7 - 1 == 127 */
    assert(INT_FAST8_MAX == (int_fast8_t)((1ULL << 7) - 1));
    /* UINT_FAST8_MAX == 2^8 - 1 == 255 */
    assert(UINT_FAST8_MAX == (uint_fast8_t)((1ULL << 8) - 1));

    /* 类型宽度至少为 8 位：能容纳上述极值 */
    int_fast8_t  s8min = INT_FAST8_MIN;
    int_fast8_t  s8max = INT_FAST8_MAX;
    uint_fast8_t u8max = UINT_FAST8_MAX;
    assert(s8min <= s8max);
    assert(u8max >= 255u);
}

/* [1] INT_FAST16_MIN / INT_FAST16_MAX / UINT_FAST16_MAX */
static void test_fast16(void)
{
    assert(INT_FAST16_MIN == -(int_fast16_t)((1ULL << 15) - 1));
    assert(INT_FAST16_MAX == (int_fast16_t)((1ULL << 15) - 1));
    assert(UINT_FAST16_MAX == (uint_fast16_t)((1ULL << 16) - 1));

    int_fast16_t  s16min = INT_FAST16_MIN;
    int_fast16_t  s16max = INT_FAST16_MAX;
    uint_fast16_t u16max = UINT_FAST16_MAX;
    assert(s16min <= s16max);
    assert(u16max >= 65535u);
}

/* [1] INT_FAST32_MIN / INT_FAST32_MAX / UINT_FAST32_MAX */
static void test_fast32(void)
{
    assert(INT_FAST32_MIN == -(int_fast32_t)((1ULL << 31) - 1));
    assert(INT_FAST32_MAX == (int_fast32_t)((1ULL << 31) - 1));
    assert(UINT_FAST32_MAX == (uint_fast32_t)((1ULL << 32) - 1));

    int_fast32_t  s32min = INT_FAST32_MIN;
    int_fast32_t  s32max = INT_FAST32_MAX;
    uint_fast32_t u32max = UINT_FAST32_MAX;
    assert(s32min <= s32max);
    assert(u32max >= 4294967295u);
}

/* [1] INT_FAST64_MIN / INT_FAST64_MAX / UINT_FAST64_MAX */
static void test_fast64(void)
{
    assert(INT_FAST64_MIN == -(int_fast64_t)((1ULL << 63) - 1));
    assert(INT_FAST64_MAX == (int_fast64_t)((1ULL << 63) - 1));
    assert(UINT_FAST64_MAX == (uint_fast64_t)((1ULL << 64) - 1));

    int_fast64_t  s64min = INT_FAST64_MIN;
    int_fast64_t  s64max = INT_FAST64_MAX;
    uint_fast64_t u64max = UINT_FAST64_MAX;
    assert(s64min <= s64max);
    assert(u64max >= 18446744073709551615ULL);
}

/* [1] 宏的类型必须能参与常规算术运算，且与对应类型兼容 */
static void test_macro_types(void)
{
    /* 宏展开后应能赋值给对应类型变量而不丢失信息 */
    int_fast8_t  a = INT_FAST8_MAX;
    int_fast16_t b = INT_FAST16_MAX;
    int_fast32_t c = INT_FAST32_MAX;
    int_fast64_t d = INT_FAST64_MAX;
    uint_fast8_t  e = UINT_FAST8_MAX;
    uint_fast16_t f = UINT_FAST16_MAX;
    uint_fast32_t g = UINT_FAST32_MAX;
    uint_fast64_t h = UINT_FAST64_MAX;

    assert(a == INT_FAST8_MAX);
    assert(b == INT_FAST16_MAX);
    assert(c == INT_FAST32_MAX);
    assert(d == INT_FAST64_MAX);
    assert(e == UINT_FAST8_MAX);
    assert(f == UINT_FAST16_MAX);
    assert(g == UINT_FAST32_MAX);
    assert(h == UINT_FAST64_MAX);

    /* 最小值与最大值的关系：MIN == -MAX（条款公式 -(2^(N-1)-1)） */
    assert(INT_FAST8_MIN  == -INT_FAST8_MAX);
    assert(INT_FAST16_MIN == -INT_FAST16_MAX);
    assert(INT_FAST32_MIN == -INT_FAST32_MAX);
    assert(INT_FAST64_MIN == -INT_FAST64_MAX);
}

/* [1] 无符号最大值应等于 2 * (有符号最大值 + 1) - 1 的关系验证 */
static void test_unsigned_relation(void)
{
    /* UINT_FASTN_MAX == 2^N - 1 == 2*(2^(N-1)-1) + 1 == 2*INT_FASTN_MAX + 1 */
    assert((uint_fast8_t)(2 * (uint_fast8_t)INT_FAST8_MAX + 1) == UINT_FAST8_MAX);
    assert((uint_fast16_t)(2 * (uint_fast16_t)INT_FAST16_MAX + 1) == UINT_FAST16_MAX);
    assert((uint_fast32_t)(2 * (uint_fast32_t)INT_FAST32_MAX + 1) == UINT_FAST32_MAX);
    assert((uint_fast64_t)(2 * (uint_fast64_t)INT_FAST64_MAX + 1) == UINT_FAST64_MAX);
}

int main(void)
{
    test_fast8();
    test_fast16();
    test_fast32();
    test_fast64();
    test_macro_types();
    test_unsigned_relation();

    printf("C99 7.18.2.3 正向测试全部通过\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「宏不是左值」：INT_FAST8_MAX 是常量表达式，不能作为赋值目标。
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment" */
void neg_assign_to_macro(void)
{
    INT_FAST8_MAX = 0;
    UINT_FAST32_MAX = 0;
}

/* 违反约束「宏不是左值」：不能对宏取地址。
 * 期望：gcc -std=c99 报错 "lvalue required as unary '&' operand" */
void neg_address_of_macro(void)
{
    int *p = &INT_FAST16_MAX;
    (void)p;
}

/* 违反约束「宏不是左值」：不能自增/自减。
 * 期望：gcc -std=c99 报错 "lvalue required as increment operand" */
void neg_increment_macro(void)
{
    INT_FAST64_MIN++;
    --UINT_FAST8_MAX;
}

/* 违反约束「未包含 <stdint.h> 时这些宏未定义」：
 * 若注释掉 #include <stdint.h>，则 INT_FAST8_MAX 等标识符未声明。
 * 期望：gcc -std=c99 报错 "‘INT_FAST8_MAX’ undeclared" */
void neg_without_header(void)
{
    /* 假设此处没有 #include <stdint.h> */
    int x = INT_FAST8_MAX;
    (void)x;
}

/* 违反约束「宏不是左值」：不能作为复合字面量/结构体成员赋值目标。
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment" */
void neg_macro_in_expression(void)
{
    (INT_FAST32_MAX) = 1;
}

#endif /* 负向测试结束 */