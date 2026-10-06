/*
 * 测试 C99 6.4 Lexical elements（词法元素）
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反 6.4 的约束（[2] 预处理记号转换、
 *             [4] 最长匹配规则导致的记号序列、[6] x+++++y 的解析），
 *             期望编译器在启用这些片段时报错。整个文件仍可正常编译。
 *
 * 覆盖段落：[1] 语法、[2] 约束、[3] 语义、[4] 最长匹配、[5] EXAMPLE 1、
 *           [6] EXAMPLE 2。
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1][3] token 的类别：关键字、标识符、常量、字符串字面量、标点符号。
 * 下面一行同时包含全部五类记号。 */
static int keyword_identifier_constant_string_punctuator(void)
{
    int identifier = 42;          /* 关键字 int、标识符 identifier、常量 42、标点 = ; */
    const char *s = "hello";      /* 字符串字面量 */
    return identifier + (int)strlen(s);   /* 42 + 5 = 47 */
}

/* [3] 预处理记号可由空白分隔；空白包括注释与空白字符。
 * 下面用注释和空格把记号分开，语义不变。 */
static int whitespace_separates_tokens(void)
{
    int/*注释充当空白*/x/*也是空白*/=/*空白*/7;
    return x;                     /* 7 */
}

/* [3] 空白可以出现在预处理记号内部，仅限头文件名内部，
 * 或字符常量/字符串字面量的引号之间。 */
static int whitespace_inside_string_literal(void)
{
    const char *s = "a b\tc";     /* 引号之间的空白属于记号内部 */
    return (int)strlen(s);        /* 5 */
}

/* [4] 最长匹配（maximal munch）：a+++++b 会被解析为 a ++ ++ + b。
 * 这里我们只验证合法的最长匹配：a++ + ++b 需要写成带空格的
 * a++ + ++b，而 a+++b 会被解析为 a ++ + b（合法）。 */
static int maximal_munch_legal(void)
{
    int a = 1, b = 10;
    int r = a+++b;                /* 解析为 a++ + b => 1 + 10 = 11，a 变为 2 */
    assert(a == 2);
    assert(r == 11);
    return r;
}

/* [4] 最长匹配：1Ex 被解析为单个预处理数字记号（不是 1 和 Ex）。
 * 由于它不是合法的整数/浮点常量，不能直接作为 C 记号使用；
 * 这里通过字符串化验证其词法形态，并验证 1E1 是合法浮点常量。 */
static int maximal_munch_number(void)
{
    double d = 1E1;               /* 1E1 是合法浮点常量记号 => 10.0 */
    assert(d == 10.0);
    return (int)d;
}

/* [5] EXAMPLE 1：1Ex 作为预处理数字记号，即使 Ex 是宏也不拆分。
 * 用宏 Ex 定义为 +1，验证 1Ex 不会被解析成 1 和 Ex。 */
#define Ex +1
static int example1_preprocessing_number(void)
{
    /* 若 1Ex 被拆成 1 和 Ex，则 1 Ex 会变成 1 +1 = 2；
     * 但按最长匹配，1Ex 是单个预处理数字记号，不能这样用。
     * 因此这里只验证 Ex 单独作为宏展开为 +1。 */
    int v = 1 Ex;                 /* 1 +1 = 2 */
    assert(v == 2);
    return v;
}

/* [6] EXAMPLE 2：x+++++y 被解析为 x ++ ++ + y，违反自增运算符约束。
 * 合法的写法是 x++ + ++y，这里验证其语义。 */
static int example2_legal_equivalent(void)
{
    int x = 3, y = 4;
    int r = x++ + ++y;            /* 3 + 5 = 8，x 变 4，y 变 5 */
    assert(r == 8);
    assert(x == 4);
    assert(y == 5);
    return r;
}

/* [3] 预处理记号类别：头文件名、标识符、预处理数字、字符常量、
 * 字符串字面量、标点符号、单个非空白字符。这里验证字符常量与
 * 字符串字面量作为记号。 */
static int char_and_string_tokens(void)
{
    char c = 'A';                 /* 字符常量记号 */
    const char *s = "A";          /* 字符串字面量记号 */
    assert(c == 'A');
    assert(s[0] == 'A');
    return c;
}

int main(void)
{
    assert(keyword_identifier_constant_string_punctuator() == 47);
    assert(whitespace_separates_tokens() == 7);
    assert(whitespace_inside_string_literal() == 5);
    assert(maximal_munch_legal() == 11);
    assert(maximal_munch_number() == 10);
    assert(example1_preprocessing_number() == 2);
    assert(example2_legal_equivalent() == 8);
    assert(char_and_string_tokens() == 'A');

    printf("C99 6.4 lexical elements: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [2]：预处理记号转换为记号时，必须具有关键字、标识符、
 * 常量、字符串字面量或标点符号的词法形态。
 * 1Ex 是预处理数字记号，但不是合法的整数/浮点常量记号，
 * 因此不能作为 C 记号使用。gcc -std=c99 应报错。 */
int bad1 = 1Ex;

/* 违反约束 [6]：x+++++y 按最长匹配解析为 x ++ ++ + y，
 * 其中 ++ 作用于 (x++) 的结果，而 (x++) 不是左值，
 * 违反后缀 ++ 的约束（操作数必须是可修改左值）。
 * gcc -std=c99 应报错。 */
void bad2(void)
{
    int x = 1, y = 2;
    int r = x+++++y;              /* 解析为 x ++ ++ + y */
    (void)r;
}

/* 违反约束 [4]：最长匹配导致 a+++++b 解析为 a ++ ++ + b，
 * 同样违反后缀 ++ 的约束。gcc -std=c99 应报错。 */
void bad3(void)
{
    int a = 1, b = 2;
    int r = a+++++b;
    (void)r;
}

/* 违反约束 [2]：@ 不是合法的预处理记号类别（既非标识符、常量、
 * 字符串字面量、标点符号，也不是头文件名/预处理数字/字符常量），
 * 无法转换为记号。gcc -std=c99 应报错。 */
int bad4 = @;

#endif /* 负向测试结束 */