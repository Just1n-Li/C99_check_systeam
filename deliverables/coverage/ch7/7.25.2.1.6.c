/*
 * 测试 C99 7.25.2.1.6 —— iswgraph 函数
 *
 * 预期行为：
 *   正向测试：包含 <wctype.h>，调用 iswgraph(wint_t)，验证其语义
 *             “iswprint(wc) 为真且 iswspace(wc) 为假时 iswgraph(wc) 为真”，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数类型错误、缺少头文件声明等）
 *             应导致编译报错；这些片段放在 #if 0 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] 概要：头文件 <wctype.h>，原型 int iswgraph(wint_t wc);
 *   [2] 描述：iswgraph 为真 <=> iswprint 为真 且 iswspace 为假
 *   Footnote 306：iswgraph 与单字节 isgraph 在打印/空白/非 ' ' 的单字节
 *                 执行字符上可能不同（此处仅作说明，不引入额外特性）
 */

#include <stdio.h>
#include <assert.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：验证头文件 <wctype.h> 提供了 iswgraph 的声明，
 *     且其参数类型为 wint_t、返回类型为 int。
 *     通过取函数指针并检查类型来静态验证原型。 */
static int (*fp_iswgraph)(wint_t) = iswgraph;

/* [2] 描述：iswgraph(wc) 为真 当且仅当 iswprint(wc) 为真 且 iswspace(wc) 为假。
 *     对一组代表性宽字符逐一验证该等价关系。 */
static void test_equivalence(void)
{
    /* 选取覆盖各类别的宽字符：
     *   L'A'  可打印、非空白  -> graph 真
     *   L'0'  可打印、非空白  -> graph 真
     *   L'!'  可打印、非空白  -> graph 真
     *   L' '  可打印、是空白  -> graph 假
     *   L'\t' 非可打印、是空白 -> graph 假
     *   L'\n' 非可打印、是空白 -> graph 假
     *   L'\0' 非可打印、非空白 -> graph 假
     *   L'\a' 非可打印、非空白 -> graph 假
     */
    static const wint_t samples[] = {
        L'A', L'0', L'!', L'~',
        L' ', L'\t', L'\n', L'\v', L'\f', L'\r',
        L'\0', L'\a', L'\b'
    };
    size_t i;
    size_t n = sizeof(samples) / sizeof(samples[0]);

    for (i = 0; i < n; ++i) {
        wint_t wc = samples[i];
        int g = iswgraph(wc);
        int p = iswprint(wc);
        int s = iswspace(wc);

        /* [2] 核心等价关系：iswgraph(wc) == (iswprint(wc) && !iswspace(wc)) */
        assert(g == (p && !s));

        /* 返回值语义：非零表示真，零表示假 */
        assert(g == 0 || g != 0);
    }
}

/* [2] 描述：iswgraph 为真时，iswprint 必为真且 iswspace 必为假。
 *     单独验证蕴含方向，强化语义覆盖。 */
static void test_implications(void)
{
    static const wint_t samples[] = {
        L'A', L'z', L'9', L'#', L'@',
        L' ', L'\t', L'\n', L'\0', L'\a'
    };
    size_t i;
    size_t n = sizeof(samples) / sizeof(samples[0]);

    for (i = 0; i < n; ++i) {
        wint_t wc = samples[i];
        if (iswgraph(wc)) {
            /* 若 graph 为真，则 print 为真且 space 为假 */
            assert(iswprint(wc) != 0);
            assert(iswspace(wc) == 0);
        }
    }
}

/* [2] 描述：iswgraph 与 iswprint / iswspace 的一致性在 WEOF 上也应成立。
 *     WEOF 不是宽字符，但函数接受 wint_t，应返回 0（非图形字符）。 */
static void test_weof(void)
{
    /* WEOF 通常为负值，iswgraph 应返回 0 */
    assert(iswgraph(WEOF) == 0);
    /* 与等价关系保持一致 */
    assert(iswgraph(WEOF) == (iswprint(WEOF) && !iswspace(WEOF)));
}

/* [1] 概要：验证函数指针类型与原型一致（int (wint_t)）。 */
static void test_prototype(void)
{
    /* 若原型不匹配，此赋值在编译期即报错 */
    int (*p)(wint_t) = fp_iswgraph;
    assert(p == iswgraph);
}

int main(void)
{
    /* 设置区域，使宽字符分类函数有确定行为。
     * 使用 "C" 区域，保证基本字符集行为可预测。 */
    setlocale(LC_ALL, "C");

    test_prototype();      /* [1] */
    test_equivalence();    /* [2] */
    test_implications();   /* [2] */
    test_weof();           /* [2] */

    printf("C99 7.25.2.1.6 iswgraph: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「iswgraph 的参数类型为 wint_t」：
 * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
struct S { int x; } s;
iswgraph(s);

/* 违反约束「iswgraph 的参数类型为 wint_t」：
 * 传入指针类型，gcc -std=c99 应报错（指针不能隐式转换为 wint_t）。 */
int *ptr = 0;
iswgraph(ptr);

/* 违反约束「iswgraph 返回 int」：
 * 试图把返回值当作结构体使用，gcc -std=c99 应报错。 */
struct S t = iswgraph(L'A');

/* 违反约束「调用前需有 iswgraph 的声明」：
 * 若未包含 <wctype.h>，隐式声明与 C99 约束冲突（C99 要求函数调用前
 * 必须有可见声明），gcc -std=c99 应报错或警告为错误。 */
/* 注意：此处假设未包含 <wctype.h> 的情形，实际测试时需单独编译。 */
/* iswgraph(L'A'); */

/* 违反约束「iswgraph 的参数为单个 wint_t」：
 * 传入两个实参，gcc -std=c99 应报错（实参数量不匹配）。 */
iswgraph(L'A', L'B');

/* 违反约束「iswgraph 的参数为单个 wint_t」：
 * 不传实参，gcc -std=c99 应报错（实参数量不匹配）。 */
iswgraph();

/* 违反约束「iswgraph 的参数类型为 wint_t」：
 * 传入 double 类型，gcc -std=c99 应报错（浮点不能隐式转换为 wint_t）。 */
iswgraph(3.14);

#endif /* 负向测试结束 */