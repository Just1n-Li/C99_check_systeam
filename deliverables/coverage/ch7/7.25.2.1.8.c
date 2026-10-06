/*
 * 测试目标：C99 7.25.2.1.8 —— iswprint 函数
 *
 * 条款内容：
 *   [1] 头文件 <wctype.h>，原型 int iswprint(wint_t wc);
 *   [2] iswprint 测试参数是否为“可打印宽字符”（printing wide character）。
 *
 * 预期行为：
 *   - 正向测试：包含 <wctype.h>，调用 iswprint，验证对可打印宽字符返回非零，
 *               对不可打印宽字符（如控制字符、换行、制表符）返回 0。
 *               整个程序应能编译并运行通过（assert 全部成立）。
 *   - 负向测试：违反约束的代码片段（如参数类型错误、缺少头文件声明等）
 *               应导致编译报错。这些片段放在 #if 0 中，不影响本文件编译。
 */

#include <stdio.h>
#include <wctype.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证头文件 <wctype.h> 提供了 iswprint 的声明，且返回类型为 int */
static int check_declaration(void)
{
    /* 取函数指针，若原型不匹配则编译失败 */
    int (*fp)(wint_t) = iswprint;
    return fp != NULL;
}

/* [2] 验证 iswprint 对可打印宽字符返回非零 */
static void test_printable(void)
{
    /* 常见可打印 ASCII 宽字符 */
    assert(iswprint(L'A') != 0);
    assert(iswprint(L'z') != 0);
    assert(iswprint(L'0') != 0);
    assert(iswprint(L' ') != 0);   /* 空格是可打印字符 */
    assert(iswprint(L'~') != 0);
    assert(iswprint(L'!') != 0);
    assert(iswprint(L'@') != 0);
    assert(iswprint(L'#') != 0);
    assert(iswprint(L'9') != 0);
    assert(iswprint(L'?') != 0);

    /* 可打印标点 */
    assert(iswprint(L'.') != 0);
    assert(iswprint(L',') != 0);
    assert(iswprint(L';') != 0);
    assert(iswprint(L':') != 0);
}

/* [2] 验证 iswprint 对不可打印宽字符返回 0 */
static void test_non_printable(void)
{
    /* 控制字符不可打印 */
    assert(iswprint(L'\n') == 0);   /* 换行 */
    assert(iswprint(L'\t') == 0);   /* 制表符 */
    assert(iswprint(L'\r') == 0);   /* 回车 */
    assert(iswprint(L'\v') == 0);   /* 垂直制表 */
    assert(iswprint(L'\f') == 0);   /* 换页 */
    assert(iswprint(L'\a') == 0);   /* 响铃 */
    assert(iswprint(L'\b') == 0);   /* 退格 */

    /* 其他控制字符 */
    assert(iswprint((wint_t)0x00) == 0);  /* NUL */
    assert(iswprint((wint_t)0x01) == 0);
    assert(iswprint((wint_t)0x1F) == 0);  /* US */
    assert(iswprint((wint_t)0x7F) == 0);  /* DEL */
}

/* [2] 验证 iswprint 的返回值语义：非零表示可打印，0 表示不可打印 */
static void test_return_semantics(void)
{
    /* 返回值只关心是否为零 */
    int r1 = iswprint(L'A');
    int r2 = iswprint(L'\n');
    assert(r1 != 0);
    assert(r2 == 0);

    /* 对同一字符多次调用结果一致 */
    assert((iswprint(L'X') != 0) == (iswprint(L'X') != 0));
    assert((iswprint(L'\n') == 0) == (iswprint(L'\n') == 0));
}

/* [2] 验证 iswprint 接受 wint_t 类型参数（包括 WEOF 等特殊值） */
static void test_wint_t_argument(void)
{
    wint_t wc;

    wc = L'A';
    assert(iswprint(wc) != 0);

    wc = L'\n';
    assert(iswprint(wc) == 0);

    /* WEOF 不是可打印字符 */
    assert(iswprint(WEOF) == 0);
}

/* [2] 验证 iswprint 与 iswgraph 的关系：可打印字符包含空格，
 *     而 iswgraph 不包含空格。这里只验证 iswprint 对空格返回非零。 */
static void test_space_is_printable(void)
{
    assert(iswprint(L' ') != 0);
}

int main(void)
{
    /* 设置 C locale，保证基本字符集行为可预期 */
    setlocale(LC_ALL, "C");

    /* [1] 声明检查 */
    assert(check_declaration());

    /* [2] 语义检查 */
    test_printable();
    test_non_printable();
    test_return_semantics();
    test_wint_t_argument();
    test_space_is_printable();

    printf("iswprint: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「iswprint 的参数类型为 wint_t」：
 * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
struct S { int x; } s;
iswprint(s);

/* 违反约束「iswprint 的参数类型为 wint_t」：
 * 传入指针类型，gcc -std=c99 应报错（指针不能隐式转换为 wint_t）。 */
int *p = 0;
iswprint(p);

/* 违反约束「iswprint 的参数类型为 wint_t」：
 * 传入浮点类型，gcc -std=c99 应报错（浮点不能隐式转换为整数类型）。 */
iswprint(3.14);

/* 违反约束「iswprint 的参数类型为 wint_t」：
 * 传入字符串字面量（char*），gcc -std=c99 应报错。 */
iswprint("A");

/* 违反约束「iswprint 的返回类型为 int」：
 * 试图将返回值赋给不兼容的类型（如结构体），gcc -std=c99 应报错。 */
struct T { int y; } t;
t = iswprint(L'A');

/* 违反约束「iswprint 的返回类型为 int」：
 * 试图将返回值赋给数组类型，gcc -std=c99 应报错。 */
int arr[3];
arr = iswprint(L'A');

/* 违反约束「iswprint 的返回类型为 int」：
 * 试图对返回值取地址并赋给不兼容的指针类型，gcc -std=c99 应报错。 */
struct U { int z; } *up;
up = &iswprint(L'A');

/* 违反约束「iswprint 的返回类型为 int」：
 * 试图将返回值作为函数调用，gcc -std=c99 应报错。 */
iswprint(L'A')();

/* 违反约束「iswprint 的参数个数为 1」：
 * 传入两个参数，gcc -std=c99 应报错（参数过多）。 */
iswprint(L'A', L'B');

/* 违反约束「iswprint 的参数个数为 1」：
 * 不传参数，gcc -std=c99 应报错（参数过少）。 */
iswprint();

/* 违反约束「iswprint 的参数类型为 wint_t」：
 * 传入 void 表达式，gcc -std=c99 应报错。 */
void f(void);
iswprint(f());

/* 违反约束「iswprint 的参数类型为 wint_t」：
 * 传入函数类型（函数名），gcc -std=c99 应报错。 */
iswprint(main);

#endif /* 负向测试结束 */