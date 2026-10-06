/*
 * 测试条款：C99 7.6.2.1  feclearexcept 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错），
 *             这些片段被放在 #if 0 ... #endif 中，不影响本文件编译。
 *
 * 覆盖段落：
 *   [1] 头文件 <fenv.h> 与原型 int feclearexcept(int excepts);
 *   [2] 语义：尝试清除参数所表示的、受支持的浮点异常。
 *   [3] 返回值：excepts 为 0 时返回 0；所有指定异常成功清除时返回 0；
 *               否则返回非零值。
 */

#include <stdio.h>
#include <assert.h>
#include <fenv.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <fenv.h> 提供 feclearexcept 的声明，返回类型为 int，
 *     参数类型为 int。这里用一个函数指针来静态验证原型签名。 */
static int (*fp_feclearexcept)(int) = feclearexcept;

int main(void)
{
    /* [1] 原型检查：函数指针类型匹配，说明声明为 int feclearexcept(int)。 */
    assert(fp_feclearexcept == feclearexcept);

    /* [3] excepts 为 0 时，feclearexcept 必须返回 0。 */
    {
        int r = feclearexcept(0);
        assert(r == 0);
    }

    /* [2][3] 清除一个受支持的异常（若实现支持 FE_DIVBYZERO）。
     * 先触发除零异常，再用 feclearexcept 清除它，然后检查异常标志已清除。
     * 若实现不支持该异常，则跳过（条款只要求处理“受支持的”异常）。 */
    {
        volatile double one = 1.0;
        volatile double zero = 0.0;
        double q;

        /* 触发 FE_DIVBYZERO（若支持）。 */
        q = one / zero;
        (void)q;

        if (fetestexcept(FE_DIVBYZERO)) {
            /* 该异常受支持且已被触发，尝试清除。 */
            int r = feclearexcept(FE_DIVBYZERO);
            /* [3] 成功清除时返回 0。 */
            assert(r == 0);
            /* [2] 清除后该异常标志应不再被置位。 */
            assert(fetestexcept(FE_DIVBYZERO) == 0);
        }
    }

    /* [2][3] 清除多个受支持的异常的组合。 */
    {
        int all = FE_ALL_EXCEPT;
        int r = feclearexcept(all);
        /* [3] 若所有指定异常都被成功清除（或本来就没有），返回 0。 */
        assert(r == 0);
        /* [2] 清除后不应再有这些异常被置位。 */
        assert(fetestexcept(all) == 0);
    }

    /* [3] 再次以 0 调用，仍应返回 0（幂等、无副作用）。 */
    assert(feclearexcept(0) == 0);

    printf("C99 7.6.2.1 feclearexcept: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「feclearexcept 的参数类型为 int」：
 * 传入结构体类型实参，gcc -std=c99 应报错
 * （incompatible type for argument 1 of 'feclearexcept'）。 */
struct S { int x; } s;
feclearexcept(s);

/* 违反约束「feclearexcept 的参数类型为 int」：
 * 传入指针类型实参，gcc -std=c99 应报错。 */
int *p = 0;
feclearexcept(p);

/* 违反约束「feclearexcept 的参数个数为 1」：
 * 不传参数，gcc -std=c99 应报错（too few arguments to function）。 */
feclearexcept();

/* 违反约束「feclearexcept 的参数个数为 1」：
 * 传两个参数，gcc -std=c99 应报错（too many arguments to function）。 */
feclearexcept(FE_DIVBYZERO, FE_INEXACT);

/* 违反约束「feclearexcept 的返回类型为 int」：
 * 把返回值赋给结构体对象，gcc -std=c99 应报错
 * （incompatible types when assigning to type 'struct S'）。 */
struct S t;
t = feclearexcept(0);

/* 违反约束「feclearexcept 的返回类型为 int」：
 * 用返回值初始化指针，gcc -std=c99 应报错
 * （initialization of 'int *' from incompatible pointer type / int）。 */
int *q = feclearexcept(0);

#endif