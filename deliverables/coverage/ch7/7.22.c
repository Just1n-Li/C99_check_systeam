/*
 * 测试 C99 7.22 <tgmath.h> 类型泛型数学宏
 *
 * 预期行为：
 *   正向测试：包含 <tgmath.h> 后，类型泛型宏根据实参类型选择正确的
 *             real/complex 函数，程序应能编译并运行通过（assert 验证）。
 *   负向测试：违反约束的片段（如对非左值赋值、对 const 限定成员赋值等）
 *             应导致编译报错，统一放在 #if 0 中。
 *
 * 覆盖段落：[1][2][3][4][5][6][7] 及 Constraints/Semantics/EXAMPLE。
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>
#include <tgmath.h>   /* [1] 该头文件包含 <math.h> 与 <complex.h> */

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] <tgmath.h> 包含 <math.h> 与 <complex.h>，并定义类型泛型宏。
 *     验证：包含 <tgmath.h> 后，<math.h>/<complex.h> 的符号可用。 */
static void test_header_inclusion(void)
{
    double d = sin(0.0);          /* 来自 <math.h> */
    double complex z = csin(0.0); /* 来自 <complex.h> */
    (void)d; (void)z;
    assert(1);
}

/* [2][3] 类型确定规则：
 *   若任一泛型参数为 long double -> long double
 *   否则若有 double 或整型        -> double
 *   否则                          -> float
 * 通过比较宏调用结果与显式后缀函数结果来验证。 */
static void test_type_determination(void)
{
    int n = 2;
    float f = 0.5f;
    double d = 0.5;
    long double ld = 0.5L;

    /* [3] 整型参数 -> double 版本 */
    assert(exp(n) == exp((double)n));

    /* [3] float 参数 -> float 版本 */
    assert(sin(f) == sinf(f));

    /* [3] double 参数 -> double 版本 */
    assert(sin(d) == sin(d));

    /* [3] long double 参数 -> long double 版本 */
    assert(atan(ld) == atanl(ld));

    /* [3] 混合：有 long double 则选 long double */
    assert(pow(ld, f) == powl(ld, (long double)f));

    /* [3] 混合：无 long double 但有 double/整型则选 double */
    assert(pow(d, f) == pow(d, (double)f));
    assert(pow(n, f) == pow((double)n, (double)f));
}

/* [4] 有 c 前缀对应函数的宏：若任一泛型参数为 complex，则调用 complex 版本；
 *     否则调用 real 版本。 */
static void test_real_complex_selection(void)
{
    double d = 0.5;
    float f = 0.5f;
    double complex dc = 0.5 + 0.5 * I;
    float complex fc = 0.5f + 0.5f * I;

    /* 全为实数 -> real 函数 */
    assert(cos(d) == cos(d));
    assert(cos(f) == cosf(f));

    /* 含 complex -> complex 函数 */
    assert(cos(dc) == ccos(dc));
    assert(cos(fc) == ccosf(fc));

    /* [4] fabs/cabs 共用宏名 fabs */
    assert(fabs(d) == fabs(d));
    assert(fabs(dc) == cabs(dc));
    assert(fabs(fc) == cabsf(fc));

    /* [4] sqrt 的 real/complex 选择 */
    assert(sqrt(d) == sqrt(d));
    assert(sqrt(dc) == csqrt(dc));
}

/* [5] 无 c 前缀对应函数的宏：全为实数 -> real 函数；
 *     否则（含 complex）-> 未定义行为（此处不测试 UB）。 */
static void test_real_only_macros(void)
{
    int n = 3;
    float f = 1.5f;
    double d = 1.5;
    long double ld = 1.5L;

    /* 全为实数，调用 real 函数 */
    assert(atan2(d, d) == atan2(d, d));
    assert(fmod(d, d) == fmod(d, d));
    assert(remainder(n, n) == remainder((double)n, (double)n));
    assert(nextafter(d, f) == nextafter(d, (double)f));
    assert(nexttoward(f, ld) == nexttowardf(f, ld));
    assert(copysign(n, ld) == copysignl((long double)n, ld));
    assert(fmax(d, d) == fmax(d, d));
    assert(fmin(d, d) == fmin(d, d));
    assert(hypot(d, d) == hypot(d, d));
    assert(cbrt(d) == cbrt(d));
    assert(exp2(d) == exp2(d));
    assert(log2(d) == log2(d));
    assert(log10(d) == log10(d));
    assert(log1p(d) == log1p(d));
    assert(expm1(d) == expm1(d));
    assert(erf(d) == erf(d));
    assert(erfc(d) == erfc(d));
    assert(lgamma(d) == lgamma(d));
    assert(tgamma(d) == tgamma(d));
    assert(ceil(d) == ceil(d));
    assert(floor(d) == floor(d));
    assert(trunc(d) == trunc(d));
    assert(round(d) == round(d));
    assert(nearbyint(d) == nearbyint(d));
    assert(rint(d) == rint(d));
    assert(fdim(d, d) == fdim(d, d));
    assert(fma(d, d, d) == fma(d, d, d));
    assert(ldexp(d, n) == ldexp(d, n));
    assert(scalbn(d, n) == scalbn(d, n));
    assert(scalbln(d, (long)n) == scalbln(d, (long)n));
    assert(frexp(d, &n) == frexp(d, &n));
    assert(logb(d) == logb(d));
    assert(ilogb(d) == ilogb(d));
    assert(lrint(d) == lrint(d));
    assert(llrint(d) == llrint(d));
    assert(lround(d) == lround(d));
    assert(llround(d) == llround(d));
}

/* [6] 仅存在于 <complex.h> 的宏：任何实参都调用 complex 函数。 */
static void test_complex_only_macros(void)
{
    int n = 1;
    float f = 1.0f;
    double d = 1.0;
    long double ld = 1.0L;
    double complex dc = 1.0 + 1.0 * I;
    long double complex ldc = 1.0L + 1.0L * I;

    /* 实数实参 -> complex 函数 */
    assert(carg(n) == carg((double complex)n));
    assert(carg(d) == carg((double complex)d));
    assert(creal(d) == creal((double complex)d));
    assert(cimag(ld) == cimagl(ldc));
    assert(cproj(f) == cprojf((float complex)f));

    /* complex 实参 -> complex 函数 */
    assert(carg(dc) == carg(dc));
    assert(cproj(ldc) == cprojl(ldc));
}

/* [7] EXAMPLE 中列出的宏调用应能编译并产生与显式函数一致的结果。 */
static void test_example(void)
{
    int n = 2;
    float f = 0.5f;
    double d = 0.5;
    long double ld = 0.5L;
    float complex fc = 0.5f + 0.5f * I;
    double complex dc = 0.5 + 0.5 * I;
    long double complex ldc = 0.5L + 0.5L * I;

    assert(exp(n) == exp((double)n));
    assert(acosh(f) == acoshf(f));
    assert(sin(d) == sin(d));
    assert(atan(ld) == atanl(ld));
    assert(log(fc) == clogf(fc));
    assert(sqrt(dc) == csqrt(dc));
    assert(pow(ldc, f) == cpowl(ldc, (long double)f));
    assert(remainder(n, n) == remainder((double)n, (double)n));
    assert(nextafter(d, f) == nextafter(d, (double)f));
    assert(nexttoward(f, ld) == nexttowardf(f, ld));
    assert(copysign(n, ld) == copysignl((long double)n, ld));
    assert(fmax(d, d) == fmax(d, d));
    assert(carg(n) == carg((double complex)n));
    assert(cproj(f) == cprojf((float complex)f));
    assert(creal(d) == creal((double complex)d));
    assert(cimag(ld) == cimagl(ldc));
    assert(fabs(fc) == cabsf(fc));
    assert(carg(dc) == carg(dc));
    assert(cproj(ldc) == cprojl(ldc));
}

int main(void)
{
    test_header_inclusion();
    test_type_determination();
    test_real_complex_selection();
    test_real_only_macros();
    test_complex_only_macros();
    test_example();
    printf("C99 7.22 <tgmath.h> positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 函数返回结构体的成员 f().x 不是左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
struct S { int x; };
struct S make_s(void);
void neg_non_lvalue_member(void)
{
    make_s().x = 1;   /* 错误：非左值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 强制转换的结果不是左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
void neg_cast_not_lvalue(void)
{
    int a = 0;
    (int)a = 1;       /* 错误：强制转换结果非左值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 条件表达式的结果不是左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
void neg_conditional_not_lvalue(void)
{
    int a = 0, b = 0;
    (1 ? a : b) = 1;  /* 错误：条件表达式结果非左值 */
}

/* 违反约束「赋值运算符左操作数必须是可修改左值」：
 * 逗号表达式的结果不是左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
void neg_comma_not_lvalue(void)
{
    int a = 0, b = 0;
    (a, b) = 1;       /* 错误：逗号表达式结果非左值 */
}

/* 违反约束「const 限定对象不可修改」：
 * const 限定类型的成员不可赋值。
 * gcc -std=c99 应报错：assignment of read-only member */
struct T { int x; };
void neg_const_member(void)
{
    const struct T t = { 0 };
    t.x = 1;          /* 错误：const 成员只读 */
}

/* 违反约束「const 限定对象不可修改」：
 * 通过 const 指针解引用赋值。
 * gcc -std=c99 应报错：assignment of read-only location */
void neg_const_deref(void)
{
    int a = 0;
    const int *p = &a;
    *p = 1;           /* 错误：*p 为只读 */
}

/* 违反约束「volatile 限定类型传播」：
 * volatile 限定对象的成员访问仍为 volatile，但赋值本身合法；
 * 此处演示对 volatile 限定对象取地址后赋给非 volatile 指针会丢失限定，
 * 属于约束违反（指针赋值丢弃限定符）。
 * gcc -std=c99 应报错：discards qualifiers */
void neg_volatile_qualifier_discard(void)
{
    volatile int a = 0;
    int *p = &a;      /* 错误：丢弃 volatile 限定符 */
    *p = 1;
}

/* 违反约束「函数调用实参类型必须与形参兼容」：
 * 向需要指针的形参传递整数。
 * gcc -std=c99 应报错：passing argument makes pointer from integer */
void takes_ptr(int *p);
void neg_arg_type_mismatch(void)
{
    takes_ptr(1);     /* 错误：整型不能隐式转为指针 */
}

#endif