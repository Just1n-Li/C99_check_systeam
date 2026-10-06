/*
 * 测试 C99 7.25.2.1.2 —— iswalpha 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可正常编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int iswalpha(wint_t wc);  声明于 <wctype.h>
 *   [2] 语义：iswalpha(wc) 为真 当且仅当 iswupper(wc) 或 iswlower(wc) 为真，
 *       或者 wc 属于 locale 特定的字母宽字符集合，且该集合中
 *       iswcntrl/iswdigit/iswpunct/iswspace 均为假。
 *   [305] iswlower 与 iswupper 对这些附加宽字符可各自独立为真/假，
 *         四种组合都可能出现。
 */

#include <wctype.h>
#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <locale.h>

int main(void)
{
    /* 使用 C locale，行为可预测 */
    setlocale(LC_ALL, "C");

    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：iswalpha 接受 wint_t 并返回 int */
    {
        int (*fp)(wint_t) = iswalpha;   /* 函数指针类型匹配，验证原型 */
        assert(fp != NULL);
        assert(fp(L'A') != 0);
    }

    /* [2] 基本语义：ASCII 大写字母 -> iswupper 为真 -> iswalpha 为真 */
    {
        const wchar_t upper[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for (size_t i = 0; upper[i] != L'\0'; ++i) {
            assert(iswupper(upper[i]) != 0);      /* 前提 */
            assert(iswalpha(upper[i]) != 0);      /* [2] 应为真 */
        }
    }

    /* [2] 基本语义：ASCII 小写字母 -> iswlower 为真 -> iswalpha 为真 */
    {
        const wchar_t lower[] = L"abcdefghijklmnopqrstuvwxyz";
        for (size_t i = 0; lower[i] != L'\0'; ++i) {
            assert(iswlower(lower[i]) != 0);      /* 前提 */
            assert(iswalpha(lower[i]) != 0);      /* [2] 应为真 */
        }
    }

    /* [2] 非字母字符：数字、标点、空白、控制字符 -> iswalpha 为假 */
    {
        const wchar_t nonalpha[] = {
            L'0', L'1', L'9',          /* 数字 */
            L'!', L'@', L'#', L'.',    /* 标点 */
            L' ', L'\t', L'\n',        /* 空白 */
            L'\0'                      /* 控制字符（NUL） */
        };
        for (size_t i = 0; i < sizeof(nonalpha)/sizeof(nonalpha[0]); ++i) {
            wint_t wc = (wint_t)nonalpha[i];
            /* 这些字符不应同时满足 iswupper 或 iswlower */
            assert(iswupper(wc) == 0);
            assert(iswlower(wc) == 0);
            assert(iswalpha(wc) == 0);            /* [2] 应为假 */
        }
    }

    /* [2] 一致性：对任意宽字符，iswalpha 为真 => iswupper 或 iswlower 为真，
     *     或（locale 特定字母且非 cntrl/digit/punct/space）。
     *     在 C locale 下，字母集合即 ASCII 字母，故可做双向检查。 */
    {
        for (wint_t wc = 0; wc < 128; ++wc) {
            int a = iswalpha(wc) != 0;
            int u = iswupper(wc) != 0;
            int l = iswlower(wc) != 0;
            if (a) {
                /* 在 C locale 中，字母必为大写或小写 */
                assert(u || l);
            }
            if (u || l) {
                assert(a);
            }
        }
    }

    /* [2] 附加字母集合的排除条件：若某字符是 locale 特定字母，
     *     则 iswcntrl/iswdigit/iswpunct/iswspace 必须全为假。
     *     在 C locale 下，用 ASCII 全范围验证该蕴含关系。 */
    {
        for (wint_t wc = 0; wc < 128; ++wc) {
            if (iswalpha(wc) && !iswupper(wc) && !iswlower(wc)) {
                /* 属于 locale 特定附加字母 */
                assert(iswcntrl(wc) == 0);
                assert(iswdigit(wc) == 0);
                assert(iswpunct(wc) == 0);
                assert(iswspace(wc) == 0);
            }
        }
    }

    /* [305] iswlower 与 iswupper 对附加宽字符可独立取值，
     *       四种组合都可能。此处验证：对任意字符，
     *       iswalpha 为真时，iswupper/iswlower 的四种组合均不违反语义。 */
    {
        for (wint_t wc = 0; wc < 128; ++wc) {
            int u = iswupper(wc) != 0;
            int l = iswlower(wc) != 0;
            /* 四种组合 (u,l) = (0,0),(0,1),(1,0),(1,1) 均合法；
             * 只要 iswalpha 为真，语义 [2] 就允许这些组合。 */
            if (iswalpha(wc)) {
                /* 无额外约束，仅确认调用不崩溃且返回 0/1 语义 */
                assert(iswalpha(wc) == 1);
            }
        }
    }

    /* [2] 边界：WEOF 不是字母 */
    assert(iswalpha(WEOF) == 0);

    printf("iswalpha: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 违反约束「iswalpha 的参数类型为 wint_t」：
     * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
    struct S { int x; } s;
    iswalpha(s);

    /* 违反约束「iswalpha 返回 int，不可作为左值赋值」：
     * 对函数调用结果赋值，应报错（非左值）。 */
    iswalpha(L'A') = 1;

    /* 违反约束「iswalpha 需要 <wctype.h> 中的原型」：
     * 若未包含头文件而隐式声明，C99 下调用参数类型不匹配会报错；
     * 此处演示对返回 int 的函数取地址赋给错误类型指针。 */
    int (*bad)(double) = iswalpha;   /* 参数类型不兼容，应报错 */

    /* 违反约束「iswalpha 接受一个实参」：
     * 实参个数不匹配，应报错。 */
    iswalpha();

    /* 违反约束「iswalpha 接受一个实参」：
     * 实参过多，应报错。 */
    iswalpha(L'A', L'B');
#endif

    return 0;
}