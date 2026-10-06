/*
 * 测试 C99 7.3.5.5 —— csin / csinf / csinl 复数正弦函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h>，调用 csin/csinf/csinl，验证返回复数正弦值，
 *             编译通过且运行断言全部成立。
 *   负向测试：违反约束的代码（如缺少 <complex.h> 声明、参数类型错误、
 *             对非左值赋值等）应导致编译报错，统一放在 #if 0 中。
 *
 * 覆盖段落：
 *   [1] 函数原型声明（double complex csin(double complex) 等）
 *   [2] 语义：计算复数 z 的正弦
 *   [3] 返回值：返回复数正弦值
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可见性：三个函数均已在 <complex.h> 中声明。
 *     通过取函数指针来验证原型签名与标准一致。 */
static double complex (*p_csin)(double complex)        = csin;
static float complex  (*p_csinf)(float complex)        = csinf;
static long double complex (*p_csinl)(long double complex) = csinl;

/* 复数近似比较辅助 */
static int dcplx_close(double complex a, double complex b, double eps)
{
    return cabs(creal(a) - creal(b)) < eps &&
           cabs(cimag(a) - cimag(b)) < eps;
}

int main(void)
{
    /* [2] 语义：csin 计算复数正弦。
     * 数学恒等式：sin(z) = sin(x)cosh(y) + i cos(x)sinh(y)，z = x + i y。
     * 取 z = 0 + 0i，则 sin(z) = 0。 */
    {
        double complex z = 0.0 + 0.0 * I;
        double complex r = csin(z);
        assert(dcplx_close(r, 0.0 + 0.0 * I, 1e-12));
    }

    /* [2][3] 纯实部：z = pi/2，sin(pi/2) = 1，虚部为 0。 */
    {
        double complex z = (M_PI / 2.0) + 0.0 * I;
        double complex r = csin(z);
        assert(dcplx_close(r, 1.0 + 0.0 * I, 1e-12));
    }

    /* [2][3] 纯虚部：z = i*y，sin(i y) = i sinh(y)。
     * 取 y = 1，则 sin(i) = i * sinh(1)。 */
    {
        double complex z = 0.0 + 1.0 * I;
        double complex r = csin(z);
        assert(dcplx_close(r, 0.0 + sinh(1.0) * I, 1e-12));
    }

    /* [2][3] 一般复数：z = 1 + 2i。
     * sin(1+2i) = sin(1)cosh(2) + i cos(1)sinh(2)。 */
    {
        double complex z = 1.0 + 2.0 * I;
        double complex expected = sin(1.0) * cosh(2.0)
                                + (cos(1.0) * sinh(2.0)) * I;
        double complex r = csin(z);
        assert(dcplx_close(r, expected, 1e-12));
    }

    /* [2][3] 奇函数性质：sin(-z) = -sin(z)。 */
    {
        double complex z = 0.7 - 1.3 * I;
        double complex r1 = csin(z);
        double complex r2 = csin(-z);
        assert(dcplx_close(r1, -r2, 1e-12));
    }

    /* [1][2][3] csinf：float complex 版本，语义相同。 */
    {
        float complex z = 0.0f + 0.0f * I;
        float complex r = csinf(z);
        assert(cabsf(crealf(r) - 0.0f) < 1e-5f);
        assert(cabsf(cimagf(r) - 0.0f) < 1e-5f);

        float complex z2 = (float)(M_PI / 2.0) + 0.0f * I;
        float complex r2 = csinf(z2);
        assert(cabsf(crealf(r2) - 1.0f) < 1e-5f);
        assert(cabsf(cimagf(r2) - 0.0f) < 1e-5f);
    }

    /* [1][2][3] csinl：long double complex 版本，语义相同。 */
    {
        long double complex z = 0.0L + 0.0L * I;
        long double complex r = csinl(z);
        assert(cabsl(creall(r) - 0.0L) < 1e-15L);
        assert(cabsl(cimagl(r) - 0.0L) < 1e-15L);

        long double complex z2 = (long double)(M_PI / 2.0) + 0.0L * I;
        long double complex r2 = csinl(z2);
        assert(cabsl(creall(r2) - 1.0L) < 1e-15L);
        assert(cabsl(cimagl(r2) - 0.0L) < 1e-15L);
    }

    /* [1] 通过函数指针调用，验证原型签名正确。 */
    {
        double complex z = 0.3 + 0.4 * I;
        double complex r1 = csin(z);
        double complex r2 = p_csin(z);
        assert(dcplx_close(r1, r2, 1e-12));

        float complex zf = 0.3f + 0.4f * I;
        float complex rf1 = csinf(zf);
        float complex rf2 = p_csinf(zf);
        assert(cabsf(rf1 - rf2) < 1e-5f);

        long double complex zl = 0.3L + 0.4L * I;
        long double complex rl1 = csinl(zl);
        long double complex rl2 = p_csinl(zl);
        assert(cabsl(rl1 - rl2) < 1e-15L);
    }

    /* [3] 返回值类型为复数：可对其取实部/虚部，且结果类型正确。 */
    {
        double complex z = 0.5 + 0.5 * I;
        double complex r = csin(z);
        double re = creal(r);
        double im = cimag(r);
        /* 与数学公式一致 */
        assert(fabs(re - sin(0.5) * cosh(0.5)) < 1e-12);
        assert(fabs(im - cos(0.5) * sinh(0.5)) < 1e-12);
    }

    printf("C99 7.3.5.5 csin/csinf/csinl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用参数类型必须与原型兼容」：
 * csin 的原型为 double complex csin(double complex)，
 * 传入 struct 类型参数，gcc -std=c99 应报错（incompatible type）。 */
struct NotComplex { int x; };
void bad_arg_type(void)
{
    struct NotComplex s;
    csin(s);            /* 期望报错：参数类型不兼容 */
}

/* 违反约束「函数调用参数个数必须与原型一致」：
 * csin 只接受 1 个参数，传 2 个应报错（too many arguments）。 */
void bad_arg_count(void)
{
    double complex z = 0.0 + 0.0 * I;
    csin(z, z);         /* 期望报错：参数过多 */
}

/* 违反约束「函数调用参数个数必须与原型一致」：
 * csin 需要 1 个参数，传 0 个应报错（too few arguments）。 */
void bad_arg_count2(void)
{
    csin();             /* 期望报错：参数过少 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 函数调用结果不是左值，不能赋值。 */
void bad_assign_call(void)
{
    double complex z = 0.0 + 0.0 * I;
    csin(z) = z;        /* 期望报错：非左值赋值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 强制转换结果不是左值，不能赋值。 */
void bad_assign_cast(void)
{
    double complex z = 0.0 + 0.0 * I;
    (double complex)z = z;   /* 期望报错：非左值赋值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 条件表达式结果不是左值，不能赋值。 */
void bad_assign_cond(void)
{
    double complex a = 0.0 + 0.0 * I;
    double complex b = 1.0 + 1.0 * I;
    (1 ? a : b) = a;    /* 期望报错：非左值赋值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 逗号表达式结果不是左值，不能赋值。 */
void bad_assign_comma(void)
{
    double complex a = 0.0 + 0.0 * I;
    double complex b = 1.0 + 1.0 * I;
    (a, b) = a;         /* 期望报错：非左值赋值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 函数返回结构体（复数）的成员访问结果不是左值，不能赋值。 */
void bad_assign_member(void)
{
    double complex z = 0.0 + 0.0 * I;
    csin(z).x = 1.0;    /* 期望报错：非左值赋值（且 complex 无 .x 成员） */
}

/* 违反约束「const 限定对象不可修改」：
 * const 限定的复数对象不能被赋值。 */
void bad_assign_const(void)
{
    const double complex z = 0.0 + 0.0 * I;
    z = 1.0 + 1.0 * I;  /* 期望报错：向只读对象赋值 */
}

/* 违反约束「函数声明与使用必须一致」：
 * 若未包含 <complex.h>，csin 未声明，C99 下隐式函数声明为约束违反
 * （C99 删除了隐式 int 规则），gcc -std=c99 应报错或警告。
 * 此处用错误原型声明来模拟不兼容声明。 */
void bad_proto_mismatch(void)
{
    extern double csin(double);   /* 与标准原型冲突 */
    double r = csin(1.0);         /* 期望报错：原型不兼容 */
    (void)r;
}

#endif /* 负向测试结束 */