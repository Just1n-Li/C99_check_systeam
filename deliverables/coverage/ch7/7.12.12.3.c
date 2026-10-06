/*
 * 测试条款：ISO/IEC 9899:1999 (C99) 7.12.12.3 —— fmin 函数族
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用 fmin / fminf / fminl，验证其返回两个参数中的
 *             最小数值（[2][3]），并验证 NaN 处理（脚注 214：与 fmax 类似，
 *             即若一个参数为 NaN，则返回另一个参数）。
 *   负向测试：违反约束的代码（参数个数错误、参数类型不可转换等）应编译报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型声明检查：三个函数均可用，且返回类型分别为 double/float/long double */
static void test_synopsis(void)
{
    double (*pd)(double, double) = fmin;
    float  (*pf)(float, float)   = fminf;
    long double (*pl)(long double, long double) = fminl;
    (void)pd; (void)pf; (void)pl;
    printf("[1] synopsis OK\n");
}

/* [2][3] fmin：返回两个参数中的最小数值 */
static void test_fmin_double(void)
{
    assert(fmin(1.0, 2.0) == 1.0);
    assert(fmin(2.0, 1.0) == 1.0);
    assert(fmin(-3.5, -1.5) == -3.5);
    assert(fmin(-1.5, -3.5) == -3.5);
    assert(fmin(0.0, 0.0) == 0.0);
    assert(fmin(-0.0, 0.0) == -0.0 || fmin(-0.0, 0.0) == 0.0);
    assert(fmin(3.25, 3.25) == 3.25);
    printf("[2][3] fmin(double) OK\n");
}

/* [2][3] fminf：float 版本 */
static void test_fminf(void)
{
    assert(fminf(1.0f, 2.0f) == 1.0f);
    assert(fminf(2.0f, 1.0f) == 1.0f);
    assert(fminf(-3.5f, -1.5f) == -3.5f);
    assert(fminf(0.0f, 0.0f) == 0.0f);
    printf("[2][3] fminf OK\n");
}

/* [2][3] fminl：long double 版本 */
static void test_fminl(void)
{
    assert(fminl(1.0L, 2.0L) == 1.0L);
    assert(fminl(2.0L, 1.0L) == 1.0L);
    assert(fminl(-3.5L, -1.5L) == -3.5L);
    assert(fminl(0.0L, 0.0L) == 0.0L);
    printf("[2][3] fminl OK\n");
}

/* 脚注 214：fmin 对 NaN 的处理与 fmax 类似 —— 若一个参数为 NaN，
 * 则返回另一个参数（数值）。 */
static void test_nan_handling(void)
{
    double nan_val = NAN;

    /* fmin(NaN, y) == y */
    assert(fmin(nan_val, 5.0) == 5.0);
    /* fmin(x, NaN) == x */
    assert(fmin(5.0, nan_val) == 5.0);
    assert(fmin(-2.5, nan_val) == -2.5);
    assert(fmin(nan_val, -2.5) == -2.5);

    /* 两个都是 NaN 时，结果也是 NaN */
    assert(isnan(fmin(nan_val, nan_val)));

    /* float / long double 版本同样处理 */
    assert(fminf((float)NAN, 3.0f) == 3.0f);
    assert(fminf(3.0f, (float)NAN) == 3.0f);
    assert(fminl((long double)NAN, 3.0L) == 3.0L);
    assert(fminl(3.0L, (long double)NAN) == 3.0L);

    printf("[footnote 214] NaN handling OK\n");
}

/* 与 fmax 的对称性检查（脚注 214 提到二者对 NaN 处理类似） */
static void test_symmetry_with_fmax(void)
{
    double a = 1.5, b = -2.5;
    assert(fmin(a, b) == b);
    assert(fmax(a, b) == a);

    double nan_val = NAN;
    assert(fmin(nan_val, a) == a);
    assert(fmax(nan_val, a) == a);

    printf("[footnote 214] symmetry with fmax OK\n");
}

int main(void)
{
    test_synopsis();
    test_fmin_double();
    test_fminf();
    test_fminl();
    test_nan_handling();
    test_symmetry_with_fmax();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用参数个数必须与原型一致」：
 * fmin 原型为 double fmin(double, double)，只传一个参数。
 * gcc -std=c99 应报错：too few arguments to function 'fmin' */
double bad1 = fmin(1.0);

/* 违反约束「函数调用参数个数必须与原型一致」：
 * 传三个参数。
 * gcc -std=c99 应报错：too many arguments to function 'fmin' */
double bad2 = fmin(1.0, 2.0, 3.0);

/* 违反约束「实参类型必须能转换为形参类型」：
 * 传入结构体，无法转换为 double。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fmin' */
struct S { int x; };
struct S s;
double bad3 = fmin(s, 1.0);

/* 违反约束「实参类型必须能转换为形参类型」：
 * 传入指针，无法隐式转换为 double。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fmin' */
double bad4 = fmin((void *)0, 1.0);

/* 违反约束「fminf 形参为 float，实参须可转换」：
 * 传入结构体。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fminf' */
float bad5 = fminf(s, 1.0f);

/* 违反约束「fminl 形参为 long double，实参须可转换」：
 * 传入结构体。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'fminl' */
long double bad6 = fminl(s, 1.0L);

/* 违反约束「函数返回值不可作为左值赋值」：
 * fmin 返回非左值，不能赋值。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment */
void bad7(void) { fmin(1.0, 2.0) = 3.0; }

/* 违反约束「函数返回值不可取地址」：
 * fmin 返回非左值，不能取地址。
 * gcc -std=c99 应报错：lvalue required as unary '&' operand */
void bad8(void) { double *p = &fmin(1.0, 2.0); (void)p; }

#endif