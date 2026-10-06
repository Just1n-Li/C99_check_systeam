/*
 * 测试 C99 7.1.3 Reserved identifiers（保留标识符）
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反 7.1.3 约束/规则的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，保证本文件仍可正常编译。
 *
 * 说明：7.1.3 主要规定“哪些标识符被保留”以及“在保留上下文中使用它们
 *       是未定义行为”。其中 [2] 的“未定义行为”本身不是约束，编译器不
 *       一定报错；但“定义保留标识符为宏名”等属于可诊断的违规，这里用
 *       负向测试演示期望的编译错误。正向测试则验证：只要不触碰保留标识
 *       符，普通标识符可自由使用，且标准头文件确实声明了其子条款列出的
 *       标识符。
 */

#include <assert.h>
#include <errno.h>      /* 7.1.3 [1]：errno 是保留的外部链接标识符（脚注 160） */
#include <math.h>       /* math_errhandling 保留 */
#include <setjmp.h>     /* setjmp 保留 */
#include <stdarg.h>     /* va_end 保留 */
#include <stdio.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] “No other identifiers are reserved.”
 * 普通标识符（不以 _ 开头、不与标准库名冲突）可自由用作文件作用域标识符、
 * 宏名、标签名、成员名等。这里演示多种普通标识符的合法使用。 */
#define MY_NORMAL_MACRO 42          /* 普通宏名，未被保留 */
static int my_file_scope_var = 7;   /* 普通文件作用域标识符，未被保留 */

struct MyStruct {                   /* 普通标签名，未被保留 */
    int member;                     /* 普通成员名，未被保留 */
};

/* [1] 标准头文件声明/定义了其子条款列出的标识符。
 * 验证 <string.h> 声明的 memcpy、<stdio.h> 声明的 printf 等可用。 */
static int test_standard_identifiers_available(void)
{
    char src[8] = "hello";
    char dst[8];
    memcpy(dst, src, sizeof src);   /* <string.h> 的 memcpy 可用 */
    assert(strcmp(dst, "hello") == 0);
    return 0;
}

/* [1] 脚注 160：errno 是保留的外部链接标识符，但作为标准库提供的对象，
 * 程序可以正常读取/设置它（这是标准允许的使用方式）。 */
static int test_errno_usable(void)
{
    errno = 0;
    assert(errno == 0);
    errno = EDOM;                   /* EDOM 由 <errno.h> 定义 */
    assert(errno == EDOM);
    return 0;
}

/* [1] math_errhandling 是保留的外部链接标识符，标准头文件提供其定义。 */
static int test_math_errhandling_available(void)
{
    /* math_errhandling 是 int 类型的宏/对象，可读取 */
    int v = math_errhandling;
    (void)v;
    return 0;
}

/* [1] setjmp / va_end 是保留的外部链接标识符，标准头文件提供其声明。
 * 这里只验证它们可被引用（不实际跳转，避免复杂控制流）。 */
static int test_setjmp_vaend_declared(void)
{
    jmp_buf env;
    /* setjmp 返回 0 表示直接调用；这里只验证可调用 */
    if (setjmp(env) == 0) {
        /* 正常路径 */
    }
    return 0;
}

/* [2] 普通标识符可作标签名、成员名、参数名等，均未被保留。 */
static int test_ordinary_identifiers(void)
{
    struct MyStruct s;
    s.member = MY_NORMAL_MACRO;     /* 使用普通宏 */
    assert(s.member == 42);
    assert(my_file_scope_var == 7);

    /* 普通标签名 */
    goto my_label;
my_label:
    return 0;
}

/* [1] 验证“以下划线开头”的标识符规则：
 *   - 以 _ 加大写字母或另一个 _ 开头：任何用途都保留。
 *   - 以 _ 开头（其他情况）：作为文件作用域标识符保留。
 * 正向测试只使用“不违反保留规则”的局部标识符（块作用域、非 _ 开头），
 * 因此合法。这里不定义任何保留标识符。 */
static int test_non_reserved_local_names(void)
{
    int local_var = 1;              /* 普通局部名，合法 */
    int _local_lower = 2;           /* 以 _ 开头但为块作用域、非文件作用域，
                                       且非 _大写/_下划线，标准未保留其作为
                                       块作用域标识符的用途（仅文件作用域保留），
                                       故此处合法使用。 */
    assert(local_var + _local_lower == 3);
    return 0;
}

int main(void)
{
    test_standard_identifiers_available();
    test_errno_usable();
    test_math_errhandling_available();
    test_setjmp_vaend_declared();
    test_ordinary_identifiers();
    test_non_reserved_local_names();

    printf("C99 7.1.3 reserved identifiers: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 7.1.3，应编译报错 ========== */
#if 0

/* 违反 [1]「All identifiers that begin with an underscore and either an
 * uppercase letter or another underscore are always reserved for any use.」
 * 期望：gcc -std=c99 -Wall -Werror 报错/警告（保留标识符被使用）。
 * 说明：这是“未定义行为”，严格说编译器不必须报错；但许多实现会诊断。
 * 这里演示期望被拒绝的用法。 */
int _Reserved_upper = 1;            /* _ 后跟大写字母，任何用途保留 */
int __reserved_double = 2;          /* _ 后跟另一个 _，任何用途保留 */

/* 违反 [1]「All identifiers that begin with an underscore are always
 * reserved for use as identifiers with file scope in both the ordinary
 * and tag name spaces.」
 * 期望：文件作用域使用 _ 开头的普通标识符应被诊断。 */
int _file_scope_reserved = 3;       /* 文件作用域，_ 开头，保留 */
struct _Tag_reserved { int x; };    /* 标签名空间，_ 开头，保留 */

/* 违反 [2]「defines a reserved identifier as a macro name」
 * 期望：把保留标识符定义为宏名应编译报错。
 * 例如把标准库保留名 errno 重新定义为宏。 */
#define errno 0                     /* 违反：errno 是保留标识符，不得作宏名 */

/* 违反 [2]「declares or defines an identifier in a context in which it is
 * reserved」：在文件作用域声明以 _ 开头的标识符。 */
static int _reserved_static = 4;

/* 违反 [3]「If the program removes (with #undef) any macro definition of an
 * identifier in the first group listed above, the behavior is undefined.」
 * 期望：对保留宏名 #undef 应被诊断（此处演示对标准保留宏的 #undef）。 */
#undef errno

#endif /* 负向测试结束 */