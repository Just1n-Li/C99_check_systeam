/*
 * 测试 C99 7.12.9.3 —— nearbyint / nearbyintf / nearbyintl
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应被编译器拒绝（编译报错），
 *             这些片段统一放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] 函数声明（原型）与头文件 <math.h>
 *   [2] 语义：按当前舍入方向舍入到浮点格式的整数值，且不引发 "inexact" 异常
 *   [3] 返回值：返回舍入后的整数值
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* 辅助：判断浮点值是否为整数值（无小数部分） */
static int is_integral_value(double v)
{
    return v == floor(v);
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三个函数均已在 <math.h> 中声明，可正常调用。
     *     通过取函数指针验证其签名（double(double)、float(float)、long double(long double)）。 */
    {
        double (*pd)(double)          = nearbyint;
        float  (*pf)(float)           = nearbyintf;
        long double (*pl)(long double) = nearbyintl;
        assert(pd != NULL && pf != NULL && pl != NULL);
    }

    /* [3] 返回值：返回舍入后的整数值（在默认舍入方向 FE_TONEAREST 下）。 */
    {
        double r = nearbyint(2.5);
        assert(is_integral_value(r));   /* 结果必须是整数值 */
        assert(r == 2.0 || r == 3.0);   /* 就近舍入，2.5 落在 2 或 3 */

        double r2 = nearbyint(-2.5);
        assert(is_integral_value(r2));
        assert(r2 == -2.0 || r2 == -3.0);

        /* 已经是整数值的输入应原样返回 */
        assert(nearbyint(7.0) == 7.0);
        assert(nearbyint(-7.0) == -7.0);
        assert(nearbyint(0.0) == 0.0);
    }

    /* [2] 语义：结果始终是浮点格式的整数值（不改变类型，仍是 double/float/long double）。 */
    {
        double d = nearbyint(3.7);
        float  f = nearbyintf(3.7f);
        long double ld = nearbyintl(3.7L);

        assert(is_integral_value(d));
        assert(f == floorf(f));          /* float 结果也是整数值 */
        assert(ld == floorl(ld));        /* long double 结果也是整数值 */

        /* 类型保持：sizeof 不因调用而改变 */
        assert(sizeof(d) == sizeof(double));
        assert(sizeof(f) == sizeof(float));
        assert(sizeof(ld) == sizeof(long double));
    }

    /* [2] 语义：使用“当前舍入方向”。
     *     在 FE_TONEAREST 下 2.5 -> 2（偶舍入），3.5 -> 4；
     *     在 FE_UPWARD 下 2.1 -> 3；在 FE_DOWNWARD 下 2.9 -> 2。
     *     若平台支持 fesetround，则验证方向确实被采用。 */
    {
        int saved = fegetround();
        if (saved != -1) {
            if (fesetround(FE_TONEAREST) == 0) {
                assert(nearbyint(2.5) == 2.0);   /* 就近偶舍入 */
                assert(nearbyint(3.5) == 4.0);
            }
            if (fesetround(FE_UPWARD) == 0) {
                assert(nearbyint(2.1) == 3.0);   /* 向上 */
                assert(nearbyint(-2.1) == -2.0);
            }
            if (fesetround(FE_DOWNWARD) == 0) {
                assert(nearbyint(2.9) == 2.0);   /* 向下 */
                assert(nearbyint(-2.1) == -3.0);
            }
            if (fesetround(FE_TOWARDZERO) == 0) {
                assert(nearbyint(2.9) == 2.0);   /* 向零 */
                assert(nearbyint(-2.9) == -2.0);
            }
            fesetround(saved);
        }
    }

    /* [2] 语义：不引发 "inexact" 浮点异常。
     *     对非整数值调用 nearbyint 后，FE_INEXACT 标志不应被置位。
     *     若平台支持 fetestexcept，则验证之。 */
    {
        if (fetestexcept(FE_ALL_EXCEPT) != -1) {
            feclearexcept(FE_ALL_EXCEPT);
            volatile double v = nearbyint(2.3);   /* 2.3 非整数，但 nearbyint 不应置 inexact */
            (void)v;
            int raised = fetestexcept(FE_INEXACT);
            assert(raised == 0);                  /* 未引发 inexact */
            feclearexcept(FE_ALL_EXCEPT);
        }
    }

    /* [2][3] 边界与特殊值：±0、大整数、负零符号保持（实现相关，仅做基本一致性检查）。 */
    {
        assert(nearbyint(0.0) == 0.0);
        assert(nearbyint(-0.0) == 0.0);
        assert(nearbyint(1e15) == 1e15);      /* 已是整数值 */
        assert(nearbyint(-1e15) == -1e15);
    }

    printf("nearbyint: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 违反约束「函数调用实参必须与原型参数类型兼容」：
     * nearbyint 的原型为 double nearbyint(double)，
     * 传入结构体类型实参无法转换为 double，gcc -std=c99 应报错。 */
    struct S { int x; } s;
    nearbyint(s);

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * nearbyint 只接受 1 个参数，传 2 个应报错。 */
    nearbyint(1.0, 2.0);

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * nearbyintf 只接受 1 个参数，传 0 个应报错。 */
    nearbyintf();

    /* 违反约束「赋值目标必须是可修改的左值」：
     * 函数调用结果不是左值，不能赋值。 */
    nearbyint(1.5) = 2.0;

    /* 违反约束「取地址操作数必须是左值或函数指示符」：
     * 函数调用结果不是左值，不能取地址。 */
    double *p = &nearbyint(1.5);

    /* 违反约束「函数指示符不可被赋值」：
     * nearbyint 是函数指示符，不是可修改左值。 */
    nearbyint = 0;
#endif

    return 0;
}