/*
 * 测试条款：C99 7.18.4.2 —— Macros for greatest-width integer constants
 *
 * 条款要求：
 *   [1] INTMAX_C(value)  展开为类型为 intmax_t  的整数常量表达式，其值由参数指定。
 *       UINTMAX_C(value) 展开为类型为 uintmax_t 的整数常量表达式，其值由参数指定。
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：本条款只规定这两个宏的展开类型与值，不涉及 UB，因此负向测试针对
 *       “宏未定义 / 头文件未包含” 这类约束性错误（使用宏前必须包含 <stdint.h>）。
 */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] INTMAX_C 展开为 intmax_t 类型的整数常量表达式 */
static void test_intmax_c_type(void)
{
    /* 用 _Generic 检查表达式的类型确实是 intmax_t（C99 无 _Generic，
       故改用“赋值给 intmax_t 不丢失信息 + 与 intmax_t 变量比较”的方式，
       并借助 sizeof 验证类型宽度与 intmax_t 一致）。 */
    intmax_t v = INTMAX_C(0);
    assert(sizeof(INTMAX_C(0)) == sizeof(intmax_t));
    assert(sizeof(INTMAX_C(12345)) == sizeof(intmax_t));
    (void)v;
}

/* [1] INTMAX_C 的值由参数指定 */
static void test_intmax_c_value(void)
{
    assert(INTMAX_C(0)    == 0);
    assert(INTMAX_C(1)    == 1);
    assert(INTMAX_C(42)   == 42);
    assert(INTMAX_C(123456789) == 123456789);

    /* 与 intmax_t 变量比较，确认值语义 */
    intmax_t a = INTMAX_C(1000);
    assert(a == 1000);

    /* 可用于常量表达式（数组维度、case 标签、静态初始化） */
    {
        char arr[INTMAX_C(8)];
        assert(sizeof(arr) == 8);
    }
    {
        intmax_t s = INTMAX_C(7);
        switch (s) {
        case INTMAX_C(7):
            break;
        default:
            assert(0 && "case INTMAX_C(7) 未匹配");
        }
    }
}

/* [1] UINTMAX_C 展开为 uintmax_t 类型的整数常量表达式 */
static void test_uintmax_c_type(void)
{
    uintmax_t v = UINTMAX_C(0);
    assert(sizeof(UINTMAX_C(0)) == sizeof(uintmax_t));
    assert(sizeof(UINTMAX_C(12345)) == sizeof(uintmax_t));
    (void)v;
}

/* [1] UINTMAX_C 的值由参数指定 */
static void test_uintmax_c_value(void)
{
    assert(UINTMAX_C(0)  == 0);
    assert(UINTMAX_C(1)  == 1);
    assert(UINTMAX_C(42) == 42);
    assert(UINTMAX_C(123456789) == 123456789);

    uintmax_t a = UINTMAX_C(1000);
    assert(a == 1000);

    /* 可用于常量表达式 */
    {
        char arr[UINTMAX_C(16)];
        assert(sizeof(arr) == 16);
    }
    {
        uintmax_t s = UINTMAX_C(9);
        switch (s) {
        case UINTMAX_C(9):
            break;
        default:
            assert(0 && "case UINTMAX_C(9) 未匹配");
        }
    }
}

/* [1] 两个宏都是“整数常量表达式”，可用于静态初始化 */
static intmax_t  g_int  = INTMAX_C(123);
static uintmax_t g_uint = UINTMAX_C(456);

static void test_constant_expression(void)
{
    assert(g_int  == 123);
    assert(g_uint == 456);

    /* 编译期常量：可用于枚举常量初始化 */
    enum { E1 = INTMAX_C(5) };
    assert(E1 == 5);
}

/* [1] 与 <inttypes.h> 中的格式宏配合使用（PRIdMAX / PRIuMAX） */
static void test_with_format_macros(void)
{
    intmax_t  i = INTMAX_C(1234567);
    uintmax_t u = UINTMAX_C(7654321);
    char buf[64];

    sprintf(buf, "%" PRIdMAX, i);
    assert(strcmp(buf, "1234567") == 0);

    sprintf(buf, "%" PRIuMAX, u);
    assert(strcmp(buf, "7654321") == 0);
}

int main(void)
{
    test_intmax_c_type();
    test_intmax_c_value();
    test_uintmax_c_type();
    test_uintmax_c_value();
    test_constant_expression();
    test_with_format_macros();

    printf("C99 7.18.4.2 正向测试全部通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「使用 INTMAX_C 前必须包含 <stdint.h>」：
   未包含 <stdint.h> 时 INTMAX_C 未定义，gcc -std=c99 应报
   "INTMAX_C undeclared" / "implicit declaration" 类错误。 */
intmax_t bad1 = INTMAX_C(1);

/* 违反约束「使用 UINTMAX_C 前必须包含 <stdint.h>」：
   同上，UINTMAX_C 未定义，应编译报错。 */
uintmax_t bad2 = UINTMAX_C(1);

/* 违反约束「宏参数必须是预处理记号序列，能构成整数常量」：
   传入非法的记号（如未定义的标识符），展开后不是合法的整数常量表达式，
   应编译报错。 */
intmax_t bad3 = INTMAX_C(not_a_number);

/* 违反约束「宏参数不能为空」：
   INTMAX_C() 缺少参数，预处理阶段应报 "requires an argument" 类错误。 */
intmax_t bad4 = INTMAX_C();

/* 违反约束「宏参数不能为空」：
   UINTMAX_C() 缺少参数，应编译报错。 */
uintmax_t bad5 = UINTMAX_C();

#endif