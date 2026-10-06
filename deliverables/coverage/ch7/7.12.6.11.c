/*
 * 测试 C99 7.12.6.11 —— logb / logbf / logbl 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 头文件 <math.h> 与三个函数原型（double/float/long double 版本）
 *   [2] 语义：提取 x 的指数（浮点格式的有符号整数值）；次正规数按正规化处理；
 *       对正有限 x 满足 1 <= x / FLT_RADIX^logb(x) < FLT_RADIX；参数为 0 时可能域错误/范围错误
 *   [3] 返回值：x 的有符号指数
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 三个函数原型均可用，返回类型分别为 double / float / long double */
    {
        double      d = logb(8.0);
        float       f = logbf(8.0f);
        long double l = logbl(8.0L);
        (void)d; (void)f; (void)l;
        /* 返回类型检查：赋值给对应类型不应有截断警告（此处仅验证可调用） */
        assert(sizeof(logb(1.0))  == sizeof(double));
        assert(sizeof(logbf(1.0f)) == sizeof(float));
        assert(sizeof(logbl(1.0L)) == sizeof(long double));
    }

    /* [3] 返回值是 x 的有符号指数：对 2 的整数次幂，logb 返回该指数 */
    {
        assert(logb(1.0)   == 0.0);
        assert(logb(2.0)   == 1.0);
        assert(logb(4.0)   == 2.0);
        assert(logb(8.0)   == 3.0);
        assert(logb(1024.0) == 10.0);
        assert(logb(0.5)   == -1.0);
        assert(logb(0.25)  == -2.0);
        assert(logb(0.125) == -3.0);
    }

    /* [3] 返回值是「浮点格式的有符号整数」：结果应为整数值（无小数部分） */
    {
        double r = logb(3.0);          /* 3.0 = 1.5 * 2^1 -> 指数 1 */
        assert(r == 1.0);
        assert(floor(r) == r);         /* 是整数值 */

        r = logb(7.0);                 /* 7.0 = 1.75 * 2^2 -> 指数 2 */
        assert(r == 2.0);
        assert(floor(r) == r);

        r = logb(0.3);                 /* 0.3 = 1.2 * 2^-2 -> 指数 -2 */
        assert(r == -2.0);
        assert(floor(r) == r);
    }

    /* [2] 对正有限 x：1 <= x / FLT_RADIX^logb(x) < FLT_RADIX */
    {
        double xs[] = { 1.0, 2.0, 3.0, 7.0, 0.3, 0.5, 123.456, 1e10, 1e-10 };
        size_t i;
        for (i = 0; i < sizeof(xs)/sizeof(xs[0]); ++i) {
            double x = xs[i];
            double e = logb(x);
            double m = x / pow((double)FLT_RADIX, e);
            assert(m >= 1.0);
            assert(m < (double)FLT_RADIX);
        }
    }

    /* [2] 次正规数按正规化处理：logb 返回其「正规化后」的指数 */
    {
        /* DBL_MIN 是最小正规数，其指数为 DBL_MIN_EXP - 1 */
        double dmin = DBL_MIN;
        assert(logb(dmin) == (double)(DBL_MIN_EXP - 1));

        /* 构造一个次正规数：DBL_MIN / 2 是次正规数（若支持） */
        double sub = DBL_MIN / 2.0;
        if (sub > 0.0 && sub < DBL_MIN) {
            /* 次正规数被当作已正规化：指数比 DBL_MIN 的指数小 1 */
            assert(logb(sub) == (double)(DBL_MIN_EXP - 2));
        }

        /* 更小的次正规数：DBL_MIN / 4 */
        double sub2 = DBL_MIN / 4.0;
        if (sub2 > 0.0 && sub2 < DBL_MIN) {
            assert(logb(sub2) == (double)(DBL_MIN_EXP - 3));
        }
    }

    /* [2] 负参数：logb 提取的是指数（与符号无关），返回同样的有符号指数 */
    {
        assert(logb(-8.0) == 3.0);
        assert(logb(-0.5) == -1.0);
        assert(logb(-3.0) == 1.0);
    }

    /* [2] 无穷大：logb(+inf) 返回 +inf（指数无穷大） */
    {
        double inf = INFINITY;
        double r = logb(inf);
        assert(isinf(r) && r > 0.0);
        r = logb(-inf);
        assert(isinf(r) && r > 0.0);
    }

    /* [2] NaN：logb(NaN) 返回 NaN */
    {
        double r = logb(NAN);
        assert(isnan(r));
    }

    /* [2] 参数为 0：可能发生域错误或范围错误（实现定义）。
     *     此处只验证调用不崩溃，并检查返回值是否为 -HUGE_VAL 或 -inf（常见实现）。
     *     不把具体返回值作为硬性断言，因为标准允许域错误/范围错误。 */
    {
        double r = logb(0.0);
        /* 常见实现返回 -HUGE_VAL；标准允许域错误或范围错误 */
        (void)r;
        /* 若实现返回有限值，也不违反标准（域错误/范围错误是「可能」发生） */
    }

    /* [1][2][3] float 与 long double 版本语义一致 */
    {
        assert(logbf(8.0f) == 3.0f);
        assert(logbf(0.5f) == -1.0f);
        assert(logbl(8.0L) == 3.0L);
        assert(logbl(0.5L) == -1.0L);

        /* 次正规 float */
        float fsub = FLT_MIN / 2.0f;
        if (fsub > 0.0f && fsub < FLT_MIN) {
            assert(logbf(fsub) == (float)(FLT_MIN_EXP - 2));
        }

        /* 次正规 long double */
        long double lsub = LDBL_MIN / 2.0L;
        if (lsub > 0.0L && lsub < LDBL_MIN) {
            assert(logbl(lsub) == (long double)(LDBL_MIN_EXP - 2));
        }
    }

    printf("C99 7.12.6.11 logb/logbf/logbl: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反约束「logb 的参数必须为实数浮点类型（算术类型）」：
     * 传入结构体类型，gcc -std=c99 应报错（incompatible type for argument）。
     */
    struct S { int x; } s;
    logb(s);

    /*
     * 违反约束「logb 的参数必须为算术类型」：
     * 传入指针类型，gcc -std=c99 应报错。
     */
    int *p = 0;
    logb(p);

    /*
     * 违反约束「logb 的参数个数必须为 1」：
     * 少传参数，gcc -std=c99 应报错（too few arguments to function 'logb'）。
     */
    logb();

    /*
     * 违反约束「logb 的参数个数必须为 1」：
     * 多传参数，gcc -std=c99 应报错（too many arguments to function 'logb'）。
     */
    logb(1.0, 2.0);

    /*
     * 违反约束「logbf 的参数必须为实数浮点类型」：
     * 传入结构体，gcc -std=c99 应报错。
     */
    logbf(s);

    /*
     * 违反约束「logbl 的参数必须为实数浮点类型」：
     * 传入指针，gcc -std=c99 应报错。
     */
    logbl(p);

    /*
     * 违反约束「logb 的返回值不可作为左值赋值」：
     * 函数调用结果不是左值，对其赋值应编译报错（lvalue required as left operand of assignment）。
     */
    logb(1.0) = 2.0;

    /*
     * 违反约束「logbf 的返回值不可作为左值赋值」：
     * 应编译报错。
     */
    logbf(1.0f) = 2.0f;

    /*
     * 违反约束「logbl 的返回值不可作为左值赋值」：
     * 应编译报错。
     */
    logbl(1.0L) = 2.0L;
#endif

    return 0;
}