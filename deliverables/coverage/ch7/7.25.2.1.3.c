/*
 * 测试 C99 7.25.2.1.3 —— iswblank 函数
 *
 * 预期行为：
 *   正向测试：包含 <wctype.h>，调用 iswblank(wint_t)，在 "C" locale 下
 *             仅对 L' ' 和 L'\t' 返回非零（true），其余宽字符返回 0。
 *   负向测试：违反约束的代码（如参数类型错误、缺少头文件声明等）应编译报错。
 *
 * 覆盖段落：
 *   [1] 函数原型：int iswblank(wint_t wc);  头文件 <wctype.h>
 *   [2] 语义：标准空白宽字符为 L' ' 与 L'\t'；"C" locale 下仅这两者返回 true。
 */

#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证函数原型可用：返回类型为 int，参数为 wint_t */
static int proto_check(wint_t wc)
{
    /* 若 iswblank 原型正确，此处赋值合法 */
    int r = iswblank(wc);
    return r;
}

int main(void)
{
    /* [2] 在 "C" locale 下测试（默认即为 "C" locale，显式设置以确保） */
    setlocale(LC_ALL, "C");

    /* [2] 标准空白宽字符：空格 L' ' 应返回 true（非零） */
    assert(iswblank(L' ') != 0);

    /* [2] 标准空白宽字符：水平制表符 L'\t' 应返回 true（非零） */
    assert(iswblank(L'\t') != 0);

    /* [2] "C" locale 下，非标准空白字符应返回 0（false） */
    assert(iswblank(L'\n') == 0);   /* 换行不是 blank */
    assert(iswblank(L'\v') == 0);   /* 垂直制表符不是 blank */
    assert(iswblank(L'\f') == 0);   /* 换页不是 blank */
    assert(iswblank(L'\r') == 0);   /* 回车不是 blank */
    assert(iswblank(L'a')  == 0);
    assert(iswblank(L'Z')  == 0);
    assert(iswblank(L'0')  == 0);
    assert(iswblank(L'.')  == 0);
    assert(iswblank(L'\0') == 0);

    /* [1] 通过辅助函数验证原型（参数为 wint_t，返回 int） */
    assert(proto_check(L' ') != 0);
    assert(proto_check(L'\t') != 0);
    assert(proto_check(L'x') == 0);

    /* [2] 返回值语义：非零表示 true，0 表示 false（此处仅验证真假性） */
    {
        int r1 = iswblank(L' ');
        int r2 = iswblank(L'x');
        assert(r1 != 0);
        assert(r2 == 0);
    }

    /* [2] 与 iswspace 的关系：在 "C" locale 下，iswblank 为真的字符
     *     也应是 iswspace 为真的字符（blank 是 space 的子集） */
    assert(iswspace(L' ') != 0);
    assert(iswspace(L'\t') != 0);

    printf("C99 7.25.2.1.3 iswblank: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「iswblank 的参数类型为 wint_t」：
 * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
struct S { int x; } s;
iswblank(s);

/* 违反约束「iswblank 的参数类型为 wint_t」：
 * 传入指针类型，gcc -std=c99 应报错（指针不能隐式转换为 wint_t）。 */
int *p;
iswblank(p);

/* 违反约束「iswblank 的参数个数为 1」：
 * 传入两个实参，gcc -std=c99 应报错（实参过多）。 */
iswblank(L' ', L'\t');

/* 违反约束「iswblank 的参数个数为 1」：
 * 不传实参，gcc -std=c99 应报错（实参过少）。 */
iswblank();

/* 违反约束「iswblank 返回 int，不能作为左值赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报错（非左值）。 */
iswblank(L' ') = 1;

/* 违反约束「iswblank 需在 <wctype.h> 中声明」：
 * 若未包含 <wctype.h>，在 C99 下调用未声明函数应报错
 * （C99 取消了隐式函数声明）。 */
/* 注意：本片段需在未包含 <wctype.h> 的翻译单元中才能触发，
 * 此处仅作示意，实际测试时请单独编译不含该头文件的文件。 */
/* iswblank(L' '); */

#endif