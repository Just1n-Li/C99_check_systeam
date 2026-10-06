/*
 * 测试条款：C99 7.18.2.1 —— 精确宽度整数类型的极限值
 *   INTN_MIN  == -(2^(N-1))
 *   INTN_MAX  ==  2^(N-1) - 1
 *   UINTN_MAX ==  2^N - 1
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 说明：7.18.2.1 本身是“值”的规定（Semantics），其约束来自 7.18.1.1
 *       （精确宽度类型为可选，若提供则必须满足这些极限值）。因此负向测试
 *       针对的是“使用这些宏时违反类型/常量约束”的情形。
 */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] INTN_MIN == -(2^(N-1))，即 -(INTN_MAX) - 1
 *     对每个精确宽度有符号类型逐一验证。 */
static void test_signed_min(void)
{
#ifdef INT8_MIN
    assert(INT8_MIN == -(INT8_MAX) - 1);
    assert(INT8_MIN == -128);
#endif
#ifdef INT16_MIN
    assert(INT16_MIN == -(INT16_MAX) - 1);
    assert(INT16_MIN == -32768);
#endif
#ifdef INT32_MIN
    assert(INT32_MIN == -(INT32_MAX) - 1);
    assert(INT32_MIN == -2147483647 - 1);
#endif
#ifdef INT64_MIN
    assert(INT64_MIN == -(INT64_MAX) - 1);
    assert(INT64_MIN == -9223372036854775807LL - 1);
#endif
}

/* [1] INTN_MAX == 2^(N-1) - 1
 *     用位移/乘法在更宽类型中独立算出期望值再比较。 */
static void test_signed_max(void)
{
#ifdef INT8_MAX
    assert(INT8_MAX == (int8_t)((1u << 7) - 1));
    assert(INT8_MAX == 127);
#endif
#ifdef INT16_MAX
    assert(INT16_MAX == (int16_t)((1u << 15) - 1));
    assert(INT16_MAX == 32767);
#endif
#ifdef INT32_MAX
    assert(INT32_MAX == (int32_t)((1ull << 31) - 1));
    assert(INT32_MAX == 2147483647);
#endif
#ifdef INT64_MAX
    assert(INT64_MAX == (int64_t)((1ull << 63) - 1));
    assert(INT64_MAX == 9223372036854775807LL);
#endif
}

/* [1] UINTN_MAX == 2^N - 1
 *     无符号类型全 1 位模式，用 ~(uintN_t)0 独立验证。 */
static void test_unsigned_max(void)
{
#ifdef UINT8_MAX
    assert(UINT8_MAX == (uint8_t)~0u);
    assert(UINT8_MAX == 255);
#endif
#ifdef UINT16_MAX
    assert(UINT16_MAX == (uint16_t)~0u);
    assert(UINT16_MAX == 65535);
#endif
#ifdef UINT32_MAX
    assert(UINT32_MAX == (uint32_t)~0u);
    assert(UINT32_MAX == 4294967295u);
#endif
#ifdef UINT64_MAX
    assert(UINT64_MAX == (uint64_t)~0ull);
    assert(UINT64_MAX == 18446744073709551615ull);
#endif
}

/* [1] 交叉一致性：UINTN_MAX == 2 * (INTN_MAX + 1) - 1
 *     即无符号最大值恰为对应有符号最大值两倍加一。 */
static void test_cross_consistency(void)
{
#ifdef INT8_MAX
    assert((uintmax_t)UINT8_MAX == 2ull * ((uintmax_t)INT8_MAX + 1) - 1);
#endif
#ifdef INT16_MAX
    assert((uintmax_t)UINT16_MAX == 2ull * ((uintmax_t)INT16_MAX + 1) - 1);
#endif
#ifdef INT32_MAX
    assert((uintmax_t)UINT32_MAX == 2ull * ((uintmax_t)INT32_MAX + 1) - 1);
#endif
#ifdef INT64_MAX
    assert((uintmax_t)UINT64_MAX == 2ull * ((uintmax_t)INT64_MAX + 1) - 1);
#endif
}

/* [1] 边界值可实际存储：INTN_MAX 可存，INTN_MAX+1 溢出为 INTN_MIN（有符号回绕
 *     在无符号运算中验证，避免 UB）。 */
static void test_boundary_storage(void)
{
#ifdef INT8_MAX
    int8_t s8 = INT8_MAX;
    assert(s8 == 127);
    assert((uint8_t)(s8 + 1) == (uint8_t)INT8_MIN); /* 127+1 回绕到 -128 */
#endif
#ifdef UINT8_MAX
    uint8_t u8 = UINT8_MAX;
    assert(u8 == 255);
    assert((uint8_t)(u8 + 1) == 0); /* 无符号回绕 */
#endif
#ifdef INT32_MAX
    int32_t s32 = INT32_MAX;
    assert(s32 == 2147483647);
#endif
#ifdef UINT32_MAX
    uint32_t u32 = UINT32_MAX;
    assert(u32 == 4294967295u);
    assert(u32 + 1u == 0u);
#endif
}

int main(void)
{
    test_signed_min();
    test_signed_max();
    test_unsigned_max();
    test_cross_consistency();
    test_boundary_storage();

    printf("C99 7.18.2.1 exact-width integer limits: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「宏展开后必须是整型常量表达式，可用于 #if 预处理条件」：
 * 若把 INT8_MIN 当作浮点或非整型常量使用，预处理 #if 会报错。
 * 期望：gcc -std=c99 报 "floating constant in preprocessor expression" 或类似错误。 */
#if INT8_MIN == 1.5
#endif

/* 违反约束「精确宽度类型为可选，未定义时使用其宏应报错」：
 * 假设某实现未提供 INT128_MIN（C99 无此类型），直接引用未定义标识符。
 * 期望：编译报 "INT128_MIN undeclared"。 */
int bad1 = INT128_MIN;

/* 违反约束「UINTN_MAX 是整型常量，不能作为结构体成员名以外的非法用法」：
 * 把宏当作类型名使用，语法错误。
 * 期望：编译报语法错误。 */
UINT32_MAX x;

/* 违反约束「INTN_MIN 为负常量，不能用于数组维度（负维度非法）」：
 * 期望：编译报 "size of array is negative"。 */
int arr[INT8_MIN];

/* 违反约束「宏名不可被重新定义为不兼容的值后再用于常量表达式」：
 * 重定义 INT8_MAX 为字符串，破坏其整型常量属性。
 * 期望：编译报类型/常量表达式错误。 */
#define INT8_MAX "not an integer"
int bad2 = INT8_MAX + 1;

#endif /* 负向测试结束 */