/*
 * 测试目标：C99 7.4.1.2 The isalpha function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 概要：<ctype.h> 中声明 int isalpha(int c);
 *   [2] 描述：isalpha 对 isupper 或 islower 为真的字符返回真；
 *       在 "C" locale 下，仅当 isupper 或 islower 为真时 isalpha 才为真。
 *   Footnote 174：islower/isupper 对附加字符分别测试，四种组合都可能。
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 概要：验证 isalpha 的声明与返回类型为 int，参数为 int */
static int check_synopsis(void)
{
    int (*fp)(int) = isalpha;   /* 函数指针类型必须为 int(*)(int) */
    int r = fp('A');
    assert(r != 0);
    return 0;
}

/* [2] 描述：isalpha 对 isupper 或 islower 为真的字符返回真 */
static void check_upper_lower_imply_alpha(void)
{
    int c;
    for (c = 0; c <= UCHAR_MAX; c++) {
        if (isupper(c) || islower(c)) {
            assert(isalpha(c) != 0);   /* 上/下字母必为字母 */
        }
    }
}

/* [2] 描述：在 "C" locale 下，isalpha 为真 <=> isupper 或 islower 为真 */
static void check_c_locale_equivalence(void)
{
    int c;
    for (c = 0; c <= UCHAR_MAX; c++) {
        int a = isalpha(c) != 0;
        int ul = (isupper(c) != 0) || (islower(c) != 0);
        assert(a == ul);   /* "C" locale 下两者等价 */
    }
}

/* [2] 描述：isalpha 为真的字符，iscntrl/isdigit/ispunct/isspace 均应为假 */
static void check_alpha_excludes_others(void)
{
    int c;
    for (c = 0; c <= UCHAR_MAX; c++) {
        if (isalpha(c)) {
            assert(!iscntrl(c));
            assert(!isdigit(c));
            assert(!ispunct(c));
            assert(!isspace(c));
        }
    }
}

/* [2] 描述：典型字母字符的显式检查 */
static void check_typical_letters(void)
{
    const char *p;
    for (p = "abcdefghijklmnopqrstuvwxyz"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; *p; p++) {
        assert(isalpha((unsigned char)*p) != 0);
    }
    /* 非字母字符 */
    assert(isalpha('0') == 0);
    assert(isalpha(' ') == 0);
    assert(isalpha('\n') == 0);
    assert(isalpha('!') == 0);
    assert(isalpha('_') == 0);
}

/* [2] 描述：参数必须可表示为 unsigned char 或 EOF；EOF 不是字母 */
static void check_eof(void)
{
    assert(isalpha(EOF) == 0);
}

/* Footnote 174：islower/isupper 对附加字符分别测试，四种组合都可能。
 * 在 "C" locale 下，附加字符集合为空，因此对每个字符：
 *   islower 与 isupper 不会同时为真（无附加字符时二者互斥）。
 * 我们验证：对 "C" locale 中所有字符，二者不同时为真。 */
static void check_footnote174(void)
{
    int c;
    for (c = 0; c <= UCHAR_MAX; c++) {
        /* 在 "C" locale 下，不存在同时为 upper 和 lower 的字符 */
        assert(!(isupper(c) && islower(c)));
    }
}

int main(void)
{
    check_synopsis();
    check_upper_lower_imply_alpha();
    check_c_locale_equivalence();
    check_alpha_excludes_others();
    check_typical_letters();
    check_eof();
    check_footnote174();

    printf("C99 7.4.1.2 isalpha: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isalpha 的参数类型为 int」：
 * 传入结构体类型实参，gcc -std=c99 应报错（参数类型不兼容）。 */
struct S { int x; } s;
isalpha(s);

/* 违反约束「isalpha 的返回类型为 int」：
 * 将 isalpha 的返回值赋给不兼容的指针类型，应报错。 */
int *p = isalpha('A');

/* 违反约束「isalpha 的声明来自 <ctype.h>」：
 * 若未包含 <ctype.h>，隐式声明与标准声明冲突（C99 中隐式函数声明为约束违反），
 * 在严格模式下应报错。此处演示重复声明为不兼容类型。 */
double isalpha(int c);   /* 与标准声明 int isalpha(int) 冲突，应报错 */

/* 违反约束「isalpha 是函数，不是对象」：
 * 对函数名取地址后当作对象使用（如赋值），应报错。 */
isalpha = 0;

#endif