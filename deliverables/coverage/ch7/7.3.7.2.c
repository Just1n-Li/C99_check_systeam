/*
 * 测试 C99 条款 7.3.7.2 —— clog / clogf / clogl 复数自然对数函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 clog/clogf/clogl，验证：
 *     [1] 三个函数原型可用，返回类型分别为 double complex / float complex /
 *         long double complex；
 *     [2] 计算以 e 为底的复数自然对数，分支切割沿负实轴；
 *     [3] 返回值虚部落在 [-pi, +pi] 区间内（沿虚轴方向有界）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 编译：gcc -std=c99 -Wall -pedantic test.c -lm
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型与返回类型检查：三个函数均声明于 <complex.h>，
 *     返回类型分别为 double complex / float complex / long double complex。
 *     通过赋值给对应类型变量并检查类型兼容性来验证。 */
static void test_synopsis_types(void)
{
    double complex        z  = 1.0 + 0.0 * I;
    float  complex        zf = 1.0f + 0.0f * I;
    long double complex   zl = 1.0L + 0.0L * I;

    double complex        r  = clog(z);    /* [1] double complex clog(double complex) */
    float  complex        rf = clogf(zf);  /* [1] float complex clogf(float complex) */
    long double complex   rl = clogl(zl);  /* [1] long double complex clogl(long double complex) */

    /* 类型兼容性：把结果赋给同类型变量，编译器不应报错 */
    double complex        chk  = r;
    float  complex        chkf = rf;
    long double complex   chkl = rl;

    (void)chk; (void)chkf; (void)chkl;
    printf("[1] clog/clogf/clogl 原型与返回类型 OK\n");
}

/* [2] 语义：clog 计算以 e 为底的复数自然对数。
 *     对正实数 z = x (x>0)，clog(x) = ln(x) + 0i。 */
static void test_real_positive(void)
{
    double complex z = 1.0 + 0.0 * I;
    double complex w = clog(z);
    /* ln(1) = 0 */
    assert(fabs(creal(w) - 0.0) < 1e-12);
    assert(fabs(cimag(w) - 0.0) < 1e-12);

    z = M_E + 0.0 * I;   /* e */
    w = clog(z);
    /* ln(e) = 1 */
    assert(fabs(creal(w) - 1.0) < 1e-12);
    assert(fabs(cimag(w) - 0.0) < 1e-12);

    printf("[2] clog 对正实数的自然对数 OK\n");
}

/* [2] 语义：clog 是 exp 的逆运算（在分支范围内）。
 *     对任意 z，exp(clog(z)) 应约等于 z。 */
static void test_inverse_of_exp(void)
{
    double complex zs[] = {
        2.0 + 3.0 * I,
        -1.5 + 0.5 * I,
        0.5 - 2.0 * I,
        3.0 + 0.0 * I
    };
    size_t i;
    for (i = 0; i < sizeof(zs) / sizeof(zs[0]); ++i) {
        double complex w = clog(zs[i]);
        double complex back = cexp(w);
        assert(fabs(creal(back) - creal(zs[i])) < 1e-10);
        assert(fabs(cimag(back) - cimag(zs[i])) < 1e-10);
    }
    printf("[2] exp(clog(z)) == z OK\n");
}

/* [2] 分支切割：沿负实轴。对 z = -1（负实轴上的点），
 *     自然对数主值为 ln(1) + i*pi = i*pi（虚部取 +pi 一侧）。 */
static void test_branch_cut_negative_real(void)
{
    double complex z = -1.0 + 0.0 * I;
    double complex w = clog(z);
    /* 实部 ln(1) = 0 */
    assert(fabs(creal(w) - 0.0) < 1e-12);
    /* 虚部为 ±pi（分支切割处，主值取 +pi） */
    assert(fabs(fabs(cimag(w)) - M_PI) < 1e-12);

    /* 负实轴上另一点 z = -e，ln(e)=1，虚部 ±pi */
    z = -M_E + 0.0 * I;
    w = clog(z);
    assert(fabs(creal(w) - 1.0) < 1e-12);
    assert(fabs(fabs(cimag(w)) - M_PI) < 1e-12);

    printf("[2] 分支切割沿负实轴 OK\n");
}

/* [3] 返回值虚部落在 [-pi, +pi] 区间内（沿虚轴方向有界）。
 *     对多个不同象限的 z 检查 cimag(clog(z)) ∈ [-pi, pi]。 */
static void test_imag_range(void)
{
    double complex zs[] = {
        1.0 + 1.0 * I,
        -1.0 + 1.0 * I,
        -1.0 - 1.0 * I,
        1.0 - 1.0 * I,
        0.0 + 1.0 * I,
        0.0 - 1.0 * I,
        5.0 + 0.0 * I,
        -5.0 + 0.0 * I
    };
    size_t i;
    for (i = 0; i < sizeof(zs) / sizeof(zs[0]); ++i) {
        double complex w = clog(zs[i]);
        double im = cimag(w);
        assert(im >= -M_PI - 1e-12);
        assert(im <=  M_PI + 1e-12);
    }
    printf("[3] 返回值虚部在 [-pi, +pi] 内 OK\n");
}

/* [3] 实部沿实轴方向无界：|z| 越大，实部 ln|z| 越大。 */
static void test_real_unbounded(void)
{
    double complex z1 = 1.0 + 0.0 * I;      /* ln|z| = 0 */
    double complex z2 = 1000.0 + 0.0 * I;   /* ln|z| = ln(1000) */
    double complex w1 = clog(z1);
    double complex w2 = clog(z2);
    assert(fabs(creal(w1) - 0.0) < 1e-12);
    assert(fabs(creal(w2) - log(1000.0)) < 1e-12);
    assert(creal(w2) > creal(w1));
    printf("[3] 实部沿实轴无界 OK\n");
}

/* [1][2][3] clogf / clogl 与 clog 语义一致（精度不同）。 */
static void test_float_long_double_variants(void)
{
    float complex zf = 2.0f + 3.0f * I;
    float complex wf = clogf(zf);
    /* exp(clogf(z)) ≈ z */
    float complex backf = cexpf(wf);
    assert(fabsf(crealf(backf) - 2.0f) < 1e-4f);
    assert(fabsf(cimagf(backf) - 3.0f) < 1e-4f);
    /* 虚部范围 */
    assert(cimagf(wf) >= -3.1415927f && cimagf(wf) <= 3.1415927f);

    long double complex zl = 2.0L + 3.0L * I;
    long double complex wl = clogl(zl);
    long double complex backl = cexpl(wl);
    assert(fabsl(creall(backl) - 2.0L) < 1e-15L);
    assert(fabsl(cimagl(backl) - 3.0L) < 1e-15L);
    assert(cimagl(wl) >= -3.14159265358979323846L &&
           cimagl(wl) <=  3.14159265358979323846L);

    printf("[1][2][3] clogf/clogl 语义一致 OK\n");
}

int main(void)
{
    test_synopsis_types();
    test_real_positive();
    test_inverse_of_exp();
    test_branch_cut_negative_real();
    test_imag_range();
    test_real_unbounded();
    test_float_long_double_variants();
    printf("所有正向测试通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「clog 的参数必须为 double complex 类型（可隐式转换）」：
 * 传入结构体类型，无法转换为 double complex，gcc -std=c99 应报错。 */
struct NotComplex { int x; };
void bad_arg_type(void)
{
    struct NotComplex s;
    double complex w = clog(s);   /* 错误：结构体不能转换为 double complex */
    (void)w;
}

/* 违反约束「clog 返回 double complex，不能赋给不兼容的标量类型」：
 * 把 double complex 直接赋给 int，丢失虚部且类型不兼容，应报错。 */
void bad_return_assign(void)
{
    double complex z = 1.0 + 1.0 * I;
    int i = clog(z);   /* 错误：double complex 不能隐式转换为 int */
    (void)i;
}

/* 违反约束「clog 需要恰好一个参数」：
 * 调用时参数个数不匹配，应报错。 */
void bad_arg_count(void)
{
    double complex z = 1.0 + 0.0 * I;
    double complex w = clog(z, z);   /* 错误：参数过多 */
    (void)w;
}

/* 违反约束「clog 需要恰好一个参数」：
 * 调用时缺少参数，应报错。 */
void bad_arg_missing(void)
{
    double complex w = clog();   /* 错误：参数过少 */
    (void)w;
}

/* 违反约束「clog 的返回类型为 double complex，不能作为左值赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
void bad_assign_to_call(void)
{
    double complex z = 1.0 + 0.0 * I;
    clog(z) = z;   /* 错误：函数调用结果不是左值 */
}

/* 违反约束「clog 的返回类型为 double complex，不能取地址」：
 * 对函数调用结果取地址应报错。 */
void bad_take_address(void)
{
    double complex z = 1.0 + 0.0 * I;
    double complex *p = &clog(z);   /* 错误：不能对非左值取地址 */
    (void)p;
}

#endif /* 负向测试结束 */