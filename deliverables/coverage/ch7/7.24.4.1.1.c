/*
 * 测试目标：C99 7.24.4.1.1 —— wcstod / wcstof / wcstold 宽字符串转浮点函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             统一放在 #if 0 ... #endif 中，保证本文件仍可编译运行。
 *
 * 覆盖段落：[1] 原型  [2] 三段分解  [3] 期望形式  [4] 解释与 endptr
 *           [5] 十六进制正确舍入  [6] 其他 locale  [7] 空/非法主体
 *           [8][9] 推荐实践  [10] 返回值/ERANGE/下溢
 */

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：三个函数存在且返回类型正确 */
static void test_prototypes(void)
{
    double (*pd)(const wchar_t *restrict, wchar_t **restrict) = wcstod;
    float  (*pf)(const wchar_t *restrict, wchar_t **restrict) = wcstof;
    long double (*pl)(const wchar_t *restrict, wchar_t **restrict) = wcstold;
    assert(pd != NULL && pf != NULL && pl != NULL);
}

/* [2] 三段分解：前导空白 + 主体 + 尾部未识别字符 */
static void test_decompose(void)
{
    wchar_t *end = NULL;
    const wchar_t *s = L"   \t\n 3.14xyz";
    double d = wcstod(s, &end);
    assert(d == 3.14);
    /* endptr 指向尾部未识别字符 'x' */
    assert(end != NULL && *end == L'x');
    assert(wcscmp(end, L"xyz") == 0);
}

/* [3] 期望形式：十进制、十六进制、INF/INFINITY、NAN */
static void test_expected_forms(void)
{
    wchar_t *end = NULL;

    /* 十进制，带符号、小数点、指数 */
    assert(wcstod(L"+1.5e2", &end) == 150.0);
    assert(wcstod(L"-0.25", &end) == -0.25);
    assert(wcstod(L"42", &end) == 42.0);
    assert(wcstod(L".5", &end) == 0.5);
    assert(wcstod(L"5.", &end) == 5.0);

    /* 十六进制浮点：0x1.8p1 == 3.0 */
    assert(wcstod(L"0x1.8p1", &end) == 3.0);
    assert(wcstod(L"0X1p4", &end) == 16.0);
    assert(wcstod(L"0x.8p1", &end) == 1.0);

    /* INF / INFINITY，大小写不敏感 */
    double inf1 = wcstod(L"INF", &end);
    double inf2 = wcstod(L"infinity", &end);
    double inf3 = wcstod(L"InFiNiTy", &end);
    assert(isinf(inf1) && inf1 > 0);
    assert(isinf(inf2) && inf2 > 0);
    assert(isinf(inf3) && inf3 > 0);

    /* -INF */
    double ninf = wcstod(L"-INF", &end);
    assert(isinf(ninf) && ninf < 0);

    /* NAN / NAN(n-char-seq)，大小写不敏感 */
    double nan1 = wcstod(L"NAN", &end);
    double nan2 = wcstod(L"nan", &end);
    double nan3 = wcstod(L"NAN(123abc)", &end);
    assert(isnan(nan1));
    assert(isnan(nan2));
    assert(isnan(nan3));

    /* 主体序列是最长初始子序列 */
    double d = wcstod(L"1.5e2abc", &end);
    assert(d == 150.0);
    assert(wcscmp(end, L"abc") == 0);
}

/* [4] 解释规则：缺省指数、负号、endptr 存储 */
static void test_interpretation(void)
{
    wchar_t *end = NULL;

    /* 十进制无指数/小数点 -> 假定指数 0 */
    assert(wcstod(L"123", &end) == 123.0);
    /* 十六进制无二进制指数 -> 假定指数 0 */
    assert(wcstod(L"0x10", &end) == 16.0);

    /* 负号取反 */
    assert(wcstod(L"-7", &end) == -7.0);

    /* endptr 可为 NULL，不应崩溃 */
    double d = wcstod(L"2.5", NULL);
    assert(d == 2.5);

    /* endptr 指向最终宽字符串 */
    const wchar_t *s = L"3.5rest";
    wcstod(s, &end);
    assert(end == s + 3);
}

/* [5] 十六进制形式且 FLT_RADIX 为 2 的幂时正确舍入 */
static void test_hex_rounding(void)
{
    wchar_t *end = NULL;
    /* 0x1.0p0 == 1.0，精确可表示 */
    assert(wcstod(L"0x1.0p0", &end) == 1.0);
    /* 0x1.8p0 == 1.5 */
    assert(wcstod(L"0x1.8p0", &end) == 1.5);
    /* 0x1p-1 == 0.5 */
    assert(wcstod(L"0x1p-1", &end) == 0.5);
}

/* [6] 其他 locale：仅验证在 "C" locale 下基本形式仍可用 */
static void test_locale(void)
{
    wchar_t *end = NULL;
    /* 在 "C" locale 下，小数点必须是 '.' */
    assert(wcstod(L"1.25", &end) == 1.25);
    assert(wcscmp(end, L"") == 0);
}

/* [7] 空主体或非法形式：不转换，endptr = nptr，返回 0 */
static void test_no_conversion(void)
{
    wchar_t *end = NULL;
    const wchar_t *s;

    /* 空字符串 */
    s = L"";
    errno = 0;
    double d = wcstod(s, &end);
    assert(d == 0.0);
    assert(end == s);

    /* 纯空白 */
    s = L"   \t\n";
    d = wcstod(s, &end);
    assert(d == 0.0);
    assert(end == s);

    /* 非法形式 */
    s = L"abc";
    d = wcstod(s, &end);
    assert(d == 0.0);
    assert(end == s);

    /* endptr 为 NULL 时也不应崩溃 */
    d = wcstod(L"xyz", NULL);
    assert(d == 0.0);
}

/* [10] 返回值：正常值、溢出 HUGE_VAL + ERANGE、下溢 */
static void test_returns(void)
{
    wchar_t *end = NULL;

    /* 正常转换 */
    assert(wcstod(L"2.5", &end) == 2.5);

    /* 溢出：返回 ±HUGE_VAL 并置 ERANGE */
    errno = 0;
    double big = wcstod(L"1e9999", &end);
    assert(isinf(big) && big > 0);
    assert(errno == ERANGE);

    errno = 0;
    double nbig = wcstod(L"-1e9999", &end);
    assert(isinf(nbig) && nbig < 0);
    assert(errno == ERANGE);

    /* wcstof 溢出：HUGE_VALF */
    errno = 0;
    float fbig = wcstof(L"1e9999", &end);
    assert(isinf(fbig) && fbig > 0);
    assert(errno == ERANGE);

    /* wcstold 溢出：HUGE_VALL */
    errno = 0;
    long double lbig = wcstold(L"1e9999", &end);
    assert(isinf(lbig) && lbig > 0);
    assert(errno == ERANGE);

    /* 下溢：返回值的量级不大于最小规格化正数 */
    errno = 0;
    double tiny = wcstod(L"1e-9999", &end);
    assert(fabs(tiny) <= DBL_MIN);
    /* errno 是否置 ERANGE 由实现定义，不做断言 */

    /* 无转换时返回 0 */
    assert(wcstod(L"zzz", &end) == 0.0);
}

/* [8][9] 推荐实践：十进制至多 DECIMAL_DIG 位有效数字应正确舍入 */
static void test_recommended(void)
{
    wchar_t *end = NULL;
    /* 简单可精确表示的值 */
    assert(wcstod(L"0.5", &end) == 0.5);
    assert(wcstod(L"0.25", &end) == 0.25);
    assert(wcstod(L"1.0", &end) == 1.0);
    /* 十六进制精确值 */
    assert(wcstod(L"0x1p0", &end) == 1.0);
}

/* 类型区分：wcstof 返回 float，wcstold 返回 long double */
static void test_types(void)
{
    wchar_t *end = NULL;
    float f = wcstof(L"1.5", &end);
    long double ld = wcstold(L"2.5", &end);
    assert(f == 1.5f);
    assert(ld == 2.5L);
}

int main(void)
{
    test_prototypes();
    test_decompose();
    test_expected_forms();
    test_interpretation();
    test_hex_rounding();
    test_locale();
    test_no_conversion();
    test_returns();
    test_recommended();
    test_types();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「wcstod 的第一个参数类型为 const wchar_t *」：
 * 传入 char* 而非 wchar_t*，gcc -std=c99 应报 incompatible pointer type 警告/错误。 */
void neg_wrong_arg_type(void)
{
    char *s = "1.5";
    wchar_t *end;
    double d = wcstod(s, &end);   /* 期望：类型不兼容报错 */
    (void)d;
}

/* 违反约束「wcstod 的第二个参数类型为 wchar_t **」：
 * 传入 char** 而非 wchar_t**，应报类型不兼容。 */
void neg_wrong_endptr_type(void)
{
    char *end;
    double d = wcstod(L"1.5", &end);  /* 期望：类型不兼容报错 */
    (void)d;
}

/* 违反约束「wcstod 返回 double」：
 * 把返回值赋给结构体类型，应报类型不兼容。 */
struct S { int x; };
void neg_wrong_return(void)
{
    struct S s;
    s = wcstod(L"1.5", NULL);   /* 期望：不能把 double 赋给 struct S */
}

/* 违反约束「wcstod 需要两个参数」：
 * 参数个数不足，应报 too few arguments。 */
void neg_too_few_args(void)
{
    double d = wcstod(L"1.5");   /* 期望：参数太少报错 */
    (void)d;
}

/* 违反约束「wcstod 需要两个参数」：
 * 参数个数过多，应报 too many arguments。 */
void neg_too_many_args(void)
{
    wchar_t *end;
    double d = wcstod(L"1.5", &end, 0);  /* 期望：参数太多报错 */
    (void)d;
}

/* 违反约束「wcstod 的第一个参数为指针类型」：
 * 传入整数，应报类型不兼容。 */
void neg_non_pointer_arg(void)
{
    double d = wcstod(42, NULL);   /* 期望：整数不能隐式转为指针 */
    (void)d;
}

/* 违反约束「wcstod 的第二个参数为 wchar_t **」：
 * 传入 wchar_t*（少一级指针），应报类型不兼容。 */
void neg_endptr_one_level(void)
{
    wchar_t buf[16];
    double d = wcstod(L"1.5", buf);   /* 期望：wchar_t* 不能转为 wchar_t** */
    (void)d;
}

#endif /* 负向测试结束 */