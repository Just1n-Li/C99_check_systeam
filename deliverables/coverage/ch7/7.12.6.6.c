/*
 * 测试条款：C99 7.12.6.6  The ldexp functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 ldexp / ldexpf / ldexpl，
 *             验证其返回 x * 2^exp 的语义，程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数类型错误、缺少 <math.h> 声明、
 *             对返回的非左值赋值等）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型 double ldexp(double,int);
 *                  float ldexpf(float,int); long double ldexpl(long double,int);
 *   [2] Description：将浮点数乘以 2 的整数次幂；可能发生范围错误。
 *   [3] Returns：返回 x * 2^exp。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：验证三个函数原型可用，且返回类型正确 */
static void test_synopsis(void)
{
    /* 通过函数指针类型检查原型签名 */
    double (*p_ldexp)(double, int)          = ldexp;
    float  (*p_ldexpf)(float, int)          = ldexpf;
    long double (*p_ldexpl)(long double, int) = ldexpl;

    assert(p_ldexp  != NULL);
    assert(p_ldexpf != NULL);
    assert(p_ldexpl != NULL);

    /* 返回类型检查：赋值给对应类型不应有警告 */
    double      rd = ldexp(1.0, 0);
    float       rf = ldexpf(1.0f, 0);
    long double rl = ldexpl(1.0L, 0);
    assert(rd == 1.0);
    assert(rf == 1.0f);
    assert(rl == 1.0L);
}

/* [2][3] 语义：ldexp(x, exp) == x * 2^exp */
static void test_semantics_double(void)
{
    /* 基本：1.0 * 2^0 = 1.0 */
    assert(ldexp(1.0, 0) == 1.0);

    /* 1.0 * 2^1 = 2.0 */
    assert(ldexp(1.0, 1) == 2.0);

    /* 1.0 * 2^10 = 1024.0 */
    assert(ldexp(1.0, 10) == 1024.0);

    /* 负指数：1.0 * 2^-1 = 0.5 */
    assert(ldexp(1.0, -1) == 0.5);

    /* 负指数：1.0 * 2^-10 = 1/1024 */
    assert(ldexp(1.0, -10) == 1.0 / 1024.0);

    /* 一般值：3.0 * 2^4 = 48.0 */
    assert(ldexp(3.0, 4) == 48.0);

    /* 与直接乘法比较 */
    {
        double x = 1.5;
        int e;
        for (e = -20; e <= 20; ++e) {
            double expected = x;
            int i;
            if (e >= 0) {
                for (i = 0; i < e; ++i) expected *= 2.0;
            } else {
                for (i = 0; i < -e; ++i) expected /= 2.0;
            }
            assert(ldexp(x, e) == expected);
        }
    }

    /* 零：0.0 * 2^5 = 0.0 */
    assert(ldexp(0.0, 5) == 0.0);
    assert(ldexp(-0.0, 5) == 0.0);

    /* 负值：-2.0 * 2^3 = -16.0 */
    assert(ldexp(-2.0, 3) == -16.0);

    /* 无穷大：inf * 2^0 = inf */
    assert(isinf(ldexp(INFINITY, 0)));
    assert(ldexp(INFINITY, 0) > 0.0);
    assert(ldexp(-INFINITY, 0) < 0.0);

    /* NaN 传播 */
    assert(isnan(ldexp(NAN, 0)));
}

/* [2][3] 语义：ldexpf 版本 */
static void test_semantics_float(void)
{
    assert(ldexpf(1.0f, 0) == 1.0f);
    assert(ldexpf(1.0f, 1) == 2.0f);
    assert(ldexpf(1.0f, 10) == 1024.0f);
    assert(ldexpf(1.0f, -1) == 0.5f);
    assert(ldexpf(3.0f, 4) == 48.0f);
    assert(ldexpf(0.0f, 5) == 0.0f);
    assert(ldexpf(-2.0f, 3) == -16.0f);
    assert(isinf(ldexpf(INFINITY, 0)));
    assert(isnan(ldexpf(NAN, 0)));
}

/* [2][3] 语义：ldexpl 版本 */
static void test_semantics_long_double(void)
{
    assert(ldexpl(1.0L, 0) == 1.0L);
    assert(ldexpl(1.0L, 1) == 2.0L);
    assert(ldexpl(1.0L, 10) == 1024.0L);
    assert(ldexpl(1.0L, -1) == 0.5L);
    assert(ldexpl(3.0L, 4) == 48.0L);
    assert(ldexpl(0.0L, 5) == 0.0L);
    assert(ldexpl(-2.0L, 3) == -16.0L);
    assert(isinf(ldexpl(INFINITY, 0)));
    assert(isnan(ldexpl(NAN, 0)));
}

/* [2] 范围错误：极大指数导致溢出（结果可能为 HUGE_VAL 或 inf） */
static void test_range_error(void)
{
    double big = ldexp(1.0, 100000);
    /* 溢出时返回 HUGE_VAL（正无穷） */
    assert(isinf(big) || big == HUGE_VAL);

    double small = ldexp(1.0, -100000);
    /* 下溢时返回 0 或极小值 */
    assert(small == 0.0 || fabs(small) < DBL_MIN);
}

/* [2] 与 frexp 互逆（同一头文件中的相关函数，验证语义一致性） */
static void test_roundtrip(void)
{
    double x = 123.456;
    int e;
    double m = frexp(x, &e);
    assert(ldexp(m, e) == x);
}

int main(void)
{
    test_synopsis();
    test_semantics_double();
    test_semantics_float();
    test_semantics_long_double();
    test_range_error();
    test_roundtrip();

    printf("C99 7.12.6.6 ldexp: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「ldexp 的第一个参数必须为 double 类型」：
 * 传入结构体类型，gcc -std=c99 应报错（incompatible type for argument 1）。 */
struct S { int x; } s;
double bad1 = ldexp(s, 3);

/* 违反约束「ldexp 的第二个参数必须为 int 类型」：
 * 传入 double，gcc -std=c99 应报错（incompatible type for argument 2）。 */
double bad2 = ldexp(1.0, 2.5);

/* 违反约束「ldexpf 的第一个参数必须为 float 类型」：
 * 传入结构体，gcc -std=c99 应报错。 */
float bad3 = ldexpf(s, 3);

/* 违反约束「ldexpl 的第一个参数必须为 long double 类型」：
 * 传入结构体，gcc -std=c99 应报错。 */
long double bad4 = ldexpl(s, 3);

/* 违反约束「ldexp 返回非左值，不能赋值」：
 * 函数调用结果不是左值，gcc -std=c99 应报错（lvalue required as left operand of assignment）。 */
ldexp(1.0, 2) = 5.0;

/* 违反约束「ldexpf 返回非左值，不能赋值」：
 * gcc -std=c99 应报错。 */
ldexpf(1.0f, 2) = 5.0f;

/* 违反约束「ldexpl 返回非左值，不能赋值」：
 * gcc -std=c99 应报错。 */
ldexpl(1.0L, 2) = 5.0L;

/* 违反约束「ldexp 返回非左值，不能取地址」：
 * gcc -std=c99 应报错（lvalue required as unary '&' operand）。 */
double *pbad = &ldexp(1.0, 2);

/* 违反约束「ldexp 返回非左值，不能自增」：
 * gcc -std=c99 应报错（lvalue required as increment operand）。 */
ldexp(1.0, 2)++;

/* 违反约束「ldexp 返回非左值，不能作为赋值目标」：
 * gcc -std=c99 应报错。 */
ldexp(1.0, 2) += 1.0;

/* 违反约束「ldexp 参数个数必须为 2」：
 * 少传参数，gcc -std=c99 应报错（too few arguments to function 'ldexp'）。 */
double bad5 = ldexp(1.0);

/* 违反约束「ldexp 参数个数必须为 2」：
 * 多传参数，gcc -std=c99 应报错（too many arguments to function 'ldexp'）。 */
double bad6 = ldexp(1.0, 2, 3);

/* 违反约束「ldexpf 参数个数必须为 2」：
 * gcc -std=c99 应报错。 */
float bad7 = ldexpf(1.0f);

/* 违反约束「ldexpl 参数个数必须为 2」：
 * gcc -std=c99 应报错。 */
long double bad8 = ldexpl(1.0L);

#endif /* 负向测试结束 */