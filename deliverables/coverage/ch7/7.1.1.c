/*
 * 测试 C99 7.1.1 Definitions of terms
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 ... #endif 中，故意违反约束，编译器应报错。
 *
 * 说明：7.1.1 主要是“术语定义”条款，本身几乎不含可执行语义，
 *       但其中若干定义（字符串、宽字符串、空字符、空宽字符、十进制小数点字符、
 *       移位序列）在标准库中都有可观察的对应行为，本程序用标准库函数
 *       对这些定义做可运行验证。
 */

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* ---------------------------------------------------------------
     * [1] 字符串：以第一个空字符结尾并包含该空字符的连续字符序列。
     *     指向字符串的指针指向其首（最低地址）字符。
     *     字符串长度 = 空字符之前的字节数。
     *     字符串的值 = 所含字符值按顺序构成的序列。
     * --------------------------------------------------------------- */
    {
        const char *s = "abc";          /* 4 个字节：'a','b','c','\0' */
        /* 指针指向首字符 */
        assert(s[0] == 'a');
        assert(*s == 'a');
        /* 以第一个空字符结尾并包含它 */
        assert(s[3] == '\0');
        /* 长度 = 空字符之前的字节数 */
        assert(strlen(s) == 3);
        /* 值 = 字符值按顺序 */
        assert(s[0] == 'a' && s[1] == 'b' && s[2] == 'c');
        /* 空字符串：长度 0，仍包含一个空字符 */
        const char *e = "";
        assert(strlen(e) == 0);
        assert(e[0] == '\0');
        /* 字符串中第一个空字符决定长度（后续字节不计入长度） */
        char buf[6] = { 'x', 'y', '\0', 'z', 'w', '\0' };
        assert(strlen(buf) == 2);
    }

    /* ---------------------------------------------------------------
     * [2] 十进制小数点字符：浮点<->字符序列转换函数用来表示小数部分
     *     起始的字符。文本/示例中用 '.' 表示，但可被 setlocale 改变。
     *     在 "C" locale 下，它就是 '.'。
     * --------------------------------------------------------------- */
    {
        /* 确保处于 "C" locale */
        char *loc = setlocale(LC_ALL, "C");
        assert(loc != NULL);

        /* 用 snprintf 观察十进制小数点字符 */
        char out[32];
        int n = snprintf(out, sizeof out, "%.2f", 3.5);
        assert(n == 4);
        assert(strcmp(out, "3.50") == 0);
        /* 小数点字符就是 '.' */
        assert(out[1] == '.');

        /* 用 strtod 观察十进制小数点字符的解析 */
        char *end = NULL;
        double d = strtod("2.25", &end);
        assert(d == 2.25);
        assert(end != NULL && *end == '\0');
    }

    /* ---------------------------------------------------------------
     * [3] 空宽字符：码值为 0 的宽字符。
     * --------------------------------------------------------------- */
    {
        wchar_t wc = L'\0';
        assert(wc == 0);
        /* 空宽字符的码值为 0 */
        assert((int)wc == 0);
    }

    /* ---------------------------------------------------------------
     * [4] 宽字符串：以第一个空宽字符结尾并包含该空宽字符的连续宽字符序列。
     *     指向宽字符串的指针指向其首（最低地址）宽字符。
     *     宽字符串长度 = 空宽字符之前的宽字符数。
     *     宽字符串的值 = 所含宽字符码值按顺序构成的序列。
     * --------------------------------------------------------------- */
    {
        const wchar_t *ws = L"abc";     /* 4 个宽字符：L'a',L'b',L'c',L'\0' */
        /* 指针指向首宽字符 */
        assert(ws[0] == L'a');
        assert(*ws == L'a');
        /* 以第一个空宽字符结尾并包含它 */
        assert(ws[3] == L'\0');
        /* 长度 = 空宽字符之前的宽字符数 */
        assert(wcslen(ws) == 3);
        /* 值 = 码值按顺序 */
        assert(ws[0] == L'a' && ws[1] == L'b' && ws[2] == L'c');
        /* 空宽字符串：长度 0，仍包含一个空宽字符 */
        const wchar_t *we = L"";
        assert(wcslen(we) == 0);
        assert(we[0] == L'\0');
    }

    /* ---------------------------------------------------------------
     * [5] 移位序列：多字节字符串中可能引起移位状态改变的连续字节序列。
     *     移位序列没有对应的宽字符，它被视为相邻多字节字符的附属。
     *     在 "C" locale 下不存在移位序列，多字节字符与单字节字符一致，
     *     因此 MB_CUR_MAX == 1，且每个多字节字符对应一个宽字符。
     * --------------------------------------------------------------- */
    {
        char *loc = setlocale(LC_ALL, "C");
        assert(loc != NULL);

        /* "C" locale 下 MB_CUR_MAX 为 1（无移位序列、无多字节编码） */
        assert(MB_CUR_MAX == 1);

        /* 多字节字符 <-> 宽字符 一一对应，无移位序列介入 */
        mbstate_t st;
        memset(&st, 0, sizeof st);
        wchar_t wc = 0;
        size_t r = mbrtowc(&wc, "A", 1, &st);
        assert(r == 1);
        assert(wc == L'A');

        /* 反向：宽字符 -> 多字节，同样 1 字节 */
        memset(&st, 0, sizeof st);
        char mb[MB_LEN_MAX];
        size_t r2 = wcrtomb(mb, L'A', &st);
        assert(r2 == 1);
        assert(mb[0] == 'A');

        /* 空宽字符转换：产生空字节（终止符），长度 1 */
        memset(&st, 0, sizeof st);
        size_t r3 = wcrtomb(mb, L'\0', &st);
        assert(r3 == 1);
        assert(mb[0] == '\0');
    }

    printf("C99 7.1.1 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 说明：7.1.1 是“术语定义”条款，本身不直接规定约束（constraint）。
 * 下面列出的是与 7.1.1 所定义术语相关的、由其它条款（如 6.5.16 赋值、
 * 6.5.2.3 成员访问、6.5.3.2 取地址、6.7.3 类型限定符）施加的约束，
 * 违反后编译器必须报错。这些片段用于确认编译器确实拒绝它们。
 */

/* 违反约束「字符串字面量不可修改」相关：字符串字面量是 const 限定对象，
 * 对其元素赋值违反 6.4.5/6.5.16 约束，gcc -std=c99 应报错。 */
void neg_string_literal_assign(void)
{
    "abc"[0] = 'x';   /* error: assignment of read-only location */
}

/* 违反约束「数组名不是可修改左值」：字符串（数组）名不可赋值，
 * 违反 6.5.16 约束，应报错。 */
void neg_array_assign(void)
{
    char s[4] = "abc";
    s = "xyz";        /* error: assignment to expression with array type */
}

/* 违反约束「取地址操作数必须是左值或函数指示符」：
 * 对字符串字面量取地址得到的是数组指针，合法；但对非左值取地址非法。
 * 这里对强制转换结果取地址，违反 6.5.3.2 约束，应报错。 */
void neg_address_of_rvalue(void)
{
    int *p = &(int){ 0 };   /* 复合字面量是左值，合法；改用纯右值： */
    (void)p;
    int *q = &(1 + 2);      /* error: lvalue required as unary '&' operand */
    (void)q;
}

/* 违反约束「赋值左操作数必须是可修改左值」：
 * 对宽字符串字面量元素赋值，违反 6.5.16 约束，应报错。 */
void neg_wide_string_literal_assign(void)
{
    L"abc"[0] = L'x';  /* error: assignment of read-only location */
}

/* 违反约束「sizeof 操作数不得为函数类型或不完整类型」：
 * 对不完整类型求 sizeof，违反 6.5.3.4 约束，应报错。 */
struct Incomplete;
void neg_sizeof_incomplete(void)
{
    size_t n = sizeof(struct Incomplete);  /* error: invalid application of 'sizeof' */
    (void)n;
}

#endif /* 负向测试结束 */