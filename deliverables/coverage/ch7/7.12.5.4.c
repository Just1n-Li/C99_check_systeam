/*
 * 测试 C99 7.12.5.4 —— cosh 函数族 (cosh / coshf / coshl)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 声明与原型：double cosh(double); float coshf(float); long double coshl(long double);
 *   [2] 语义：计算双曲余弦；|x| 过大时发生 range error。
 *   [3] 返回值：返回 cosh x。
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：三个函数应可用，且返回类型分别为 double / float / long double。
 *     通过取函数指针并比较返回类型来静态验证原型。 */
static double (*p_cosh)(double)          = cosh;
static float  (*p_coshf)(float)          = coshf;
static long double (*p_coshl)(long double) = coshl;

/* 用 _Generic 无法在 C99 使用，改用赋值兼容性检查返回类型。 */
static void check_prototypes(void)
{
    double d;
    float  f;
    long double ld;

    /* [1] 调用形式：接受一个实参，返回对应类型 */
    d  = cosh(0.0);
    f  = coshf(0.0f);
    ld = coshl(0.0L);

    /* 若返回类型不符，下面的赋值会因精度/类型不匹配产生告警或错误；
     * 这里用 sizeof 静态断言返回类型大小。 */
    assert(sizeof(d)  == sizeof(double));
    assert(sizeof(f)  == sizeof(float));
    assert(sizeof(ld) == sizeof(long double));

    (void)p_cosh; (void)p_coshf; (void)p_coshl;
}

/* [2][3] 语义与返回值：cosh(0) == 1，cosh 为偶函数，cosh x = (e^x + e^-x)/2 */
static void check_semantics(void)
{
    /* [3] cosh(0) = 1 */
    assert(cosh(0.0) == 1.0);
    assert(coshf(0.0f) == 1.0f);
    assert(coshl(0.0L) == 1.0L);

    /* [2][3] 偶函数：cosh(-x) == cosh(x) */
    assert(cosh(1.0) == cosh(-1.0));
    assert(coshf(2.0f) == coshf(-2.0f));
    assert(coshl(3.0L) == coshl(-3.0L));

    /* [2][3] 与定义式 (e^x + e^-x)/2 一致（用容差比较） */
    {
        double x = 1.5;
        double expected = (exp(x) + exp(-x)) / 2.0;
        double got = cosh(x);
        assert(fabs(got - expected) < 1e-12);
    }
    {
        float x = 0.75f;
        float expected = (expf(x) + expf(-x)) / 2.0f;
        float got = coshf(x);
        assert(fabsf(got - expected) < 1e-5f);
    }
    {
        long double x = 2.25L;
        long double expected = (expl(x) + expl(-x)) / 2.0L;
        long double got = coshl(x);
        assert(fabsl(got - expected) < 1e-15L);
    }

    /* [2][3] 已知值：cosh(1) ≈ 1.5430806348152437 */
    assert(fabs(cosh(1.0) - 1.5430806348152437) < 1e-12);

    /* [2] 大参数：cosh 单调递增，cosh(10) > cosh(1) */
    assert(cosh(10.0) > cosh(1.0));

    /* [2] range error 情形：|x| 过大时函数仍返回一个值（可能为 HUGE_VAL 或 inf），
     *     标准只要求发生 range error，不要求特定返回值，故此处仅调用不 assert 具体值。 */
    {
        volatile double big = 1e300;
        double r = cosh(big);   /* 可能触发 range error，返回 HUGE_VAL */
        (void)r;
    }
}

int main(void)
{
    check_prototypes();
    check_semantics();
    printf("C99 7.12.5.4 cosh functions: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「cosh 接受一个实参」：实参个数不匹配，gcc -std=c99 应报错
 * error: too few arguments to function 'cosh' */
double a1 = cosh();

/* 违反约束「cosh 接受一个实参」：实参过多，应报错
 * error: too many arguments to function 'cosh' */
double a2 = cosh(1.0, 2.0);

/* 违反约束「实参须为算术类型」：结构体不能传给 cosh，应报错
 * error: incompatible type for argument 1 of 'cosh' */
struct S { int x; } s;
double a3 = cosh(s);

/* 违反约束「实参须为算术类型」：指针不能传给 cosh，应报错
 * error: incompatible type for argument 1 of 'cosh' */
double a4 = cosh("hello");

/* 违反约束「coshf 参数为 float」：用不兼容的指针类型实参，应报错
 * error: incompatible type for argument 1 of 'coshf' */
float a5 = coshf(&s);

/* 违反约束「coshl 参数为 long double」：结构体实参，应报错
 * error: incompatible type for argument 1 of 'coshl' */
long double a6 = coshl(s);

/* 违反约束「函数返回非左值」：对函数调用结果赋值，应报错
 * error: lvalue required as left operand of assignment */
void bad_assign(void) {
    cosh(1.0) = 2.0;
}

/* 违反约束「函数返回非左值」：对函数调用结果取地址，应报错
 * error: lvalue required as unary '&' operand */
void bad_addr(void) {
    double *p = &cosh(1.0);
    (void)p;
}

#endif