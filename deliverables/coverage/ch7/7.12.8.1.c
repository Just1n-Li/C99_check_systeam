/*
 * 测试条款：C99 7.12.8.1 —— The erf functions
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，erf / erff / erfl 三个函数可被调用，
 *             返回类型分别为 double / float / long double，
 *             且返回值等于误差函数 erf(x) = (2/sqrt(pi)) * ∫_0^x e^{-t^2} dt。
 *   负向测试：违反约束的代码（如参数个数错误、对非函数名调用等）应编译报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：三个函数声明存在于 <math.h>，返回类型分别为
 *     double / float / long double。用 _Generic 无法在 C99 用，
 *     改用赋值给对应类型变量并检查类型兼容性。 */
static void test_synopsis(void)
{
    double      (*pd)(double)      = erf;   /* [1] double erf(double) */
    float       (*pf)(float)       = erff;  /* [1] float erff(float)  */
    long double (*pl)(long double) = erfl;  /* [1] long double erfl(long double) */

    /* 通过函数指针调用，确认签名匹配 */
    double      rd = pd(0.5);
    float       rf = pf(0.5f);
    long double rl = pl(0.5L);

    /* 三个结果应彼此接近（同一数学函数的不同精度版本） */
    assert(fabs(rd - (double)rf) < 1e-6);
    assert(fabs(rd - (double)rl) < 1e-6);
    printf("[1] Synopsis OK: erf=%g erff=%g erfl=%Lg\n", rd, rf, rl);
}

/* [2] Description：计算 x 的误差函数。验证若干已知值。 */
static void test_description_known_values(void)
{
    /* erf(0) = 0 */
    assert(erf(0.0) == 0.0);
    assert(erff(0.0f) == 0.0f);
    assert(erfl(0.0L) == 0.0L);

    /* erf 是奇函数：erf(-x) = -erf(x) */
    double x = 0.7;
    assert(fabs(erf(-x) + erf(x)) < 1e-12);

    /* 已知数值：erf(1) ≈ 0.8427007929497149 */
    assert(fabs(erf(1.0) - 0.8427007929497149) < 1e-12);

    /* 已知数值：erf(0.5) ≈ 0.5204998778130465 */
    assert(fabs(erf(0.5) - 0.5204998778130465) < 1e-12);

    /* 大 x 时 erf(x) → 1 */
    assert(fabs(erf(6.0) - 1.0) < 1e-12);

    printf("[2] Description OK: erf(1)=%.16f\n", erf(1.0));
}

/* [3] Returns：返回值等于 (2/sqrt(pi)) * ∫_0^x e^{-t^2} dt。
 *     用数值积分（辛普森法）独立计算，与库函数比较。 */
static double erf_by_integration(double x)
{
    /* 用辛普森法对 e^{-t^2} 在 [0, x] 上积分，再乘 2/sqrt(pi) */
    const int n = 20000;              /* 偶数 */
    double a = 0.0, b = x;
    double h = (b - a) / n;
    double sum = exp(-a * a) + exp(-b * b);
    int i;
    for (i = 1; i < n; i++) {
        double t = a + i * h;
        double f = exp(-t * t);
        sum += (i % 2 == 0) ? 2.0 * f : 4.0 * f;
    }
    double integral = sum * h / 3.0;
    return (2.0 / sqrt(M_PI)) * integral;
}

static void test_returns_formula(void)
{
    double xs[] = { 0.1, 0.3, 0.5, 1.0, 1.5, 2.0 };
    int i;
    for (i = 0; i < (int)(sizeof(xs) / sizeof(xs[0])); i++) {
        double x = xs[i];
        double expected = erf_by_integration(x);
        double got = erf(x);
        /* 数值积分精度约 1e-9，留出余量 */
        assert(fabs(got - expected) < 1e-7);
    }
    printf("[3] Returns formula OK: erf(1.5)=%.16f (integral=%.16f)\n",
           erf(1.5), erf_by_integration(1.5));
}

/* [3] 补充：验证 erff / erfl 与 erf 的一致性（精度范围内） */
static void test_float_longdouble_consistency(void)
{
    float xf = 0.75f;
    long double xl = 0.75L;
    double ref = erf(0.75);

    assert(fabs((double)erff(xf) - ref) < 1e-6);
    assert(fabs((double)erfl(xl) - ref) < 1e-15);

    printf("[3] erff/erfl consistency OK\n");
}

int main(void)
{
    test_synopsis();
    test_description_known_values();
    test_returns_formula();
    test_float_longdouble_consistency();
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数必须与形参个数一致」：
 * erf 接受 1 个参数，这里传 2 个，gcc -std=c99 应报错
 *   error: too many arguments to function 'erf' */
double bad1 = erf(1.0, 2.0);

/* 违反约束「函数调用实参个数必须与形参个数一致」：
 * erff 接受 1 个参数，这里传 0 个，gcc -std=c99 应报错
 *   error: too few arguments to function 'erff' */
float bad2 = erff();

/* 违反约束「被调用者必须是函数或函数指针」：
 * 对非函数标识符使用函数调用语法，gcc -std=c99 应报错
 *   error: called object 'bad3' is not a function or function pointer */
int bad3 = 42;
double bad4 = bad3(1.0);

/* 违反约束「实参类型必须可转换为形参类型」：
 * 结构体类型无法隐式转换为 double，gcc -std=c99 应报错
 *   error: incompatible type for argument 1 of 'erf' */
struct NotArith { int a; };
double bad5 = erf((struct NotArith){ 1 });

/* 违反约束「函数返回类型不可作为赋值目标（非左值）」：
 * erf(1.0) 是右值，不能赋值，gcc -std=c99 应报错
 *   error: lvalue required as left operand of assignment */
void bad6(void) { erf(1.0) = 0.5; }

/* 违反约束「函数名不可被赋值（非左值）」：
 * erf 是函数指示符，不能作为赋值目标，gcc -std=c99 应报错
 *   error: lvalue required as left operand of assignment */
void bad7(void) { erf = 0; }

#endif