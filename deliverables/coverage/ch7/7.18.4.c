/*
 * 测试 C99 7.18.4 —— Macros for integer constants
 *
 * 预期行为：
 *   正向测试：包含 <stdint.h> 后，INT8_C/INT16_C/INT32_C/INT64_C 以及
 *             UINT8_C/UINT16_C/UINT32_C/UINT64_C 这些宏应能展开为可用于
 *             初始化对应整数类型对象的整数常量表达式，其类型为对应类型
 *             经整数提升后的类型，其值等于实参的值；并且可用于 #if。
 *   负向测试：实参不是无后缀整数常量（如带后缀、浮点、字符串、标识符等）
 *             时，违反 [2] 的约束，编译器应报错。
 *
 * 说明：负向片段统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 宏名与 <stdint.h> 中的类型名对应，展开为适合初始化对应类型对象的整数常量 */
int main(void)
{
    /* [1] 用 INT8_C / UINT8_C 初始化 int_least8_t / uint_least8_t 等对应类型 */
    int_least8_t   i8  = INT8_C(42);
    uint_least8_t  u8  = UINT8_C(200);
    int_least16_t  i16 = INT16_C(1234);
    uint_least16_t u16 = UINT16_C(60000);
    int_least32_t  i32 = INT32_C(123456);
    uint_least32_t u32 = UINT32_C(4000000000u);
    int_least64_t  i64 = INT64_C(1234567890123);
    uint_least64_t u64 = UINT64_C(18000000000000000000u);

    /* [3] 值等于实参的值 */
    assert(i8  == 42);
    assert(u8  == 200);
    assert(i16 == 1234);
    assert(u16 == 60000);
    assert(i32 == 123456);
    assert(u32 == 4000000000u);
    assert(i64 == 1234567890123LL);
    assert(u64 == 18000000000000000000ULL);

    /* [3] 类型为对应类型经整数提升后的类型。
     * 对于 int8_t/uint8_t/int16_t/uint16_t，提升后为 int（若 int 能表示所有值）。
     * 用 _Generic 检查表达式的类型（C11 特性，这里仅作演示；若编译器为 C99
     * 严格模式，可改用 sizeof 比较）。
     * 为保持 C99 兼容，这里用 sizeof 验证提升后的类型大小。 */
    assert(sizeof(INT8_C(1))  == sizeof(int));
    assert(sizeof(UINT8_C(1)) == sizeof(int));
    assert(sizeof(INT16_C(1)) == sizeof(int));
    assert(sizeof(UINT16_C(1))== sizeof(int));
    /* int32_t/uint32_t 提升后通常为 int/unsigned int */
    assert(sizeof(INT32_C(1)) == sizeof(int32_t));
    assert(sizeof(UINT32_C(1))== sizeof(uint32_t));
    /* int64_t/uint64_t 提升后为 long long / unsigned long long（或 long 等） */
    assert(sizeof(INT64_C(1)) == sizeof(int64_t));
    assert(sizeof(UINT64_C(1))== sizeof(uint64_t));

    /* [3] 可用于 #if 预处理指令（整数常量表达式） */
#if INT32_C(100) > INT32_C(50)
    assert(1);
#else
    assert(0);
#endif

#if UINT32_C(10) == 10
    assert(1);
#else
    assert(0);
#endif

    /* [3] 宏展开结果可用于常量初始化（静态存储期对象） */
    static const int_least32_t sci = INT32_C(777);
    assert(sci == 777);

    /* [2] 实参为无后缀整数常量，值不超过对应类型的限制 */
    /* 边界值测试：INT8_C 的最大值 127 */
    int_least8_t max8 = INT8_C(127);
    assert(max8 == 127);
    /* UINT8_C 的最大值 255 */
    uint_least8_t umax8 = UINT8_C(255);
    assert(umax8 == 255);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [2]：实参必须是无后缀整数常量。
 * 带后缀的整数常量（如 42L）不是「无后缀整数常量」，应报错。 */
int_least32_t bad1 = INT32_C(42L);

/* 违反约束 [2]：实参必须是无后缀整数常量。
 * 浮点常量不是整数常量，应报错。 */
int_least32_t bad2 = INT32_C(3.14);

/* 违反约束 [2]：实参必须是无后缀整数常量。
 * 字符串字面量不是整数常量，应报错。 */
int_least32_t bad3 = INT32_C("42");

/* 违反约束 [2]：实参必须是无后缀整数常量。
 * 标识符不是整数常量，应报错。 */
int x = 5;
int_least32_t bad4 = INT32_C(x);

/* 违反约束 [2]：实参的值不得超过对应类型的限制。
 * INT8_C 的实参 128 超过 int8_t 的上限 127，应报错。 */
int_least8_t bad5 = INT8_C(128);

/* 违反约束 [2]：实参的值不得超过对应类型的限制。
 * UINT8_C 的实参 256 超过 uint8_t 的上限 255，应报错。 */
uint_least8_t bad6 = UINT8_C(256);

/* 违反约束 [2]：实参的值不得超过对应类型的限制。
 * INT16_C 的实参 32768 超过 int16_t 的上限 32767，应报错。 */
int_least16_t bad7 = INT16_C(32768);

/* 违反约束 [2]：实参的值不得超过对应类型的限制。
 * UINT16_C 的实参 65536 超过 uint16_t 的上限 65535，应报错。 */
uint_least16_t bad8 = UINT16_C(65536);

/* 违反约束 [2]：实参必须是无后缀整数常量。
 * 负号是一元运算符，-1 不是「无后缀整数常量」（它是表达式），应报错。 */
int_least32_t bad9 = INT32_C(-1);

/* 违反约束 [2]：实参必须是无后缀整数常量。
 * 括号表达式不是「无后缀整数常量」，应报错。 */
int_least32_t bad10 = INT32_C((42));

#endif /* 负向测试结束 */

/* 附注：
 * 1. 正向测试覆盖了 [1]（宏名与类型对应、展开为整数常量）、
 *    [2]（实参为无后缀整数常量且值不超限）、
 *    [3]（展开为整数常量表达式、类型为提升后类型、值等于实参、可用于 #if）。
 * 2. 负向测试覆盖了 [2] 的约束：实参必须是无后缀整数常量，且值不超限。
 *    违反时编译器应报错。
 * 3. 脚注 230 是 C++ 相关建议，不适用于 C 测试。
 */