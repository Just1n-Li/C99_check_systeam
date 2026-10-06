/*
 * 测试条款：C99 7.12.6.9  The log1p functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 log1p / log1pf / log1pl，
 *             验证返回值等于 ln(1+x)，并验证小量 x 时 log1p(x) 比 log(1+x) 更精确（脚注 209）。
 *   负向测试：违反约束的代码（如参数个数错误、对非算术类型调用等）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] 原型声明（double log1p(double); float log1pf(float); long double log1pl(long double);）
 *   [2] 语义：计算 ln(1+x)；x < -1 定义域错误；x == -1 可能范围错误
 *   [3] 返回值：loge(1+x)
 *   脚注 209：小量 x 时 log1p(x) 比 log(1+x) 更精确
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证三个函数的原型存在且返回类型正确 */
static void test_prototypes(void)
{
    /* 通过函数指针类型检查原型签名 */
    double (*p1)(double)          = log1p;
    float  (*p2)(float)           = log1pf;
    long double (*p3)(long double) = log1pl;

    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(p3 != NULL);

    /* 调用一次，确保链接成功 */
    volatile double r1 = log1p(0.0);
    volatile float  r2 = log1pf(0.0f);
    volatile long double r3 = log1pl(0.0L);
    (void)r1; (void)r2; (void)r3;
}

/* [2][3] 验证 log1p(x) == ln(1+x) 的语义 */
static void test_semantics(void)
{
    /* x = 0 时，ln(1+0) = 0 */
    assert(log1p(0.0) == 0.0);

    /* x = e - 1 时，ln(1 + (e-1)) = ln(e) = 1 */
    double e_minus_1 = exp(1.0) - 1.0;
    double r = log1p(e_minus_1);
    assert(fabs(r - 1.0) < 1e-12);

    /* 一般值：与 log(1+x) 比较（在非小量时二者接近） */
    double x = 1.0;
    double expected = log(1.0 + x);
    double got = log1p(x);
    assert(fabs(got - expected) < 1e-12);

    /* 负值但 > -1：ln(1 + (-0.5)) = ln(0.5) */
    double rn = log1p(-0.5);
    assert(fabs(rn - log(0.5)) < 1e-12);

    /* float 版本 */
    float xf = 1.0f;
    float rf = log1pf(xf);
    assert(fabsf(rf - logf(1.0f + xf)) < 1e-5f);

    /* long double 版本 */
    long double xl = 1.0L;
    long double rl = log1pl(xl);
    assert(fabsl(rl - logl(1.0L + xl)) < 1e-15L);
}

/* [2] 定义域错误：x < -1 时发生 domain error（返回 NaN，errno 可能为 EDOM） */
static void test_domain_error(void)
{
    errno = 0;
    double r = log1p(-2.0);   /* x < -1，定义域错误 */
    /* 标准要求发生 domain error；实现通常返回 NaN */
    assert(isnan(r) || errno == EDOM);
}

/* [2] 范围错误：x == -1 时可能发生 range error（返回 -HUGE_VAL） */
static void test_range_error(void)
{
    errno = 0;
    double r = log1p(-1.0);   /* ln(0) = -inf，可能 range error */
    /* 实现通常返回 -HUGE_VAL 或 -inf */
    assert(isinf(r) && r < 0.0);
}

/* 脚注 209：小量 x 时 log1p(x) 比 log(1+x) 更精确 */
static void test_footnote_209(void)
{
    /* 取一个非常小的 x，使得 1+x 在 double 中丢失精度 */
    double x = 1e-16;
    double a = log1p(x);        /* 精确：约等于 x */
    double b = log(1.0 + x);    /* 1.0 + 1e-16 舍入为 1.0，log(1.0)=0 */

    /* log1p 应给出接近 x 的结果 */
    assert(fabs(a - x) < 1e-20);

    /* log(1+x) 因舍入丢失信息，结果与 x 相差较大 */
    assert(fabs(b - x) > fabs(a - x));

    /* 更极端：x = DBL_EPSILON/2 */
    double y = DBL_EPSILON / 2.0;
    double ay = log1p(y);
    double by = log(1.0 + y);
    assert(fabs(ay - y) < 1e-30);
    assert(fabs(by - y) > fabs(ay - y));
}

int main(void)
{
    test_prototypes();
    test_semantics();
    test_domain_error();
    test_range_error();
    test_footnote_209();

    printf("All positive tests for C99 7.12.6.9 (log1p) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用参数个数必须与原型一致」：
 * log1p 原型为 double log1p(double)，传入两个参数应报错。
 * 期望：gcc -std=c99 报 "too many arguments to function 'log1p'" */
double bad1 = log1p(1.0, 2.0);

/* 违反约束「函数调用参数个数必须与原型一致」：
 * log1p 需要一个参数，不传参数应报错。
 * 期望：gcc -std=c99 报 "too few arguments to function 'log1p'" */
double bad2 = log1p();

/* 违反约束「实参类型必须可转换为形参类型」：
 * 传入结构体类型，无法转换为 double，应报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 1 of 'log1p'" */
struct S { int x; } s;
double bad3 = log1p(s);

/* 违反约束「函数名必须已声明」：
 * 未包含 <math.h> 且未声明 log1p 时调用，C99 下隐式声明被禁止（约束违反）。
 * 期望：gcc -std=c99 报 "implicit declaration of function 'log1p'" */
/* 注意：此处已在文件顶部包含 <math.h>，故用未声明的名字演示 */
double bad4 = log1p_undeclared(1.0);

/* 违反约束「赋值目标必须是可修改左值」：
 * 函数调用结果不是左值，不能赋值。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment" */
void bad5(void) { log1p(1.0) = 2.0; }

/* 违反约束「取地址操作数必须是左值或函数指示符」：
 * 函数返回的 double 不是左值，不能取地址。
 * 期望：gcc -std=c99 报 "lvalue required as unary '&' operand" */
void bad6(void) { double *p = &log1p(1.0); (void)p; }

#endif /* 负向测试结束 */