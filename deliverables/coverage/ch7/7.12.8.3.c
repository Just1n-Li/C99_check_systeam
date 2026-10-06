/*
 * 测试 C99 7.12.8.3 —— lgamma 函数族
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 lgamma / lgammaf / lgammal，
 *             验证返回值为 ln|Γ(x)|，程序应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数个数错误、缺少 <math.h> 声明、
 *             对返回类型赋值给不兼容类型等）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] 函数原型声明（double lgamma(double); float lgammaf(float);
 *       long double lgammal(long double);）
 *   [2] 语义：计算 ln|Γ(x)|；x 过大时发生 range error；
 *       x 为零或负整数时可能发生 range error。
 *   [3] 返回值：log_e |Γ(x)|
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <errno.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证三个函数原型存在且返回类型正确 */
static void test_prototypes(void)
{
    /* 通过函数指针类型检查返回类型与参数类型 */
    double (*p_lgamma)(double)          = lgamma;
    float  (*p_lgammaf)(float)          = lgammaf;
    long double (*p_lgammal)(long double) = lgammal;

    assert(p_lgamma  != NULL);
    assert(p_lgammaf != NULL);
    assert(p_lgammal != NULL);

    /* 调用以确认可链接 */
    (void)p_lgamma(1.0);
    (void)p_lgammaf(1.0f);
    (void)p_lgammal(1.0L);
}

/* [2][3] 验证 lgamma(x) == ln|Γ(x)| 的数值语义 */
static void test_values(void)
{
    /* Γ(1) = 1  =>  ln|Γ(1)| = ln(1) = 0 */
    assert(fabs(lgamma(1.0) - 0.0) < 1e-12);

    /* Γ(2) = 1  =>  ln|Γ(2)| = 0 */
    assert(fabs(lgamma(2.0) - 0.0) < 1e-12);

    /* Γ(3) = 2  =>  ln|Γ(3)| = ln(2) */
    assert(fabs(lgamma(3.0) - log(2.0)) < 1e-12);

    /* Γ(4) = 6  =>  ln|Γ(4)| = ln(6) */
    assert(fabs(lgamma(4.0) - log(6.0)) < 1e-12);

    /* Γ(0.5) = sqrt(pi)  =>  ln|Γ(0.5)| = 0.5*ln(pi) */
    assert(fabs(lgamma(0.5) - 0.5 * log(3.14159265358979323846)) < 1e-12);

    /* 负非整数：Γ(-0.5) = -2*sqrt(pi)  =>  ln|Γ(-0.5)| = ln(2*sqrt(pi)) */
    assert(fabs(lgamma(-0.5) - log(2.0 * sqrt(3.14159265358979323846))) < 1e-12);

    /* 递推关系：Γ(x+1) = x*Γ(x)  =>  lgamma(x+1) = lgamma(x) + ln|x| */
    {
        double x = 3.7;
        double lhs = lgamma(x + 1.0);
        double rhs = lgamma(x) + log(fabs(x));
        assert(fabs(lhs - rhs) < 1e-10);
    }
}

/* [1][2][3] 验证 float 版本 lgammaf */
static void test_lgammaf(void)
{
    /* Γ(1) = 1 => ln|Γ(1)| = 0 */
    assert(fabsf(lgammaf(1.0f) - 0.0f) < 1e-5f);

    /* Γ(3) = 2 => ln|Γ(3)| = ln(2) */
    assert(fabsf(lgammaf(3.0f) - logf(2.0f)) < 1e-5f);

    /* Γ(0.5) = sqrt(pi) */
    assert(fabsf(lgammaf(0.5f) - 0.5f * logf(3.14159265f)) < 1e-5f);
}

/* [1][2][3] 验证 long double 版本 lgammal */
static void test_lgammal(void)
{
    /* Γ(1) = 1 => ln|Γ(1)| = 0 */
    assert(fabsl(lgammal(1.0L) - 0.0L) < 1e-15L);

    /* Γ(3) = 2 => ln|Γ(3)| = ln(2) */
    assert(fabsl(lgammal(3.0L) - logl(2.0L)) < 1e-15L);

    /* Γ(0.5) = sqrt(pi) */
    assert(fabsl(lgammal(0.5L) - 0.5L * logl(3.14159265358979323846L)) < 1e-15L);
}

/* [2] 验证 x 为零或负整数时可能发生 range error（不强制，但检查行为合理） */
static void test_range_error(void)
{
    /* 注意：C99 说 "may occur"，因此不强制要求 errno 被设置。
     * 我们只验证函数返回一个值（不崩溃），并检查若 errno 被设置
     * 则为 ERANGE。 */
    errno = 0;
    double r1 = lgamma(0.0);
    (void)r1;
    /* 若实现报告 range error，errno 应为 ERANGE */
    if (errno != 0) {
        assert(errno == ERANGE);
    }

    errno = 0;
    double r2 = lgamma(-1.0);
    (void)r2;
    if (errno != 0) {
        assert(errno == ERANGE);
    }

    errno = 0;
    double r3 = lgamma(-2.0);
    (void)r3;
    if (errno != 0) {
        assert(errno == ERANGE);
    }
}

/* [2] 验证 x 过大时发生 range error（溢出） */
static void test_overflow(void)
{
    /* lgamma(DBL_MAX) 会溢出，应设置 errno = ERANGE 并返回 HUGE_VAL */
    errno = 0;
    double r = lgamma(DBL_MAX);
    /* 标准说 "A range error occurs if x is too large"，
     * 因此 errno 应被设置为 ERANGE，返回值为 HUGE_VAL（正无穷） */
    assert(errno == ERANGE);
    assert(isinf(r) && r > 0);
}

int main(void)
{
    test_prototypes();
    test_values();
    test_lgammaf();
    test_lgammal();
    test_range_error();
    test_overflow();

    printf("C99 7.12.8.3 lgamma: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用参数个数必须与原型匹配」：
 * lgamma 原型为 double lgamma(double)，传入两个参数应报错。
 * 期望：gcc -std=c99 报 "too many arguments to function 'lgamma'" */
double bad1 = lgamma(1.0, 2.0);

/* 违反约束「函数调用参数个数必须与原型匹配」：
 * lgamma 需要一个参数，不传参数应报错。
 * 期望：gcc -std=c99 报 "too few arguments to function 'lgamma'" */
double bad2 = lgamma();

/* 违反约束「lgammaf 参数类型为 float」：
 * 虽然 C 允许隐式转换，但传入结构体类型无法转换为 float，应报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 1" */
struct NotANumber { int x; };
float bad3 = lgammaf((struct NotANumber){1});

/* 违反约束「lgammal 参数类型为 long double」：
 * 传入指针类型无法隐式转换为 long double，应报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 1" */
long double bad4 = lgammal((int *)0);

/* 违反约束「函数返回值不能作为赋值目标（非左值）」：
 * lgamma(1.0) 是函数调用结果，不是左值，不能赋值。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment" */
void bad5(void) { lgamma(1.0) = 2.0; }

/* 违反约束「函数返回值不能取地址（非左值）」：
 * 期望：gcc -std=c99 报 "lvalue required as unary '&' operand" */
void bad6(void) { double *p = &lgamma(1.0); (void)p; }

/* 违反约束「未声明标识符不能使用」：
 * 若未包含 <math.h>，lgamma 未声明，C99 不允许隐式函数声明。
 * 期望：gcc -std=c99 报 "implicit declaration of function 'lgamma'" */
/* 注意：此处已在文件顶部包含 <math.h>，故用 #undef 无法模拟。
 * 以下片段假设在无 <math.h> 的翻译单元中：
 *   double x = lgamma(1.0);  // 应报 implicit declaration 错误
 */

/* 违反约束「lgamma 返回 double，不能直接初始化为结构体」：
 * 期望：gcc -std=c99 报 "incompatible types when initializing type" */
struct S { double d; };
struct S bad7 = lgamma(1.0);

#endif /* 负向测试结束 */