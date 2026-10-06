/*
 * 测试目标：C99 7.11 <locale.h> —— Localization
 *
 * 预期行为：
 *   正向测试：包含 <locale.h>，使用 struct lconv 的全部成员、LC_* 宏、setlocale/localeconv，
 *             在 "C" locale 下验证 7.11.2.1 规定的成员取值，程序应能编译并运行通过。
 *   负向测试：违反 7.11 约束的代码（如把 LC_* 宏当字符串、对 struct lconv 成员做非法操作等），
 *             期望编译器报错；这些片段放在 #if 0 中，保证本文件仍可编译运行。
 *
 * 覆盖段落：[1] 头文件声明内容；[2] struct lconv 成员及 "C" locale 取值；
 *           [3] NULL 与 LC_* 宏（整数常量表达式、互不相同）。
 */

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] <locale.h> 声明两个函数、一个类型、若干宏。
 *     这里通过取地址/调用验证 setlocale 与 localeconv 的存在与签名。 */
static void test_header_declarations(void)
{
    /* setlocale: char *setlocale(int, const char *); */
    char *(*p_setlocale)(int, const char *) = setlocale;
    /* localeconv: struct lconv *localeconv(void); */
    struct lconv *(*p_localeconv)(void) = localeconv;

    assert(p_setlocale != NULL);
    assert(p_localeconv != NULL);

    /* 切到 "C" locale，保证后续取值符合标准规定 */
    char *r = setlocale(LC_ALL, "C");
    assert(r != NULL);
}

/* [2] struct lconv 至少包含标准列出的成员；在 "C" locale 下取值应符合注释。 */
static void test_lconv_members_in_C_locale(void)
{
    struct lconv *lc = localeconv();
    assert(lc != NULL);

    /* 数值格式成员 */
    assert(lc->decimal_point != NULL);
    assert(strcmp(lc->decimal_point, ".") == 0);   /* "." */

    assert(lc->thousands_sep != NULL);
    assert(strcmp(lc->thousands_sep, "") == 0);    /* "" */

    assert(lc->grouping != NULL);
    assert(strcmp(lc->grouping, "") == 0);         /* "" */

    /* 货币格式成员（字符串） */
    assert(lc->mon_decimal_point != NULL);
    assert(strcmp(lc->mon_decimal_point, "") == 0);

    assert(lc->mon_thousands_sep != NULL);
    assert(strcmp(lc->mon_thousands_sep, "") == 0);

    assert(lc->mon_grouping != NULL);
    assert(strcmp(lc->mon_grouping, "") == 0);

    assert(lc->positive_sign != NULL);
    assert(strcmp(lc->positive_sign, "") == 0);

    assert(lc->negative_sign != NULL);
    assert(strcmp(lc->negative_sign, "") == 0);

    assert(lc->currency_symbol != NULL);
    assert(strcmp(lc->currency_symbol, "") == 0);

    assert(lc->int_curr_symbol != NULL);
    assert(strcmp(lc->int_curr_symbol, "") == 0);

    /* 货币格式成员（char，C locale 下均为 CHAR_MAX） */
    assert(lc->frac_digits       == CHAR_MAX);
    assert(lc->p_cs_precedes     == CHAR_MAX);
    assert(lc->n_cs_precedes     == CHAR_MAX);
    assert(lc->p_sep_by_space    == CHAR_MAX);
    assert(lc->n_sep_by_space    == CHAR_MAX);
    assert(lc->p_sign_posn       == CHAR_MAX);
    assert(lc->n_sign_posn       == CHAR_MAX);

    assert(lc->int_frac_digits     == CHAR_MAX);
    assert(lc->int_p_cs_precedes   == CHAR_MAX);
    assert(lc->int_n_cs_precedes   == CHAR_MAX);
    assert(lc->int_p_sep_by_space  == CHAR_MAX);
    assert(lc->int_n_sep_by_space  == CHAR_MAX);
    assert(lc->int_p_sign_posn     == CHAR_MAX);
    assert(lc->int_n_sign_posn     == CHAR_MAX);
}

/* [3] LC_* 宏是整数常量表达式，且互不相同，可作为 setlocale 的第一个实参。
 *     同时验证 NULL 宏可用（来自 <locale.h>，与 7.17 一致）。 */
static void test_macros(void)
{
    /* 整数常量表达式：可用于数组维度、case 标签、位域等常量上下文 */
    char arr[LC_ALL + 1];
    (void)arr;

    /* 互不相同 */
    assert(LC_ALL      != LC_COLLATE);
    assert(LC_ALL      != LC_CTYPE);
    assert(LC_ALL      != LC_MONETARY);
    assert(LC_ALL      != LC_NUMERIC);
    assert(LC_ALL      != LC_TIME);
    assert(LC_COLLATE  != LC_CTYPE);
    assert(LC_COLLATE  != LC_MONETARY);
    assert(LC_COLLATE  != LC_NUMERIC);
    assert(LC_COLLATE  != LC_TIME);
    assert(LC_CTYPE    != LC_MONETARY);
    assert(LC_CTYPE    != LC_NUMERIC);
    assert(LC_CTYPE    != LC_TIME);
    assert(LC_MONETARY != LC_NUMERIC);
    assert(LC_MONETARY != LC_TIME);
    assert(LC_NUMERIC  != LC_TIME);

    /* 作为 setlocale 的第一个实参使用 */
    assert(setlocale(LC_ALL,      "C") != NULL);
    assert(setlocale(LC_COLLATE,  "C") != NULL);
    assert(setlocale(LC_CTYPE,    "C") != NULL);
    assert(setlocale(LC_MONETARY, "C") != NULL);
    assert(setlocale(LC_NUMERIC,  "C") != NULL);
    assert(setlocale(LC_TIME,     "C") != NULL);

    /* NULL 宏（7.17）在 <locale.h> 中可用 */
    void *p = NULL;
    assert(p == NULL);
}

/* [2] 成员访问：struct lconv 的成员可读；字符串成员是指向 char 的指针。 */
static void test_member_types(void)
{
    struct lconv *lc = localeconv();
    char *dp = lc->decimal_point;   /* char * 类型 */
    char fd = lc->frac_digits;      /* char 类型 */
    assert(dp != NULL);
    (void)fd;
}

int main(void)
{
    test_header_declarations();
    test_lconv_members_in_C_locale();
    test_macros();
    test_member_types();

    printf("C99 7.11 <locale.h> positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「LC_* 宏展开为整数常量表达式」：
 * 把 LC_ALL 当字符串字面量使用（例如传给需要 const char * 的上下文之外，
 * 或直接做字符串拼接），gcc -std=c99 应报错。 */
const char *s = LC_ALL "x";   /* error: LC_ALL 不是字符串字面量 */

/* 违反约束「setlocale 第一个实参为 int」：
 * 传入字符串字面量而非 LC_* 整数常量，类型不匹配，应报错。 */
void bad_setlocale(void)
{
    setlocale("C", "C");      /* error: 第一个实参应为 int */
}

/* 违反约束「struct lconv 成员为 char *」：
 * 把字符串成员当整数使用（算术运算），应报错。 */
void bad_member_arith(void)
{
    struct lconv *lc = localeconv();
    int x = lc->decimal_point + 1;   /* error: char * 不能与 int 相加 */
    (void)x;
}

/* 违反约束「struct lconv 成员为 char」：
 * 对 char 成员取地址后赋给 char * 之外的错误类型，或把 char 成员当指针解引用，
 * 应报错。 */
void bad_member_deref(void)
{
    struct lconv *lc = localeconv();
    char c = *lc->frac_digits;       /* error: char 不能解引用 */
    (void)c;
}

/* 违反约束「localeconv 返回 struct lconv *」：
 * 把返回值赋给不兼容的指针类型，应报错（无显式转换）。 */
void bad_return_type(void)
{
    int *p = localeconv();           /* error: struct lconv * 与 int * 不兼容 */
    (void)p;
}

/* 违反约束「setlocale 返回 char *」：
 * 把返回值赋给不兼容类型，应报错。 */
void bad_setlocale_return(void)
{
    int n = setlocale(LC_ALL, "C");  /* error: char * 赋给 int */
    (void)n;
}

#endif /* 负向测试结束 */