/*
 * 测试条款：C99 7.12.6.7 The log functions
 *
 * 预期行为：
 *   正向测试：<math.h> 提供 log / logf / logl 三个函数，分别接受 double /
 *             float / long double 参数，返回自然对数（底 e）。对正数参数
 *             结果正确；log(1)==0；log(e)==1；logf/logl 类型与精度符合声明。
 *             负参数触发 domain error（返回 NaN 并置 errno=EDOM）；
 *             零参数可能触发 range error（返回 -HUGE_VAL 并置 errno=ERANGE）。
 *   负向测试：违反约束的代码（如参数个数错误、把非算术类型传给 log、
 *             对函数名赋值等）应导致编译报错。
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型声明：三个函数均可用，返回类型分别为 double/float/long double */
static void test_synopsis(void)
{
    double      (*pd)(double)          = log;
    float       (*pf)(float)           = logf;
    long double (*pl)(long double)     = logl;
    (void)pd; (void)pf; (void)pl;
    printf("[1] log/logf/logl 原型可用\n");
}

/* [2][3] log 计算底 e 自然对数，返回 log_e x */
static void test_log_double(void)
{
    double r;

    /* log(1) == 0 */
    r = log(1.0);
    assert(r == 0.0);

    /* log(e) == 1，e 用 exp(1.0) 表示 */
    r = log(exp(1.0));
    assert(fabs(r - 1.0) < 1e-12);

    /* log(e^2) == 2 */
    r = log(exp(2.0));
    assert(fabs(r - 2.0) < 1e-12);

    /* log(10) 的已知值 */
    r = log(10.0);
    assert(fabs(r - 2.30258509299404568402) < 1e-12);

    /* 单调性：0 < a < b  =>  log(a) < log(b) */
    assert(log(2.0) < log(3.0));

    printf("[2][3] log(double) 语义正确\n");
}

/* [1][3] logf 接受 float，返回 float */
static void test_logf(void)
{
    float r = logf(1.0f);
    assert(r == 0.0f);

    r = logf((float)exp(1.0));
    assert(fabsf(r - 1.0f) < 1e-5f);

    /* 返回类型确为 float */
    assert(sizeof(logf(2.0f)) == sizeof(float));

    printf("[1][3] logf(float) 语义正确\n");
}

/* [1][3] logl 接受 long double，返回 long double */
static void test_logl(void)
{
    long double r = logl(1.0L);
    assert(r == 0.0L);

    r = logl(expl(1.0L));
    assert(fabsl(r - 1.0L) < 1e-15L);

    /* 返回类型确为 long double */
    assert(sizeof(logl(2.0L)) == sizeof(long double));

    printf("[1][3] logl(long double) 语义正确\n");
}

/* [2] 负参数：domain error，返回 NaN，errno 置 EDOM */
static void test_domain_error(void)
{
    double r;

    errno = 0;
    r = log(-1.0);
    assert(isnan(r));
    assert(errno == EDOM);

    errno = 0;
    r = log(-0.5);
    assert(isnan(r));
    assert(errno == EDOM);

    printf("[2] 负参数触发 domain error (NaN, EDOM)\n");
}

/* [2] 零参数：range error 可能发生，返回 -HUGE_VAL，errno 置 ERANGE */
static void test_range_error_zero(void)
{
    double r;

    errno = 0;
    r = log(0.0);
    /* 标准允许实现返回 -HUGE_VAL 并置 ERANGE */
    assert(r == -HUGE_VAL || isinf(r));
    assert(errno == ERANGE);

    printf("[2] 零参数触发 range error (-HUGE_VAL, ERANGE)\n");
}

/* [2][3] 参数为 +inf 时 log(+inf) == +inf（非错误） */
static void test_infinity(void)
{
    double r = log(INFINITY);
    assert(isinf(r) && r > 0.0);
    printf("[2][3] log(+inf) == +inf\n");
}

int main(void)
{
    test_synopsis();
    test_log_double();
    test_logf();
    test_logl();
    test_domain_error();
    test_range_error_zero();
    test_infinity();

    printf("\n所有正向测试通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数必须与原型一致」：
 * log 原型只接受 1 个参数，传 2 个应报错。
 * 期望：gcc -std=c99 报 "too many arguments to function 'log'" */
double bad1 = log(1.0, 2.0);

/* 违反约束「函数调用实参个数必须与原型一致」：
 * log 原型要求 1 个参数，传 0 个应报错。
 * 期望：gcc -std=c99 报 "too few arguments to function 'log'" */
double bad2 = log();

/* 违反约束「实参必须为算术类型」：
 * 结构体不能传给 log（无法转换为 double）。
 * 期望：gcc -std=c99 报 incompatible type for argument 1 of 'log' */
struct S { int x; };
double bad3(struct S s) { return log(s); }

/* 违反约束「函数名不是可修改左值」：
 * 对函数指示符赋值应报错。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment" */
void bad4(void) { log = 0; }

/* 违反约束「函数指示符不能作为取地址以外的操作数参与算术」：
 * log + 1 非法。
 * 期望：gcc -std=c99 报 invalid operands to binary + */
double bad5 = log + 1;

/* 违反约束「返回类型为 double 的函数结果不能作为结构体成员访问」：
 * log(1.0).x 非法，double 无成员。
 * 期望：gcc -std=c99 报 "request for member 'x' in something not a structure or union" */
double bad6 = log(1.0).x;

#endif