/*
 * 测试 C99 7.8.2.4 —— wcstoimax / wcstoumax 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <inttypes.h>、<stddef.h>（wchar_t）
 *   [2] 语义：等价于 wcstol/wcstoll/wcstoul/wcstoull，但结果为 intmax_t / uintmax_t
 *   [3] 返回值：正常转换值；无法转换返回 0；越界返回 INTMAX_MAX/INTMAX_MIN/UINTMAX_MAX 并置 errno=ERANGE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可用性检查：取函数地址，验证签名与 restrict 限定兼容 */
static intmax_t (*p_wcstoimax)(const wchar_t * restrict, wchar_t ** restrict, int) = wcstoimax;
static uintmax_t (*p_wcstoumax)(const wchar_t * restrict, wchar_t ** restrict, int) = wcstoumax;

static void test_basic_conversion(void)
{
    /* [2] 基本十进制转换，等价于 wcstol 系列但返回 intmax_t */
    const wchar_t *s = L"12345";
    wchar_t *end = NULL;
    intmax_t v = wcstoimax(s, &end, 10);
    assert(v == (intmax_t)12345);
    assert(end != NULL && *end == L'\0');

    /* [2] 无符号版本 */
    uintmax_t u = wcstoumax(L"12345", &end, 10);
    assert(u == (uintmax_t)12345);
    assert(end != NULL && *end == L'\0');
}

static void test_base_handling(void)
{
    wchar_t *end = NULL;

    /* [2] base = 0：自动识别 0x / 0 前缀 */
    assert(wcstoimax(L"0x1F", &end, 0) == (intmax_t)31);
    assert(wcstoimax(L"017",  &end, 0) == (intmax_t)15);
    assert(wcstoimax(L"42",   &end, 0) == (intmax_t)42);

    /* [2] base = 16 */
    assert(wcstoimax(L"ff", &end, 16) == (intmax_t)255);
    assert(wcstoumax(L"FF", &end, 16) == (uintmax_t)255);

    /* [2] base = 2 */
    assert(wcstoimax(L"1010", &end, 2) == (intmax_t)10);

    /* [2] base = 8 */
    assert(wcstoimax(L"777", &end, 8) == (intmax_t)511);
}

static void test_sign_and_whitespace(void)
{
    wchar_t *end = NULL;

    /* [2] 前导空白与符号，等价于 wcstol 行为 */
    assert(wcstoimax(L"   -42", &end, 10) == (intmax_t)-42);
    assert(wcstoimax(L"\t\n+7", &end, 10) == (intmax_t)7);

    /* [2] 无符号版本对 '-' 的处理（按 wcstoul 语义取模） */
    uintmax_t u = wcstoumax(L"-1", &end, 10);
    assert(u == (uintmax_t)-1); /* 即 UINTMAX_MAX */
}

static void test_endptr(void)
{
    wchar_t *end = NULL;

    /* [2] endptr 指向未转换部分 */
    intmax_t v = wcstoimax(L"123abc", &end, 10);
    assert(v == (intmax_t)123);
    assert(end != NULL && wcscmp(end, L"abc") == 0);

    /* [2] endptr 可为 NULL */
    v = wcstoimax(L"99", NULL, 10);
    assert(v == (intmax_t)99);

    /* [2] 无转换时 endptr 应等于 nptr */
    const wchar_t *bad = L"xyz";
    end = NULL;
    v = wcstoimax(bad, &end, 10);
    assert(v == 0);
    assert(end == bad);
}

static void test_no_conversion(void)
{
    wchar_t *end = NULL;

    /* [3] 无法转换时返回 0 */
    assert(wcstoimax(L"", &end, 10) == 0);
    assert(wcstoimax(L"abc", &end, 10) == 0);
    assert(wcstoumax(L"", &end, 10) == 0);
    assert(wcstoumax(L"abc", &end, 10) == 0);
}

static void test_range_errors(void)
{
    wchar_t *end = NULL;

    /* [3] 正溢出：返回 INTMAX_MAX 且 errno == ERANGE */
    errno = 0;
    intmax_t v = wcstoimax(L"999999999999999999999999999999999999999999", &end, 10);
    assert(v == INTMAX_MAX);
    assert(errno == ERANGE);

    /* [3] 负溢出：返回 INTMAX_MIN 且 errno == ERANGE */
    errno = 0;
    v = wcstoimax(L"-999999999999999999999999999999999999999999", &end, 10);
    assert(v == INTMAX_MIN);
    assert(errno == ERANGE);

    /* [3] 无符号溢出：返回 UINTMAX_MAX 且 errno == ERANGE */
    errno = 0;
    uintmax_t u = wcstoumax(L"999999999999999999999999999999999999999999", &end, 10);
    assert(u == UINTMAX_MAX);
    assert(errno == ERANGE);

    /* [3] 无符号负溢出：返回 UINTMAX_MAX 且 errno == ERANGE */
    errno = 0;
    u = wcstoumax(L"-999999999999999999999999999999999999999999", &end, 10);
    assert(u == UINTMAX_MAX);
    assert(errno == ERANGE);
}

static void test_equivalence_with_wcstol(void)
{
    /* [2] 与 wcstol 系列等价性：对可表示范围内的值结果一致 */
    const wchar_t *samples[] = { L"0", L"1", L"-1", L"123456", L"-654321", L"0x7F", L"0777" };
    size_t i;
    for (i = 0; i < sizeof(samples)/sizeof(samples[0]); ++i) {
        wchar_t *e1 = NULL, *e2 = NULL;
        long a = wcstol(samples[i], &e1, 0);
        intmax_t b = wcstoimax(samples[i], &e2, 0);
        assert((intmax_t)a == b);
        assert(e1 != NULL && e2 != NULL && wcscmp(e1, e2) == 0);
    }
}

static void test_uintmax_equivalence(void)
{
    /* [2] 与 wcstoul 等价性 */
    const wchar_t *samples[] = { L"0", L"1", L"4294967295", L"0xFF", L"0777" };
    size_t i;
    for (i = 0; i < sizeof(samples)/sizeof(samples[0]); ++i) {
        wchar_t *e1 = NULL, *e2 = NULL;
        unsigned long a = wcstoul(samples[i], &e1, 0);
        uintmax_t b = wcstoumax(samples[i], &e2, 0);
        assert((uintmax_t)a == b);
        assert(e1 != NULL && e2 != NULL && wcscmp(e1, e2) == 0);
    }
}

static void test_prototype_pointers(void)
{
    /* [1] 通过函数指针调用，验证原型与 restrict 兼容 */
    wchar_t *end = NULL;
    assert(p_wcstoimax(L"777", &end, 8) == (intmax_t)511);
    assert(p_wcstoumax(L"777", &end, 8) == (uintmax_t)511);
}

int main(void)
{
    test_basic_conversion();
    test_base_handling();
    test_sign_and_whitespace();
    test_endptr();
    test_no_conversion();
    test_range_errors();
    test_equivalence_with_wcstol();
    test_uintmax_equivalence();
    test_prototype_pointers();

    printf("C99 7.8.2.4 wcstoimax/wcstoumax: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数原型参数类型必须匹配」：
 * wcstoimax 的第一个参数是 const wchar_t *，传入 char * 应报错
 * （gcc -std=c99 应给出 incompatible pointer type 警告/错误）。 */
{
    char *s = "123";
    wchar_t *end;
    intmax_t v = wcstoimax(s, &end, 10); /* 期望：类型不兼容 */
    (void)v;
}

/* 违反约束「函数原型参数类型必须匹配」：
 * 第二个参数应为 wchar_t **，传入 char ** 应报错。 */
{
    const wchar_t *s = L"123";
    char *end;
    intmax_t v = wcstoimax(s, &end, 10); /* 期望：类型不兼容 */
    (void)v;
}

/* 违反约束「函数原型参数个数必须匹配」：
 * wcstoimax 需要 3 个参数，只传 2 个应报错。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    intmax_t v = wcstoimax(s, &end); /* 期望：too few arguments */
    (void)v;
}

/* 违反约束「函数原型参数个数必须匹配」：
 * wcstoumax 需要 3 个参数，传 4 个应报错。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    uintmax_t v = wcstoumax(s, &end, 10, 0); /* 期望：too many arguments */
    (void)v;
}

/* 违反约束「函数返回类型不可作为左值」：
 * 函数调用结果不是左值，不能赋值。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    wcstoimax(s, &end, 10) = 5; /* 期望：lvalue required as left operand of assignment */
}

/* 违反约束「函数返回类型不可取地址」：
 * 函数调用结果不是左值，不能取地址。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    intmax_t *p = &wcstoimax(s, &end, 10); /* 期望：lvalue required as unary '&' operand */
    (void)p;
}

/* 违反约束「函数返回类型不可自增」：
 * 函数调用结果不是左值，不能 ++。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    ++wcstoimax(s, &end, 10); /* 期望：lvalue required as increment operand */
}

/* 违反约束「函数返回类型不可自增」：
 * 无符号版本同理。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    wcstoumax(s, &end, 10)++; /* 期望：lvalue required as increment operand */
}

/* 违反约束「函数原型参数类型必须匹配」：
 * 第三个参数 base 应为 int，传入指针应报错。 */
{
    const wchar_t *s = L"123";
    wchar_t *end;
    int base = 10;
    intmax_t v = wcstoimax(s, &end, &base); /* 期望：类型不兼容 */
    (void)v;
}

#endif /* 负向测试结束 */