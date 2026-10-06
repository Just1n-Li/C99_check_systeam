/*
 * 验证 C99 条款 6.4.5 (String literals)
 * 预期行为：
 * - 正向测试：能编译并运行通过，assert 验证语义。
 * - 负向测试：违反语法约束，应编译报错。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>

/* 辅助函数：用于验证字符串字面量的静态存储期 */
const char* get_literal(void) {
    return "static_storage";
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [2] 字符串字面量与宽字符串字面量 */
    char *s_narrow = "xyz";
    wchar_t *s_wide = L"xyz";
    assert(strcmp(s_narrow, "xyz") == 0);
    assert(wcscmp(s_wide, L"xyz") == 0);

    /* [3] 单引号可单独使用或转义，双引号必须转义 */
    char *s_quote1 = "'";
    char *s_quote2 = "\'";
    char *s_dquote = "\"";
    assert(strcmp(s_quote1, "'") == 0);
    assert(strcmp(s_quote2, "'") == 0);
    assert(strcmp(s_dquote, "\"") == 0);

    /* [4] 相邻字符串字面量在阶段 6 拼接 */
    char *s_concat1 = "ab" "cd";
    wchar_t *s_concat2 = L"ab" "cd"; /* 含宽字符串，结果为宽字符串 */
    wchar_t *s_concat3 = "ab" L"cd"; /* 含宽字符串，结果为宽字符串 */
    assert(strcmp(s_concat1, "abcd") == 0);
    assert(wcscmp(s_concat2, L"abcd") == 0);
    assert(wcscmp(s_concat3, L"abcd") == 0);

    /* [5] 阶段 7 追加零字节，并初始化静态存储期数组 */
    assert(sizeof("abc") == 4); /* 3字符 + '\0' */
    assert(sizeof(L"abc") == 4 * sizeof(wchar_t)); /* 宽字符同理 */
    assert(get_literal() != NULL); /* 验证静态存储期，函数返回后指针仍有效 */

    /* [6] 数组是否不同未指定(unspecified)，修改数组是未定义行为(UB)。
       此处不进行修改测试以避免 UB，仅验证读取正确 */
    char *s1 = "identical";
    char *s2 = "identical";
    assert(strcmp(s1, s2) == 0); /* 无论是否指向同一数组，内容必须一致 */

    /* [7] EXAMPLE: "\x12" "3" 产生包含 '\x12' 和 '3' 的字符串 */
       因为转义序列在相邻字符串拼接前转换 */
    char *s_ex = "\x12" "3";
    assert(sizeof("\x12" "3") == 3); /* 2字符 + '\0' */
    assert((unsigned char)s_ex[0] == 0x12);
    assert(s_ex[1] == '3');
    assert(s_ex[2] == '\0');

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反语法约束：s-char 不能是未转义的换行符。
   字符串字面量中不能直接包含物理换行，gcc -std=c99 应报错 "missing terminating \" character" */
char *s_bad_newline = "abc
def";

/* [1] 违反语法约束：s-char 不能是未转义的双引号。
   双引号必须用 \" 表示，gcc -std=c99 应报错 */
char *s_bad_dquote = "a"b";

/* [1] 违反语法约束：s-char 不能是未转义的反斜杠。
   反斜杠后跟换行符会被当作行续接，导致字符串未正确闭合，gcc -std=c99 应报错 */
char *s_bad_backslash = "abc\

def";
#endif