/*
 * 测试条款：C99 7.12.7.5  The sqrt functions
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明：double sqrt(double); float sqrtf(float); long double sqrtl(long double);
 *   [2] 计算非负平方根；参数小于零时发生 domain error。
 *   [3] 返回 (sqrt)(x)。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型声明：三个函数均声明于 <math.h>，返回类型与参数类型正确。
 *     通过取函数指针并检查其类型来验证原型存在且签名正确。 */
static void test_prototypes(void)
{
    double (*p_d)(double)          = sqrt;
    float  (*p_f)(float)           = sqrtf;
    long double (*p_l)(long double)= sqrtl;

    assert(p_d != NULL);
    assert(p_f != NULL);
    assert(p_l != NULL);

    /* 调用形式与原型一致 */
    double  rd = sqrt(4.0);
    float   rf = sqrtf(4.0f);
    long double rl = sqrtl(4.0L);

    assert(rd == 2.0);
    assert(rf == 2.0f);
    assert(rl == 2.0L);
}

/* [2] 计算非负平方根：对非负参数，结果 r 满足 r >= 0 且 r*r == x（在浮点精度内）。 */
static void test_nonnegative_root(void)
{
    double x = 9.0;
    double r = sqrt(x);
    assert(r >= 0.0);          /* 非负平方根 */
    assert(r == 3.0);

    /* 0 的平方根为 0 */
    assert(sqrt(0.0) == 0.0);
    assert(sqrtf(0.0f) == 0.0f);
    assert(sqrtl(0.0L) == 0.0L);

    /* 一般值：r*r 应约等于 x */
    double y = 2.0;
    double s = sqrt(y);
    assert(s >= 0.0);
    assert(fabs(s * s - y) < 1e-12);

    /* 大数与小数 */
    assert(fabs(sqrt(1e100) - 1e50) < 1e40);
    assert(fabs(sqrt(1e-100) - 1e-50) < 1e-60);
}

/* [2] 参数小于零时发生 domain error。
 *     标准要求 domain error 时返回实现定义的值，并设置 errno 为 EDOM（若实现支持）。
 *     这里只验证「发生 domain error」这一可观察行为：errno 被置为 EDOM。 */
static void test_domain_error(void)
{
    errno = 0;
    volatile double neg = -1.0;
    double r = sqrt(neg);
    (void)r; /* 返回值是实现定义的，不检查具体值 */
    assert(errno == EDOM);   /* 参数 < 0 应触发 domain error */

    errno = 0;
    volatile float negf = -4.0f;
    float rf = sqrtf(negf);
    (void)rf;
    assert(errno == EDOM);

    errno = 0;
    volatile long double negl = -9.0L;
    long double rl = sqrtl(negl);
    (void)rl;
    assert(errno == EDOM);
}

/* [3] 返回 (sqrt)(x)：返回值等于以 x 为参数调用 sqrt 的结果。
 *     用函数指针调用与直接调用结果一致来验证。 */
static void test_returns_sqrt_x(void)
{
    double (*fp)(double) = sqrt;
    double x = 16.0;
    assert(sqrt(x) == fp(x));
    assert(sqrt(x) == 4.0);

    /* 与恒等式 sqrt(x*x) == |x| 一致（x 非负时） */
    double a = 5.0;
    assert(sqrt(a * a) == a);
}

int main(void)
{
    test_prototypes();
    test_nonnegative_root();
    test_domain_error();
    test_returns_sqrt_x();

    printf("C99 7.12.7.5 sqrt: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「sqrt 的原型为 double sqrt(double)」：
 * 用不兼容的参数类型调用（结构体无法隐式转换为 double），
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'sqrt'。 */
struct S { int x; };
void bad_call_struct(void)
{
    struct S s;
    double r = sqrt(s);   /* 错误：实参类型与原型不兼容 */
    (void)r;
}

/* 违反约束「sqrt 返回 double」：
 * 将返回值赋给不兼容类型（结构体）应报错。 */
void bad_return_type(void)
{
    struct S s;
    s = sqrt(4.0);        /* 错误：double 不能赋给 struct S */
    (void)s;
}

/* 违反约束「sqrt 接受一个参数」：
 * 参数个数不匹配应报错。 */
void bad_arg_count(void)
{
    double r = sqrt(4.0, 9.0);  /* 错误：参数过多 */
    (void)r;
}

/* 违反约束「sqrt 接受一个参数」：
 * 缺少参数应报错。 */
void bad_arg_missing(void)
{
    double r = sqrt();    /* 错误：参数过少 */
    (void)r;
}

/* 违反约束「sqrtf 的原型为 float sqrtf(float)」：
 * 用不兼容的指针类型赋值给函数指针应报错。 */
void bad_proto_ptr(void)
{
    double (*p)(double) = sqrtf;  /* 错误：float(*)(float) 与 double(*)(double) 不兼容 */
    (void)p;
}

#endif