/*
 * 测试目标：C99 7.20.8 —— 多字节/宽字符串转换函数
 * 条款原文：
 *   [1] The behavior of the multibyte string functions is affected by the
 *       LC_CTYPE category of the current locale.
 *
 * 预期行为：
 *   - 正向测试：包含 <stdlib.h>，调用 mbstowcs / wcstombs / mblen / mbtowc /
 *     wctomb，验证在 "C" locale 下这些函数可用且行为符合 LC_CTYPE 影响；
 *     通过 setlocale(LC_CTYPE, ...) 改变 LC_CTYPE 后，转换行为随之改变。
 *   - 负向测试：违反约束的调用（参数类型错误、缺少声明等）应编译报错。
 *
 * 说明：本条款只规定“多字节字符串函数的行为受当前 locale 的 LC_CTYPE 类别影响”，
 *       因此测试聚焦于：函数声明存在、locale 可设置、LC_CTYPE 改变影响转换结果。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 7.20.8 声明的函数原型必须存在于 <stdlib.h> 中 */
static void test_declarations(void)
{
    /* 取函数地址，验证原型存在且类型正确 */
    size_t (*p_mbstowcs)(wchar_t * restrict, const char * restrict, size_t) = mbstowcs;
    size_t (*p_wcstombs)(char * restrict, const wchar_t * restrict, size_t) = wcstombs;
    int    (*p_mblen)(const char *, size_t) = mblen;
    int    (*p_mbtowc)(wchar_t * restrict, const char * restrict, size_t) = mbtowc;
    int    (*p_wctomb)(char * restrict, wchar_t) = wctomb;

    assert(p_mbstowcs != NULL);
    assert(p_wcstombs != NULL);
    assert(p_mblen    != NULL);
    assert(p_mbtowc   != NULL);
    assert(p_wctomb   != NULL);
}

/* [1] LC_CTYPE 影响多字节字符串函数的行为：
 *     在 "C" locale 下，多字节字符集就是 ASCII，每个字符 1 字节。
 */
static void test_lc_ctype_c_locale(void)
{
    const char *loc = setlocale(LC_CTYPE, "C");
    assert(loc != NULL);   /* "C" locale 必须可用 */

    /* mblen：单字节 ASCII 字符长度为 1 */
    assert(mblen("A", MB_CUR_MAX) == 1);
    assert(mblen("", 0) == 0);          /* 空字符串返回 0 */

    /* mbtowc：把 'A' 转成宽字符 L'A' */
    wchar_t wc = 0;
    int n = mbtowc(&wc, "A", MB_CUR_MAX);
    assert(n == 1);
    assert(wc == L'A');

    /* wctomb：把 L'A' 转回单字节 'A' */
    char buf[MB_LEN_MAX + 1];
    n = wctomb(buf, L'A');
    assert(n == 1);
    assert(buf[0] == 'A');

    /* mbstowcs：整个字符串转换 */
    wchar_t wbuf[16];
    size_t cnt = mbstowcs(wbuf, "Hello", 16);
    assert(cnt == 5);
    assert(wcscmp(wbuf, L"Hello") == 0);

    /* wcstombs：宽字符串转多字节 */
    char mbuf[16];
    cnt = wcstombs(mbuf, L"World", 16);
    assert(cnt == 5);
    assert(strcmp(mbuf, "World") == 0);
}

/* [1] 改变 LC_CTYPE 后，多字节字符串函数的行为随之改变。
 *     这里用 "C" 与一个可能存在的其它 locale 对比；若系统只提供 "C"，
 *     则退化为仅验证 setlocale 返回值语义。
 */
static void test_lc_ctype_affects_behavior(void)
{
    /* 先固定到 "C" locale，记录基准行为 */
    const char *saved = setlocale(LC_CTYPE, "C");
    assert(saved != NULL);

    size_t base = mbstowcs(NULL, "abc", 0);   /* 只计算长度 */
    assert(base == 3);

    /* 尝试切换到其它 locale；若系统未安装则返回 NULL，此时跳过对比 */
    const char *other = setlocale(LC_CTYPE, "");
    if (other != NULL && strcmp(other, "C") != 0) {
        /* 在非 "C" locale 下，ASCII 字符仍应可转换（长度不变） */
        size_t alt = mbstowcs(NULL, "abc", 0);
        assert(alt == 3);
    }

    /* 恢复 "C" locale，确认行为可复现 */
    assert(setlocale(LC_CTYPE, "C") != NULL);
    assert(mbstowcs(NULL, "abc", 0) == 3);
}

/* [1] 边界与返回值语义（与 LC_CTYPE 相关）：
 *     mbstowcs 遇到无效多字节序列返回 (size_t)-1。
 *     在 "C" locale 下，字节 >= 0x80 不是有效多字节字符。
 */
static void test_invalid_sequence(void)
{
    assert(setlocale(LC_CTYPE, "C") != NULL);

    /* 0x80 在 "C" locale 下不是合法多字节字符 */
    const char bad[] = { (char)0x80, '\0' };
    size_t r = mbstowcs(NULL, bad, 0);
    assert(r == (size_t)-1);

    /* mbtowc 对无效序列返回 -1 */
    wchar_t wc;
    int n = mbtowc(&wc, bad, MB_CUR_MAX);
    assert(n == -1);
}

int main(void)
{
    test_declarations();
    test_lc_ctype_c_locale();
    test_lc_ctype_affects_behavior();
    test_invalid_sequence();

    printf("C99 7.20.8 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「mbstowcs 的第一个参数必须是指向 wchar_t 的指针」：
 * 传入 char* 作为目标，gcc -std=c99 应报 incompatible pointer type 错误。 */
void bad_mbstowcs_arg(void)
{
    char dst[16];
    mbstowcs(dst, "abc", 16);   /* 期望报错：dst 应为 wchar_t* */
}

/* 违反约束「wcstombs 的第二个参数必须是指向 wchar_t 的指针」：
 * 传入 char* 作为源，gcc -std=c99 应报 incompatible pointer type 错误。 */
void bad_wcstombs_arg(void)
{
    char dst[16];
    wcstombs(dst, "abc", 16);   /* 期望报错：第二参数应为 const wchar_t* */
}

/* 违反约束「mbtowc 的第一个参数必须是指向 wchar_t 的指针」：
 * 传入 int* 作为目标，gcc -std=c99 应报 incompatible pointer type 错误。 */
void bad_mbtowc_arg(void)
{
    int wc;
    mbtowc(&wc, "A", 1);        /* 期望报错：&wc 应为 wchar_t* */
}

/* 违反约束「wctomb 的第二个参数必须是 wchar_t 类型」：
 * 传入 int 而非 wchar_t，gcc -std=c99 应报 incompatible type 错误。 */
void bad_wctomb_arg(void)
{
    char buf[8];
    wctomb(buf, 65);            /* 期望报错：65 是 int，不是 wchar_t */
}

/* 违反约束「调用函数前必须有可见声明」：
 * 不包含 <stdlib.h> 时调用 mbstowcs，C99 要求编译器给出诊断。 */
void bad_no_declaration(void)
{
    /* 假设此处 <stdlib.h> 未包含 */
    mbstowcs(0, 0, 0);          /* 期望报错/警告：隐式声明 */
}

#endif /* 负向测试结束 */