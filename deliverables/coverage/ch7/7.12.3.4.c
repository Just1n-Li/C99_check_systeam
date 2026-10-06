/*
 * 测试目标：C99 7.12.3.4 The isnan macro
 *
 * 预期行为：
 *   正向测试：包含 <math.h> 后，isnan(x) 对 NaN 返回非零，对非 NaN 返回 0；
 *             参数先被转换到其语义类型，再基于该类型判定。
 *   负向测试：违反约束的代码应导致编译报错（见 #if 0 块）。
 *
 * 覆盖段落：[1] 概要、[2] 描述、[3] 返回值、脚注 206。
 */

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

/* 辅助：构造一个 quiet NaN（不依赖 NAN 宏是否可用） */
static double make_nan(void)
{
    double z = 0.0;
    return z / z;   /* 0.0/0.0 产生 NaN（IEEE 754 环境） */
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 概要：isnan 是 <math.h> 中声明的宏，接受 real-floating 参数。
     *     这里验证它可被调用且返回 int 类结果。 */
    {
        double d = 1.0;
        int r = isnan(d);
        assert(r == 0);                 /* 非 NaN -> 0 */
    }

    /* [3] 返回值：当且仅当参数为 NaN 时返回非零值。
     *     测试 double 类型的 NaN 与非 NaN。 */
    {
        double nan_val = make_nan();
        double num_val = 3.14;

        assert(isnan(nan_val) != 0);    /* NaN -> 非零 */
        assert(isnan(num_val) == 0);    /* 非 NaN -> 0 */

        /* 若实现提供 NAN 宏，也一并验证 */
#ifdef NAN
        assert(isnan(NAN) != 0);
#endif
        /* 若实现提供 INFINITY，无穷不是 NaN */
#ifdef INFINITY
        assert(isnan(INFINITY) == 0);
        assert(isnan(-INFINITY) == 0);
#endif
    }

    /* [3] 返回值：对 float 类型同样成立（real-floating 涵盖 float）。 */
    {
        float fnan = (float)make_nan();
        float fnum = 2.5f;
        assert(isnan(fnan) != 0);
        assert(isnan(fnum) == 0);
    }

    /* [3] 返回值：对 long double 类型同样成立。 */
    {
        long double lnan = (long double)make_nan();
        long double lnum = 1.0L;
        assert(isnan(lnan) != 0);
        assert(isnan(lnum) == 0);
    }

    /* [2] 描述：参数若以比其语义类型更宽的格式表示，先转换到语义类型，
     *     再基于该类型判定。这里用 float 参数（可能被提升/以更宽格式求值）
     *     验证结果仍正确。 */
    {
        float f = (float)make_nan();
        /* 无论内部以何种宽度求值，语义类型是 float，结果应为 NaN */
        assert(isnan(f) != 0);

        float g = 0.0f;
        assert(isnan(g) == 0);
    }

    /* [2] 描述：基于参数类型判定。对同一 NaN 值，用不同语义类型
     *     （float / double / long double）判定结果一致。 */
    {
        double dnan = make_nan();
        float  fnan = (float)dnan;
        long double lnan = (long double)dnan;

        assert(isnan(dnan) != 0);
        assert(isnan(fnan) != 0);
        assert(isnan(lnan) != 0);
    }

    /* [3] 返回值：isnan 的结果可直接用于条件判断（非零即真）。 */
    {
        double d = make_nan();
        if (isnan(d)) {
            /* 正确进入此分支 */
        } else {
            assert(0 && "isnan(NaN) 应为真");
        }
    }

    /* [3] 返回值：非 NaN 的常见值（0、负数、极大值）均返回 0。 */
    {
        assert(isnan(0.0) == 0);
        assert(isnan(-0.0) == 0);
        assert(isnan(-1.0e300) == 0);
        assert(isnan(1.0e300) == 0);
    }

    /* 脚注 206：判定所用类型通常无关紧要，除非实现支持求值类型中的 NaN
     * 而不支持语义类型中的 NaN。此处仅验证常规环境下判定结果与语义类型
     * 一致（float 语义类型下 NaN 仍被识别）。 */
    {
        float f = (float)make_nan();
        assert(isnan(f) != 0);
    }

    printf("正向测试全部通过：isnan 宏行为符合 C99 7.12.3.4\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「isnan 的参数必须为 real-floating 类型」：
     * 传入结构体类型，gcc -std=c99 应报错（类型不兼容）。 */
    struct S { int x; } s;
    isnan(s);

    /* 违反约束「isnan 的参数必须为 real-floating 类型」：
     * 传入指针类型，应报错。 */
    double *p = 0;
    isnan(p);

    /* 违反约束「isnan 的参数必须为 real-floating 类型」：
     * 传入整数类型（int 不是 real-floating），应报错。 */
    int i = 0;
    isnan(i);

    /* 违反约束「isnan 的参数必须为 real-floating 类型」：
     * 传入复数类型（complex 不是 real-floating），应报错。 */
    double _Complex c = 0.0;
    isnan(c);

    /* 违反约束「isnan 的参数必须为 real-floating 类型」：
     * 传入字符串字面量（char*），应报错。 */
    isnan("hello");

#endif /* 负向测试结束 */

    return 0;
}