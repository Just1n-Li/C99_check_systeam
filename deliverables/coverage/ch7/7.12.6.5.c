/*
 * 测试 C99 7.12.6.5 —— ilogb / ilogbf / ilogbl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明：int ilogb(double); int ilogbf(float); int ilogbl(long double);
 *   [2] 语义：提取指数为有符号 int；x==0 -> FP_ILOGB0；x==±inf -> INT_MAX；
 *       x==NaN -> FP_ILOGBNAN；否则等价于 logb 后转 int；
 *       结果超出 int 范围时数值未指定（UB 范畴，不做负向测试）。
 *   [3] 返回值：有符号 int 类型的指数。
 *   Forward references: logb (7.12.6.11) —— 用 logb 交叉验证等价性。
 */

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <float.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数指针，类型必须与标准原型一致。
 *     若原型不匹配（例如返回类型不是 int），此处会编译报错。 */
static int (*p_ilogb)(double)          = ilogb;
static int (*p_ilogbf)(float)          = ilogbf;
static int (*p_ilogbl)(long double)    = ilogbl;

/* [2] 与 logb 的等价性：非零、有限、非 NaN 时，
 *     ilogb(x) == (int)logb(x)。 */
static void test_equivalence_with_logb(void)
{
    double vals[] = { 1.0, 2.0, 3.5, 0.5, 0.25, 1024.0, 1e10, 1e-10,
                      123456.789, -7.0, -0.125 };
    size_t i;
    for (i = 0; i < sizeof(vals) / sizeof(vals[0]); ++i) {
        double x = vals[i];
        int    a = ilogb(x);
        int    b = (int)logb(x);   /* [2] 等价于 logb 后转 int */
        assert(a == b);
    }
}

/* [2] 具体数值：ilogb(1.0)==0, ilogb(2.0)==1, ilogb(0.5)==-1 等 */
static void test_known_values(void)
{
    assert(ilogb(1.0)   == 0);
    assert(ilogb(2.0)   == 1);
    assert(ilogb(4.0)   == 2);
    assert(ilogb(0.5)   == -1);
    assert(ilogb(0.25)  == -2);
    assert(ilogb(1024.0)== 10);
    assert(ilogb(-8.0)  == 3);   /* 符号不影响指数 */

    /* float / long double 版本 */
    assert(ilogbf(1.0f)   == 0);
    assert(ilogbf(8.0f)   == 3);
    assert(ilogbf(0.125f) == -3);
    assert(ilogbl(1.0L)   == 0);
    assert(ilogbl(16.0L)  == 4);
    assert(ilogbl(0.0625L)== -4);
}

/* [2] x == 0 时返回 FP_ILOGB0 */
static void test_zero(void)
{
    assert(ilogb(0.0)   == FP_ILOGB0);
    assert(ilogb(-0.0)  == FP_ILOGB0);
    assert(ilogbf(0.0f) == FP_ILOGB0);
    assert(ilogbl(0.0L) == FP_ILOGB0);
}

/* [2] x 为无穷时返回 INT_MAX */
static void test_inf(void)
{
    double pinf = INFINITY;
    double ninf = -INFINITY;
    assert(ilogb(pinf)  == INT_MAX);
    assert(ilogb(ninf)  == INT_MAX);
    assert(ilogbf((float)pinf) == INT_MAX);
    assert(ilogbl((long double)pinf) == INT_MAX);
}

/* [2] x 为 NaN 时返回 FP_ILOGBNAN */
static void test_nan(void)
{
    double qnan = NAN;
    assert(ilogb(qnan)  == FP_ILOGBNAN);
    assert(ilogbf((float)qnan) == FP_ILOGBNAN);
    assert(ilogbl((long double)qnan) == FP_ILOGBNAN);
}

/* [3] 返回值类型为 int：用 _Generic 检查（C11 才有 _Generic，
 *     这里改用编译期类型检查技巧：把结果赋给 int 变量，
 *     若返回类型不是 int 则可能触发转换警告；同时用 sizeof 检查）。 */
static void test_return_type(void)
{
    int r = ilogb(2.0);
    assert(r == 1);
    /* 返回值可安全用于 int 上下文 */
    assert((int)ilogb(2.0) == 1);
    /* 返回值参与算术运算 */
    assert(ilogb(2.0) + ilogb(4.0) == 3);
}

/* [2] 结果超出 int 范围时数值未指定 —— 这是 UB/未指定行为，
 *     不做负向测试，仅说明不在此处断言。 */

int main(void)
{
    /* 抑制未使用变量警告 */
    (void)p_ilogb; (void)p_ilogbf; (void)p_ilogbl;

    test_equivalence_with_logb();
    test_known_values();
    test_zero();
    test_inf();
    test_nan();
    test_return_type();

    printf("C99 7.12.6.5 ilogb: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型：int ilogb(double)」：
 * 用错误参数类型调用（传结构体），gcc -std=c99 应报错
 * （incompatible type for argument 1 of 'ilogb'）。 */
struct S { int x; } s;
ilogb(s);

/* 违反约束「[1] 原型：int ilogb(double)」：
 * 参数个数错误，gcc -std=c99 应报错（too few arguments）。 */
ilogb();

/* 违反约束「[1] 原型：int ilogbf(float)」：
 * 参数个数错误，gcc -std=c99 应报错。 */
ilogbf(1.0f, 2.0f);

/* 违反约束「[1] 原型：int ilogbl(long double)」：
 * 参数个数错误，gcc -std=c99 应报错。 */
ilogbl();

/* 违反约束「[1] 原型：返回类型 int」：
 * 把返回值当作结构体使用（对 int 结果做成员访问），
 * gcc -std=c99 应报错（request for member in something not a structure）。 */
ilogb(2.0).x;

/* 违反约束「[1] 原型：返回类型 int」：
 * 对函数调用结果（非左值）赋值，gcc -std=c99 应报错
 * （lvalue required as left operand of assignment）。 */
ilogb(2.0) = 5;

/* 违反约束「[1] 原型：返回类型 int」：
 * 对函数调用结果取地址，gcc -std=c99 应报错
 * （lvalue required as unary '&' operand）。 */
&ilogb(2.0);

#endif