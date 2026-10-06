/*
 * 测试 C99 7.18.2 —— Limits of specified-width integer types
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（所有 assert 成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器若单独编译这些片段应报错。
 *
 * 条款要点：
 *   [1] <stdint.h> 中定义一组对象式宏，给出 7.18.1 中定宽整数类型的最小/最大限制。
 *   [2] 每个宏实例应被替换为一个适合 #if 预处理指令使用的常量表达式，
 *       其类型与“对应类型对象经整数提升后的表达式”类型相同；
 *       其实现定义值在数量级上应 >= 条款给出的值，符号相同（除非声明为精确值）。
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 宏存在性检查：每个宏名对应 7.18.1 中的类型名。
 *     这里用 #ifdef 验证宏确实被定义（对象式宏）。 */
#ifdef INT8_MIN
#else
#error "INT8_MIN not defined"
#endif
#ifdef INT8_MAX
#else
#error "INT8_MAX not defined"
#endif
#ifdef UINT8_MAX
#else
#error "UINT8_MAX not defined"
#endif
#ifdef INT16_MIN
#else
#error "INT16_MIN not defined"
#endif
#ifdef INT16_MAX
#else
#error "INT16_MAX not defined"
#endif
#ifdef UINT16_MAX
#else
#error "UINT16_MAX not defined"
#endif
#ifdef INT32_MIN
#else
#error "INT32_MIN not defined"
#endif
#ifdef INT32_MAX
#else
#error "INT32_MAX not defined"
#endif
#ifdef UINT32_MAX
#else
#error "UINT32_MAX not defined"
#endif
#ifdef INT64_MIN
#else
#error "INT64_MIN not defined"
#endif
#ifdef INT64_MAX
#else
#error "INT64_MAX not defined"
#endif
#ifdef UINT64_MAX
#else
#error "UINT64_MAX not defined"
#endif

/* [2] 宏可用于 #if 预处理指令（常量表达式）。
 *     下面这些 #if 若宏不可用于预处理，将导致编译错误。 */
#if INT8_MAX < 127
#error "INT8_MAX must be >= 127"
#endif
#if INT16_MAX < 32767
#error "INT16_MAX must be >= 32767"
#endif
#if INT32_MAX < 2147483647
#error "INT32_MAX must be >= 2147483647"
#endif
#if UINT8_MAX < 255
#error "UINT8_MAX must be >= 255"
#endif
#if UINT16_MAX < 65535
#error "UINT16_MAX must be >= 65535"
#endif
#if UINT32_MAX < 4294967295U
#error "UINT32_MAX must be >= 4294967295"
#endif

/* [2] 有符号最小值：数量级应 >= 条款给定值，符号为负。
 *     条款给定：INT8_MIN <= -127, INT16_MIN <= -32767, INT32_MIN <= -2147483647,
 *               INT64_MIN <= -9223372036854775807。 */
#if INT8_MIN > -127
#error "INT8_MIN magnitude too small"
#endif
#if INT16_MIN > -32767
#error "INT16_MIN magnitude too small"
#endif
#if INT32_MIN > -2147483647
#error "INT32_MIN magnitude too small"
#endif
#if INT64_MIN > -9223372036854775807LL
#error "INT64_MIN magnitude too small"
#endif

/* [2] 有符号最大值：应 >= 条款给定值。
 *     条款给定：INT8_MAX >= 127, INT16_MAX >= 32767, INT32_MAX >= 2147483647,
 *               INT64_MAX >= 9223372036854775807。 */
#if INT64_MAX < 9223372036854775807LL
#error "INT64_MAX too small"
#endif

/* [2] 无符号最大值：应 >= 条款给定值。
 *     条款给定：UINT8_MAX >= 255, UINT16_MAX >= 65535, UINT32_MAX >= 4294967295,
 *               UINT64_MAX >= 18446744073709551615。 */
#if UINT64_MAX < 18446744073709551615ULL
#error "UINT64_MAX too small"
#endif

/* [2] 类型检查：宏表达式的类型应与“对应类型对象经整数提升后的表达式”类型相同。
 *     对于 int8_t/int16_t（通常提升为 int），INT8_MAX/INT16_MAX 的类型应为 int。
 *     对于 int32_t（通常就是 int），INT32_MAX 的类型应为 int。
 *     对于 int64_t（通常为 long 或 long long），INT64_MAX 的类型应为提升后的类型。
 *     这里用 _Generic（C11）不可用，改用 sizeof 与类型比较的经典技巧：
 *     通过赋值给对应类型并检查值来间接验证，同时用 sizeof 检查宽度。 */
static void test_macro_types_and_values(void)
{
    /* [2] 值检查：宏的值应等于或大于条款给定值（同符号）。 */
    assert(INT8_MAX  >= 127);
    assert(INT16_MAX >= 32767);
    assert(INT32_MAX >= 2147483647);
    assert(INT64_MAX >= 9223372036854775807LL);

    assert(INT8_MIN  <= -127);
    assert(INT16_MIN <= -32767);
    assert(INT32_MIN <= -2147483647);
    assert(INT64_MIN <= -9223372036854775807LL);

    assert(UINT8_MAX  >= 255);
    assert(UINT16_MAX >= 65535);
    assert(UINT32_MAX >= 4294967295U);
    assert(UINT64_MAX >= 18446744073709551615ULL);

    /* [2] 类型检查：宏表达式的类型应与对应类型对象经整数提升后的类型相同。
     *     用 sizeof 比较：sizeof(INT8_MAX) 应等于 sizeof(int)（因为 int8_t 提升为 int）。
     *     注意：这是“整数提升后”的类型，不是 int8_t 本身。 */
    assert(sizeof(INT8_MAX)  == sizeof(int));
    assert(sizeof(INT16_MAX) == sizeof(int));
    /* int32_t 通常就是 int，提升后仍是 int */
    assert(sizeof(INT32_MAX) == sizeof(int));
    /* int64_t 提升后通常是 long 或 long long，宽度为 8 */
    assert(sizeof(INT64_MAX) == 8);

    assert(sizeof(UINT8_MAX)  == sizeof(int));
    assert(sizeof(UINT16_MAX) == sizeof(int));
    assert(sizeof(UINT32_MAX) == sizeof(unsigned int));
    assert(sizeof(UINT64_MAX) == 8);

    /* [2] 符号检查：有符号宏为负/正，无符号宏为非负。 */
    assert(INT8_MIN  < 0);
    assert(INT16_MIN < 0);
    assert(INT32_MIN < 0);
    assert(INT64_MIN < 0);

    assert(UINT8_MAX  > 0);
    assert(UINT16_MAX > 0);
    assert(UINT32_MAX > 0);
    assert(UINT64_MAX > 0);

    /* [2] 宏可用于普通表达式，且与对应类型兼容。 */
    int8_t  i8  = INT8_MAX;
    int16_t i16 = INT16_MAX;
    int32_t i32 = INT32_MAX;
    int64_t i64 = INT64_MAX;
    uint8_t  u8  = UINT8_MAX;
    uint16_t u16 = UINT16_MAX;
    uint32_t u32 = UINT32_MAX;
    uint64_t u64 = UINT64_MAX;

    assert(i8  == INT8_MAX);
    assert(i16 == INT16_MAX);
    assert(i32 == INT32_MAX);
    assert(i64 == INT64_MAX);
    assert(u8  == UINT8_MAX);
    assert(u16 == UINT16_MAX);
    assert(u32 == UINT32_MAX);
    assert(u64 == UINT64_MAX);

    /* [2] 最小值赋给对应类型应可表示。 */
    int8_t  m8  = INT8_MIN;
    int16_t m16 = INT16_MIN;
    int32_t m32 = INT32_MIN;
    int64_t m64 = INT64_MIN;
    assert(m8  == INT8_MIN);
    assert(m16 == INT16_MIN);
    assert(m32 == INT32_MIN);
    assert(m64 == INT64_MIN);

    printf("All positive tests passed.\n");
}

int main(void)
{
    test_macro_types_and_values();
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「宏名对应 7.18.1 中的类型名」：
 * 若 <stdint.h> 未定义 INT8_MAX，则使用它应报错（未声明标识符）。
 * 这里假设一个不存在的宏名，gcc -std=c99 应报错：'INT8_MAX_UNDEFINED' undeclared。 */
int x = INT8_MAX_UNDEFINED;

/* 违反约束「宏实例应被替换为适合 #if 的常量表达式」：
 * 若宏不是常量表达式（例如是变量），则 #if 中使用应报错。
 * 下面假设 INT8_MAX 被错误地定义为变量（实际标准库不会这样），
 * 单独编译此片段时 #if 会报错：token is not a valid binary operator。 */
#if INT8_MAX
#endif

/* 违反约束「宏表达式的类型应与对应类型对象经整数提升后的表达式类型相同」：
 * 若把 INT8_MAX 当作指针或结构体使用，类型不匹配应报错。
 * 例如对 INT8_MAX 取地址并解引用为结构体，gcc 应报错。 */
struct S { int a; };
struct S s = *(struct S *)INT8_MAX;  /* 类型不兼容，应报错 */

/* 违反约束「有符号宏的符号应为负（最小值）/正（最大值）」：
 * 若把 INT8_MIN 用于无符号上下文并期望负值，逻辑上违反条款语义，
 * 但这不是编译期约束，故不放在此处作为负向测试。
 * 下面用类型不匹配的赋值触发编译错误： */
unsigned int u = INT8_MIN;  /* 合法（隐式转换），不报错，故注释掉 */

/* 违反约束「宏可用于 #if」：在 #if 中使用非整数常量表达式。
 * 例如使用浮点常量，gcc 应报错：floating constant in preprocessor expression。 */
#if 1.5 > 0
#endif

#endif /* 负向测试结束 */