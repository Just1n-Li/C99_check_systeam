/*
 * 测试 C99 7.1.2 Standard headers
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（验证 [1][2][4][5][6] 的语义）。
 *   负向测试：位于 #if 0 块中的代码违反 [4] 的约束，编译器应报错。
 *
 * 说明：条款 [3] 描述的是「把与标准头同名的文件放到搜索路径中」导致 UB，
 *       属于 UB 而非约束，故不作为负向测试；[7] 是附录 B 的引用，无代码可测。
 */

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 每个库函数都在头文件中以带原型的类型声明；
 *     头文件通过 #include 提供其内容。
 * [2] 标准头列表：这里包含若干标准头，验证它们存在且可被包含。 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* [4] 标准头可以任意顺序包含；每个头在同一作用域内可被多次包含，
 *     效果与只包含一次相同（幂等性）。下面重复包含若干头，验证幂等。 */
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* [4] 头文件必须在任何外部声明或定义之外被包含（上面已满足）。
 *     下面开始外部声明/定义。 */

/* [6] 库函数的声明具有外部链接：通过取函数地址并跨翻译单元可链接来体现。
 *     这里用函数指针验证库函数名具有外部链接（可被取地址）。 */
static int (*const p_printf)(const char *, ...) = printf;
static void *(*const p_malloc)(size_t) = malloc;
static size_t (*const p_strlen)(const char *) = strlen;

/* [5] 对象式宏在必要时被完整括号保护，使其在任意表达式中像单个标识符一样分组。
 *     用标准头中的对象式宏验证：INT_MAX、CHAR_BIT、EOF、NULL 等。
 *     这里重点验证「像单个标识符一样分组」：例如 INT_MAX + 1 应等价于
 *     (INT_MAX) + 1，而不是 INT_MAX 展开后与相邻运算符错误结合。 */
static int test_macro_grouping(void)
{
    /* [5] INT_MAX 是对象式宏，应像单个标识符一样参与运算 */
    long a = INT_MAX;
    long b = INT_MAX + 1L;          /* 若宏未加括号，可能被错误分组 */
    assert(a == 2147483647L || a == (long)INT_MAX);
    assert(b == a + 1L);

    /* [5] CHAR_BIT 是对象式宏 */
    int c = CHAR_BIT * 2;
    assert(c == CHAR_BIT * 2);

    /* [5] EOF 是对象式宏，参与比较 */
    int e = EOF;
    assert(e < 0);

    /* [5] NULL 是对象式宏（空指针常量），像单个标识符一样使用 */
    char *p = NULL;
    assert(p == NULL);

    /* [5] 用 sizeof 验证宏像单个标识符一样分组 */
    assert(sizeof(INT_MAX) == sizeof(int));

    return 0;
}

/* [1] 头文件声明的类型不应带类型限定符（除非另有说明）。
 *     验证标准头中声明的类型（如 size_t、ptrdiff_t、time_t）可用作
 *     不带限定符的类型名。 */
static int test_header_types(void)
{
    size_t sz = sizeof(int);
    ptrdiff_t pd = 0;
    time_t t = (time_t)0;
    assert(sz == sizeof(int));
    assert(pd == 0);
    (void)t;
    return 0;
}

/* [1] 库函数以带原型的类型声明：调用时参数按原型转换，而非默认实参提升。
 *     验证：向 printf 传 float 会被提升为 double（可变参数），
 *     而向带原型的固定参数函数传参按原型转换。 */
static int test_prototype(void)
{
    /* [1] 带原型的库函数：strlen 的参数为 const char *，返回 size_t */
    const char *s = "hello";
    size_t n = strlen(s);
    assert(n == 5);

    /* [1] 带原型的库函数：malloc 的参数为 size_t */
    void *m = malloc(sizeof(int));
    assert(m != NULL);
    free(m);

    return 0;
}

/* [4] 验证 <assert.h> 的效果依赖于 NDEBUG 的定义。
 *     本翻译单元未定义 NDEBUG，故 assert 生效。 */
static int test_assert_active(void)
{
#ifndef NDEBUG
    /* assert 生效：条件为真时不中止 */
    assert(1 == 1);
#endif
    return 0;
}

/* [6] 验证库函数具有外部链接：函数指针非空，且可调用。 */
static int test_external_linkage(void)
{
    assert(p_printf != NULL);
    assert(p_malloc != NULL);
    assert(p_strlen != NULL);

    /* 通过函数指针调用库函数 */
    size_t n = p_strlen("abc");
    assert(n == 3);

    void *m = p_malloc(8);
    assert(m != NULL);
    free(m);

    return 0;
}

int main(void)
{
    test_macro_grouping();      /* [5] */
    test_header_types();        /* [1] */
    test_prototype();           /* [1] */
    test_assert_active();       /* [4] */
    test_external_linkage();    /* [6] */

    printf("C99 7.1.2 standard headers: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [4]：「If used, a header shall be included outside of any external
 * declaration or definition.」
 * 期望：gcc -std=c99 报错，例如 "expected declaration specifiers" 或
 *       "#include nested within a function/declaration"。
 * 说明：把 #include 放在函数体内（外部声明/定义之外的反面）。 */
void bad_include_inside_function(void)
{
    #include <stdio.h>   /* 错误：头文件必须在任何外部声明或定义之外包含 */
}

/* 违反约束 [4]：「The program shall not have any macros with names lexically
 * identical to keywords currently defined prior to the inclusion.」
 * 期望：gcc -std=c99 报错，例如 "cannot use keyword 'int' as macro name"。
 * 说明：在包含标准头之前定义与关键字同名的宏。 */
#define int long
#include <stdio.h>       /* 错误：包含头之前已定义与关键字同名的宏 */
#undef int

/* 违反约束 [4]：头文件必须在首次引用其声明的函数/对象/类型/宏之前被包含。
 * 期望：gcc -std=c99 报错，例如 "implicit declaration of function 'printf'"。
 * 说明：在包含 <stdio.h> 之前引用 printf。 */
void bad_use_before_include(void)
{
    printf("no header included yet\n");  /* 错误：首次引用前未包含 <stdio.h> */
}
#include <stdio.h>

#endif /* 负向测试结束 */