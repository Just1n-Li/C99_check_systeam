/*
 * 测试 C99 7.20.1.3 —— strtod / strtof / strtold
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反约束，gcc -std=c99 应报错。
 *
 * 覆盖段落：[1] 原型、[2] 三段分解、[3] 主题序列形式、[4] 转换与 endptr、
 *           [5] 十六进制正确舍入、[6] 其他 locale、[7] 空/非法主题序列、
 *           [8][9] 推荐实践、[10] 返回值/ERANGE、脚注 258/259。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在且返回类型正确：double / float / long double */
static void test_prototypes(void)
{
    double (*pd)(const char *restrict, char **restrict) = strtod;
    float  (*pf)(const char *restrict, char **restrict) = strtof;
    long double (*pl)(const char *restrict, char **restrict) = strtold;
    assert(pd != NULL && pf != NULL && pl != NULL);
}

/* [2] 三段分解：前导空白 + 主题序列 + 尾部未识别字符 */
static void test_three_parts(void)
{
    char *end;
    const char *s = "  \t\n 3.14xyz";
    double d = strtod(s, &end);
    assert(d == 3.14);
    /* endptr 指向尾部未识别字符 'x' */
    assert(end != NULL && *end == 'x');
    assert(strcmp(end, "xyz") == 0);
}

/* [3] 十进制形式：可选符号 + 数字（可含小数点）+ 可选指数 */
static void test_decimal_forms(void)
{
    char *end;
    assert(strtod("42", &end) == 42.0 && *end == '\0');
    assert(strtod("+42", &end) == 42.0 && *end == '\0');
    assert(strtod("-42", &end) == -42.0 && *end == '\0');
    assert(strtod("3.14", &end) == 3.14 && *end == '\0');
    assert(strtod(".5", &end) == 0.5 && *end == '\0');
    assert(strtod("5.", &end) == 5.0 && *end == '\0');
    assert(strtod("1e3", &end) == 1000.0 && *end == '\0');
    assert(strtod("1E3", &end) == 1000.0 && *end == '\0');
    assert(strtod("1.5e-2", &end) == 0.015 && *end == '\0');
    assert(strtod("1e+2", &end) == 100.0 && *end == '\0');
}

/* [3] 十六进制形式：0x/0X + 十六进制数字（可含小数点）+ 可选二进制指数 */
static void test_hex_forms(void)
{
    char *end;
    assert(strtod("0x1p4", &end) == 16.0 && *end == '\0');
    assert(strtod("0X1P4", &end) == 16.0 && *end == '\0');
    assert(strtod("0x1.8p1", &end) == 3.0 && *end == '\0');
    assert(strtod("0x.8p1", &end) == 1.0 && *end == '\0');
    assert(strtod("0x1.p4", &end) == 16.0 && *end == '\0');
    /* 无二进制指数时假定指数为 0 */
    assert(strtod("0x10", &end) == 16.0 && *end == '\0');
}

/* [3] INF / INFINITY（忽略大小写） */
static void test_inf(void)
{
    char *end;
    double d = strtod("INF", &end);
    assert(isinf(d) && d > 0 && *end == '\0');
    d = strtod("infinity", &end);
    assert(isinf(d) && d > 0 && *end == '\0');
    d = strtod("-Inf", &end);
    assert(isinf(d) && d < 0 && *end == '\0');
    d = strtod("+INFINITY", &end);
    assert(isinf(d) && d > 0 && *end == '\0');
}

/* [3][4] NAN / NAN(n-char-sequence)（忽略大小写） */
static void test_nan(void)
{
    char *end;
    double d = strtod("NAN", &end);
    assert(isnan(d) && *end == '\0');
    d = strtod("nan", &end);
    assert(isnan(d) && *end == '\0');
    d = strtod("NAN(abc123)", &end);
    assert(isnan(d) && *end == '\0');
    d = strtod("-nan(0x1)", &end);
    assert(isnan(d) && *end == '\0');
}

/* [3] 主题序列是最长初始子序列 */
static void test_longest_subsequence(void)
{
    char *end;
    double d = strtod("1.5e3abc", &end);
    assert(d == 1500.0);
    assert(strcmp(end, "abc") == 0);

    d = strtod("0x1p4zzz", &end);
    assert(d == 16.0);
    assert(strcmp(end, "zzz") == 0);

    /* "1e" 中 'e' 后无数字，主题序列为 "1"，'e' 属于尾部 */
    d = strtod("1e", &end);
    assert(d == 1.0);
    assert(strcmp(end, "e") == 0);
}

/* [4] 负号：主题序列以 '-' 开头时结果取负 */
static void test_negation(void)
{
    char *end;
    assert(strtod("-3.5", &end) == -3.5);
    assert(strtod("-0x1p4", &end) == -16.0);
    /* 脚注 258：支持有符号零时应保留零的符号 */
    double z = strtod("-0.0", &end);
    assert(z == 0.0);
    assert(signbit(z));
}

/* [4] endptr 为 NULL 时不应写入 */
static void test_null_endptr(void)
{
    double d = strtod("2.5", NULL);
    assert(d == 2.5);
    d = strtod("abc", NULL);
    assert(d == 0.0);
}

/* [4] 十进制无指数/小数点时假定指数为 0；十六进制无二进制指数时假定为 0 */
static void test_assumed_exponent(void)
{
    char *end;
    assert(strtod("123", &end) == 123.0);
    assert(strtod("0x1f", &end) == 31.0);
}

/* [5] 十六进制形式且 FLT_RADIX 为 2 的幂时正确舍入 */
static void test_hex_correct_rounding(void)
{
    char *end;
    /* 0x1.0000000000001p0 在 double 中应正确舍入 */
    double d = strtod("0x1.0000000000001p0", &end);
    assert(d == 1.0 + 0x1p-52);
    /* 0x1.00000000000008p0 恰为两个 double 的中点，按当前舍入方向 */
    d = strtod("0x1.00000000000008p0", &end);
    assert(d == 1.0 || d == 1.0 + 0x1p-52);
}

/* [6] 其他 locale：C locale 下至少应接受标准形式（此处仅验证 C locale 行为） */
static void test_locale_c(void)
{
    char *end;
    /* C locale 下小数点字符为 '.' */
    double d = strtod("1.25", &end);
    assert(d == 1.25 && *end == '\0');
}

/* [7] 主题序列为空或形式不符：不转换，endptr 存 nptr，返回 0 */
static void test_no_conversion(void)
{
    char *end;
    const char *s1 = "abc";
    double d = strtod(s1, &end);
    assert(d == 0.0);
    assert(end == s1);

    const char *s2 = "   ";
    d = strtod(s2, &end);
    assert(d == 0.0);
    assert(end == s2);

    const char *s3 = "";
    d = strtod(s3, &end);
    assert(d == 0.0);
    assert(end == s3);

    /* "+" 单独出现：无数字，形式不符 */
    const char *s4 = "+";
    d = strtod(s4, &end);
    assert(d == 0.0);
    assert(end == s4);

    /* "0x" 后无十六进制数字：形式不符 */
    const char *s5 = "0x";
    d = strtod(s5, &end);
    assert(d == 0.0);
    assert(end == s5);
}

/* [10] 溢出：返回 ±HUGE_VAL，errno 置 ERANGE */
static void test_overflow(void)
{
    char *end;
    errno = 0;
    double d = strtod("1e400", &end);
    assert(isinf(d) && d > 0);
    assert(errno == ERANGE);

    errno = 0;
    d = strtod("-1e400", &end);
    assert(isinf(d) && d < 0);
    assert(errno == ERANGE);

    errno = 0;
    float f = strtof("1e40", &end);
    assert(isinf(f) && f > 0);
    assert(errno == ERANGE);

    errno = 0;
    long double ld = strtold("1e5000L", &end);
    assert(isinf(ld) && ld > 0);
    assert(errno == ERANGE);
}

/* [10] 下溢：返回值量级不大于最小规格化正数 */
static void test_underflow(void)
{
    char *end;
    errno = 0;
    double d = strtod("1e-400", &end);
    assert(fabs(d) <= DBL_MIN);
    /* errno 是否为 ERANGE 由实现定义，不做断言 */
}

/* [10] 无转换时返回 0 */
static void test_return_zero(void)
{
    char *end;
    assert(strtod("xyz", &end) == 0.0);
    assert(strtof("xyz", &end) == 0.0f);
    assert(strtold("xyz", &end) == 0.0L);
}

/* [10] 正常转换返回值 */
static void test_normal_return(void)
{
    char *end;
    assert(strtod("2.5", &end) == 2.5);
    assert(strtof("2.5", &end) == 2.5f);
    assert(strtold("2.5", &end) == 2.5L);
}

/* [8][9] 推荐实践：十进制至多 DECIMAL_DIG 位有效数字应正确舍入 */
static void test_recommended_decimal(void)
{
    char *end;
    /* 1.0 的十进制表示 */
    double d = strtod("1.0", &end);
    assert(d == 1.0);
    /* 0.5 精确可表示 */
    d = strtod("0.5", &end);
    assert(d == 0.5);
    /* 0.1 舍入到最近的 double */
    d = strtod("0.1", &end);
    assert(d == 0.1);
}

/* 脚注 259：NAN(n-char-sequence) 的 n-char 含义由实现定义，仅验证可解析 */
static void test_nan_nchar(void)
{
    char *end;
    double d = strtod("NAN(123abc)", &end);
    assert(isnan(d));
    assert(*end == '\0');
}

int main(void)
{
    test_prototypes();
    test_three_parts();
    test_decimal_forms();
    test_hex_forms();
    test_inf();
    test_nan();
    test_longest_subsequence();
    test_negation();
    test_null_endptr();
    test_assumed_exponent();
    test_hex_correct_rounding();
    test_locale_c();
    test_no_conversion();
    test_overflow();
    test_underflow();
    test_return_zero();
    test_normal_return();
    test_recommended_decimal();
    test_nan_nchar();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strtod 的第一个参数类型为 const char *」：
 * 传入 int 类型实参，gcc -std=c99 应报 incompatible type 错误。 */
{
    int x = 0;
    strtod(x, NULL);
}

/* 违反约束「strtod 的第二个参数类型为 char **」：
 * 传入 int * 类型实参，gcc -std=c99 应报 incompatible type 错误。 */
{
    int *p = NULL;
    strtod("1.0", p);
}

/* 违反约束「strtof 返回 float」：
 * 将返回值赋给不兼容的指针类型，gcc -std=c99 应报错。 */
{
    int *q = strtof("1.0", NULL);
}

/* 违反约束「strtold 返回 long double」：
 * 将返回值赋给不兼容的结构体类型，gcc -std=c99 应报错。 */
{
    struct S { int a; } s;
    s = strtold("1.0", NULL);
}

/* 违反约束「strtod 需要两个实参」：
 * 只传一个实参，gcc -std=c99 应报 too few arguments 错误。 */
{
    strtod("1.0");
}

/* 违反约束「strtod 至多两个实参」：
 * 传三个实参，gcc -std=c99 应报 too many arguments 错误。 */
{
    strtod("1.0", NULL, NULL);
}

/* 违反约束「restrict 限定符只能用于指针类型」：
 * 对非指针类型使用 restrict，gcc -std=c99 应报错。 */
{
    restrict int r = 0;
}

#endif