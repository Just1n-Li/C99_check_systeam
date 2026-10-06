/*
 * 测试 C99 7.18 <stdint.h> Integer types
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，编译器应报错。
 *
 * 覆盖段落：
 *   [1] 头文件声明指定宽度的整数类型集合，并定义相应宏（含 limits 宏）。
 *   [2] 类型类别：精确宽度、至少指定宽度、最快、可容纳指针、最大宽度。
 *   [3] 相应宏指定类型极限并构造合适常量。
 *   [4] 实现提供的类型必须声明 typedef 并定义关联宏；不提供的不得声明/定义；
 *       "required" 类型必须提供，"optional" 类型可不提供。
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件存在，且声明了指定宽度的整数类型集合与相应宏。
 *     这里通过使用这些类型与宏来验证其存在性。 */

/* [2] 精确宽度类型（exact width）：int8_t ... int64_t / uint8_t ... uint64_t
 *     这些是 required 类型（见 7.18.1.1）。 */
static void test_exact_width(void)
{
    /* 精确宽度：sizeof 必须精确等于名字中的位数/CHAR_BIT */
    assert(sizeof(int8_t)   * CHAR_BIT == 8);
    assert(sizeof(int16_t)  * CHAR_BIT == 16);
    assert(sizeof(int32_t)  * CHAR_BIT == 32);
    assert(sizeof(int64_t)  * CHAR_BIT == 64);

    assert(sizeof(uint8_t)  * CHAR_BIT == 8);
    assert(sizeof(uint16_t) * CHAR_BIT == 16);
    assert(sizeof(uint32_t) * CHAR_BIT == 32);
    assert(sizeof(uint64_t) * CHAR_BIT == 64);

    /* 有符号/无符号对应关系 */
    assert(sizeof(int8_t)  == sizeof(uint8_t));
    assert(sizeof(int16_t) == sizeof(uint16_t));
    assert(sizeof(int32_t) == sizeof(uint32_t));
    assert(sizeof(int64_t) == sizeof(uint64_t));

    /* 实际赋值与运算 */
    int8_t  a = -128;
    uint8_t b = 255;
    int64_t c = INT64_C(9223372036854775807);
    uint64_t d = UINT64_C(18446744073709551615);
    assert(a == -128);
    assert(b == 255);
    assert(c == 9223372036854775807LL);
    assert(d == 18446744073709551615ULL);
}

/* [2] 至少指定宽度类型（least width）：int_leastN_t / uint_leastN_t
 *     required 类型（见 7.18.1.2）。 */
static void test_least_width(void)
{
    assert(sizeof(int_least8_t)   * CHAR_BIT >= 8);
    assert(sizeof(int_least16_t)  * CHAR_BIT >= 16);
    assert(sizeof(int_least32_t)  * CHAR_BIT >= 32);
    assert(sizeof(int_least64_t)  * CHAR_BIT >= 64);

    assert(sizeof(uint_least8_t)  * CHAR_BIT >= 8);
    assert(sizeof(uint_least16_t) * CHAR_BIT >= 16);
    assert(sizeof(uint_least32_t) * CHAR_BIT >= 32);
    assert(sizeof(uint_least64_t) * CHAR_BIT >= 64);

    int_least32_t x = 123456;
    uint_least64_t y = 9876543210ULL;
    assert(x == 123456);
    assert(y == 9876543210ULL);
}

/* [2] 最快类型（fastest）：int_fastN_t / uint_fastN_t
 *     required 类型（见 7.18.1.3）。 */
static void test_fast_width(void)
{
    assert(sizeof(int_fast8_t)   * CHAR_BIT >= 8);
    assert(sizeof(int_fast16_t)  * CHAR_BIT >= 16);
    assert(sizeof(int_fast32_t)  * CHAR_BIT >= 32);
    assert(sizeof(int_fast64_t)  * CHAR_BIT >= 64);

    assert(sizeof(uint_fast8_t)  * CHAR_BIT >= 8);
    assert(sizeof(uint_fast16_t) * CHAR_BIT >= 16);
    assert(sizeof(uint_fast32_t) * CHAR_BIT >= 32);
    assert(sizeof(uint_fast64_t) * CHAR_BIT >= 64);

    int_fast16_t f = 42;
    assert(f == 42);
}

/* [2] 可容纳对象指针的整数类型：intptr_t / uintptr_t
 *     optional 类型（见 7.18.1.4）。实现若提供则必须声明 typedef 并定义宏。
 *     这里用条件编译检测：若提供了 intptr_t，则测试其可容纳指针。 */
static void test_intptr(void)
{
#ifdef INTPTR_MAX
    /* 提供了 intptr_t / uintptr_t */
    int x = 7;
    int *p = &x;
    intptr_t ip = (intptr_t)p;
    uintptr_t up = (uintptr_t)p;
    /* 指针 -> 整数 -> 指针 往返 */
    int *q = (int *)(intptr_t)ip;
    assert(q == p);
    assert((intptr_t)up == ip);
    /* 至少能容纳指针宽度 */
    assert(sizeof(intptr_t) >= sizeof(void *));
    assert(sizeof(uintptr_t) >= sizeof(void *));
#else
    /* 实现未提供 intptr_t：按 [4] 不得声明该 typedef，也不得定义关联宏。
     * 这里无法直接"证明不存在"，但可确认宏未定义。 */
    assert(1);
#endif
}

/* [2] 最大宽度类型：intmax_t / uintmax_t
 *     required 类型（见 7.18.1.5）。 */
static void test_max_width(void)
{
    /* 最大宽度类型至少不窄于任何其它有符号/无符号整数类型 */
    assert(sizeof(intmax_t)  >= sizeof(int64_t));
    assert(sizeof(uintmax_t) >= sizeof(uint64_t));
    assert(sizeof(intmax_t)  >= sizeof(long));
    assert(sizeof(uintmax_t) >= sizeof(unsigned long));

    intmax_t  m = INTMAX_C(9223372036854775807);
    uintmax_t u = UINTMAX_C(18446744073709551615);
    assert(m == 9223372036854775807LL);
    assert(u == 18446744073709551615ULL);
}

/* [3] 相应宏指定类型极限并构造合适常量。
 *     验证 limits 宏与常量构造宏。 */
static void test_limits_and_constants(void)
{
    /* 精确宽度类型的极限宏（7.18.2.1） */
    assert(INT8_MAX  == 127);
    assert(INT8_MIN  == -128);
    assert(UINT8_MAX == 255);

    assert(INT16_MAX  == 32767);
    assert(INT16_MIN  == -32768);
    assert(UINT16_MAX == 65535);

    assert(INT32_MAX  == 2147483647);
    assert(INT32_MIN  == (-2147483647 - 1));
    assert(UINT32_MAX == 4294967295U);

    assert(INT64_MAX  == 9223372036854775807LL);
    assert(INT64_MIN  == (-9223372036854775807LL - 1));
    assert(UINT64_MAX == 18446744073709551615ULL);

    /* least 类型极限宏（7.18.2.2） */
    assert(INT_LEAST8_MAX  >= 127);
    assert(UINT_LEAST8_MAX >= 255);
    assert(INT_LEAST64_MAX >= 9223372036854775807LL);

    /* fast 类型极限宏（7.18.2.3） */
    assert(INT_FAST8_MAX  >= 127);
    assert(UINT_FAST8_MAX >= 255);

    /* intptr 极限宏（7.18.2.4，optional） */
#ifdef INTPTR_MAX
    assert(INTPTR_MAX >= 0);
    assert(UINTPTR_MAX >= 0);
#endif

    /* intmax 极限宏（7.18.2.5） */
    assert(INTMAX_MAX  >= 9223372036854775807LL);
    assert(UINTMAX_MAX >= 18446744073709551615ULL);

    /* 常量构造宏（7.18.4）：构造合适类型的常量 */
    int_least8_t  c8  = INT8_C(100);
    uint_least8_t uc8 = UINT8_C(200);
    int_least16_t c16 = INT16_C(30000);
    uint_least16_t uc16 = UINT16_C(60000);
    int_least32_t c32 = INT32_C(2000000000);
    uint_least32_t uc32 = UINT32_C(4000000000);
    int_least64_t c64 = INT64_C(9000000000000000000);
    uint_least64_t uc64 = UINT64_C(18000000000000000000);
    intmax_t  cm  = INTMAX_C(9000000000000000000);
    uintmax_t ucm = UINTMAX_C(18000000000000000000);

    assert(c8 == 100);
    assert(uc8 == 200);
    assert(c16 == 30000);
    assert(uc16 == 60000);
    assert(c32 == 2000000000);
    assert(uc32 == 4000000000U);
    assert(c64 == 9000000000000000000LL);
    assert(uc64 == 18000000000000000000ULL);
    assert(cm == 9000000000000000000LL);
    assert(ucm == 18000000000000000000ULL);
}

/* [4] 实现提供的类型必须声明 typedef 并定义关联宏。
 *     这里通过"使用即验证"：若 typedef 未声明，编译失败。
 *     同时验证 required 类型一定存在（上面已用）。 */
static void test_required_types_exist(void)
{
    /* required 类型：精确宽度、least、fast、intmax。
     * 只要上面编译通过，即证明这些 typedef 已声明。 */
    int8_t   a = 0;  (void)a;
    int16_t  b = 0;  (void)b;
    int32_t  c = 0;  (void)c;
    int64_t  d = 0;  (void)d;
    uint8_t  e = 0;  (void)e;
    uint16_t f = 0;  (void)f;
    uint32_t g = 0;  (void)g;
    uint64_t h = 0;  (void)h;

    int_least8_t   i = 0;  (void)i;
    int_least16_t  j = 0;  (void)j;
    int_least32_t  k = 0;  (void)k;
    int_least64_t  l = 0;  (void)l;
    uint_least8_t  m = 0;  (void)m;
    uint_least16_t n = 0;  (void)n;
    uint_least32_t o = 0;  (void)o;
    uint_least64_t p = 0;  (void)p;

    int_fast8_t   q = 0;  (void)q;
    int_fast16_t  r = 0;  (void)r;
    int_fast32_t  s = 0;  (void)s;
    int_fast64_t  t = 0;  (void)t;
    uint_fast8_t  u = 0;  (void)u;
    uint_fast16_t v = 0;  (void)v;
    uint_fast32_t w = 0;  (void)w;
    uint_fast64_t x = 0;  (void)x;

    intmax_t  y = 0;  (void)y;
    uintmax_t z = 0;  (void)z;

    assert(1);
}

int main(void)
{
    test_exact_width();
    test_least_width();
    test_fast_width();
    test_intptr();
    test_max_width();
    test_limits_and_constants();
    test_required_types_exist();

    printf("C99 7.18 <stdint.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [4]：实现未提供的 optional 类型不得声明其 typedef。
 * 这里假设某实现未提供 intptr_t，则使用 intptr_t 应报错。
 * 期望：gcc -std=c99 报 "unknown type name 'intptr_t'"。
 * 注意：在提供 intptr_t 的实现上此片段不会报错，故仅作示意。 */
intptr_t bad_intptr;

/* 违反约束 [4]：实现未提供的 optional 类型不得定义关联宏。
 * 若实现未提供 intptr_t，则 INTPTR_MAX 不应被定义；
 * 使用未定义的宏在 #if 中会被当作 0，但直接引用标识符会报错。
 * 期望：报 "INTPTR_MAX undeclared"（若未定义）。 */
int bad_macro = INTPTR_MAX;

/* 违反约束 [3]：常量构造宏的参数必须是整型常量表达式。
 * 传入浮点常量违反约束。
 * 期望：报错（INT32_C 要求整型常量）。 */
int bad_const = INT32_C(1.5);

/* 违反约束 [3]：UINT8_C 的参数超出 uint_least8_t 可表示范围时，
 * 标准要求参数为"适合该类型的整型常量表达式"，超范围违反约束。
 * 期望：报错或警告（视实现）。 */
uint_least8_t bad_range = UINT8_C(999999);

/* 违反约束 [2]：int8_t 等精确宽度类型是 typedef 名，不是关键字，
 * 不能用于定义新的类型名（如 struct int8_t）。
 * 期望：报错。 */
struct int8_t bad_struct;

/* 违反约束 [1]：<stdint.h> 只声明类型与宏，不声明函数。
 * 调用不存在的函数违反约束（隐式声明在 C99 中已移除）。
 * 期望：报 "implicit declaration of function" 错误。 */
int bad_func = stdint_undefined_function();

#endif