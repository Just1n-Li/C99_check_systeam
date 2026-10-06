/*
 * 测试 C99 7.12.12.1 —— fdim / fdimf / fdiml 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 条款要点：
 *   [1] 三个函数原型：double fdim(double,double);
 *       float fdimf(float,float); long double fdiml(long double,long double);
 *   [2] 语义：x > y 时返回 x - y；x <= y 时返回 +0。可能发生范围错误。
 *   [3] 返回正值差。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证三个函数均已声明且类型正确 */
static double (*p_fdim)(double, double)   = fdim;
static float  (*p_fdimf)(float, float)    = fdimf;
static long double (*p_fdiml)(long double, long double) = fdiml;

int main(void)
{
    /* [1] 三个函数可正常调用 */
    double      d = fdim(5.0, 3.0);
    float       f = fdimf(5.0f, 3.0f);
    long double l = fdiml(5.0L, 3.0L);

    /* [2][3] x > y 时返回 x - y（正值差） */
    assert(d == 2.0);
    assert(f == 2.0f);
    assert(l == 2.0L);

    /* [2][3] x < y 时返回 +0 */
    assert(fdim(3.0, 5.0) == 0.0);
    assert(fdimf(3.0f, 5.0f) == 0.0f);
    assert(fdiml(3.0L, 5.0L) == 0.0L);

    /* [2][3] x == y 时返回 +0 */
    assert(fdim(4.0, 4.0) == 0.0);
    assert(fdimf(4.0f, 4.0f) == 0.0f);
    assert(fdiml(4.0L, 4.0L) == 0.0L);

    /* [2][3] 返回 +0 时符号位应为正（+0 而非 -0） */
    assert(signbit(fdim(3.0, 5.0)) == 0);
    assert(signbit(fdim(4.0, 4.0)) == 0);
    assert(signbit(fdimf(3.0f, 5.0f)) == 0);
    assert(signbit(fdiml(3.0L, 5.0L)) == 0);

    /* [2][3] 负值参与运算：x > y 时结果仍为正值差 */
    assert(fdim(-1.0, -4.0) == 3.0);
    assert(fdim(-4.0, -1.0) == 0.0);
    assert(fdim(1.0, -4.0) == 5.0);
    assert(fdim(-4.0, 1.0) == 0.0);

    /* [2][3] 零与零：+0 - +0 情形返回 +0 */
    assert(fdim(0.0, 0.0) == 0.0);
    assert(fdim(0.0, -0.0) == 0.0);   /* 0.0 <= -0.0 为真，返回 +0 */
    assert(fdim(-0.0, 0.0) == 0.0);

    /* [2][3] 大数：正值差 */
    assert(fdim(1e300, 1.0) == 1e300 - 1.0);

    /* [2][3] 小数：正值差 */
    assert(fdim(1.0, 0.5) == 0.5);
    assert(fdimf(1.0f, 0.25f) == 0.75f);

    /* [2] 范围错误可能发生：x - y 溢出时结果可为 HUGE_VAL（不强制断言，
     *     仅验证调用不崩溃且返回有限或无穷，符合“may occur”的宽松语义） */
    {
        double big = fdim(DBL_MAX, -DBL_MAX); /* 可能溢出，可能置 errno/范围错误 */
        (void)big; /* 不强制断言具体值，因为范围错误是“可能”发生 */
    }

    /* [1] 通过函数指针调用，验证原型类型 */
    assert(p_fdim(10.0, 4.0) == 6.0);
    assert(p_fdimf(10.0f, 4.0f) == 6.0f);
    assert(p_fdiml(10.0L, 4.0L) == 6.0L);

    /* [2][3] 与 fabs(x - y) 在 x > y 时一致（语义交叉验证） */
    assert(fdim(7.5, 2.5) == fabs(7.5 - 2.5));

    printf("All positive tests for C99 7.12.12.1 (fdim/fdimf/fdiml) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「函数调用实参个数必须与原型一致」：
 * fdim 原型为 double fdim(double, double)，只传一个实参。
 * gcc -std=c99 应报错：too few arguments to function 'fdim' */
double bad1 = fdim(1.0);

/* 违反约束「函数调用实参个数必须与原型一致」：
 * 传三个实参。gcc -std=c99 应报错：too many arguments to function 'fdim' */
double bad2 = fdim(1.0, 2.0, 3.0);

/* 违反约束「实参类型必须可转换为形参类型」：
 * 传入结构体，无法转换为 double。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fdim' */
struct S { int x; } s;
double bad3 = fdim(s, 1.0);

/* 违反约束「实参类型必须可转换为形参类型」：
 * 传入指针，无法隐式转换为 double。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fdim' */
double bad4 = fdim((int *)0, 1.0);

/* 违反约束「函数返回类型不可作为赋值目标（非左值）」：
 * 函数调用结果不是左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
fdim(1.0, 2.0) = 3.0;

/* 违反约束「取地址操作数必须是左值」：
 * 函数调用结果不是左值，不能取地址。
 * gcc -std=c99 应报错：lvalue required as unary '&' operand */
double *bad5 = &fdim(1.0, 2.0);

/* 违反约束「fdimf 形参为 float，实参须可转换为 float」：
 * 传入结构体，无法转换。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fdimf' */
float bad6 = fdimf(s, 1.0f);

/* 违反约束「fdiml 形参为 long double，实参须可转换为 long double」：
 * 传入结构体，无法转换。
 * gcc -std=c99 应报错：incompatible type for argument 2 of 'fdiml' */
long double bad7 = fdiml(1.0L, s);

#endif