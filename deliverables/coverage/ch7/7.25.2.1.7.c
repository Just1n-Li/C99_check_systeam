/*
 * 测试 C99 7.25.2.1.7 —— iswlower 函数
 *
 * 预期行为：
 *   正向测试：包含 <wctype.h>，调用 iswlower(wint_t)，验证：
 *             - 小写字母（如 L'a'..L'z'）返回非零
 *             - 大写字母、数字、标点、空白、控制字符返回 0
 *             - 返回值类型为 int
 *             - 参数类型为 wint_t
 *   负向测试：违反约束的代码应编译报错（见 #if 0 块）。
 *
 * 注意：本条款只规定 iswlower 的语义，不涉及 UB 测试。
 */

#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <wctype.h> 提供 iswlower 声明，原型为 int iswlower(wint_t) */
static int (*fp_iswlower)(wint_t) = iswlower;   /* 验证函数指针类型匹配 */

int main(void)
{
    /* [1] 原型检查：返回类型为 int，参数为 wint_t */
    {
        wint_t wc = L'a';
        int r = iswlower(wc);
        (void)r;
        /* 通过函数指针调用，确认签名 */
        assert(fp_iswlower(L'a') != 0);
    }

    /* [2] 小写字母：'a'..'z' 应返回非零 */
    {
        for (wint_t c = L'a'; c <= L'z'; ++c) {
            assert(iswlower(c) != 0);
        }
    }

    /* [2] 大写字母：'A'..'Z' 应返回 0 */
    {
        for (wint_t c = L'A'; c <= L'Z'; ++c) {
            assert(iswlower(c) == 0);
        }
    }

    /* [2] 数字：'0'..'9' 应返回 0（iswdigit 为真，故不属于小写字母集合） */
    {
        for (wint_t c = L'0'; c <= L'9'; ++c) {
            assert(iswlower(c) == 0);
        }
    }

    /* [2] 标点：典型标点应返回 0（iswpunct 为真） */
    {
        assert(iswlower(L'!') == 0);
        assert(iswlower(L'.') == 0);
        assert(iswlower(L',') == 0);
        assert(iswlower(L';') == 0);
        assert(iswlower(L'?') == 0);
    }

    /* [2] 空白：空格、制表、换行应返回 0（iswspace 为真） */
    {
        assert(iswlower(L' ')  == 0);
        assert(iswlower(L'\t') == 0);
        assert(iswlower(L'\n') == 0);
        assert(iswlower(L'\r') == 0);
        assert(iswlower(L'\f') == 0);
        assert(iswlower(L'\v') == 0);
    }

    /* [2] 控制字符：应返回 0（iswcntrl 为真） */
    {
        assert(iswlower(L'\0') == 0);
        assert(iswlower(L'\a') == 0);
        assert(iswlower(L'\b') == 0);
        assert(iswlower((wint_t)0x1B) == 0); /* ESC */
    }

    /* [2] 非字母宽字符（如中文字符）在 "C" locale 下通常返回 0；
     *     条款允许 locale-specific 集合，因此这里只验证返回值是 int 且
     *     不崩溃，不强制断言具体值。 */
    {
        wint_t wc = (wint_t)0x4E2D; /* '中' */
        int r = iswlower(wc);
        (void)r; /* 结果依赖 locale，不做断言 */
    }

    /* [2] 边界：WEOF 应返回 0（不是小写字母） */
    {
        assert(iswlower(WEOF) == 0);
    }

    /* [2] 返回值语义：非零表示真，零表示假；验证典型真值 */
    {
        assert(iswlower(L'a') != 0);
        assert(iswlower(L'z') != 0);
        assert(iswlower(L'A') == 0);
    }

    /* [2] 与 iswupper 互补性（在 C locale 下对 ASCII 字母成立） */
    {
        for (wint_t c = L'a'; c <= L'z'; ++c) {
            assert(iswlower(c) != 0);
            assert(iswupper(c) == 0);
        }
        for (wint_t c = L'A'; c <= L'Z'; ++c) {
            assert(iswlower(c) == 0);
            assert(iswupper(c) != 0);
        }
    }

    printf("iswlower: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「iswlower 的参数类型为 wint_t」：
 * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
struct S { int x; } s;
iswlower(s);

/* 违反约束「iswlower 的参数类型为 wint_t」：
 * 传入指针类型，gcc -std=c99 应报错。 */
int *p = 0;
iswlower(p);

/* 违反约束「iswlower 的参数类型为 wint_t」：
 * 传入 double，gcc -std=c99 应报错（不能隐式转换为 wint_t 的整型参数）。 */
iswlower(1.5);

/* 违反约束「iswlower 的返回类型为 int」：
 * 试图把返回值赋给结构体，gcc -std=c99 应报错。 */
struct T { int y; } t;
t = iswlower(L'a');

/* 违反约束「iswlower 是函数，不是对象」：
 * 对函数名取地址后解引用赋值，gcc -std=c99 应报错。 */
*iswlower = 0;

/* 违反约束「iswlower 需要 <wctype.h> 声明」：
 * 若未包含头文件，隐式声明在 C99 中为约束违反（gcc -std=c99 -Werror 报错）。 */
/* 注意：此处已在文件顶部包含 <wctype.h>，故本片段仅作示意。 */

#endif /* 负向测试结束 */