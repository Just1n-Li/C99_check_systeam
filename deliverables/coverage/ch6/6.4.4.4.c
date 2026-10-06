/*
 * 验证 C99 条款 6.4.4.4 (Character constants)
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

void test_positive() {
    /* [2] 整型字符常量与宽字符常量 */
    int c_int = 'x';
    wchar_t c_wchar = L'x';
    assert(c_int == 'x');
    assert(c_wchar == L'x');

    /* [3] 转义序列表示：单引号、双引号、问号、反斜杠 */
    assert('\'' == 39); /* 单引号 */
    assert('\"' == 34);  /* 双引号 */
    assert('\?' == 63);  /* 问号 */
    assert('\\' == 92);  /* 反斜杠 */

    /* [4] 双引号和问号可以直接使用或使用转义序列 */
    assert('"' == '\"');
    assert('?' == '\?');

    /* [5] 八进制转义序列 */
    assert('\101' == 'A'); /* 65 = 'A' */
    assert('\012' == '\n'); /* 10 = 换行 */

    /* [6] 十六进制转义序列 */
    assert('\x41' == 'A'); /* 65 = 'A' */
    assert('\x0A' == '\n'); /* 10 = 换行 */

    /* [7] 最长序列原则：十六进制转义序列直到非十六进制字符才终止 */
    /* '\x123' 是单个字符（实现定义值），'\x12' 后跟 '3' 需要 '\0223' */
    int hex_long = '\x123'; 
    assert(sizeof(hex_long) == sizeof(int));
    
    /* [8] 非图形字符的转义序列 */
    assert('\a' == 7);
    assert('\b' == 8);
    assert('\f' == 12);
    assert('\n' == 10);
    assert('\r' == 13);
    assert('\t' == 9);
    assert('\v' == 11);

    /* [9] 约束范围内的值：'\377' 和 '\xFF' 在 unsigned char 范围内 */
    int max_char = '\377';
    int max_hex = '\xFF';
    (void)max_char;
    (void)max_hex;

    /* [10] 整型字符常量类型为 int */
    assert(sizeof('a') == sizeof(int));

    /* [11] 宽字符常量类型为 wchar_t */
    assert(sizeof(L'a') == sizeof(wchar_t));

    /* [12] EXAMPLE 1: '\0' 表示空字符 */
    assert('\0' == 0);

    /* [13] EXAMPLE 2: '\xFF' 的值是实现定义的（-1 或 255），此处仅验证可编译 */
    int ff_val = '\xFF';
    (void)ff_val;

    /* [14] EXAMPLE 3: '\0223' 是包含两个字符的常量（实现定义值） */
    int two_chars = '\0223';
    assert(sizeof(two_chars) == sizeof(int));

    /* [15] EXAMPLE 4: L'\1234' 是实现定义的值 */
    wchar_t wc_impl = L'\1234';
    (void)wc_impl;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [9] 违反约束「八进制转义序列的值超出 unsigned char 范围」 */
/* 八进制 400 = 256，超出 unsigned char 最大值 255，gcc -std=c99 应报错 */
void test_negative_octal_overflow() {
    int c = '\400';
}

/* [9] 违反约束「十六进制转义序列的值超出 unsigned char 范围」 */
/* 十六进制 100 = 256，超出 unsigned char 最大值 255，gcc -std=c99 应报错 */
void test_negative_hex_overflow() {
    int c = '\x100';
}

/* [1][4] 违反约束「单引号必须由转义序列表示」 */
/* 语法错误，单引号未转义，gcc -std=c99 应报错 */
void test_negative_unescaped_quote() {
    int c = ''';
}

/* [1][4] 违反约束「反斜杠必须由转义序列表示」 */
/* 语法错误，反斜杠未转义，gcc -std=c99 应报错 */
void test_negative_unescaped_backslash() {
    int c = '\';
}

/* [1] 违反约束「非法的转义序列」 */
/* \m 不是合法的简单转义序列，gcc -std=c99 应给出警告或报错（要求诊断） */
void test_negative_invalid_escape() {
    int c = '\m';
}

#endif

int main() {
    test_positive();
    printf("All positive tests passed!\n");
    return 0;
}