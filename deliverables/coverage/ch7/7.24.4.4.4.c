/*
 * 测试条款：C99 7.24.4.4.4  wcsxfrm 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>
 *   [2] 变换语义：wcscmp(变换后) 与 wcscoll(原始) 符号一致；
 *       最多写入 n 个宽字符（含结尾空宽字符）；n==0 时 s1 可为 NULL。
 *   [3] 返回值：变换后宽字符串长度（不含结尾空宽字符）；
 *       返回值 >= n 时 s1 内容不确定。
 *   [4] EXAMPLE：1 + wcsxfrm(NULL, s, 0) 为所需数组长度。
 */

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <locale.h>

int main(void)
{
    /* 使用 C locale，保证 wcscoll 行为确定、可移植 */
    setlocale(LC_ALL, "C");

    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：正确声明与调用（编译通过即验证原型） */
    {
        wchar_t buf[64];
        const wchar_t *src = L"hello";
        size_t r = wcsxfrm(buf, src, 64);
        assert(r == wcslen(src));          /* [3] 返回变换后长度 */
        assert(wcscmp(buf, src) == 0);     /* C locale 下变换即自身 */
    }

    /* [2] 变换语义：wcscmp(变换后) 与 wcscoll(原始) 符号一致 */
    {
        const wchar_t *a = L"apple";
        const wchar_t *b = L"banana";
        wchar_t ta[64], tb[64];
        size_t ra = wcsxfrm(ta, a, 64);
        size_t rb = wcsxfrm(tb, b, 64);
        (void)ra; (void)rb;

        int cmp_transformed = wcscmp(ta, tb);
        int cmp_original    = wcscoll(a, b);

        /* 符号必须一致：>0 / ==0 / <0 对应相同 */
        assert((cmp_transformed > 0) == (cmp_original > 0));
        assert((cmp_transformed == 0) == (cmp_original == 0));
        assert((cmp_transformed < 0) == (cmp_original < 0));
    }

    /* [2] 相等字符串：变换后 wcscmp 返回 0，wcscoll 也返回 0 */
    {
        const wchar_t *a = L"same";
        const wchar_t *b = L"same";
        wchar_t ta[64], tb[64];
        wcsxfrm(ta, a, 64);
        wcsxfrm(tb, b, 64);
        assert(wcscmp(ta, tb) == 0);
        assert(wcscoll(a, b) == 0);
    }

    /* [2] 最多写入 n 个宽字符（含结尾空宽字符）：
     *     当 n 足够大时，写入完整变换串 + 结尾空宽字符。 */
    {
        const wchar_t *src = L"abc";
        wchar_t buf[8];
        size_t r = wcsxfrm(buf, src, 8);
        assert(r == 3);                    /* 长度不含结尾空宽字符 */
        assert(buf[0] == L'a');
        assert(buf[1] == L'b');
        assert(buf[2] == L'c');
        assert(buf[3] == L'\0');           /* 结尾空宽字符已写入 */
    }

    /* [2] n 恰好等于所需长度（含结尾空宽字符）时，可完整写入 */
    {
        const wchar_t *src = L"xyz";       /* 长度 3，需 4 个宽字符 */
        wchar_t buf[4];
        size_t r = wcsxfrm(buf, src, 4);
        assert(r == 3);
        assert(wcscmp(buf, src) == 0);
    }

    /* [2] n 小于所需长度：只写入 n 个宽字符，返回值仍为完整长度 */
    {
        const wchar_t *src = L"abcdef";    /* 长度 6，需 7 个宽字符 */
        wchar_t buf[4];
        memset(buf, 0x7f, sizeof buf);     /* 预填充，便于观察 */
        size_t r = wcsxfrm(buf, src, 4);
        assert(r == 6);                    /* [3] 返回完整长度 */
        /* 返回值 >= n，s1 内容不确定，故不检查 buf 内容 */
    }

    /* [2] n == 0 时 s1 允许为 NULL 指针 */
    {
        const wchar_t *src = L"probe";
        size_t r = wcsxfrm(NULL, src, 0);  /* 不得崩溃 */
        assert(r == wcslen(src));          /* 返回所需长度 */
    }

    /* [3] 返回值 >= n 时 s1 内容不确定：仅验证返回值语义 */
    {
        const wchar_t *src = L"longer-than-buffer";
        size_t need = wcslen(src);
        wchar_t small[2];
        size_t r = wcsxfrm(small, src, 2);
        assert(r == need);
        assert(r >= 2);                    /* 触发“内容不确定”条件 */
    }

    /* [4] EXAMPLE：1 + wcsxfrm(NULL, s, 0) 为所需数组长度 */
    {
        const wchar_t *s = L"example";
        size_t needed = 1 + wcsxfrm(NULL, s, 0);
        assert(needed == wcslen(s) + 1);

        wchar_t *buf = (wchar_t *)malloc(needed * sizeof(wchar_t));
        assert(buf != NULL);
        size_t r = wcsxfrm(buf, s, needed);
        assert(r == wcslen(s));
        assert(wcscmp(buf, s) == 0);
        free(buf);
    }

    /* [4] EXAMPLE 与 [2] 结合：用 EXAMPLE 计算的长度做完整变换，
     *     再与 wcscoll 的符号一致性交叉验证 */
    {
        const wchar_t *p = L"aaa";
        const wchar_t *q = L"aab";
        size_t np = 1 + wcsxfrm(NULL, p, 0);
        size_t nq = 1 + wcsxfrm(NULL, q, 0);
        wchar_t *tp = (wchar_t *)malloc(np * sizeof(wchar_t));
        wchar_t *tq = (wchar_t *)malloc(nq * sizeof(wchar_t));
        assert(tp && tq);
        wcsxfrm(tp, p, np);
        wcsxfrm(tq, q, nq);
        int ct = wcscmp(tp, tq);
        int co = wcscoll(p, q);
        assert((ct > 0) == (co > 0));
        assert((ct == 0) == (co == 0));
        assert((ct < 0) == (co < 0));
        free(tp);
        free(tq);
    }

    printf("All positive tests for C99 7.24.4.4.4 wcsxfrm passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「wcsxfrm 的第一个参数类型为 wchar_t * restrict」：
     * 传入 const wchar_t * 会丢弃 const 限定，gcc -std=c99 应报错
     * （discards qualifiers / assignment of read-only ...）。
     */
    {
        const wchar_t *dst = L"readonly";
        const wchar_t *src = L"src";
        wcsxfrm(dst, src, 8);   /* 错误：dst 为 const wchar_t * */
    }

    /*
     * 违反约束「wcsxfrm 的第二个参数类型为 const wchar_t * restrict」：
     * 传入 int * 类型不兼容，gcc -std=c99 应报错
     * （incompatible pointer type）。
     */
    {
        wchar_t buf[8];
        int *bad = 0;
        wcsxfrm(buf, bad, 8);   /* 错误：第二参数类型不兼容 */
    }

    /*
     * 违反约束「wcsxfrm 的第三个参数类型为 size_t」：
     * 传入指针类型不兼容，gcc -std=c99 应报错
     * （incompatible type for argument 3）。
     */
    {
        wchar_t buf[8];
        const wchar_t *src = L"x";
        wcsxfrm(buf, src, (void *)0);  /* 错误：第三参数应为 size_t */
    }

    /*
     * 违反约束「wcsxfrm 返回 size_t，调用需在 <wchar.h> 声明下进行」：
     * 未包含 <wchar.h> 时调用，C99 下隐式声明被禁止，
     * gcc -std=c99 应报错（implicit declaration of function）。
     * 此处以注释形式说明，实际测试需单独编译单元。
     */
    /* wcsxfrm(NULL, L"x", 0);  // 无 <wchar.h> 声明时：implicit declaration */

    /*
     * 违反约束「wcsxfrm 的返回类型为 size_t，不可赋给不兼容类型」：
     * 将返回值赋给结构体类型，gcc -std=c99 应报错
     * （incompatible types in assignment）。
     */
    {
        struct S { int x; } s;
        const wchar_t *src = L"y";
        s = wcsxfrm(NULL, src, 0);  /* 错误：size_t 不能赋给 struct S */
    }
#endif

    return 0;
}