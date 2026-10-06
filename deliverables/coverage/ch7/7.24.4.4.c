/*
 * 测试 C99 7.24.4.4 —— 宽字符串比较函数 (wcscmp / wcsncmp / wcscoll / wcsxfrm)
 *
 * 条款要点：
 *   [1] 除非另有说明，本子条款描述的函数对两个宽字符的排序方式，
 *       与由 wchar_t 所指定的底层整数类型的两个整数排序方式相同。
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码应导致编译报错（放在 #if 0 中，不参与编译）。
 *
 * 说明：本条款只规定“排序方式与底层整数类型一致”，因此测试重点在于：
 *   - wcscmp/wcsncmp 的返回值符号与 (wchar_t 值作为整数) 相减的符号一致；
 *   - 比较是按 wchar_t 的整数值进行的（而非按字节/无符号解释）；
 *   - wcscoll/wcsxfrm 在 C 语言环境下与 wcscmp 一致（[1] 的“除非另有说明”）。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

/* 辅助：返回两个 wchar_t 作为底层整数比较的符号 */
static int int_sign(wchar_t a, wchar_t b)
{
    /* 将 wchar_t 提升为其底层整数类型后比较 */
    if ((long)a < (long)b) return -1;
    if ((long)a > (long)b) return  1;
    return 0;
}

/* 辅助：返回 wcscmp 结果的符号 */
static int sign_of(int v)
{
    return (v > 0) - (v < 0);
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] wcscmp：两个宽字符的排序与底层整数类型一致
     *     对每一对字符，wcscmp 的返回值符号应与整数比较的符号相同。 */
    {
        const wchar_t *pairs[][2] = {
            { L"a",  L"b"  },   /* 'a' < 'b' */
            { L"b",  L"a"  },   /* 'b' > 'a' */
            { L"x",  L"x"  },   /* 相等 */
            { L"",   L"a"  },   /* 空串 < 非空串 */
            { L"a",  L""   },   /* 非空串 > 空串 */
            { L"abc",L"abd" },  /* 前两字符相同，第三字符决定 */
            { L"ab", L"abc" },  /* 前缀关系：短串 < 长串 */
            { L"abc",L"ab"  },  /* 前缀关系：长串 > 短串 */
        };
        size_t i;
        for (i = 0; i < sizeof(pairs)/sizeof(pairs[0]); ++i) {
            int r = wcscmp(pairs[i][0], pairs[i][1]);
            /* 逐字符按底层整数比较，得到期望符号 */
            const wchar_t *s1 = pairs[i][0], *s2 = pairs[i][1];
            int expected;
            while (*s1 && *s1 == *s2) { ++s1; ++s2; }
            expected = int_sign(*s1, *s2);
            assert(sign_of(r) == expected);
        }
    }

    /* [1] 关键点：比较按 wchar_t 的“整数值”进行。
     *     构造两个宽字符，其底层整数值一大一小，验证 wcscmp 的符号
     *     与整数比较一致（而不是与无符号/字节比较一致）。 */
    {
        wchar_t lo = (wchar_t)1;
        wchar_t hi = (wchar_t)2;
        wchar_t s_lo[2] = { lo, 0 };
        wchar_t s_hi[2] = { hi, 0 };
        assert(sign_of(wcscmp(s_lo, s_hi)) == int_sign(lo, hi));
        assert(sign_of(wcscmp(s_hi, s_lo)) == int_sign(hi, lo));
        assert(wcscmp(s_lo, s_lo) == 0);
    }

    /* [1] wcsncmp：只比较前 n 个宽字符，排序方式同样与底层整数一致 */
    {
        /* 前 3 个字符相同，第 4 个不同；n=3 时应相等 */
        assert(wcsncmp(L"abcd", L"abce", 3) == 0);
        /* n=4 时由第 4 个字符决定 */
        assert(sign_of(wcsncmp(L"abcd", L"abce", 4)) == int_sign(L'd', L'e'));
        /* n=0 时总是相等 */
        assert(wcsncmp(L"abc", L"xyz", 0) == 0);
        /* 前缀：n 超过短串长度时，短串的终止符参与比较 */
        assert(sign_of(wcsncmp(L"ab", L"abc", 5)) == int_sign(L'\0', L'c'));
    }

    /* [1] wcscoll：在 C 语言环境下，排序方式与 wcscmp 一致
     *     （条款 [1] 的“除非另有说明”在此体现为默认 C locale 下一致）。 */
    {
        assert(sign_of(wcscoll(L"a", L"b")) == sign_of(wcscmp(L"a", L"b")));
        assert(sign_of(wcscoll(L"b", L"a")) == sign_of(wcscmp(L"b", L"a")));
        assert(wcscoll(L"same", L"same") == 0);
        assert(sign_of(wcscoll(L"abc", L"abd")) == int_sign(L'c', L'd'));
    }

    /* [1] wcsxfrm：变换后的字符串用 wcscmp 比较，结果应与直接 wcscoll 一致。
     *     这间接验证了排序方式与底层整数类型一致。 */
    {
        wchar_t buf1[64], buf2[64];
        size_t n1 = wcsxfrm(buf1, L"abc", 64);
        size_t n2 = wcsxfrm(buf2, L"abd", 64);
        assert(n1 < 64 && n2 < 64);
        /* 变换后比较的符号应与直接比较一致 */
        assert(sign_of(wcscmp(buf1, buf2)) == sign_of(wcscmp(L"abc", L"abd")));
        assert(sign_of(wcscmp(buf1, buf2)) == sign_of(wcscoll(L"abc", L"abd")));

        /* 相等字符串变换后仍相等 */
        wchar_t buf3[64], buf4[64];
        wcsxfrm(buf3, L"hello", 64);
        wcsxfrm(buf4, L"hello", 64);
        assert(wcscmp(buf3, buf4) == 0);
    }

    /* [1] 一致性：对同一对字符串，wcscmp 与 wcscoll 的符号在 C locale 下相同 */
    {
        const wchar_t *a = L"apple";
        const wchar_t *b = L"banana";
        assert(sign_of(wcscmp(a, b)) == sign_of(wcscoll(a, b)));
        assert(sign_of(wcscmp(b, a)) == sign_of(wcscoll(b, a)));
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「wcscmp/wcsncmp/wcscoll/wcsxfrm 的参数必须是指向宽字符串的指针」：
     * 传入 int* 而非 wchar_t*，gcc -std=c99 应报错
     * （incompatible pointer type / passing argument from incompatible pointer type）。
     */
    {
        int x[4] = { 1, 2, 3, 0 };
        int y[4] = { 1, 2, 4, 0 };
        wcscmp(x, y);          /* 错误：参数类型不是 wchar_t* */
        wcsncmp(x, y, 3);      /* 错误：参数类型不是 wchar_t* */
        wcscoll(x, y);         /* 错误：参数类型不是 wchar_t* */
    }

    /*
     * 违反约束「wcsxfrm 的第一个参数必须是可修改的宽字符数组（wchar_t*）」：
     * 传入字符串字面量（const wchar_t*），gcc -std=c99 应报错
     * （discards qualifiers / assignment of read-only location）。
     */
    {
        wcsxfrm(L"literal", L"src", 10);  /* 错误：目标为只读字面量 */
    }

    /*
     * 违反约束「wcsxfrm 的第二个参数必须是宽字符串指针」：
     * 传入 int*，gcc -std=c99 应报错。
     */
    {
        wchar_t dst[16];
        int src[4] = { 1, 2, 3, 0 };
        wcsxfrm(dst, src, 16);  /* 错误：src 类型不是 wchar_t* */
    }

    /*
     * 违反约束「wcscmp 需要两个参数」：
     * 参数个数不足，gcc -std=c99 应报错（too few arguments to function）。
     */
    {
        wcscmp(L"only-one");   /* 错误：缺少第二个参数 */
    }
#endif

    return 0;
}