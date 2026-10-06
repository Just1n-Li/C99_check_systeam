/*
 * 测试 C99 7.18.3 —— Limits of other integer types
 *
 * 预期行为：
 *   正向测试：包含 <stdint.h> 后，PTRDIFF_MIN/MAX、SIZE_MAX、SIG_ATOMIC_MIN/MAX、
 *             WCHAR_MIN/MAX、WINT_MIN/MAX 这些宏应被定义（若对应类型存在），
 *             其值满足条款 [2][3][4][5] 给出的最小幅度要求，且可用于 #if。
 *   负向测试：违反约束的代码（如对宏赋值、把非整型常量表达式用于 #if 等）
 *             应导致编译报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <wchar.h>
#include <wctype.h>
#include <signal.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 这些宏指定“其他标准头中定义的整型”的最小/最大限制。
 *     这里验证它们确实由 <stdint.h> 提供（对宿主实现而言）。 */

/* [2] 每个宏必须能被替换为适合 #if 的常量表达式。
 *     下面用 #if 直接使用这些宏，若它们不是整型常量表达式则编译失败。 */
#if !defined(PTRDIFF_MIN) || !defined(PTRDIFF_MAX)
#  error "PTRDIFF_MIN/PTRDIFF_MAX must be defined"
#endif
#if !defined(SIZE_MAX)
#  error "SIZE_MAX must be defined"
#endif
#if !defined(SIG_ATOMIC_MIN) || !defined(SIG_ATOMIC_MAX)
#  error "SIG_ATOMIC_MIN/SIG_ATOMIC_MAX must be defined"
#endif
#if !defined(WCHAR_MIN) || !defined(WCHAR_MAX)
#  error "WCHAR_MIN/WCHAR_MAX must be defined"
#endif
#if !defined(WINT_MIN) || !defined(WINT_MAX)
#  error "WINT_MIN/WINT_MAX must be defined"
#endif

/* [2] 在 #if 中使用这些宏，验证它们是可用于预处理指令的常量表达式。 */
#if PTRDIFF_MAX < 65535
#  error "PTRDIFF_MAX must be >= 65535"
#endif
#if PTRDIFF_MIN > -65535
#  error "PTRDIFF_MIN must be <= -65535"
#endif
#if SIZE_MAX < 65535
#  error "SIZE_MAX must be >= 65535"
#endif

/* [2] 宏的类型应与“对应类型经整型提升后的类型”相同。
 *     用 _Generic（C11）不可用，这里用 sizeof 与类型比较的近似方式：
 *     对 PTRDIFF_MAX，其类型应为 ptrdiff_t 提升后的类型（通常为 long/int）。
 *     这里只做运行期一致性检查。 */

/* [3] sig_atomic_t 的符号性决定 SIG_ATOMIC_MIN/MAX 的边界。 */
#if defined(SIG_ATOMIC_MIN) && defined(SIG_ATOMIC_MAX)
/* 若 sig_atomic_t 为有符号：MIN <= -127 且 MAX >= 127
 * 若为无符号：MIN == 0 且 MAX >= 255
 * 用运行期判断符号性并检查。 */
#endif

/* [4] wchar_t 的符号性决定 WCHAR_MIN/MAX 的边界。 */
/* [5] wint_t 的符号性决定 WINT_MIN/MAX 的边界。 */

int main(void)
{
    /* [2] 运行期验证 PTRDIFF_MIN/MAX 的幅度要求 */
    assert(PTRDIFF_MAX >= 65535);
    assert(PTRDIFF_MIN <= -65535);

    /* [2] SIZE_MAX 幅度要求 */
    assert(SIZE_MAX >= 65535);

    /* [2] 宏的类型应与对应类型提升后的类型一致。
     *     用 sizeof 比较：PTRDIFF_MAX 的类型大小应等于 ptrdiff_t 提升后的大小。 */
    {
        /* 提升后的 ptrdiff_t 类型大小 */
        size_t sz_promoted = sizeof(+(ptrdiff_t)0);
        assert(sizeof(PTRDIFF_MAX) == sz_promoted);
        assert(sizeof(PTRDIFF_MIN) == sz_promoted);
    }
    {
        /* SIZE_MAX 的类型应为 size_t 提升后的类型 */
        size_t sz_promoted = sizeof(+(size_t)0);
        assert(sizeof(SIZE_MAX) == sz_promoted);
    }

    /* [3] sig_atomic_t 边界检查 */
    {
        sig_atomic_t s = 0;
        if ((sig_atomic_t)-1 < (sig_atomic_t)0) {
            /* 有符号 */
            assert(SIG_ATOMIC_MIN <= -127);
            assert(SIG_ATOMIC_MAX >= 127);
        } else {
            /* 无符号 */
            assert(SIG_ATOMIC_MIN == 0);
            assert(SIG_ATOMIC_MAX >= 255);
        }
        (void)s;
    }

    /* [4] wchar_t 边界检查 */
    {
        wchar_t w = 0;
        if ((wchar_t)-1 < (wchar_t)0) {
            /* 有符号 */
            assert(WCHAR_MIN <= -127);
            assert(WCHAR_MAX >= 127);
        } else {
            /* 无符号 */
            assert(WCHAR_MIN == 0);
            assert(WCHAR_MAX >= 255);
        }
        (void)w;
    }

    /* [5] wint_t 边界检查 */
    {
        wint_t wi = 0;
        if ((wint_t)-1 < (wint_t)0) {
            /* 有符号 */
            assert(WINT_MIN <= -32767);
            assert(WINT_MAX >= 32767);
        } else {
            /* 无符号 */
            assert(WINT_MIN == 0);
            assert(WINT_MAX >= 65535);
        }
        (void)wi;
    }

    /* [2] 宏的值应与对应类型的极值一致（实现定义，但需满足幅度要求）。
     *     这里验证 PTRDIFF_MAX 与 ptrdiff_t 的实际最大值一致。 */
    {
        ptrdiff_t p = (ptrdiff_t)PTRDIFF_MAX;
        assert(p == PTRDIFF_MAX);
        assert((ptrdiff_t)PTRDIFF_MIN == PTRDIFF_MIN);
    }
    {
        size_t s = (size_t)SIZE_MAX;
        assert(s == SIZE_MAX);
    }

    /* [2] 验证这些宏可用于 #if（已在文件顶部通过 #if 检查）。 */

    printf("C99 7.18.3 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [2]「每个宏应被替换为适合 #if 的常量表达式」：
 * 若把宏当作变量赋值，则它不是常量表达式，且宏本身不可赋值。
 * 期望：gcc -std=c99 报错（lvalue required / assignment to read-only）。 */
void neg_assign_macro(void)
{
    PTRDIFF_MAX = 0;   /* 错误：宏不是左值 */
    SIZE_MAX = 0;      /* 错误：宏不是左值 */
    WCHAR_MAX = 0;     /* 错误：宏不是左值 */
}

/* 违反约束 [2]「适合用于 #if 预处理指令的常量表达式」：
 * 在 #if 中使用非整型常量表达式（如浮点常量）应报错。
 * 期望：gcc -std=c99 报错（floating constant in preprocessor expression）。 */
#if 1.0
#  error "should not compile"
#endif

/* 违反约束 [2]「适合用于 #if 预处理指令的常量表达式」：
 * 在 #if 中使用字符串字面量应报错。
 * 期望：gcc -std=c99 报错（token "..." is not valid in preprocessor expressions）。 */
#if "abc"
#  error "should not compile"
#endif

/* 违反约束 [2]「适合用于 #if 预处理指令的常量表达式」：
 * 在 #if 中使用未定义的标识符（非宏）应报错（在严格模式下）。
 * 期望：gcc -std=c99 -Werror 报错（"X" is not defined）。 */
#if UNDEFINED_IDENTIFIER_XYZ
#  error "should not compile"
#endif

/* 违反约束 [2]「宏的类型应与对应类型提升后的类型相同」：
 * 把 PTRDIFF_MAX 赋给不兼容的指针类型，触发类型不匹配诊断。
 * 期望：gcc -std=c99 报错（incompatible types）。 */
void neg_type_mismatch(void)
{
    int *p = PTRDIFF_MAX;   /* 错误：整数赋给指针 */
    (void)p;
}

/* 违反约束 [2]「宏应被替换为常量表达式」：
 * 对宏取地址，宏不是对象，不能取地址。
 * 期望：gcc -std=c99 报错（lvalue required as unary '&' operand）。 */
void neg_address_of_macro(void)
{
    void *p = &SIZE_MAX;    /* 错误：宏不是对象 */
    (void)p;
}

/* 违反约束 [2]「宏应被替换为常量表达式」：
 * 在需要整型常量表达式的场合（如数组维度、case 标签）使用非整型常量。
 * 期望：gcc -std=c99 报错。 */
void neg_non_constant_case(void)
{
    int x = 0;
    switch (x) {
    case PTRDIFF_MAX:   /* 合法：PTRDIFF_MAX 是整型常量表达式 */
        break;
    case 1.5:           /* 错误：case 标签必须是整型常量表达式 */
        break;
    }
}

#endif /* 负向测试结束 */