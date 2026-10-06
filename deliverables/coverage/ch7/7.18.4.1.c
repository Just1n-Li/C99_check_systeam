/*
 * 测试条款：C99 7.18.4.1  Macros for minimum-width integer constants
 *
 * 条款要求：
 *   [1] INTN_C(value)  展开为对应 int_leastN_t  类型的整型常量表达式。
 *       UINTN_C(value) 展开为对应 uint_leastN_t 类型的整型常量表达式。
 *       例如 uint_least64_t 为 unsigned long long int 时，
 *       UINT64_C(0x123) 可能展开为 0x123ULL。
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（放在 #if 0 中，不参与编译）。
 */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] INTN_C / UINTN_C 展开为整型常量表达式，可用于需要常量表达式的场合 */

/* [1] 用于静态初始化（要求常量表达式） */
static const int_least8_t   i8  = INT8_C(42);
static const int_least16_t  i16 = INT16_C(1234);
static const int_least32_t  i32 = INT32_C(123456);
static const int_least64_t  i64 = INT64_C(1234567890);

static const uint_least8_t  u8  = UINT8_C(200);
static const uint_least16_t u16 = UINT16_C(60000);
static const uint_least32_t u32 = UINT32_C(4000000000u);
static const uint_least64_t u64 = UINT64_C(0x123);

/* [1] 用于数组维度（要求整型常量表达式） */
static int arr8[INT8_C(4)];
static int arr16[INT16_C(8)];
static int arr32[INT32_C(16)];
static int arr64[INT64_C(32)];

/* [1] 用于 case 标签（要求整型常量表达式） */
static int switch_test(int x)
{
    switch (x) {
    case INT8_C(1):  return 1;
    case INT16_C(2): return 2;
    case INT32_C(3): return 3;
    case INT64_C(4): return 4;
    default:         return 0;
    }
}

/* [1] 用于枚举常量（要求整型常量表达式） */
enum { E8 = INT8_C(10), E16 = INT16_C(20), E32 = INT32_C(30), E64 = INT64_C(40) };

/* [1] 用于 #if 预处理条件（要求整型常量表达式） */
#if INT8_C(1) == 1
#  define PREPROCESSOR_OK 1
#else
#  define PREPROCESSOR_OK 0
#endif

int main(void)
{
    /* [1] 值正确性 */
    assert(i8  == 42);
    assert(i16 == 1234);
    assert(i32 == 123456);
    assert(i64 == 1234567890);

    assert(u8  == 200);
    assert(u16 == 60000);
    assert(u32 == 4000000000u);
    assert(u64 == 0x123);

    /* [1] 类型正确性：展开结果应具有对应的 least 类型 */
    /* 通过 _Generic 检查表达式类型（C99 无 _Generic，改用 sizeof 与类型宽度比较） */
    assert(sizeof(INT8_C(1))  == sizeof(int_least8_t));
    assert(sizeof(INT16_C(1)) == sizeof(int_least16_t));
    assert(sizeof(INT32_C(1)) == sizeof(int_least32_t));
    assert(sizeof(INT64_C(1)) == sizeof(int_least64_t));

    assert(sizeof(UINT8_C(1))  == sizeof(uint_least8_t));
    assert(sizeof(UINT16_C(1)) == sizeof(uint_least16_t));
    assert(sizeof(UINT32_C(1)) == sizeof(uint_least32_t));
    assert(sizeof(UINT64_C(1)) == sizeof(uint_least64_t));

    /* [1] 无符号宏展开结果应为无符号类型：UINT64_C 的值应 >= 0 且能表示大值 */
    assert(UINT64_C(0xFFFFFFFFFFFFFFFF) > 0);

    /* [1] 有符号宏展开结果应能表示负值（通过取负验证其为有符号类型） */
    assert(INT32_C(0) - INT32_C(1) < 0);

    /* [1] 数组维度使用常量表达式 */
    assert(sizeof(arr8)  / sizeof(arr8[0])  == 4);
    assert(sizeof(arr16) / sizeof(arr16[0]) == 8);
    assert(sizeof(arr32) / sizeof(arr32[0]) == 16);
    assert(sizeof(arr64) / sizeof(arr64[0]) == 32);

    /* [1] case 标签使用常量表达式 */
    assert(switch_test(1) == 1);
    assert(switch_test(2) == 2);
    assert(switch_test(3) == 3);
    assert(switch_test(4) == 4);
    assert(switch_test(99) == 0);

    /* [1] 枚举常量 */
    assert(E8 == 10 && E16 == 20 && E32 == 30 && E64 == 40);

    /* [1] 预处理条件 */
    assert(PREPROCESSOR_OK == 1);

    /* [1] 与 PRI/SCN 宏配合使用（同属 <inttypes.h> 家族） */
    {
        char buf[64];
        sprintf(buf, "%" PRId64, INT64_C(1234567890));
        assert(buf[0] == '1');
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「INTN_C/UINTN_C 的参数必须是整型常量表达式」：
 * 传入浮点常量，宏展开后无法构成整型常量表达式，gcc -std=c99 应报错。 */
int bad1 = INT32_C(1.5);

/* 违反约束「INTN_C/UINTN_C 的参数必须是整型常量表达式」：
 * 传入变量（非常量表达式），用于数组维度时 gcc -std=c99 应报错。 */
int n = 4;
int bad2[INT32_C(n)];

/* 违反约束「INTN_C/UINTN_C 的参数必须是整型常量表达式」：
 * 传入字符串字面量，宏展开后不是整型常量表达式，gcc -std=c99 应报错。 */
int bad3 = UINT64_C("123");

/* 违反约束「INTN_C/UINTN_C 的参数必须是整型常量表达式」：
 * 传入函数调用，不是常量表达式，用于 case 标签时 gcc -std=c99 应报错。 */
int f(void);
int bad4(void) {
    switch (0) {
    case INT32_C(f()): return 1;
    default: return 0;
    }
}

#endif