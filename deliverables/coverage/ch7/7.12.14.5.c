/*
 * 测试 C99 7.12.14.5 —— islessgreater 宏
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，islessgreater(x, y) 可用；
 *             对有序操作数，其返回值等价于 (x) < (y) || (x) > (y)；
 *             对无序操作数（含 NaN），返回 0 且不引发 "invalid" 浮点异常；
 *             宏参数只求值一次（无重复求值副作用）。
 *   负向测试：违反约束的用法应导致编译报错（见文件末尾 #if 0 块）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -pedantic test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <fenv.h>

/* 用于验证「参数只求值一次」的辅助计数器 */
static int eval_count = 0;
static double counted(double v) { eval_count++; return v; }

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] Synopsis：包含 <math.h> 后 islessgreater 可用，返回 int */
    {
        int r = islessgreater(1.0, 2.0);
        assert(r == 1 || r == 0);   /* 返回值为 int 类型的 0/1 */
    }

    /* [2][3] 有序操作数：等价于 (x) < (y) || (x) > (y) */
    {
        double a = 1.0, b = 2.0;
        assert(islessgreater(a, b) == ((a) < (b) || (a) > (b)));  /* 1 */
        assert(islessgreater(a, b) == 1);

        assert(islessgreater(b, a) == ((b) < (a) || (b) > (a)));  /* 1 */
        assert(islessgreater(b, a) == 1);

        assert(islessgreater(a, a) == ((a) < (a) || (a) > (a)));  /* 0 */
        assert(islessgreater(a, a) == 0);

        /* 负数、零、无穷等有序情形 */
        assert(islessgreater(-1.0, 1.0) == 1);
        assert(islessgreater(1.0, -1.0) == 1);
        assert(islessgreater(0.0, -0.0) == 0);          /* 0 与 -0 相等 */
        assert(islessgreater(INFINITY, 1.0) == 1);
        assert(islessgreater(1.0, INFINITY) == 1);
        assert(islessgreater(INFINITY, INFINITY) == 0);
        assert(islessgreater(-INFINITY, INFINITY) == 1);
    }

    /* [2][3] 无序操作数（NaN）：返回 0，且不引发 "invalid" 异常 */
    {
        double nan_v = NAN;
        double one   = 1.0;

        /* 与 (x)<(y) || (x)>(y) 的「值」一致：NaN 参与比较均为假 => 0 */
        assert(islessgreater(nan_v, one) == 0);
        assert(islessgreater(one, nan_v) == 0);
        assert(islessgreater(nan_v, nan_v) == 0);

        /* 关键语义：不引发 "invalid" 浮点异常 */
        if (feclearexcept(FE_ALL_EXCEPT) == 0) {
            volatile int r1 = islessgreater(nan_v, one);
            volatile int r2 = islessgreater(one, nan_v);
            volatile int r3 = islessgreater(nan_v, nan_v);
            (void)r1; (void)r2; (void)r3;
            /* 若实现支持 fenv，则不应置起 FE_INVALID */
            assert(fetestexcept(FE_INVALID) == 0);
        }
    }

    /* [2] 参数只求值一次（无重复求值副作用） */
    {
        eval_count = 0;
        int r = islessgreater(counted(1.0), counted(2.0));
        assert(r == 1);
        assert(eval_count == 2);   /* 每个参数恰好求值一次 */

        eval_count = 0;
        r = islessgreater(counted(3.0), counted(3.0));
        assert(r == 0);
        assert(eval_count == 2);
    }

    /* [2] 与手写 (x)<(y) || (x)>(y) 在有序情形下结果一致（抽样） */
    {
        double vals[] = { -2.0, -1.0, 0.0, 1.0, 2.0 };
        size_t n = sizeof(vals) / sizeof(vals[0]);
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                int expect = (vals[i] < vals[j]) || (vals[i] > vals[j]);
                assert(islessgreater(vals[i], vals[j]) == expect);
            }
        }
    }

    printf("C99 7.12.14.5 islessgreater: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「islessgreater 的参数必须为 real-floating 类型」：
     * 传入复数类型（complex）不是 real-floating，gcc -std=c99 应报错。
     */
    {
        double _Complex z = 1.0 + 2.0 * I;
        (void)islessgreater(z, z);   /* 期望：编译错误（类型不匹配） */
    }

    /*
     * 违反约束「islessgreater 的参数必须为 real-floating 类型」：
     * 传入指针类型不是 real-floating，应报错。
     */
    {
        double *p = 0;
        (void)islessgreater(p, p);   /* 期望：编译错误（类型不匹配） */
    }

    /*
     * 违反约束「islessgreater 的参数必须为 real-floating 类型」：
     * 传入结构体类型不是 real-floating，应报错。
     */
    {
        struct S { double d; } s;
        (void)islessgreater(s, s);   /* 期望：编译错误（类型不匹配） */
    }

    /*
     * 违反约束「islessgreater 是宏，需要两个参数」：
     * 参数个数不足，应报错。
     */
    {
        (void)islessgreater(1.0);    /* 期望：编译错误（宏参数个数不匹配） */
    }

    /*
     * 违反约束「islessgreater 是宏，需要两个参数」：
     * 参数个数过多，应报错。
     */
    {
        (void)islessgreater(1.0, 2.0, 3.0);  /* 期望：编译错误（宏参数个数不匹配） */
    }
#endif

    return 0;
}