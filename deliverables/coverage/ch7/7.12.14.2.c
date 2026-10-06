/*
 * 测试 C99 7.12.14.2 —— isgreaterequal 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，isgreaterequal(x, y) 可用；
 *             其值恒等于 (x) >= (y)；
 *             当 x、y 无序（NaN 参与）时，不引发 "invalid" 浮点异常，
 *             且返回 0（因为 (x) >= (y) 为假）。
 *   负向测试：违反约束的代码应导致编译报错（见文件末尾 #if 0 块）。
 *
 * 覆盖段落：[1] 概要、[2] 描述、[3] 返回值。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* 辅助：判断当前是否设置了 FE_INVALID 标志 */
static int invalid_raised(void)
{
    return fetestexcept(FE_INVALID) != 0;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 概要：isgreaterequal 是 <math.h> 中声明的宏，接受两个 real-floating 实参。
     *     这里用 double 与 float 实参验证其可用性。 */
    double a = 3.0, b = 2.0;
    float  fa = 1.5f, fb = 2.5f;

    /* [2][3] 值恒等于 (x) >= (y)：真值情形 */
    assert(isgreaterequal(a, b) == (a >= b));   /* 3.0 >= 2.0 -> 1 */
    assert(isgreaterequal(a, b) != 0);

    /* [2][3] 相等情形：>= 为真 */
    assert(isgreaterequal(a, a) == (a >= a));   /* 3.0 >= 3.0 -> 1 */
    assert(isgreaterequal(a, a) != 0);

    /* [2][3] 小于情形：>= 为假 */
    assert(isgreaterequal(b, a) == (b >= a));   /* 2.0 >= 3.0 -> 0 */
    assert(isgreaterequal(b, a) == 0);

    /* [2][3] float 实参同样适用 */
    assert(isgreaterequal(fa, fb) == (fa >= fb)); /* 1.5 >= 2.5 -> 0 */
    assert(isgreaterequal(fb, fa) == (fb >= fa)); /* 2.5 >= 1.5 -> 1 */

    /* [2] 关键语义：当 x 与 y 无序（NaN 参与）时，
     *     isgreaterequal 不引发 "invalid" 浮点异常，
     *     而普通 (x) >= (y) 会引发。 */
    {
        double nan_v = NAN;
        double one   = 1.0;

        /* 先清空浮点异常标志 */
        feclearexcept(FE_ALL_EXCEPT);

        /* 使用 isgreaterequal：不应引发 FE_INVALID */
        int r1 = isgreaterequal(nan_v, one);
        assert(r1 == 0);                 /* 无序时 (x) >= (y) 为假 */
        assert(!invalid_raised());       /* 未引发 invalid 异常 */

        /* 对照：普通 >= 在无序时会引发 FE_INVALID */
        feclearexcept(FE_ALL_EXCEPT);
        volatile int r2 = (nan_v >= one); /* 触发 invalid */
        (void)r2;
        assert(invalid_raised());        /* 普通 >= 确实引发 invalid */

        /* 再验证 isgreaterequal 在另一侧 NaN 时也不引发 */
        feclearexcept(FE_ALL_EXCEPT);
        int r3 = isgreaterequal(one, nan_v);
        assert(r3 == 0);
        assert(!invalid_raised());

        /* 两侧都是 NaN 时同样不引发，返回 0 */
        feclearexcept(FE_ALL_EXCEPT);
        int r4 = isgreaterequal(nan_v, nan_v);
        assert(r4 == 0);
        assert(!invalid_raised());
    }

    /* [2][3] 与 (x) >= (y) 的等价性在有序值上逐一比对 */
    {
        double xs[] = { -1.0, 0.0, 1.0, 2.0, 1e300, -1e300 };
        double ys[] = { -1.0, 0.0, 1.0, 2.0, 1e300, -1e300 };
        size_t i, j;
        for (i = 0; i < sizeof xs / sizeof xs[0]; ++i) {
            for (j = 0; j < sizeof ys / sizeof ys[0]; ++j) {
                int via_macro = isgreaterequal(xs[i], ys[j]);
                int via_op    = (xs[i] >= ys[j]);
                assert(via_macro == via_op);
            }
        }
    }

    printf("正向测试全部通过：isgreaterequal 语义符合 C99 7.12.14.2\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「isgreaterequal 的实参必须为 real-floating 类型」：
     * 传入结构体类型，gcc -std=c99 应报错（类型不兼容 / 无法转换）。
     */
    struct S { int x; } s1, s2;
    (void)isgreaterequal(s1, s2);

    /*
     * 违反约束「实参必须为 real-floating 类型」：
     * 传入指针类型，gcc -std=c99 应报错。
     */
    int *p1 = 0, *p2 = 0;
    (void)isgreaterequal(p1, p2);

    /*
     * 违反约束「实参必须为 real-floating 类型」：
     * 传入复数类型（C99 中 _Complex 不是 real-floating），
     * gcc -std=c99 应报错。
     */
    double _Complex c1 = 1.0, c2 = 2.0;
    (void)isgreaterequal(c1, c2);

    /*
     * 违反约束「isgreaterequal 是宏，需要两个实参」：
     * 只提供一个实参，gcc -std=c99 应报错（宏参数数目不匹配）。
     */
    (void)isgreaterequal(1.0);
#endif

    return 0;
}