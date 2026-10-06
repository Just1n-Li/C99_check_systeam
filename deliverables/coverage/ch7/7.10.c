/*
 * 测试条款：C99 7.10 Sizes of integer types <limits.h>
 *
 * 预期行为：
 *   正向测试：包含 <limits.h> 后，其中定义的各宏应能正常展开为整数常量表达式，
 *             其值满足 5.2.4.2.1 给出的最小值/最大值约束，程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如把宏当作非整数常量使用、对宏赋值等）应编译报错。
 *
 * 说明：7.10 [1] 规定 <limits.h> 定义若干宏，展开为各种整数类型的极限与参数；
 *       7.10 [2] 规定这些宏的含义及其取值约束列于 5.2.4.2.1。
 *       因此本测试逐项检查 5.2.4.2.1 中列出的每个宏的存在性与取值约束。
 */

#include <limits.h>
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <limits.h> 定义了若干宏，展开为整数类型的极限与参数。
 *     这里验证每个宏都能作为整数常量表达式使用（出现在 #if 中）。 */

/* [2] 依据 5.2.4.2.1 的取值约束，逐项静态检查（编译期）。 */

/* --- char 类型 --- */
#if CHAR_BIT < 8
#error "CHAR_BIT must be >= 8"
#endif
#if CHAR_MIN > 0
#error "CHAR_MIN must be <= 0"
#endif
#if SCHAR_MAX < 127
#error "SCHAR_MAX must be >= 127"
#endif
#if SCHAR_MIN > -127
#error "SCHAR_MIN must be <= -127"
#endif
#if UCHAR_MAX < 255
#error "UCHAR_MAX must be >= 255"
#endif

/* --- 多字节字符 --- */
#if MB_LEN_MAX < 1
#error "MB_LEN_MAX must be >= 1"
#endif

/* --- short --- */
#if SHRT_MAX < 32767
#error "SHRT_MAX must be >= 32767"
#endif
#if SHRT_MIN > -32767
#error "SHRT_MIN must be <= -32767"
#endif
#if USHRT_MAX < 65535
#error "USHRT_MAX must be >= 65535"
#endif

/* --- int --- */
#if INT_MAX < 32767
#error "INT_MAX must be >= 32767"
#endif
#if INT_MIN > -32767
#error "INT_MIN must be <= -32767"
#endif
#if UINT_MAX < 65535
#error "UINT_MAX must be >= 65535"
#endif

/* --- long --- */
#if LONG_MAX < 2147483647L
#error "LONG_MAX must be >= 2147483647"
#endif
#if LONG_MIN > -2147483647L
#error "LONG_MIN must be <= -2147483647"
#endif
#if ULONG_MAX < 4294967295UL
#error "ULONG_MAX must be >= 4294967295"
#endif

/* --- long long (C99 新增) --- */
#if LLONG_MAX < 9223372036854775807LL
#error "LLONG_MAX must be >= 9223372036854775807"
#endif
#if LLONG_MIN > -9223372036854775807LL
#error "LLONG_MIN must be <= -9223372036854775807"
#endif
#if ULLONG_MAX < 18446744073709551615ULL
#error "ULLONG_MAX must be >= 18446744073709551615"
#endif

/* 运行期验证：宏可作为整数常量表达式参与运算，且类型/符号关系自洽。 */
static void test_limits_runtime(void)
{
    /* [1] 宏展开为整数常量表达式，可用于初始化与运算 */
    int cbit = CHAR_BIT;
    assert(cbit >= 8);

    /* 有符号/无符号关系：U*_MAX == 2 * *_MAX + 1（二进制补码/反码/原码均成立） */
    assert((unsigned char)UCHAR_MAX == (unsigned char)(2 * SCHAR_MAX + 1));
    assert((unsigned short)USHRT_MAX == (unsigned short)(2 * SHRT_MAX + 1));
    assert((unsigned int)UINT_MAX == (unsigned int)(2 * INT_MAX + 1));
    assert((unsigned long)ULONG_MAX == (unsigned long)(2 * LONG_MAX + 1));
    assert((unsigned long long)ULLONG_MAX == (unsigned long long)(2 * LLONG_MAX + 1));

    /* 类型宽度单调不减：char <= short <= int <= long <= long long */
    assert(sizeof(char) <= sizeof(short));
    assert(sizeof(short) <= sizeof(int));
    assert(sizeof(int) <= sizeof(long));
    assert(sizeof(long) <= sizeof(long long));

    /* 各 *_MIN 与 *_MAX 的符号关系 */
    assert(SCHAR_MIN < 0 && SCHAR_MAX > 0);
    assert(SHRT_MIN < 0 && SHRT_MAX > 0);
    assert(INT_MIN < 0 && INT_MAX > 0);
    assert(LONG_MIN < 0 && LONG_MAX > 0);
    assert(LLONG_MIN < 0 && LLONG_MAX > 0);

    /* CHAR_MIN/CHAR_MAX 与 char 的符号性一致 */
    if ((char)-1 < 0) {
        /* char 为有符号 */
        assert(CHAR_MIN == SCHAR_MIN);
        assert(CHAR_MAX == SCHAR_MAX);
    } else {
        /* char 为无符号 */
        assert(CHAR_MIN == 0);
        assert(CHAR_MAX == UCHAR_MAX);
    }

    /* MB_LEN_MAX 至少为 1 */
    assert(MB_LEN_MAX >= 1);

    printf("All <limits.h> positive tests passed.\n");
}

int main(void)
{
    test_limits_runtime();
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「宏展开为整数常量表达式，不可作为左值」：
 * 对 INT_MAX 赋值，gcc -std=c99 应报错（lvalue required / assignment to read-only）。 */
INT_MAX = 0;

/* 违反约束「宏展开为整数常量表达式，不可取地址」：
 * 对宏取地址，gcc -std=c99 应报错（lvalue required as unary '&' operand）。 */
int *p = &INT_MAX;

/* 违反约束「宏展开为整数常量表达式，不可自增」：
 * 对宏使用 ++，gcc -std=c99 应报错（lvalue required as increment operand）。 */
++UCHAR_MAX;

/* 违反约束「宏展开为整数常量表达式，不可作为函数实参以外的可变对象」：
 * 对宏取地址并解引用赋值，gcc -std=c99 应报错。 */
*(&SHRT_MAX) = 1;

#endif