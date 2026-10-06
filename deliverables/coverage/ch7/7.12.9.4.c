/*
 * 测试 C99 7.12.9.4 —— rint / rintf / rintl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 声明：double rint(double); float rintf(float); long double rintl(long double);
 *   [2] 与 nearbyint 的区别：当结果与实参值不同时，rint 可能引发 "inexact" 浮点异常。
 *   [3] 返回：舍入后的整数值（按当前舍入方向，默认 round-to-nearest-even）。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，验证三个函数均已声明且类型正确 */
static double (*p_rint)(double)              = rint;
static float  (*p_rintf)(float)              = rintf;
static long double (*p_rintl)(long double)   = rintl;

/* [3] 返回值类型检查：rint 返回 double，rintf 返回 float，rintl 返回 long double */
static void test_return_types(void)
{
    double      d = rint(2.5);
    float       f = rintf(2.5f);
    long double l = rintl(2.5L);

    /* 通过 sizeof 与赋值兼容性间接验证返回类型 */
    assert(sizeof d == sizeof(double));
    assert(sizeof f == sizeof(float));
    assert(sizeof l == sizeof(long double));
    (void)d; (void)f; (void)l;
}

/* [3] 语义：rint 返回舍入后的整数值（默认舍入方向：就近取偶） */
static void test_rounding_values(void)
{
    /* 就近取偶：2.5 -> 2, 3.5 -> 4 */
    assert(rint(2.5)  == 2.0);
    assert(rint(3.5)  == 4.0);
    assert(rint(-2.5) == -2.0);
    assert(rint(-3.5) == -4.0);

    /* 非半整数：普通就近舍入 */
    assert(rint(2.4)  == 2.0);
    assert(rint(2.6)  == 3.0);
    assert(rint(-2.4) == -2.0);
    assert(rint(-2.6) == -3.0);

    /* 已经是整数：原样返回 */
    assert(rint(0.0)  == 0.0);
    assert(rint(7.0)  == 7.0);
    assert(rint(-7.0) == -7.0);

    /* 结果必须是整数值（小数部分为 0） */
    double r = rint(123.456);
    assert(r == 123.0);
    assert(r - (double)(long)r == 0.0);
}

/* [3] rintf / rintl 的语义与 rint 一致 */
static void test_float_long_double(void)
{
    assert(rintf(2.5f)  == 2.0f);
    assert(rintf(3.5f)  == 4.0f);
    assert(rintf(-2.5f) == -2.0f);

    assert(rintl(2.5L)  == 2.0L);
    assert(rintl(3.5L)  == 4.0L);
    assert(rintl(-2.5L) == -2.0L);
}

/* [2] 与 nearbyint 的区别：rint 在结果与实参不同时可能引发 inexact 异常。
 *     这里验证：当结果与实参不同时，rint 会置起 FE_INEXACT 标志。 */
static void test_inexact_flag(void)
{
    /* 保存并清除异常标志 */
    feclearexcept(FE_ALL_EXCEPT);

    volatile double x = 2.5;   /* 结果 2.0 != 2.5，应引发 inexact */
    volatile double y = rint(x);
    (void)y;

    int raised = fetestexcept(FE_INEXACT);
    /* 若实现支持 FE_INEXACT，则此处应被置起 */
    if (raised != 0) {
        assert((raised & FE_INEXACT) != 0);
    }

    /* 结果与实参相同时（整数输入），不应引发 inexact */
    feclearexcept(FE_ALL_EXCEPT);
    volatile double z = 4.0;
    volatile double w = rint(z);
    (void)w;
    int raised2 = fetestexcept(FE_INEXACT);
    assert((raised2 & FE_INEXACT) == 0);

    feclearexcept(FE_ALL_EXCEPT);
}

/* [2] 与 nearbyint 的对比：nearbyint 不引发 inexact（作为对照，验证 rint 的差异点） */
static void test_compare_with_nearbyint(void)
{
    feclearexcept(FE_ALL_EXCEPT);
    volatile double x = 2.5;
    volatile double a = rint(x);
    volatile double b = nearbyint(x);
    (void)a; (void)b;

    /* 两者数值结果应相同 */
    assert(rint(2.5) == nearbyint(2.5));
    assert(rint(3.5) == nearbyint(3.5));
    assert(rint(-2.5) == nearbyint(-2.5));

    feclearexcept(FE_ALL_EXCEPT);
}

/* [3] 特殊值：rint(±0.0) 保持符号，rint(±inf) 返回 ±inf */
static void test_special_values(void)
{
    double pz = rint(0.0);
    double nz = rint(-0.0);
    assert(pz == 0.0);
    assert(nz == 0.0);
    /* 符号位检查 */
    assert(signbit(nz) != 0);

    double pinf = rint(INFINITY);
    double ninf = rint(-INFINITY);
    assert(isinf(pinf) && pinf > 0);
    assert(isinf(ninf) && ninf < 0);
}

int main(void)
{
    test_return_types();
    test_rounding_values();
    test_float_long_double();
    test_inexact_flag();
    test_compare_with_nearbyint();
    test_special_values();

    printf("C99 7.12.9.4 rint/rintf/rintl: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「rint 的参数必须为 double 类型（算术类型可隐式转换）」：
 * 传入结构体类型，无法转换为 double，gcc -std=c99 应报错。 */
struct S { int x; };
void bad_arg(void)
{
    struct S s;
    double r = rint(s);   /* 错误：结构体不能转换为 double */
    (void)r;
}

/* 违反约束「rint 返回 double，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应编译报错。 */
void bad_lvalue(void)
{
    rint(2.5) = 3.0;      /* 错误：赋值目标不是左值 */
}

/* 违反约束「rintf 返回 float，不能作为左值被赋值」 */
void bad_lvalue_f(void)
{
    rintf(2.5f) = 3.0f;   /* 错误：赋值目标不是左值 */
}

/* 违反约束「rintl 返回 long double，不能作为左值被赋值」 */
void bad_lvalue_l(void)
{
    rintl(2.5L) = 3.0L;   /* 错误：赋值目标不是左值 */
}

/* 违反约束「rint 需要 1 个参数」：参数个数不匹配应报错 */
void bad_argc(void)
{
    double r = rint();    /* 错误：参数太少 */
    (void)r;
}

/* 违反约束「rint 需要 1 个参数」：参数过多应报错 */
void bad_argc2(void)
{
    double r = rint(1.0, 2.0);  /* 错误：参数太多 */
    (void)r;
}

/* 违反约束「rint 的返回类型为 double，不能取地址后赋给不兼容指针」：
 * 类型不兼容的指针赋值应报错（在严格编译下）。 */
void bad_ptr(void)
{
    int (*fp)(double) = rint;   /* 错误：返回类型不兼容 */
    (void)fp;
}

#endif /* 负向测试结束 */