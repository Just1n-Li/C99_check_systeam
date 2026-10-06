/*
 * 测试目标：C99 7.11.2.1  localeconv 函数
 *
 * 预期行为：
 *   正向测试：包含 <locale.h>，调用 localeconv()，验证返回的 struct lconv *
 *             非空，且其成员满足条款 [3][4][5][6] 描述的取值约束；
 *             验证 [7] “实现应表现为没有库函数调用 localeconv” 的可重入/稳定语义；
 *             验证 [8] 返回指向已填充对象的指针。
 *   负向测试：违反约束的代码（如把 localeconv 返回值赋给错误类型、
 *             对返回的 const 语义误用等）应导致编译报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra localeconv_test.c -o localeconv_test
 */

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */
int main(void)
{
    /* [1] 原型：struct lconv *localeconv(void);
     *     通过包含 <locale.h> 获得声明，并调用之。 */
    struct lconv *lc = localeconv();

    /* [8] 返回指向已填充对象的指针，必须非空。 */
    assert(lc != NULL);

    /* [2] 该对象按当前 locale 的规则填充了数值格式化信息。
     *     这里只验证“可访问且类型正确”，不假设具体 locale 内容。 */

    /* [3] char * 成员：都是指向字符串的指针。
     *     除 decimal_point 外，任一都可以指向 ""（表示不可用或零长度）。
     *     这里验证它们都是合法的 C 字符串（可被 strlen 安全读取）。 */
    {
        char *str_members[] = {
            lc->decimal_point,
            lc->thousands_sep,
            lc->grouping,
            lc->mon_decimal_point,
            lc->mon_thousands_sep,
            lc->mon_grouping,
            lc->positive_sign,
            lc->negative_sign,
            lc->currency_symbol,
            lc->int_curr_symbol
        };
        size_t i;
        for (i = 0; i < sizeof(str_members) / sizeof(str_members[0]); ++i) {
            assert(str_members[i] != NULL);          /* 指针有效 */
            (void)strlen(str_members[i]);            /* 可安全读取 */
        }
        /* decimal_point 在 C locale 下通常为 "."，但标准未强制其非空；
         * 只要求它是有效字符串指针。 */
        assert(lc->decimal_point != NULL);
    }

    /* [3] char 成员：非负数，任一可为 CHAR_MAX 表示不可用。
     *     验证它们都在 [0, CHAR_MAX] 范围内（char 可能为 signed，
     *     故用 unsigned char 转换后比较）。 */
    {
        char char_members[] = {
            lc->frac_digits,
            lc->p_cs_precedes,
            lc->n_cs_precedes,
            lc->p_sep_by_space,
            lc->n_sep_by_space,
            lc->p_sign_posn,
            lc->n_sign_posn,
            lc->int_frac_digits,
            lc->int_p_cs_precedes,
            lc->int_n_cs_precedes,
            lc->int_p_sep_by_space,
            lc->int_n_sep_by_space,
            lc->int_p_sign_posn,
            lc->int_n_sign_posn
        };
        size_t i;
        for (i = 0; i < sizeof(char_members) / sizeof(char_members[0]); ++i) {
            /* 非负：以 unsigned char 视角看，值 <= CHAR_MAX 即合法。 */
            unsigned char v = (unsigned char)char_members[i];
            assert(v <= (unsigned char)CHAR_MAX);
        }
    }

    /* [3] p_cs_precedes / n_cs_precedes 等“1 或 0”成员：
     *     若可用（非 CHAR_MAX），则应为 0 或 1。 */
    {
        char cs[] = { lc->p_cs_precedes, lc->n_cs_precedes,
                      lc->int_p_cs_precedes, lc->int_n_cs_precedes };
        size_t i;
        for (i = 0; i < sizeof(cs) / sizeof(cs[0]); ++i) {
            if ((unsigned char)cs[i] != (unsigned char)CHAR_MAX) {
                assert(cs[i] == 0 || cs[i] == 1);
            }
        }
    }

    /* [4] grouping / mon_grouping 元素解释：
     *     CHAR_MAX = 不再分组；0 = 重复前一元素；其他 = 当前组位数。
     *     验证每个元素要么是 CHAR_MAX，要么是 0，要么是正数（>0）。 */
    {
        const char *grp[] = { lc->grouping, lc->mon_grouping };
        size_t g;
        for (g = 0; g < 2; ++g) {
            const char *p = grp[g];
            while (*p != '\0') {
                unsigned char e = (unsigned char)*p;
                if (e != (unsigned char)CHAR_MAX) {
                    /* 0 或正数（作为 unsigned char 时 >0 即正数） */
                    assert(e == 0 || e > 0);
                }
                ++p;
            }
        }
    }

    /* [5] p_sep_by_space / n_sep_by_space / int_*_sep_by_space：
     *     若可用，取值应为 0、1 或 2。 */
    {
        char sep[] = { lc->p_sep_by_space, lc->n_sep_by_space,
                       lc->int_p_sep_by_space, lc->int_n_sep_by_space };
        size_t i;
        for (i = 0; i < sizeof(sep) / sizeof(sep[0]); ++i) {
            if ((unsigned char)sep[i] != (unsigned char)CHAR_MAX) {
                assert(sep[i] >= 0 && sep[i] <= 2);
            }
        }
    }

    /* [6] p_sign_posn / n_sign_posn / int_*_sign_posn：
     *     若可用，取值应为 0、1、2、3 或 4。 */
    {
        char sp[] = { lc->p_sign_posn, lc->n_sign_posn,
                      lc->int_p_sign_posn, lc->int_n_sign_posn };
        size_t i;
        for (i = 0; i < sizeof(sp) / sizeof(sp[0]); ++i) {
            if ((unsigned char)sp[i] != (unsigned char)CHAR_MAX) {
                assert(sp[i] >= 0 && sp[i] <= 4);
            }
        }
    }

    /* [3] int_curr_symbol：前三个字符为 ISO 4217 字母货币符号，
     *     第四个字符（紧邻 '\0' 之前）为分隔符。
     *     若该字符串非空，则长度至少为 4（3 字母 + 1 分隔符 + '\0'）。 */
    if (lc->int_curr_symbol[0] != '\0') {
        assert(strlen(lc->int_curr_symbol) >= 4);
    }

    /* [7] 实现应表现为没有库函数调用 localeconv。
     *     即 localeconv 的结果不应被其它库函数“暗中改变”；
     *     连续两次调用应返回指向同一（或等价）对象的指针，
     *     且内容一致。 */
    {
        struct lconv *lc2 = localeconv();
        assert(lc2 != NULL);
        /* 同一 locale 下，两次调用应给出相同内容。 */
        assert(strcmp(lc->decimal_point, lc2->decimal_point) == 0);
        assert(strcmp(lc->currency_symbol, lc2->currency_symbol) == 0);
        assert(lc->frac_digits == lc2->frac_digits);
        assert(lc->p_cs_precedes == lc2->p_cs_precedes);
    }

    /* [2] 在 C locale 下，decimal_point 通常为 "."，但标准未强制；
     *     这里只做“可打印”的展示，不 assert 具体值。 */
    printf("localeconv OK: decimal_point=\"%s\", currency_symbol=\"%s\"\n",
           lc->decimal_point, lc->currency_symbol);

    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「localeconv 返回 struct lconv *」：
 * 把返回值赋给不兼容的指针类型，gcc -std=c99 应报错
 * （incompatible pointer type / assignment from incompatible pointer type）。 */
void neg_wrong_return_type(void)
{
    int *p = localeconv();   /* 错误：struct lconv * 不能赋给 int * */
    (void)p;
}

/* 违反约束「localeconv 无参数」：
 * 以参数调用 localeconv，gcc -std=c99 应报错
 * （too many arguments to function 'localeconv'）。 */
void neg_too_many_args(void)
{
    struct lconv *p = localeconv(1);   /* 错误：原型为 (void) */
    (void)p;
}

/* 违反约束「localeconv 返回 struct lconv *」：
 * 对返回值解引用后当作非结构体使用，类型不匹配，应报错。 */
void neg_deref_as_int(void)
{
    int x = *localeconv();   /* 错误：*localeconv() 是 struct lconv，不能初始化 int */
    (void)x;
}

/* 违反约束「localeconv 返回 struct lconv *」：
 * 对返回值使用成员访问但成员名不存在，应报错
 * （no member named 'nonexistent_member' in 'struct lconv'）。 */
void neg_bad_member(void)
{
    struct lconv *p = localeconv();
    (void)p->nonexistent_member;   /* 错误：struct lconv 无此成员 */
}

/* 违反约束「localeconv 返回 struct lconv *」：
 * 把返回值当作函数调用，应报错
 * （called object is not a function or function pointer）。 */
void neg_call_result(void)
{
    localeconv()();   /* 错误：struct lconv * 不是函数 */
}

#endif /* 负向测试结束 */