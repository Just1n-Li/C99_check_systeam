/*
 * 测试 C99 7.20.7.3 —— wctomb 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 声明：int wctomb(char *s, wchar_t wc);  需要 <stdlib.h>
 *   [2] 描述：确定所需字节数并存入 s；最多 MB_CUR_MAX 字节；
 *       wc 为 L'\0' 时存入空字节并恢复初始转换状态。
 *   [3] 实现行为如同没有库函数调用 wctomb（可重入/无隐藏状态依赖）。
 *   [4] 返回值：s==NULL 时返回非零/零表示是否有状态相关编码；
 *       s!=NULL 时返回 -1（无效）或字节数。
 *   [5] 返回值绝不超过 MB_CUR_MAX。
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 函数原型存在且签名正确：int wctomb(char *, wchar_t) */
static int (*wctomb_proto_check)(char *, wchar_t) = wctomb;

int main(void)
{
    /* [1] 通过函数指针调用，验证原型可用 */
    assert(wctomb_proto_check != NULL);

    /* [4] s == NULL 时：返回非零或零，表示是否有状态相关编码。
     *     无论哪种取值都合法，只需验证调用不崩溃且返回值为 int。 */
    {
        int r = wctomb(NULL, L'A');
        /* 合法取值：0（无状态相关编码）或非零（有状态相关编码） */
        (void)r;
        /* 再次调用应得到一致结果（无隐藏状态被破坏） */
        assert(wctomb(NULL, L'A') == r);
    }

    /* [2][4][5] s != NULL 且 wc 为普通宽字符：返回字节数，且 <= MB_CUR_MAX */
    {
        char buf[MB_CUR_MAX + 8];
        memset(buf, 0x7f, sizeof buf);

        int n = wctomb(buf, L'A');
        assert(n >= 1);                 /* 'A' 至少 1 字节 */
        assert(n <= (int)MB_CUR_MAX);   /* [5] 不超过 MB_CUR_MAX */

        /* 存入的字节应能通过 mbtowc 还原为 L'A'（同一转换状态） */
        wchar_t back = 0;
        int m = mbtowc(&back, buf, n);
        assert(m == n);
        assert(back == L'A');
    }

    /* [2][4] wc 为 L'\0'：存入空字节，返回字节数（通常为 1），
     *        并恢复初始转换状态。 */
    {
        char buf[MB_CUR_MAX + 8];
        memset(buf, 0x7f, sizeof buf);

        int n = wctomb(buf, L'\0');
        assert(n >= 1);
        assert(n <= (int)MB_CUR_MAX);   /* [5] */
        /* 最后一个字节必须是空字节（空宽字符的表示） */
        assert(buf[n - 1] == '\0');

        /* [2] 调用后函数处于初始转换状态：紧接着转换一个普通字符应成功 */
        int n2 = wctomb(buf, L'A');
        assert(n2 >= 1 && n2 <= (int)MB_CUR_MAX);
    }

    /* [2][5] 遍历若干宽字符，验证返回值范围与 MB_CUR_MAX 约束 */
    {
        char buf[MB_CUR_MAX + 8];
        wchar_t samples[] = { L'A', L'z', L'0', L' ', L'\t', L'\n', L'\0' };
        size_t i;
        for (i = 0; i < sizeof samples / sizeof samples[0]; ++i) {
            int n = wctomb(buf, samples[i]);
            /* 对基本字符集成员，应能成功转换 */
            assert(n >= 1);
            assert(n <= (int)MB_CUR_MAX);   /* [5] */
        }
    }

    /* [4] 无效宽字符：返回 -1（若实现支持该宽字符则跳过此检查）。
     *     使用一个极不可能对应有效多字节字符的宽值。 */
    {
        char buf[MB_CUR_MAX + 8];
        int n = wctomb(buf, (wchar_t)0x7fffffff);
        /* 合法结果：-1（无效）或 >=1 且 <= MB_CUR_MAX（实现恰好支持） */
        assert(n == -1 || (n >= 1 && n <= (int)MB_CUR_MAX));
    }

    /* [3] 实现行为如同没有库函数调用 wctomb：
     *     连续多次调用之间不依赖隐藏的库内部状态；
     *     相同输入应产生相同输出。 */
    {
        char b1[MB_CUR_MAX + 8], b2[MB_CUR_MAX + 8];
        int n1 = wctomb(b1, L'Q');
        int n2 = wctomb(b2, L'Q');
        assert(n1 == n2);
        assert(n1 >= 1);
        assert(memcmp(b1, b2, (size_t)n1) == 0);
    }

    /* [2] 最多 MB_CUR_MAX 个字符被存储：缓冲区足够大时不会越界。
     *     这里用带哨兵字节的缓冲区验证写入范围。 */
    {
        unsigned char buf[MB_CUR_MAX + 4];
        memset(buf, 0xAA, sizeof buf);
        int n = wctomb((char *)buf, L'A');
        assert(n >= 1 && n <= (int)MB_CUR_MAX);
        /* 第 n 个字节之后（哨兵区）不应被写入 */
        assert(buf[MB_CUR_MAX + 1] == 0xAA);
        assert(buf[MB_CUR_MAX + 2] == 0xAA);
        assert(buf[MB_CUR_MAX + 3] == 0xAA);
    }

    printf("C99 7.20.7.3 wctomb: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wctomb 的第二个参数类型为 wchar_t」：
 * 传入不兼容的指针类型，gcc -std=c99 应报错（参数类型不匹配）。 */
{
    char buf[8];
    int *p = 0;
    wctomb(buf, p);   /* error: incompatible type for argument 2 */
}

/* 违反约束「wctomb 的第一个参数类型为 char *」：
 * 传入 int * 而非 char *，应报错。 */
{
    int ibuf[8];
    wctomb(ibuf, L'A');   /* error: incompatible pointer type */
}

/* 违反约束「wctomb 返回 int」：
 * 试图把返回值赋给不兼容的结构体类型，应报错。 */
{
    struct S { int x; } s;
    s = wctomb(NULL, L'A');   /* error: incompatible types in assignment */
}

/* 违反约束「wctomb 需要两个实参」：
 * 实参个数不足，应报错。 */
{
    wctomb(NULL);   /* error: too few arguments to function 'wctomb' */
}

/* 违反约束「wctomb 需要两个实参」：
 * 实参个数过多，应报错。 */
{
    char buf[8];
    wctomb(buf, L'A', 0);   /* error: too many arguments to function 'wctomb' */
}

/* 违反约束「wctomb 声明于 <stdlib.h>」：
 * 若未包含 <stdlib.h> 且无自身声明，调用未声明函数在 C99 中为约束违反。
 * （此处仅作说明；实际测试需在独立翻译单元中移除 #include <stdlib.h>。） */

#endif /* 负向测试结束 */