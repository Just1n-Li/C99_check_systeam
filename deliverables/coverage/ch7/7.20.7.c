/*
 * 测试 C99 7.20.7 —— 多字节/宽字符转换函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖点：
 *   [1] 行为受当前 locale 的 LC_CTYPE 类别影响；
 *       s 为 NULL 时把函数置于初始转换状态；
 *       s 为 NULL 时，若编码有状态依赖则返回非零，否则返回零；
 *       改变 LC_CTYPE 会使转换状态变为不确定。
 *   脚注 266：用于改变移位状态的特殊字节不产生单独的宽字符码，
 *             而是与相邻多字节字符归为一组。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
#include <wchar.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 行为受 LC_CTYPE 影响：先设置一个已知 locale。
 * 使用 "C" locale 保证可移植性（任何实现都必须支持）。 */
static void test_lc_ctype_affects_behavior(void)
{
    char *ret = setlocale(LC_CTYPE, "C");
    /* "C" locale 必须被支持，setlocale 不应返回 NULL */
    assert(ret != NULL);

    /* 在 "C" locale 下，多字节字符就是单字节字符 */
    mbstate_t st;
    memset(&st, 0, sizeof st);

    /* mbrtowc：把多字节序列转换为宽字符 */
    wchar_t wc = 0;
    size_t n = mbrtowc(&wc, "A", 1, &st);
    assert(n == 1);
    assert(wc == L'A');

    /* wcrtomb：把宽字符转换为多字节序列 */
    memset(&st, 0, sizeof st);
    char buf[MB_LEN_MAX + 1];
    size_t m = wcrtomb(buf, L'B', &st);
    assert(m == 1);
    assert(buf[0] == 'B');
}

/* [1] s 为 NULL 时把函数置于初始转换状态；
 *     并且 s 为 NULL 时，若编码有状态依赖则返回非零，否则返回零。
 *
 * 对 mbrlen / mbrtowc / wcrtomb 等函数，s == NULL 的调用用于重置状态。
 * 在 "C" locale 下编码无状态依赖，因此返回 0。 */
static void test_null_pointer_resets_state(void)
{
    assert(setlocale(LC_CTYPE, "C") != NULL);

    /* mbrlen(s, n, ps)：s == NULL 时返回 0（无状态依赖） */
    mbstate_t st;
    memset(&st, 0, sizeof st);
    size_t r1 = mbrlen(NULL, 0, &st);
    assert(r1 == 0);

    /* mbrtowc(s, n, ps)：s == NULL 时返回 0（无状态依赖） */
    memset(&st, 0, sizeof st);
    size_t r2 = mbrtowc(NULL, NULL, 0, &st);
    assert(r2 == 0);

    /* wcrtomb(s, wc, ps)：s == NULL 时返回 0（无状态依赖） */
    memset(&st, 0, sizeof st);
    size_t r3 = wcrtomb(NULL, L'\0', &st);
    assert(r3 == 0);

    /* 注意：标准规定 s == NULL 时返回非零当且仅当编码有状态依赖。
     * 在 "C" locale 下无状态依赖，故上述返回值必须为 0。 */
}

/* [1] 后续以非 NULL 的 s 调用会按需改变内部转换状态。
 * 这里验证连续调用可以正常推进转换。 */
static void test_subsequent_calls_alter_state(void)
{
    assert(setlocale(LC_CTYPE, "C") != NULL);

    mbstate_t st;
    memset(&st, 0, sizeof st);

    const char *s = "ABC";
    wchar_t wc = 0;
    size_t n;

    n = mbrtowc(&wc, s, 1, &st);
    assert(n == 1 && wc == L'A');

    n = mbrtowc(&wc, s + 1, 1, &st);
    assert(n == 1 && wc == L'B');

    n = mbrtowc(&wc, s + 2, 1, &st);
    assert(n == 1 && wc == L'C');
}

/* [1] 改变 LC_CTYPE 会使转换状态变为不确定。
 * 我们无法直接观察“不确定”，但可以验证：
 * 改变 LC_CTYPE 后，重新用 NULL 指针调用可把状态重置为初始状态，
 * 从而得到确定的行为。 */
static void test_change_lc_ctype_then_reset(void)
{
    assert(setlocale(LC_CTYPE, "C") != NULL);

    mbstate_t st;
    memset(&st, 0, sizeof st);

    /* 先做一次转换，使状态可能被改变 */
    wchar_t wc = 0;
    size_t n = mbrtowc(&wc, "X", 1, &st);
    assert(n == 1 && wc == L'X');

    /* 改变 LC_CTYPE（切到 "C" 再切回 "C"，语义上仍是改变调用） */
    assert(setlocale(LC_CTYPE, "C") != NULL);

    /* 用 NULL 指针调用把状态重置为初始状态 */
    size_t r = mbrlen(NULL, 0, &st);
    assert(r == 0);

    /* 重置后再次转换应得到确定结果 */
    n = mbrtowc(&wc, "Y", 1, &st);
    assert(n == 1 && wc == L'Y');
}

/* 脚注 266：若 locale 使用特殊字节改变移位状态，
 * 这些字节不产生单独的宽字符码，而是与相邻多字节字符归为一组。
 *
 * 在 "C" locale 下不存在移位字节，因此每个字节对应一个宽字符。
 * 我们验证：多字节字符串 "AB" 转换为宽字符串后长度为 2，
 * 且每个宽字符与对应字节相等——即没有额外的“移位字节”产生
 * 独立的宽字符码。 */
static void test_no_separate_shift_codes_in_C_locale(void)
{
    assert(setlocale(LC_CTYPE, "C") != NULL);

    const char *mb = "AB";
    wchar_t wbuf[8];
    mbstate_t st;
    memset(&st, 0, sizeof st);

    size_t i = 0;
    const char *p = mb;
    while (*p) {
        size_t n = mbrtowc(&wbuf[i], p, 1, &st);
        assert(n == 1);
        p += n;
        ++i;
    }
    assert(i == 2);
    assert(wbuf[0] == L'A');
    assert(wbuf[1] == L'B');

    /* 用 mbstowcs 交叉验证 */
    wchar_t wbuf2[8];
    size_t cnt = mbstowcs(wbuf2, mb, 8);
    assert(cnt == 2);
    assert(wbuf2[0] == L'A' && wbuf2[1] == L'B');
}

/* 综合：mbsinit 用于判断 mbstate_t 是否处于初始状态。
 * 这是 7.20.7 相关函数族的一部分，用于配合 s == NULL 的重置语义。 */
static void test_mbsinit(void)
{
    mbstate_t st;
    memset(&st, 0, sizeof st);
    /* 全零的 mbstate_t 表示初始转换状态 */
    assert(mbsinit(&st) != 0);

    /* 在 "C" locale 下，转换后状态仍为初始状态 */
    assert(setlocale(LC_CTYPE, "C") != NULL);
    wchar_t wc;
    size_t n = mbrtowc(&wc, "Z", 1, &st);
    assert(n == 1 && wc == L'Z');
    assert(mbsinit(&st) != 0);
}

int main(void)
{
    test_lc_ctype_affects_behavior();
    test_null_pointer_resets_state();
    test_subsequent_calls_alter_state();
    test_change_lc_ctype_then_reset();
    test_no_separate_shift_codes_in_C_locale();
    test_mbsinit();

    printf("C99 7.20.7 positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「mbstate_t 是不完整类型，不能定义对象」：
 * C99 7.24.1 规定 mbstate_t 是完整对象类型，但 7.20.7 相关函数
 * 要求通过指针传递。此处故意把 mbstate_t 当作可解引用的结构体
 * 直接访问其成员——标准未定义其成员，任何实现都不应允许。
 * 期望：gcc -std=c99 报错（"dereferencing pointer to incomplete type"
 *       或 "invalid use of undefined type"）。 */
void bad_mbstate_member_access(void)
{
    mbstate_t st;
    st.__count = 0;   /* 错误：mbstate_t 的成员不可直接访问 */
}

/* 违反约束「mbrtowc 的第二个参数是 const char *」：
 * 传入非字符指针类型（int *）应触发类型不兼容的诊断。
 * 期望：gcc -std=c99 报错（"incompatible pointer type"）。 */
void bad_mbrtowc_arg_type(void)
{
    mbstate_t st;
    wchar_t wc;
    int ival = 0;
    int *ip = &ival;
    mbrtowc(&wc, ip, 1, &st);   /* 错误：应为 const char * */
}

/* 违反约束「wcrtomb 的第一个参数是 char *」：
 * 传入 wchar_t * 应触发类型不兼容的诊断。
 * 期望：gcc -std=c99 报错（"incompatible pointer type"）。 */
void bad_wcrtomb_arg_type(void)
{
    mbstate_t st;
    wchar_t wbuf[4];
    wcrtomb(wbuf, L'A', &st);   /* 错误：应为 char * */
}

/* 违反约束「mbrlen 的第三个参数是 mbstate_t *」：
 * 传入 int * 应触发类型不兼容的诊断。
 * 期望：gcc -std=c99 报错（"incompatible pointer type"）。 */
void bad_mbrlen_arg_type(void)
{
    int x = 0;
    mbrlen("A", 1, &x);   /* 错误：应为 mbstate_t * */
}

/* 违反约束「mbstowcs 的第二个参数是 wchar_t *」：
 * 传入 char * 应触发类型不兼容的诊断。
 * 期望：gcc -std=c99 报错（"incompatible pointer type"）。 */
void bad_mbstowcs_arg_type(void)
{
    char buf[8];
    mbstowcs(buf, "A", 8);   /* 错误：应为 wchar_t * */
}

/* 违反约束「wcstombs 的第二个参数是 char *」：
 * 传入 wchar_t * 应触发类型不兼容的诊断。
 * 期望：gcc -std=c99 报错（"incompatible pointer type"）。 */
void bad_wcstombs_arg_type(void)
{
    wchar_t wbuf[8];
    wcstombs(wbuf, L"A", 8);   /* 错误：应为 char * */
}

/* 违反约束「mbrtowc 的返回类型为 size_t，不能对其赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。
 * 期望：gcc -std=c99 报错（"lvalue required as left operand of assignment"）。 */
void bad_assign_to_call_result(void)
{
    mbstate_t st;
    wchar_t wc;
    mbrtowc(&wc, "A", 1, &st) = 0;   /* 错误：非左值不能赋值 */
}

/* 违反约束「mbsinit 的参数是 const mbstate_t *」：
 * 传入 int * 应触发类型不兼容的诊断。
 * 期望：gcc -std=c99 报错（"incompatible pointer type"）。 */
void bad_mbsinit_arg_type(void)
{
    int x = 0;
    mbsinit(&x);   /* 错误：应为 const mbstate_t * */
}

#endif /* 负向测试结束 */