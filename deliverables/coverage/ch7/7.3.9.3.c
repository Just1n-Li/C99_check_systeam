/*
 * 测试 C99 7.3.9.3 —— conj / conjf / conjl 复数共轭函数
 *
 * 预期行为：
 *   正向测试：包含 <complex.h> 后调用 conj/conjf/conjl，编译并运行通过，
 *             验证返回值等于“虚部取反”的共轭值。
 *   负向测试：违反约束的代码（如参数类型错误、缺少头文件声明等）应编译报错。
 *
 * 覆盖段落：
 *   [1] Synopsis：三个函数原型，参数/返回类型为 double/float/long double complex
 *   [2] Description：共轭 = 虚部符号取反
 *   [3] Returns：返回共轭值
 */

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证签名与标准一致 */
static double complex (*p_conj)(double complex)        = conj;
static float  complex (*p_conjf)(float complex)        = conjf;
static long double complex (*p_conjl)(long double complex) = conjl;

int main(void)
{
    /* ---------- [2][3] double complex：conj ---------- */
    {
        double complex z = 3.0 + 4.0 * I;
        double complex w = conj(z);
        /* 共轭：实部不变，虚部取反 */
        assert(creal(w) == 3.0);
        assert(cimag(w) == -4.0);
        /* 双重共轭应还原 */
        assert(creal(conj(w)) == 3.0);
        assert(cimag(conj(w)) == 4.0);
    }

    /* ---------- [2][3] 纯虚数 ---------- */
    {
        double complex z = 0.0 + 5.0 * I;
        double complex w = conj(z);
        assert(creal(w) == 0.0);
        assert(cimag(w) == -5.0);
    }

    /* ---------- [2][3] 纯实数：共轭等于自身 ---------- */
    {
        double complex z = 7.0 + 0.0 * I;
        double complex w = conj(z);
        assert(creal(w) == 7.0);
        assert(cimag(w) == 0.0);
    }

    /* ---------- [2][3] 负虚部：共轭后虚部变正 ---------- */
    {
        double complex z = -1.5 - 2.5 * I;
        double complex w = conj(z);
        assert(creal(w) == -1.5);
        assert(cimag(w) == 2.5);
    }

    /* ---------- [2][3] 零值 ---------- */
    {
        double complex z = 0.0 + 0.0 * I;
        double complex w = conj(z);
        assert(creal(w) == 0.0);
        assert(cimag(w) == 0.0);
    }

    /* ---------- [1][2][3] float complex：conjf ---------- */
    {
        float complex z = 1.5f + 2.5f * I;
        float complex w = conjf(z);
        assert(crealf(w) == 1.5f);
        assert(cimagf(w) == -2.5f);
    }

    /* ---------- [1][2][3] long double complex：conjl ---------- */
    {
        long double complex z = 1.25L + 3.75L * I;
        long double complex w = conjl(z);
        assert(creall(w) == 1.25L);
        assert(cimagl(w) == -3.75L);
    }

    /* ---------- [2] 共轭与加法/乘法的一致性：conj(a+b)=conj(a)+conj(b) ---------- */
    {
        double complex a = 1.0 + 2.0 * I;
        double complex b = 3.0 - 4.0 * I;
        double complex lhs = conj(a + b);
        double complex rhs = conj(a) + conj(b);
        assert(creal(lhs) == creal(rhs));
        assert(cimag(lhs) == cimag(rhs));
    }

    /* ---------- [2] conj(a*b) = conj(a)*conj(b) ---------- */
    {
        double complex a = 1.0 + 2.0 * I;
        double complex b = 3.0 - 4.0 * I;
        double complex lhs = conj(a * b);
        double complex rhs = conj(a) * conj(b);
        assert(fabs(creal(lhs) - creal(rhs)) < 1e-12);
        assert(fabs(cimag(lhs) - cimag(rhs)) < 1e-12);
    }

    /* ---------- [2] |conj(z)| == |z| ---------- */
    {
        double complex z = 3.0 + 4.0 * I;
        double complex w = conj(z);
        assert(fabs(cabs(w) - cabs(z)) < 1e-12);
        assert(fabs(cabs(w) - 5.0) < 1e-12);
    }

    /* ---------- [1] 通过函数指针调用，验证原型签名 ---------- */
    {
        double complex z = 2.0 + 9.0 * I;
        double complex w = p_conj(z);
        assert(creal(w) == 2.0 && cimag(w) == -9.0);

        float complex zf = 2.0f + 9.0f * I;
        float complex wf = p_conjf(zf);
        assert(crealf(wf) == 2.0f && cimagf(wf) == -9.0f);

        long double complex zl = 2.0L + 9.0L * I;
        long double complex wl = p_conjl(zl);
        assert(creall(wl) == 2.0L && cimagl(wl) == -9.0L);
    }

    printf("C99 7.3.9.3 conj/conjf/conjl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 参数类型必须为复数类型」：
 * 传入 double 实参给 conj（无隐式转换到 double complex 的合法调用），
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'conj'
 * （注：C99 中 double 不能隐式转换为 double complex 作为函数实参匹配原型）
 */
double bad1(void) {
    double x = 1.0;
    double complex r = conj(x);   /* 期望报错 */
    return creal(r);
}

/* 违反约束「[1] 返回类型为复数」：
 * 把 conj 的返回值赋给 double（复数到实数无隐式转换），
 * gcc -std=c99 应报错：incompatible types in assignment
 */
double bad2(void) {
    double complex z = 1.0 + 2.0 * I;
    double d = conj(z);           /* 期望报错 */
    return d;
}

/* 违反约束「[1] 参数个数必须为 1」：
 * 调用 conj 时传入两个实参，gcc -std=c99 应报错：too many arguments
 */
double complex bad3(void) {
    double complex z = 1.0 + 2.0 * I;
    return conj(z, z);            /* 期望报错 */
}

/* 违反约束「[1] 参数个数必须为 1」：
 * 调用 conj 时零实参，gcc -std=c99 应报错：too few arguments
 */
double complex bad4(void) {
    return conj();                /* 期望报错 */
}

/* 违反约束「[1] 函数名拼写/声明」：
 * 使用未声明的 conjx，C99 中隐式函数声明已被移除，
 * gcc -std=c99 应报错：implicit declaration of function 'conjx'
 */
double complex bad5(void) {
    double complex z = 1.0 + 2.0 * I;
    return conjx(z);              /* 期望报错 */
}

/* 违反约束「[1] 参数类型不匹配」：
 * 把 long double complex 传给 conjf（float complex 形参），
 * 复数类型之间无隐式转换，gcc -std=c99 应报错：incompatible type
 */
float complex bad6(void) {
    long double complex z = 1.0L + 2.0L * I;
    return conjf(z);              /* 期望报错 */
}

/* 违反约束「[1] 参数类型不匹配」：
 * 把 float complex 传给 conjl（long double complex 形参），
 * gcc -std=c99 应报错：incompatible type
 */
long double complex bad7(void) {
    float complex z = 1.0f + 2.0f * I;
    return conjl(z);              /* 期望报错 */
}

/* 违反约束「[1] 参数必须为复数类型」：
 * 传入 int 给 conj，gcc -std=c99 应报错：incompatible type
 */
double complex bad8(void) {
    int n = 5;
    return conj(n);               /* 期望报错 */
}

#endif /* 负向测试结束 */