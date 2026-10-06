/*
 * 测试 C99 7.25.2.2.2 —— wctype 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型：wctype_t wctype(const char *property);
 *   [2] 构造 wctype_t 值，描述由 property 标识的宽字符类。
 *   [3] iswctype 描述中列出的字符串在所有 locale 下都必须是合法的 property 实参。
 *   [4] 合法类返回非零值（可作为 iswctype 第二实参）；否则返回 0。
 */

#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与声明完全一致 */
static wctype_t (*wctype_proto_check)(const char *) = wctype;

int main(void)
{
    /* [1] 原型：返回类型为 wctype_t，参数为 const char * */
    assert(wctype_proto_check == wctype);

    /* [2] 构造 wctype_t 值：对合法 property 应返回一个 wctype_t 值 */
    wctype_t t_alpha = wctype("alpha");
    wctype_t t_digit = wctype("digit");
    wctype_t t_space = wctype("space");
    wctype_t t_upper = wctype("upper");
    wctype_t t_lower = wctype("lower");
    wctype_t t_punct = wctype("punct");
    wctype_t t_cntrl = wctype("cntrl");
    wctype_t t_graph = wctype("graph");
    wctype_t t_print = wctype("print");
    wctype_t t_blank = wctype("blank");
    wctype_t t_xdigit = wctype("xdigit");

    /* [3] iswctype 描述中列出的字符串在所有 locale 下都必须是合法 property。
     *     因此这些调用在任意 locale 下都必须返回非零值。 */
    assert(t_alpha  != 0);
    assert(t_digit  != 0);
    assert(t_space  != 0);
    assert(t_upper  != 0);
    assert(t_lower  != 0);
    assert(t_punct  != 0);
    assert(t_cntrl  != 0);
    assert(t_graph  != 0);
    assert(t_print  != 0);
    assert(t_blank  != 0);
    assert(t_xdigit != 0);

    /* [4] 返回值可作为 iswctype 的第二实参使用，且语义一致 */
    assert(iswctype(L'A', t_alpha) != 0);
    assert(iswctype(L'7', t_digit) != 0);
    assert(iswctype(L' ', t_space) != 0);
    assert(iswctype(L'A', t_upper) != 0);
    assert(iswctype(L'a', t_lower) != 0);
    assert(iswctype(L'.', t_punct) != 0);
    assert(iswctype(L'\n', t_cntrl) != 0);
    assert(iswctype(L'A', t_graph) != 0);
    assert(iswctype(L'A', t_print) != 0);
    assert(iswctype(L' ', t_blank) != 0);
    assert(iswctype(L'F', t_xdigit) != 0);

    /* [4] 与对应的 isw* 函数结果一致（同一 locale 下） */
    assert((iswctype(L'A', t_alpha) != 0) == (iswalpha(L'A') != 0));
    assert((iswctype(L'7', t_digit) != 0) == (iswdigit(L'7') != 0));
    assert((iswctype(L' ', t_space) != 0) == (iswspace(L' ') != 0));

    /* [4] 非法 property：应返回 0 */
    assert(wctype("no_such_class_xyz") == 0);
    assert(wctype("") == 0);
    assert(wctype("ALPHA") == 0);   /* 大小写敏感，非标准名 */

    /* [4] 返回 0 时，作为 iswctype 第二实参使用应得到 0（无字符属于该类） */
    wctype_t t_bad = wctype("no_such_class_xyz");
    assert(t_bad == 0);
    assert(iswctype(L'A', t_bad) == 0);
    assert(iswctype(L' ', t_bad) == 0);

    /* [2][4] 在另一个 locale 下重新构造，仍应满足 [3] 的“所有 locale 合法”要求 */
    {
        const char *saved = setlocale(LC_CTYPE, NULL);
        char savedbuf[128];
        if (saved != NULL) {
            strncpy(savedbuf, saved, sizeof savedbuf - 1);
            savedbuf[sizeof savedbuf - 1] = '\0';
        } else {
            savedbuf[0] = '\0';
        }

        /* 尝试切换到 "C" locale（一定可用） */
        if (setlocale(LC_CTYPE, "C") != NULL) {
            assert(wctype("alpha")  != 0);
            assert(wctype("digit")  != 0);
            assert(wctype("space")  != 0);
            assert(wctype("upper")  != 0);
            assert(wctype("lower")  != 0);
            assert(wctype("punct")  != 0);
            assert(wctype("cntrl")  != 0);
            assert(wctype("graph")  != 0);
            assert(wctype("print")  != 0);
            assert(wctype("blank")  != 0);
            assert(wctype("xdigit") != 0);
            assert(wctype("no_such_class_xyz") == 0);
        }

        /* 恢复原 locale */
        if (savedbuf[0] != '\0') {
            setlocale(LC_CTYPE, savedbuf);
        }
    }

    printf("C99 7.25.2.2.2 wctype: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wctype 的参数类型为 const char *」：
 * 传入 wchar_t* 或 int 等不兼容类型，gcc -std=c99 应报错
 * （incompatible type for argument 1 of 'wctype' / passing argument 1 ...）。 */
#include <wchar.h>
void neg_wrong_arg_type(void)
{
    wchar_t wbuf[8] = L"alpha";
    wctype_t t1 = wctype(wbuf);   /* 错误：wchar_t* 不能隐式转换为 const char* */
    wctype_t t2 = wctype(42);     /* 错误：int 不能隐式转换为 const char* */
    (void)t1; (void)t2;
}

/* 违反约束「wctype 返回 wctype_t，不能当作指针解引用」：
 * 对返回值使用 * 运算符，gcc -std=c99 应报错
 * （invalid type argument of unary '*'）。 */
void neg_deref_return(void)
{
    char c = *wctype("alpha");    /* 错误：wctype_t 不是指针类型 */
    (void)c;
}

/* 违反约束「wctype 返回 wctype_t，不能当作函数调用」：
 * 对返回值使用函数调用运算符，gcc -std=c99 应报错
 * （called object is not a function or function pointer）。 */
void neg_call_return(void)
{
    wctype("alpha")();            /* 错误：wctype_t 不是函数 */
}

/* 违反约束「wctype 返回 wctype_t，不能对其赋值」：
 * 函数调用结果不是左值，gcc -std=c99 应报错
 * （lvalue required as left operand of assignment）。 */
void neg_assign_return(void)
{
    wctype("alpha") = 0;          /* 错误：非左值不能赋值 */
}

/* 违反约束「wctype 的参数个数必须为 1」：
 * 少传或多传实参，gcc -std=c99 应报错
 * （too few arguments to function 'wctype' / too many arguments）。 */
void neg_wrong_arg_count(void)
{
    wctype_t t1 = wctype();               /* 错误：缺少实参 */
    wctype_t t2 = wctype("alpha", "x");   /* 错误：实参过多 */
    (void)t1; (void)t2;
}

/* 违反约束「wctype 的返回类型为 wctype_t，不能赋给不兼容类型」：
 * 将返回值赋给结构体等不兼容类型，gcc -std=c99 应报错
 * （incompatible types when assigning）。 */
struct NegS { int x; };
void neg_incompatible_assign(void)
{
    struct NegS s;
    s = wctype("alpha");          /* 错误：wctype_t 不能赋给 struct NegS */
}

#endif /* 负向测试结束 */