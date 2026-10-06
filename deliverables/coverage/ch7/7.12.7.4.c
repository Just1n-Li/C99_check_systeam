/*
 * 测试 C99 7.12.7.4 —— pow 函数族 (pow / powf / powl)
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 pow/powf/powl，验证返回值语义，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 说明：条款 [3] 原文在标准中为 "return x^y"（此处文本被截断为 "return x"），
 *       按标准语义 pow(x,y) 返回 x 的 y 次幂。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* 辅助：判断两个 double 是否近似相等 */
static int dclose(double a, double b)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= 1e-9 * (1.0 + (b < 0 ? -b : b));
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：三个函数均声明于 <math.h>，原型为
 *     double pow(double, double);
 *     float  powf(float, float);
 *     long double powl(long double, long double);
 *     通过取函数指针类型来验证原型签名。 */
static void test_synopsis(void)
{
    double (*pd)(double, double)          = pow;
    float  (*pf)(float, float)            = powf;
    long double (*pl)(long double, long double) = powl;

    assert(pd != NULL);
    assert(pf != NULL);
    assert(pl != NULL);
    printf("[1] Synopsis: pow/powf/powl 原型签名正确\n");
}

/* [2] Description：pow 计算 x 的 y 次幂（正常定义域内） */
static void test_description_normal(void)
{
    /* 2^10 = 1024 */
    assert(dclose(pow(2.0, 10.0), 1024.0));
    /* 9^0.5 = 3 */
    assert(dclose(pow(9.0, 0.5), 3.0));
    /* 任意非零数的 0 次幂 = 1 */
    assert(dclose(pow(5.0, 0.0), 1.0));
    assert(dclose(pow(-5.0, 0.0), 1.0));
    /* 1 的任意次幂 = 1 */
    assert(dclose(pow(1.0, 123.456), 1.0));
    /* 负底数 + 整数指数：定义域内，结果有定义 */
    assert(dclose(pow(-2.0, 3.0), -8.0));
    assert(dclose(pow(-2.0, 4.0), 16.0));
    printf("[2] Description: 正常定义域内 pow 计算正确\n");
}

/* [2] Description：x 为有限负数且 y 为有限非整数 -> 定义域错误 */
static void test_domain_error_neg_noninteger(void)
{
    double r = pow(-2.0, 0.5);   /* 定义域错误 */
    assert(isnan(r));            /* 实现应返回 NaN */
    printf("[2] Domain error: pow(-2.0, 0.5) = NaN (errno=%d)\n", errno);
}

/* [2] Description：x 为 0 且 y 为 0 -> 可能发生定义域错误 */
static void test_domain_error_zero_zero(void)
{
    /* 标准说 "may occur"，因此只检查返回值是 NaN 或 1.0 之一 */
    double r = pow(0.0, 0.0);
    assert(isnan(r) || dclose(r, 1.0));
    printf("[2] pow(0.0, 0.0) = %g (允许 NaN 或 1.0)\n", r);
}

/* [2] Description：x 为 0 且 y < 0 -> 可能定义域错误或范围错误 */
static void test_domain_or_range_zero_neg(void)
{
    double r = pow(0.0, -1.0);
    /* 允许 NaN（定义域错误）或 +inf（范围错误/极点） */
    assert(isnan(r) || isinf(r));
    printf("[2] pow(0.0, -1.0) = %g (允许 NaN 或 inf)\n", r);
}

/* [2] Description：范围错误可能发生（结果溢出） */
static void test_range_error_overflow(void)
{
    double r = pow(DBL_MAX, 2.0);   /* 溢出 -> 范围错误 */
    assert(isinf(r) && r > 0);
    printf("[2] Range error: pow(DBL_MAX, 2.0) = %g (inf)\n", r);
}

/* [3] Returns：pow 返回 x 的 y 次幂；powf/powl 同理 */
static void test_returns(void)
{
    /* double 版本 */
    assert(dclose(pow(3.0, 4.0), 81.0));

    /* float 版本 */
    float rf = powf(2.0f, 8.0f);
    assert(rf == 256.0f);

    /* long double 版本 */
    long double rl = powl(2.0L, 16.0L);
    assert(rl == 65536.0L);

    printf("[3] Returns: pow=81, powf=256, powl=65536 均正确\n");
}

/* [3] Returns：验证返回类型分别为 double / float / long double */
static void test_return_types(void)
{
    /* 用 sizeof 检查返回类型大小（在常见平台上 double=8, float=4, long double>=8） */
    assert(sizeof(pow(2.0, 2.0))  == sizeof(double));
    assert(sizeof(powf(2.0f, 2.0f)) == sizeof(float));
    assert(sizeof(powl(2.0L, 2.0L)) == sizeof(long double));
    printf("[3] Return types: double/float/long double 尺寸正确\n");
}

int main(void)
{
    test_synopsis();
    test_description_normal();
    test_domain_error_neg_noninteger();
    test_domain_error_zero_zero();
    test_domain_or_range_zero_neg();
    test_range_error_overflow();
    test_returns();
    test_return_types();

    printf("\n所有正向测试通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「pow 的参数必须为算术类型（double）」：
 * 传入结构体类型，gcc -std=c99 应报错：
 *   error: incompatible type for argument 1 of 'pow' */
struct S { int x; } s;
pow(s, 2.0);

/* 违反约束「pow 的参数个数必须为 2」：
 * 参数个数不匹配，应报错：
 *   error: too few arguments to function 'pow' */
pow(2.0);

/* 违反约束「pow 的参数个数必须为 2」：
 * 参数过多，应报错：
 *   error: too many arguments to function 'pow' */
pow(2.0, 3.0, 4.0);

/* 违反约束「pow 的参数必须为算术类型」：
 * 传入指针类型，应报错：
 *   error: incompatible type for argument 1 of 'pow' */
double *p = 0;
pow(p, 2.0);

/* 违反约束「powf 的参数必须为算术类型」：
 * 传入结构体，应报错 */
powf(s, 2.0f);

/* 违反约束「powl 的参数必须为算术类型」：
 * 传入结构体，应报错 */
powl(s, 2.0L);

/* 违反约束「pow 的返回值不可作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错：
 *   error: lvalue required as left operand of assignment */
pow(2.0, 3.0) = 5.0;

/* 违反约束「powf 的返回值不可作为左值赋值」：
 * 应报错：lvalue required as left operand of assignment */
powf(2.0f, 3.0f) = 5.0f;

/* 违反约束「powl 的返回值不可作为左值赋值」：
 * 应报错：lvalue required as left operand of assignment */
powl(2.0L, 3.0L) = 5.0L;

#endif