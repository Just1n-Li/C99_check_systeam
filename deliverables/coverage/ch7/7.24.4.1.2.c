/*
 * 测试 C99 7.24.4.1.2 —— wcstol / wcstoll / wcstoul / wcstoull
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束的片段应被编译器拒绝。
 *
 * 覆盖段落：[1] 原型、[2] 三段分解、[3] base 规则、[4] subject sequence 定义、
 *           [5] 转换与 endptr、[6] locale、[7] 无转换时 endptr=nptr、
 *           [8] 返回值与 ERANGE。
 */

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <wctype.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性：取函数地址，验证签名与 restrict 限定兼容 */
static long int (*p_wcstol)(const wchar_t *restrict, wchar_t **restrict, int) = wcstol;
static long long int (*p_wcstoll)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoll;
static unsigned long int (*p_wcstoul)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoul;
static unsigned long long int (*p_wcstoull)(const wchar_t *restrict, wchar_t **restrict, int) = wcstoull;

static void test_basic_conversion(void)
{
    /* [2][3][5] base=10 十进制转换，endptr 指向 final string */
    wchar_t *end = NULL;
    const wchar_t *s = L"  12345abc";
    long v = wcstol(s, &end, 10);
    assert(v == 12345L);
    assert(end != NULL);
    assert(*end == L'a');          /* final wide string 从 'a' 开始 */

    /* [2] 前导空白（iswspace）被跳过 */
    end = NULL;
    v = wcstol(L"\t\n\v\f\r 42", &end, 10);
    assert(v == 42L);
    assert(end != NULL && *end == L'\0');

    /* [3] base=16 允许 0x / 0X 前缀 */
    end = NULL;
    v = wcstol(L"0x1F", &end, 16);
    assert(v == 31L);
    assert(end != NULL && *end == L'\0');

    end = NULL;
    v = wcstol(L"0X1f", &end, 16);
    assert(v == 31L);

    /* [3] base=16 时 0x 前缀可选 */
    end = NULL;
    v = wcstol(L"1F", &end, 16);
    assert(v == 31L);

    /* [3] base=0 时按 6.4.4.1 整数常量规则：0x 前缀 => 十六进制 */
    end = NULL;
    v = wcstol(L"0x10", &end, 0);
    assert(v == 16L);

    /* [3] base=0 时前导 0 => 八进制 */
    end = NULL;
    v = wcstol(L"010", &end, 0);
    assert(v == 8L);

    /* [3] base=0 时无前缀 => 十进制 */
    end = NULL;
    v = wcstol(L"10", &end, 0);
    assert(v == 10L);

    /* [3] 字母 a..z / A..Z 取值 10..35，base=36 */
    end = NULL;
    v = wcstol(L"z", &end, 36);
    assert(v == 35L);
    end = NULL;
    v = wcstol(L"Z", &end, 36);
    assert(v == 35L);
    end = NULL;
    v = wcstol(L"a", &end, 36);
    assert(v == 10L);

    /* [3] 只有取值 < base 的字母/数字被允许：base=10 时 'a' 不是 subject */
    end = NULL;
    v = wcstol(L"12a", &end, 10);
    assert(v == 12L);
    assert(end != NULL && *end == L'a');

    /* [3][5] 负号：结果取负 */
    end = NULL;
    v = wcstol(L"-123", &end, 10);
    assert(v == -123L);

    /* [3][5] 正号 */
    end = NULL;
    v = wcstol(L"+123", &end, 10);
    assert(v == 123L);

    /* [3] 符号在 0x 前缀之前 */
    end = NULL;
    v = wcstol(L"-0x10", &end, 16);
    assert(v == -16L);
}

static void test_subject_sequence(void)
{
    /* [4] subject sequence 是最长的、符合期望形式的初始子序列 */
    wchar_t *end = NULL;
    long v = wcstol(L"1234567890xyz", &end, 10);
    assert(v == 1234567890L);
    assert(end != NULL && *end == L'x');

    /* [4] 空串：subject 为空 */
    end = (wchar_t *)0x1;
    v = wcstol(L"", &end, 10);
    assert(v == 0L);
    /* [7] 无转换时 endptr 存 nptr */
    assert(end != NULL && *end == L'\0');

    /* [4] 全空白：subject 为空 */
    end = NULL;
    v = wcstol(L"   \t\n", &end, 10);
    assert(v == 0L);
    assert(end != NULL && *end == L'\0');

    /* [4] 首个非空白字符既非符号也非允许的字母/数字 */
    end = NULL;
    v = wcstol(L"   @123", &end, 10);
    assert(v == 0L);
    /* [7] endptr 指向 nptr（即原串开头） */
    assert(end != NULL && *end == L' ');

    /* [4][7] 首个非空白字符是 '.' */
    end = NULL;
    v = wcstol(L".5", &end, 10);
    assert(v == 0L);
    assert(end != NULL && *end == L'.');
}

static void test_endptr_null(void)
{
    /* [5][7] endptr 为 NULL 时不得解引用 */
    long v = wcstol(L"123", NULL, 10);
    assert(v == 123L);

    v = wcstol(L"abc", NULL, 10);
    assert(v == 0L);
}

static void test_unsigned_and_longlong(void)
{
    /* [2][5] wcstoul 返回 unsigned long */
    wchar_t *end = NULL;
    unsigned long uv = wcstoul(L"4294967295", &end, 10);
    assert(uv == 4294967295UL);

    /* [3] wcstoul 接受负号，结果按无符号取模（C99 6.3.1.3 转换） */
    end = NULL;
    uv = wcstoul(L"-1", &end, 10);
    assert(uv == ULONG_MAX);

    /* [2][5] wcstoll 返回 long long */
    end = NULL;
    long long llv = wcstoll(L"9223372036854775807", &end, 10);
    assert(llv == 9223372036854775807LL);

    /* [2][5] wcstoull 返回 unsigned long long */
    end = NULL;
    unsigned long long ullv = wcstoull(L"18446744073709551615", &end, 10);
    assert(ullv == 18446744073709551615ULL);

    /* [3] wcstoll base=16 */
    end = NULL;
    llv = wcstoll(L"0xFFFFFFFFFFFFFFFF", &end, 16);
    assert(llv == -1LL);   /* 按 long long 解释为全 1 */

    /* [3] wcstoull base=16 */
    end = NULL;
    ullv = wcstoull(L"0xFFFFFFFFFFFFFFFF", &end, 16);
    assert(ullv == 18446744073709551615ULL);
}

static void test_range_errors(void)
{
    /* [8] 溢出：LONG_MAX 之上 => LONG_MAX 且 errno == ERANGE */
    wchar_t *end = NULL;
    errno = 0;
    long v = wcstol(L"99999999999999999999999999", &end, 10);
    assert(v == LONG_MAX);
    assert(errno == ERANGE);

    /* [8] 下溢：LONG_MIN 之下 => LONG_MIN 且 errno == ERANGE */
    errno = 0;
    v = wcstol(L"-99999999999999999999999999", &end, 10);
    assert(v == LONG_MIN);
    assert(errno == ERANGE);

    /* [8] wcstoul 溢出 => ULONG_MAX 且 ERANGE */
    errno = 0;
    unsigned long uv = wcstoul(L"99999999999999999999999999", &end, 10);
    assert(uv == ULONG_MAX);
    assert(errno == ERANGE);

    /* [8] wcstoll 溢出 => LLONG_MAX 且 ERANGE */
    errno = 0;
    long long llv = wcstoll(L"99999999999999999999999999999999", &end, 10);
    assert(llv == LLONG_MAX);
    assert(errno == ERANGE);

    /* [8] wcstoll 下溢 => LLONG_MIN 且 ERANGE */
    errno = 0;
    llv = wcstoll(L"-99999999999999999999999999999999", &end, 10);
    assert(llv == LLONG_MIN);
    assert(errno == ERANGE);

    /* [8] wcstoull 溢出 => ULLONG_MAX 且 ERANGE */
    errno = 0;
    unsigned long long ullv = wcstoull(L"99999999999999999999999999999999", &end, 10);
    assert(ullv == ULLONG_MAX);
    assert(errno == ERANGE);

    /* [8] 无转换时返回 0，且不设置 ERANGE */
    errno = 0;
    v = wcstol(L"xyz", &end, 10);
    assert(v == 0L);
    assert(errno == 0);
}

static void test_base_2_and_36(void)
{
    /* [3] base 范围 2..36 含端点 */
    wchar_t *end = NULL;
    long v = wcstol(L"1010", &end, 2);
    assert(v == 10L);

    end = NULL;
    v = wcstol(L"zz", &end, 36);
    assert(v == 35 * 36 + 35);

    /* [3] base=8 */
    end = NULL;
    v = wcstol(L"777", &end, 8);
    assert(v == 511L);

    /* [3] base=10 时 '8' 合法，'9' 合法，'a' 不合法 */
    end = NULL;
    v = wcstol(L"89", &end, 10);
    assert(v == 89L);
}

static void test_iswspace_consistency(void)
{
    /* [2] 前导空白由 iswspace 判定 */
    assert(iswspace(L' ') != 0);
    assert(iswspace(L'\t') != 0);
    assert(iswspace(L'\n') != 0);

    wchar_t *end = NULL;
    long v = wcstol(L" \t\n\r\v\f 7", &end, 10);
    assert(v == 7L);
    assert(end != NULL && *end == L'\0');
}

int main(void)
{
    test_basic_conversion();
    test_subject_sequence();
    test_endptr_null();
    test_unsigned_and_longlong();
    test_range_errors();
    test_base_2_and_36();
    test_iswspace_consistency();

    /* 引用函数指针，避免未使用告警 */
    assert(p_wcstol == wcstol);
    assert(p_wcstoll == wcstoll);
    assert(p_wcstoul == wcstoul);
    assert(p_wcstoull == wcstoull);

    printf("All positive tests for C99 7.24.4.1.2 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须匹配原型」：wcstol 第一参数为 const wchar_t *，
 * 传入 char * 应报错（-Wincompatible-pointer-types / 错误）。 */
void neg_wrong_first_arg(void)
{
    char *s = "123";
    wcstol(s, NULL, 10);   /* 期望：编译报错，类型不兼容 */
}

/* 违反约束「实参类型必须匹配原型」：第二参数为 wchar_t **，
 * 传入 int ** 应报错。 */
void neg_wrong_endptr(void)
{
    int *p = NULL;
    wcstol(L"123", &p, 10);   /* 期望：编译报错，wchar_t ** 与 int ** 不兼容 */
}

/* 违反约束「实参类型必须匹配原型」：第三参数为 int，
 * 传入指针应报错。 */
void neg_wrong_base(void)
{
    wcstol(L"123", NULL, (void *)0);   /* 期望：编译报错，指针不能隐式转 int */
}

/* 违反约束「函数返回类型固定」：wcstol 返回 long int，
 * 不能赋给结构体类型。 */
struct S { int x; };
void neg_wrong_return(void)
{
    struct S s;
    s = wcstol(L"1", NULL, 10);   /* 期望：编译报错，long 不能赋给 struct S */
}

/* 违反约束「实参个数必须匹配原型」：wcstol 需要 3 个实参。 */
void neg_too_few_args(void)
{
    wcstol(L"123");   /* 期望：编译报错，实参太少 */
}

/* 违反约束「实参个数必须匹配原型」：wcstoull 需要 3 个实参。 */
void neg_too_many_args(void)
{
    wcstoull(L"1", NULL, 10, 20);   /* 期望：编译报错，实参太多 */
}

/* 违反约束「restrict 限定指针的实参必须为对象指针」：
 * 传入函数指针给 wchar_t ** 参数应报错。 */
void neg_func_ptr_as_endptr(void)
{
    void (*fp)(void) = 0;
    wcstol(L"1", (wchar_t **)fp, 10);   /* 期望：编译报错（不兼容指针转换） */
}

/* 违反约束「const 限定不可丢弃」：把 const wchar_t * 传给
 * 需要可写 wchar_t * 的接口（此处用 wcstol 的 endptr 语义演示）。 */
void neg_const_discard(void)
{
    const wchar_t *cs = L"123";
    wchar_t *end = NULL;
    /* 下面这行本身合法（第一参数就是 const），但把 cs 赋给非 const 指针非法： */
    wchar_t *bad = cs;   /* 期望：编译报错，丢弃 const 限定 */
    (void)bad;
    (void)end;
}

#endif /* 负向测试结束 */