/*
 * 测试 C99 7.12.14.4 —— islessequal 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，islessequal(x, y) 可用，其返回值恒等于 (x) <= (y)；
 *             当 x、y 无序（NaN 参与）时，不引发 "invalid" 浮点异常。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：[1] 原型/头文件、[2] 描述（等价于 <=，且无序时不引发 invalid）、[3] 返回值。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

#pragma STDC FENV_ACCESS ON

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 头文件 <math.h> 提供 islessequal 宏，返回 int，接受两个 real-floating 实参 */
    {
        int r = islessequal(1.0, 2.0);
        assert(r == 1);
        r = islessequal(2.0, 1.0);
        assert(r == 0);
    }

    /* [2][3] islessequal(x, y) 的值恒等于 (x) <= (y)：小于、等于、大于三种情形 */
    {
        double a = 3.0, b = 5.0;
        assert(islessequal(a, b) == (a <= b));   /* 小于：1 */
        assert(islessequal(b, a) == (b <= a));   /* 大于：0 */
        assert(islessequal(a, a) == (a <= a));   /* 等于：1 */

        float fa = -1.5f, fb = -1.5f;
        assert(islessequal(fa, fb) == (fa <= fb)); /* 等于：1 */

        long double la = 0.0L, lb = 1.0L;
        assert(islessequal(la, lb) == (la <= lb)); /* 小于：1 */
    }

    /* [2] 与 (x) <= (y) 的等价性：对普通有序值逐一比对 */
    {
        double vals[] = { -2.0, -1.0, 0.0, 1.0, 2.0 };
        int i, j;
        for (i = 0; i < 5; i++) {
            for (j = 0; j < 5; j++) {
                assert(islessequal(vals[i], vals[j]) == (vals[i] <= vals[j]));
            }
        }
    }

    /* [2] 关键语义：x 与 y 无序（NaN 参与）时，islessequal 不引发 "invalid" 异常，
     *     而普通 (x) <= (y) 会引发。这里验证 islessequal 不引发。 */
    {
        double nan_val = NAN;
        double one = 1.0;
        int r;

        feclearexcept(FE_ALL_EXCEPT);
        r = islessequal(nan_val, one);   /* NaN 与 1.0 无序 */
        assert(r == 0);                  /* 无序时 (x) <= (y) 为假 */
        assert(fetestexcept(FE_INVALID) == 0); /* 不引发 invalid */

        feclearexcept(FE_ALL_EXCEPT);
        r = islessequal(one, nan_val);   /* 1.0 与 NaN 无序 */
        assert(r == 0);
        assert(fetestexcept(FE_INVALID) == 0); /* 不引发 invalid */

        feclearexcept(FE_ALL_EXCEPT);
        r = islessequal(nan_val, nan_val); /* 两个 NaN 无序 */
        assert(r == 0);
        assert(fetestexcept(FE_INVALID) == 0); /* 不引发 invalid */
    }

    /* [2][3] 无序时 islessequal 的值仍等于 (x) <= (y) 的值（均为 0） */
    {
        double nan_val = NAN;
        double one = 1.0;
        assert(islessequal(nan_val, one) == (nan_val <= one));
        assert(islessequal(one, nan_val) == (one <= nan_val));
    }

    /* [2] 无穷大参与的有序比较 */
    {
        double inf = INFINITY;
        assert(islessequal(1.0, inf) == 1);
        assert(islessequal(inf, 1.0) == 0);
        assert(islessequal(inf, inf) == 1);
        assert(islessequal(-inf, 1.0) == 1);
    }

    printf("正向测试全部通过：islessequal 语义符合 C99 7.12.14.4\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「islessequal 接受 real-floating 实参」：
     * 传入结构体类型实参，gcc -std=c99 应报错（类型不兼容）。 */
    struct S { int x; } s1, s2;
    islessequal(s1, s2);

    /* 违反约束「islessequal 接受 real-floating 实参」：
     * 传入指针类型实参，gcc -std=c99 应报错。 */
    int *p1, *p2;
    islessequal(p1, p2);

    /* 违反约束「islessequal 接受 real-floating 实参」：
     * 传入复数类型实参（C99 复数不是 real-floating），gcc -std=c99 应报错。 */
    double _Complex c1, c2;
    islessequal(c1, c2);

    /* 违反约束「islessequal 是宏，需两个实参」：
     * 只给一个实参，gcc -std=c99 应报错（宏参数数目不匹配）。 */
    islessequal(1.0);

    /* 违反约束「islessequal 是宏，需两个实参」：
     * 给三个实参，gcc -std=c99 应报错。 */
    islessequal(1.0, 2.0, 3.0);

#endif

    return 0;
}