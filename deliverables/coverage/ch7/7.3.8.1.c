/*
 * 测试条款：C99 7.3.8.1 —— cabs / cabsf / cabsl 复数绝对值函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 cabs/cabsf/cabsl 计算复数模，
 *             结果应等于 sqrt(re^2 + im^2)，程序编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段（放在 #if 0 中）应导致编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型，参数为 double/float/long double complex，返回对应实数类型。
 *   [2] Description：计算复数绝对值（模）。
 *   [3] Returns：返回复数绝对值。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证原型存在且返回类型正确：cabs 返回 double，cabsf 返回 float，cabsl 返回 long double。
 *     通过 _Generic 在编译期检查返回类型（C99 无 _Generic，改用赋值 + 类型比较）。 */
static void test_prototypes(void)
{
    double complex   zd = 3.0 + 4.0 * I;
    float  complex   zf = 3.0f + 4.0f * I;
    long double complex zl = 3.0L + 4.0L * I;

    /* [1] 返回类型检查：把返回值赋给对应精度的实数变量，不应有精度丢失警告。 */
    double      rd = cabs(zd);
    float       rf = cabsf(zf);
    long double rl = cabsl(zl);

    /* [3] 3-4-5 直角三角形：|3+4i| = 5 */
    assert(fabs(rd - 5.0) < 1e-12);
    assert(fabsf(rf - 5.0f) < 1e-5f);
    assert(fabsl(rl - 5.0L) < 1e-15L);

    printf("[1][3] cabs(3+4i)=%g  cabsf=%g  cabsl=%Lg\n",
           rd, (double)rf, rl);
}

/* [2] 验证“复数绝对值（模）”语义：|z| = sqrt(re^2 + im^2) */
static void test_magnitude_semantics(void)
{
    double complex z = 1.0 + 1.0 * I;
    double expected = sqrt(1.0 * 1.0 + 1.0 * 1.0);
    double got = cabs(z);
    assert(fabs(got - expected) < 1e-12);
    printf("[2] |1+1i| = %g (expected %g)\n", got, expected);

    /* 纯实数复数：模 = 绝对值 */
    double complex zr = -7.5 + 0.0 * I;
    assert(fabs(cabs(zr) - 7.5) < 1e-12);

    /* 纯虚数复数：模 = 虚部绝对值 */
    double complex zi = 0.0 + 2.5 * I;
    assert(fabs(cabs(zi) - 2.5) < 1e-12);

    /* 零复数：模 = 0 */
    double complex z0 = 0.0 + 0.0 * I;
    assert(cabs(z0) == 0.0);

    /* 一般情形：|5+12i| = 13 */
    double complex z5 = 5.0 + 12.0 * I;
    assert(fabs(cabs(z5) - 13.0) < 1e-12);

    printf("[2] |5+12i| = %g\n", cabs(z5));
}

/* [3] 验证返回值非负（模的性质） */
static void test_nonnegative(void)
{
    double complex zs[] = {
        3.0 + 4.0 * I,
        -3.0 - 4.0 * I,
        0.0 + 0.0 * I,
        1e10 + 1e-10 * I,
        -1.0 + 0.0 * I
    };
    size_t i;
    for (i = 0; i < sizeof(zs) / sizeof(zs[0]); ++i) {
        double m = cabs(zs[i]);
        assert(m >= 0.0);
    }
    printf("[3] 所有模值均非负\n");
}

/* [2] 验证 cabs 与手工 sqrt 计算一致（多组数据） */
static void test_consistency(void)
{
    double re, im;
    for (re = -3.0; re <= 3.0; re += 1.0) {
        for (im = -3.0; im <= 3.0; im += 1.0) {
            double complex z = re + im * I;
            double expected = sqrt(re * re + im * im);
            double got = cabs(z);
            assert(fabs(got - expected) < 1e-12);
        }
    }
    printf("[2] cabs 与 sqrt(re^2+im^2) 在网格上一致\n");
}

int main(void)
{
    test_prototypes();
    test_magnitude_semantics();
    test_nonnegative();
    test_consistency();
    printf("所有正向测试通过。\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「cabs 的参数类型为 double complex」：
 * 传入普通 double（非复数类型）在 C99 中会触发类型不匹配的诊断。
 * 期望：gcc -std=c99 报错/警告（-Werror 下报错）。 */
#include <complex.h>
void bad_arg_type(void)
{
    double x = 3.0;
    double r = cabs(x);   /* 参数应为 double complex，而非 double */
    (void)r;
}

/* 违反约束「cabs 返回 double，不能当作复数使用」：
 * 把 cabs 的返回值（实数）赋给 double complex 变量本身合法，
 * 但把返回值当作复数参与复数运算并期望复数语义是类型误用。
 * 这里演示：对 cabs 返回值取 .real 成员（实数无成员）应报错。 */
#include <complex.h>
void bad_member_access(void)
{
    double complex z = 1.0 + 2.0 * I;
    double r = cabs(z);
    double bad = r.real;   /* double 无 .real 成员，应报错 */
    (void)bad;
}

/* 违反约束「cabs 需要 <complex.h> 中的原型」：
 * 未包含 <complex.h> 时调用 cabs，C99 中隐式声明返回 int，
 * 与实数返回类型冲突，-Werror 下应报错。 */
void bad_no_header(void)
{
    double complex z = 1.0 + 2.0 * I;   /* 此处 double complex 也需 <complex.h> */
    double r = cabs(z);
    (void)r;
}

/* 违反约束「cabs 参数个数为 1」：
 * 传入两个参数应报错。 */
#include <complex.h>
void bad_arg_count(void)
{
    double complex z = 1.0 + 2.0 * I;
    double r = cabs(z, z);   /* 参数过多，应报错 */
    (void)r;
}

/* 违反约束「cabs 返回 double，不能直接作为函数指针目标类型不匹配」：
 * 将 cabs 赋给返回 int 的函数指针，类型不兼容，应报错。 */
#include <complex.h>
void bad_func_ptr(void)
{
    int (*fp)(double complex) = cabs;   /* 返回类型不匹配：double vs int */
    (void)fp;
}

#endif /* 负向测试结束 */