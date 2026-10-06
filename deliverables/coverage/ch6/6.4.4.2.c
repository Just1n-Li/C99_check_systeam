/*
 * 测试 C99 6.4.4.2 —— Floating constants（浮点常量）
 *
 * 预期行为：
 *   正向测试：以下使用各种合法浮点常量形式的代码应能编译并运行通过，
 *             且运行结果符合条款 [3][4][5] 的语义。
 *   负向测试：违反语法/约束的浮点常量写法应导致编译报错（放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：[1] 语法、[2] 描述、[3] 语义、[4] 类型、[5] 翻译期转换、
 *           [6] 推荐实践、[7] 推荐实践。
 */

#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1][2] 十进制浮点常量：fractional-constant 形式
 *   digit-sequence . digit-sequence
 *   digit-sequence .
 *   . digit-sequence
 */
static double dec_frac_1 = 1.5;      /* 整数部分.小数部分 */
static double dec_frac_2 = 1.;       /* 整数部分.        */
static double dec_frac_3 = .5;       /* .小数部分        */

/* [1][2] 十进制浮点常量：带 exponent-part
 *   digit-sequence exponent-part
 *   fractional-constant exponent-part
 */
static double dec_exp_1 = 1e3;       /* 1 * 10^3 */
static double dec_exp_2 = 1.5e2;     /* 1.5 * 10^2 */
static double dec_exp_3 = 1.5E-2;    /* 1.5 * 10^-2 */
static double dec_exp_4 = 1e+3;      /* 带正号指数 */

/* [1][4] 后缀 f/F -> float，l/L -> long double，无后缀 -> double */
static float       suf_f = 1.5f;
static float       suf_F = 1.5F;
static long double suf_l = 1.5l;
static long double suf_L = 1.5L;
static double      suf_none = 1.5;

/* [1][2] 十六进制浮点常量：hexadecimal-prefix hexadecimal-fractional-constant
 *   binary-exponent-part [floating-suffix]
 *   注意：十六进制浮点常量必须有 binary-exponent-part（p/P）。
 */
static double hex_frac_1 = 0x1.8p1;   /* 1.5 * 2^1 = 3.0 */
static double hex_frac_2 = 0x1p4;     /* 1 * 2^4 = 16.0 */
static double hex_frac_3 = 0x.8p1;    /* 0.5 * 2^1 = 1.0 */
static double hex_frac_4 = 0x1.p4;    /* 1 * 2^4 = 16.0 */
static double hex_frac_5 = 0x1.8P-1;  /* 1.5 * 2^-1 = 0.75 */
static float  hex_f     = 0x1.8p1f;   /* 后缀 f */
static long double hex_l = 0x1.8p1L;  /* 后缀 L */

/* [3] 语义：十进制常量按 10 的幂缩放 */
static void test_decimal_semantics(void)
{
    /* [3] 十进制：指数表示 10 的幂 */
    assert(dec_frac_1 == 1.5);
    assert(dec_frac_2 == 1.0);
    assert(dec_frac_3 == 0.5);

    assert(dec_exp_1 == 1000.0);
    assert(dec_exp_2 == 150.0);
    assert(dec_exp_3 == 0.015);
    assert(dec_exp_4 == 1000.0);

    /* [3] 十进制常量结果是最接近的可表示值（或相邻值），
     *     这里用可精确表示的值验证 */
    assert(0.5 == 1.0 / 2.0);
    assert(0.25 == 1.0 / 4.0);
    assert(2.5 == 5.0 / 2.0);
}

/* [3] 语义：十六进制常量按 2 的幂缩放；当 FLT_RADIX 是 2 的幂时正确舍入 */
static void test_hex_semantics(void)
{
    /* [3] 十六进制：指数表示 2 的幂 */
    assert(hex_frac_1 == 3.0);
    assert(hex_frac_2 == 16.0);
    assert(hex_frac_3 == 1.0);
    assert(hex_frac_4 == 16.0);
    assert(hex_frac_5 == 0.75);

    /* [3] 当 FLT_RADIX 是 2 的幂时，结果正确舍入。
     *     0x1.0p0 == 1.0，0x1.0p-1 == 0.5 等可精确表示。 */
    assert(0x1.0p0 == 1.0);
    assert(0x1.0p-1 == 0.5);
    assert(0x1.0p1 == 2.0);
    assert(0x1.0p10 == 1024.0);

    /* 十六进制与十进制表示同一个值 */
    assert(0x1.8p1 == 1.5 * 2.0);
    assert(0x1.8p1 == 3.0);
}

/* [4] 类型：无后缀 double，f/F float，l/L long double */
static void test_types(void)
{
    /* [4] 无后缀浮点常量类型为 double */
    assert(sizeof(1.5) == sizeof(double));
    assert(sizeof(1e3) == sizeof(double));
    assert(sizeof(0x1.8p1) == sizeof(double));

    /* [4] f/F 后缀类型为 float */
    assert(sizeof(1.5f) == sizeof(float));
    assert(sizeof(1.5F) == sizeof(float));
    assert(sizeof(0x1.8p1f) == sizeof(float));

    /* [4] l/L 后缀类型为 long double */
    assert(sizeof(1.5l) == sizeof(long double));
    assert(sizeof(1.5L) == sizeof(long double));
    assert(sizeof(0x1.8p1L) == sizeof(long double));

    /* 静态变量类型检查 */
    assert(sizeof(suf_f) == sizeof(float));
    assert(sizeof(suf_F) == sizeof(float));
    assert(sizeof(suf_l) == sizeof(long double));
    assert(sizeof(suf_L) == sizeof(long double));
    assert(sizeof(suf_none) == sizeof(double));
    assert(sizeof(hex_f) == sizeof(float));
    assert(sizeof(hex_l) == sizeof(long double));
}

/* [5] 翻译期转换：浮点常量在翻译期转换为内部格式，
 *     执行期不应引发浮点异常。这里验证常量值在运行期可用且正确。 */
static void test_translation_time_conversion(void)
{
    /* [5] 常量在翻译期已转换，运行期直接使用其值 */
    double a = 3.14159265358979323846;
    double b = 2.71828182845904523536;
    assert(a > 3.14 && a < 3.15);
    assert(b > 2.71 && b < 2.72);

    /* [5] 使用常量不应引发浮点异常（正常值） */
    double c = 1.0e10;
    double d = 1.0e-10;
    assert(c == 10000000000.0);
    assert(d == 0.0000000001);
}

/* [6] 推荐实践：十六进制常量若不能精确表示，实现应产生诊断信息
 *     （这是推荐实践，不是约束，因此这里只做正向验证：
 *      可精确表示的十六进制常量应能正常使用） */
static void test_hex_exact(void)
{
    /* 这些十六进制常量在 double 中可精确表示 */
    assert(0x1.0p0 == 1.0);
    assert(0x1.0p1 == 2.0);
    assert(0x1.0p2 == 4.0);
    assert(0x1.8p0 == 1.5);
    assert(0x1.4p0 == 1.25);
}

/* [7] 推荐实践：翻译期转换应与 strtod 等库函数执行期转换一致。
 *     这里用可精确表示的常量验证一致性。 */
static void test_strtod_consistency(void)
{
    /* 对于可精确表示的十进制常量，翻译期与 strtod 结果应一致 */
    const char *s1 = "1.5";
    const char *s2 = "0.25";
    const char *s3 = "100.0";

    assert(1.5 == strtod(s1, NULL));
    assert(0.25 == strtod(s2, NULL));
    assert(100.0 == strtod(s3, NULL));

    /* 十六进制字符串转换（C99 strtod 支持十六进制） */
    assert(0x1.8p1 == strtod("0x1.8p1", NULL));
    assert(0x1.0p4 == strtod("0x1.0p4", NULL));
}

int main(void)
{
    test_decimal_semantics();
    test_hex_semantics();
    test_types();
    test_translation_time_conversion();
    test_hex_exact();
    test_strtod_consistency();

    printf("C99 6.4.4.2 floating constants: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反 [1] 语法：十进制浮点常量必须有小数点或指数部分。
 * "1" 是整数常量，不是浮点常量；下面试图把整数当浮点常量用不会报错，
 * 但 "1e" 缺少指数数字序列，违反 exponent-part 语法。
 * 期望：gcc -std=c99 报错（invalid suffix 或 exponent has no digits）。 */
double bad1 = 1e;

/* 违反 [1] 语法：exponent-part 的 e/E 后必须有（可选符号的）digit-sequence。
 * 期望：编译报错。 */
double bad2 = 1.5e+;

/* 违反 [1] 语法：十六进制浮点常量必须有 binary-exponent-part（p/P）。
 * "0x1.8" 缺少 p 指数部分，不是合法的十六进制浮点常量。
 * 期望：编译报错（hexadecimal floating constants require an exponent）。 */
double bad3 = 0x1.8;

/* 违反 [1] 语法：十六进制浮点常量的 binary-exponent-part 必须有数字。
 * 期望：编译报错。 */
double bad4 = 0x1.8p;

/* 违反 [1] 语法：floating-suffix 只能是 f/F/l/L 之一。
 * "1.5x" 中的 x 不是合法后缀。
 * 期望：编译报错（invalid suffix "x" on floating constant）。 */
double bad5 = 1.5x;

/* 违反 [1] 语法：十六进制浮点常量的后缀同样只能是 f/F/l/L。
 * 期望：编译报错。 */
double bad6 = 0x1.8p1z;

/* 违反 [1] 语法：十进制浮点常量中不能出现十六进制数字。
 * "1.5a" 中的 a 不是合法后缀也不是合法数字。
 * 期望：编译报错。 */
double bad7 = 1.5a;

/* 违反 [1] 语法：小数点后不能直接跟指数符号而没有数字。
 * 期望：编译报错。 */
double bad8 = 1.e;

/* 违反 [1] 语法：十六进制浮点常量的小数点两侧不能都为空。
 * "0x.p1" 既无整数部分也无小数部分。
 * 期望：编译报错。 */
double bad9 = 0x.p1;

/* 违反 [1] 语法：binary-exponent-part 的 p/P 后必须有数字。
 * 期望：编译报错。 */
double bad10 = 0x1p;

#endif