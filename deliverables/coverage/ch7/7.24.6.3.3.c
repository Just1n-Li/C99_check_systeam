/*
 * 测试目标：C99 7.24.6.3.3  wcrtomb 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <wchar.h>
 *   [2] s 为 NULL 时等价于 wcrtomb(buf, L'\0', ps)
 *   [3] s 非 NULL 时：写入多字节表示，最多 MB_CUR_MAX 字节；
 *       wc 为 L'\0' 时写入空字节并回到初始转换状态
 *   [4] 返回值：写入的字节数；wc 非法时置 errno=EILSEQ 并返回 (size_t)(-1)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型一致 */
static size_t (*fp_wcrtomb)(char *restrict, wchar_t, mbstate_t *restrict) = wcrtomb;

int main(void)
{
    /* 使用 C locale 以保证行为可预测（ASCII 范围内） */
    if (setlocale(LC_ALL, "C") == NULL) {
        fprintf(stderr, "setlocale failed\n");
        return 1;
    }

    /* [1] 原型可用性：能取地址、能通过函数指针调用 */
    assert(fp_wcrtomb != NULL);

    /* ---------- [3] s 非 NULL：普通宽字符 ---------- */
    {
        char buf[MB_LEN_MAX + 8];
        mbstate_t st;
        size_t n;

        memset(&st, 0, sizeof st);
        memset(buf, 0x7f, sizeof buf);

        /* 'A' 在 C locale 下为单字节 0x41 */
        n = wcrtomb(buf, L'A', &st);
        assert(n == 1);
        assert(buf[0] == 'A');

        /* [3] 最多写入 MB_CUR_MAX 字节：返回值不得超过 MB_CUR_MAX */
        assert(n <= (size_t)MB_CUR_MAX);
    }

    /* ---------- [3] wc 为 L'\0'：写入空字节，回到初始转换状态 ---------- */
    {
        char buf[MB_LEN_MAX + 8];
        mbstate_t st;
        size_t n;

        memset(&st, 0, sizeof st);
        memset(buf, 0x7f, sizeof buf);

        n = wcrtomb(buf, L'\0', &st);
        assert(n >= 1);
        assert(buf[n - 1] == '\0');   /* 末尾必须是空字节 */

        /* [3] 结果状态为初始转换状态：可用 mbsinit 验证 */
        assert(mbsinit(&st) != 0);
    }

    /* ---------- [2] s 为 NULL：等价于 wcrtomb(buf, L'\0', ps) ---------- */
    {
        mbstate_t st1, st2;
        size_t n_null, n_zero;
        char buf[MB_LEN_MAX + 8];

        memset(&st1, 0, sizeof st1);
        memset(&st2, 0, sizeof st2);

        /* 先让 st1 处于初始状态，再调用 s==NULL 形式 */
        n_null = wcrtomb(NULL, L'x', &st1);   /* wc 参数被忽略，等价于写 L'\0' */
        assert(n_null >= 1);

        /* 与显式 wcrtomb(buf, L'\0', ps) 比较 */
        memset(buf, 0x7f, sizeof buf);
        n_zero = wcrtomb(buf, L'\0', &st2);
        assert(n_zero == n_null);
        assert(buf[n_zero - 1] == '\0');

        /* 两者都应回到初始转换状态 */
        assert(mbsinit(&st1) != 0);
        assert(mbsinit(&st2) != 0);
    }

    /* ---------- [2] s 为 NULL 且 ps 为 NULL：使用内部静态状态 ---------- */
    {
        size_t n = wcrtomb(NULL, L'\0', NULL);
        assert(n >= 1);
    }

    /* ---------- [3] 连续转换：状态在多次调用间保持 ---------- */
    {
        char buf[MB_LEN_MAX + 8];
        mbstate_t st;
        size_t n1, n2;

        memset(&st, 0, sizeof st);

        n1 = wcrtomb(buf, L'H', &st);
        assert(n1 == 1 && buf[0] == 'H');

        n2 = wcrtomb(buf, L'i', &st);
        assert(n2 == 1 && buf[0] == 'i');

        /* 结束：写空字符 */
        n2 = wcrtomb(buf, L'\0', &st);
        assert(n2 >= 1 && buf[n2 - 1] == '\0');
        assert(mbsinit(&st) != 0);
    }

    /* ---------- [4] 非法宽字符：errno=EILSEQ，返回 (size_t)(-1) ---------- */
    {
        char buf[MB_LEN_MAX + 8];
        mbstate_t st;
        size_t n;

        memset(&st, 0, sizeof st);
        memset(buf, 0x7f, sizeof buf);

        errno = 0;
        /* 在 C locale 下，0x110000 超出 Unicode 范围，通常为非法宽字符。
         * 若实现接受它，则跳过该断言（避免依赖具体实现）。 */
        n = wcrtomb(buf, (wchar_t)0x110000, &st);
        if (n == (size_t)(-1)) {
            /* [4] 编码错误：errno 必须为 EILSEQ */
            assert(errno == EILSEQ);
        } else {
            /* 实现接受该字符：返回值仍须合法 */
            assert(n <= (size_t)MB_CUR_MAX);
        }
    }

    /* ---------- [4] 返回值语义：等于实际写入的字节数 ---------- */
    {
        char buf[MB_LEN_MAX + 8];
        mbstate_t st;
        size_t n;

        memset(&st, 0, sizeof st);
        memset(buf, 0x7f, sizeof buf);

        n = wcrtomb(buf, L'Z', &st);
        assert(n == 1);
        assert(buf[0] == 'Z');
        /* 第 n 个字节之后不应被本函数写入（此处仅检查前 n 字节有效） */
        assert(buf[n - 1] == 'Z');
    }

    printf("All positive tests for C99 7.24.6.3.3 wcrtomb passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcrtomb 的第一个参数类型为 char * restrict」：
 * 传入 const char * 会丢弃 const 限定，gcc -std=c99 应报错
 * （discards qualifiers / incompatible pointer type）。 */
{
    const char cbuf[16];
    mbstate_t st;
    wcrtomb(cbuf, L'A', &st);   /* 期望：编译错误 */
}

/* 违反约束「wcrtomb 的第二个参数类型为 wchar_t」：
 * 传入指针类型不匹配，gcc -std=c99 应报错。 */
{
    char buf[16];
    mbstate_t st;
    wcrtomb(buf, (wchar_t *)0, &st);   /* 期望：编译错误 */
}

/* 违反约束「wcrtomb 的第三个参数类型为 mbstate_t * restrict」：
 * 传入 int * 类型不兼容，gcc -std=c99 应报错。 */
{
    char buf[16];
    int x;
    wcrtomb(buf, L'A', &x);   /* 期望：编译错误 */
}

/* 违反约束「wcrtomb 返回 size_t，不能作为左值赋值」：
 * 函数调用结果不是左值，赋值应报错。 */
{
    char buf[16];
    mbstate_t st;
    wcrtomb(buf, L'A', &st) = 0;   /* 期望：编译错误（lvalue required） */
}

/* 违反约束「wcrtomb 参数个数必须为 3」：
 * 参数过少，gcc -std=c99 应报错。 */
{
    char buf[16];
    wcrtomb(buf, L'A');   /* 期望：编译错误（too few arguments） */
}

/* 违反约束「wcrtomb 参数个数必须为 3」：
 * 参数过多，gcc -std=c99 应报错。 */
{
    char buf[16];
    mbstate_t st;
    wcrtomb(buf, L'A', &st, 0);   /* 期望：编译错误（too many arguments） */
}

#endif /* 负向测试结束 */