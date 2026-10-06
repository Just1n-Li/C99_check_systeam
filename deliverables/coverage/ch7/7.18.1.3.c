/*
 * 测试 C99 7.18.1.3 —— Fastest minimum-width integer types
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，若单独编译应报错；
 *             此处被 #if 0 屏蔽，保证整个文件仍可编译运行。
 *
 * 覆盖段落：
 *   [1] 这些类型表示“至少具有指定宽度、且通常运算最快”的整数类型。
 *   [2] int_fastN_t 为至少 N 位的最快有符号类型；
 *       uint_fastN_t 为至少 N 位的最快无符号类型。
 *   [3] 必须提供 int_fast8_t/16/32/64_t 与 uint_fast8_t/16/32/64_t。
 *   Footnote 225：所选类型不保证对所有用途都最快。
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [3] 必须提供的 8 个 typedef 名称，若缺失则编译失败 */
static int_fast8_t   v_i8;
static uint_fast8_t  v_u8;
static int_fast16_t  v_i16;
static uint_fast16_t v_u16;
static int_fast32_t  v_i32;
static uint_fast32_t v_u32;
static int_fast64_t  v_i64;
static uint_fast64_t v_u64;

int main(void)
{
    /* [3] 这些类型必须存在，且为整数类型（可做算术运算） */
    v_i8  = 1;  v_u8  = 1;
    v_i16 = 1;  v_u16 = 1;
    v_i32 = 1;  v_u32 = 1;
    v_i64 = 1;  v_u64 = 1;

    /* [2] 有符号类型：宽度至少为 N 位 */
    assert(sizeof(int_fast8_t)  * CHAR_BIT >= 8);
    assert(sizeof(int_fast16_t) * CHAR_BIT >= 16);
    assert(sizeof(int_fast32_t) * CHAR_BIT >= 32);
    assert(sizeof(int_fast64_t) * CHAR_BIT >= 64);

    /* [2] 无符号类型：宽度至少为 N 位 */
    assert(sizeof(uint_fast8_t)  * CHAR_BIT >= 8);
    assert(sizeof(uint_fast16_t) * CHAR_BIT >= 16);
    assert(sizeof(uint_fast32_t) * CHAR_BIT >= 32);
    assert(sizeof(uint_fast64_t) * CHAR_BIT >= 64);

    /* [2] 有符号类型必须能表示对应宽度的正负范围（至少 N 位有符号） */
    assert(INT_FAST8_MAX  >= INT8_MAX);
    assert(INT_FAST16_MAX >= INT16_MAX);
    assert(INT_FAST32_MAX >= INT32_MAX);
    assert(INT_FAST64_MAX >= INT64_MAX);

    assert(INT_FAST8_MIN  <= INT8_MIN);
    assert(INT_FAST16_MIN <= INT16_MIN);
    assert(INT_FAST32_MIN <= INT32_MIN);
    assert(INT_FAST64_MIN <= INT64_MIN);

    /* [2] 无符号类型必须能表示对应宽度的最大值 */
    assert(UINT_FAST8_MAX  >= UINT8_MAX);
    assert(UINT_FAST16_MAX >= UINT16_MAX);
    assert(UINT_FAST32_MAX >= UINT32_MAX);
    assert(UINT_FAST64_MAX >= UINT64_MAX);

    /* [2] 有符号/无符号的“符号性”验证：无符号类型不会为负 */
    {
        uint_fast8_t  a = 0;
        uint_fast16_t b = 0;
        uint_fast32_t c = 0;
        uint_fast64_t d = 0;
        assert(a == 0 && b == 0 && c == 0 && d == 0);
        a = (uint_fast8_t)-1;   /* 回绕为最大值 */
        assert(a == UINT_FAST8_MAX);
    }

    /* [1] “通常运算最快”：仅验证其可用作常规整数运算，不强制比较速度 */
    {
        int_fast32_t x = 100, y = 200;
        int_fast32_t z = x + y;
        assert(z == 300);

        uint_fast64_t p = 1;
        p <<= 40;               /* 至少 64 位，移位安全 */
        assert(p == ((uint_fast64_t)1 << 40));
    }

    /* [2] 类型可用于指针、数组、sizeof 等常规整数用法 */
    {
        int_fast16_t arr[3] = { 1, 2, 3 };
        int_fast16_t *q = arr;
        assert(*q == 1 && q[2] == 3);
        assert(sizeof(arr) == 3 * sizeof(int_fast16_t));
    }

    /* Footnote 225：所选类型不保证对所有用途都最快 —— 只要求满足符号性与宽度 */
    assert(sizeof(int_fast8_t) >= 1);
    assert(sizeof(uint_fast8_t) >= 1);

    printf("C99 7.18.1.3 fast minimum-width integer types: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[3] 必须提供的类型」：若实现未提供 int_fast8_t，
 * 使用该名称应报 “unknown type name” 错误。
 * 注意：符合标准的实现必须提供它，因此这里仅演示“缺失即报错”的形态。 */
int_fast8_t missing_type_use;   /* 若未定义 int_fast8_t，gcc -std=c99 报错 */

/* 违反约束「[2] 类型为整数类型」：把 fast 类型当作结构体使用应报错 */
struct int_fast32_t { int x; };  /* 与 typedef 名冲突，重定义错误 */

/* 违反约束「[2] 类型为整数类型」：对整数类型做成员访问应报错 */
void bad_member_access(void)
{
    int_fast16_t v = 0;
    v.member = 1;   /* 错误：int_fast16_t 不是结构体/联合体，无成员 */
}

/* 违反约束「[2] 类型为整数类型」：对整数类型解引用应报错 */
void bad_deref(void)
{
    int_fast64_t v = 0;
    *v = 1;         /* 错误：一元 * 的操作数必须为指针 */
}

/* 违反约束「[2] 类型为整数类型」：对整数类型取下标应报错 */
void bad_subscript(void)
{
    uint_fast32_t v = 0;
    v[0] = 1;       /* 错误：下标操作要求指针或数组 */
}

#endif /* 负向测试结束 */