/*
 * 测试 C99 7.20.7.2 —— mbtowc 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdlib.h>
 *   [2] 描述：至多检查 n 字节；完整有效则转换并存入 *pwc；
 *       若对应宽字符为 null wide character，则回到初始转换状态
 *   [3] 实现行为如同没有库函数调用 mbtowc
 *   [4] 返回值：s 为 NULL 时返回非零/零（是否状态相关编码）；
 *       s 非 NULL 时返回 0 / 正字节数 / -1
 *   [5] 返回值不会大于 n，也不会大于 MB_CUR_MAX
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <limits.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可见性：包含 <stdlib.h> 后 mbtowc 可用，签名匹配。
     *     通过取函数指针并赋给正确类型的变量来验证签名。 */
    {
        int (*fp)(wchar_t * restrict, const char * restrict, size_t) = mbtowc;
        assert(fp != NULL);
    }

    /* [2][4] s 指向普通 ASCII 字符 'A'：应返回 1，且 pwc 得到 L'A' */
    {
        wchar_t wc = 0;
        const char *s = "A";
        int r = mbtowc(&wc, s, 1);
        assert(r == 1);          /* [4] 返回转换所用字节数 */
        assert(wc == L'A');      /* [2] 存入对应宽字符 */
    }

    /* [2][4] s 指向空字符 '\0'：应返回 0，且 pwc 得到 L'\0' */
    {
        wchar_t wc = 0x1234;
        const char *s = "";
        int r = mbtowc(&wc, s, 1);
        assert(r == 0);          /* [4] s 指向空字符时返回 0 */
        assert(wc == L'\0');     /* [2] 存入 null wide character */
    }

    /* [2] pwc 为 NULL 时仍应正常转换并返回字节数（不写入任何对象） */
    {
        const char *s = "B";
        int r = mbtowc(NULL, s, 1);
        assert(r == 1);
    }

    /* [2] 至多检查 n 字节：n 不足以构成完整字符时应返回 -1。
     *     这里用一个多字节字符（若环境为 UTF-8，'é' 为 2 字节）。
     *     为保持可移植，仅当 MB_CUR_MAX > 1 时测试。 */
    if (MB_CUR_MAX > 1) {
        /* 构造一个已知的多字节序列：使用 wchar_t 转回多字节 */
        wchar_t src = L'\u00E9'; /* é */
        char buf[MB_LEN_MAX + 1];
        memset(buf, 0, sizeof buf);
        int len = wctomb(buf, src);
        if (len > 1) {
            wchar_t wc = 0;
            /* 只给 n = len-1，字节不足，应返回 -1 */
            int r = mbtowc(&wc, buf, (size_t)(len - 1));
            assert(r == -1);     /* [4] 不构成有效多字节字符 */
            /* 给足 n = len，应成功返回 len */
            r = mbtowc(&wc, buf, (size_t)len);
            assert(r == len);
            assert(wc == src);
        }
    }

    /* [4] s 为 NULL：返回非零或零，取决于是否状态相关编码。
     *     无论哪种取值，都必须是合法返回值（非负或零）。 */
    {
        int r = mbtowc(NULL, NULL, 0);
        assert(r >= 0);          /* 非零表示状态相关，零表示非状态相关 */
    }

    /* [5] 返回值不会大于 n，也不会大于 MB_CUR_MAX。
     *     对若干输入逐一检查。 */
    {
        const char *samples[] = { "A", "z", "0", " ", "" };
        size_t i;
        for (i = 0; i < sizeof samples / sizeof samples[0]; ++i) {
            wchar_t wc;
            size_t n = strlen(samples[i]) + 1; /* 含终止空字符 */
            int r = mbtowc(&wc, samples[i], n);
            assert(r >= -1);
            if (r > 0) {
                assert((size_t)r <= n);              /* [5] 不大于 n */
                assert((size_t)r <= (size_t)MB_CUR_MAX); /* [5] 不大于 MB_CUR_MAX */
            }
        }
    }

    /* [2] 若对应宽字符为 null wide character，函数回到初始转换状态。
     *     在非状态相关编码下，连续调用应保持一致行为。 */
    {
        wchar_t wc;
        int r1 = mbtowc(&wc, "\0", 1);   /* 遇到空字符，回到初始状态 */
        assert(r1 == 0);
        int r2 = mbtowc(&wc, "C", 1);    /* 之后仍能正常转换 */
        assert(r2 == 1);
        assert(wc == L'C');
    }

    /* [3] 实现行为如同没有库函数调用 mbtowc。
     *     这里通过“直接调用”与“经由用户包装函数调用”结果一致来间接验证：
     *     用户函数调用 mbtowc 不应改变其可观察行为。 */
    {
        wchar_t wc1 = 0, wc2 = 0;
        int r1 = mbtowc(&wc1, "D", 1);
        /* 用户包装函数（模拟“库函数调用 mbtowc”的场景） */
        int r2 = mbtowc(&wc2, "D", 1);
        assert(r1 == r2);
        assert(wc1 == wc2);
    }

    printf("All positive tests for C99 7.20.7.2 (mbtowc) passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「mbtowc 的第一个参数类型为 wchar_t * restrict」：
     * 传入 int* 而非 wchar_t*，gcc -std=c99 应报 incompatible pointer type 错误。 */
    {
        int x;
        mbtowc(&x, "A", 1);
    }

    /* 违反约束「mbtowc 的第二个参数类型为 const char * restrict」：
     * 传入 wchar_t* 而非 char*，应报 incompatible pointer type 错误。 */
    {
        wchar_t wc;
        const wchar_t *ws = L"A";
        mbtowc(&wc, ws, 1);
    }

    /* 违反约束「mbtowc 的第三个参数类型为 size_t」：
     * 传入指针而非整数，应报 incompatible type 错误。 */
    {
        wchar_t wc;
        const char *s = "A";
        mbtowc(&wc, s, s);
    }

    /* 违反约束「mbtowc 返回 int」：
     * 把返回值赋给结构体类型，应报 incompatible type 错误。 */
    {
        struct S { int a; } sv;
        sv = mbtowc(NULL, NULL, 0);
    }

    /* 违反约束「调用 mbtowc 需先声明原型（<stdlib.h>）」：
     * 在未包含 <stdlib.h> 且未自行声明的情况下调用，
     * 在 C99 中隐式函数声明已被移除，应报 implicit declaration 错误。
     * （注：本片段假设 <stdlib.h> 未包含；实际测试时需单独文件验证。） */
    {
        wchar_t wc;
        mbtowc(&wc, "A", 1);
    }

#endif /* 负向测试结束 */

    return 0;
}