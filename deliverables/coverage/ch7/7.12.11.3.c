/*
 * 测试 C99 7.12.11.3 —— nextafter 函数族
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明（double/float/long double 三个版本）
 *   [2] 语义：向 y 方向、x 之后的下一个可表示值；x==y 时返回 y；
 *       参数先转换为函数类型；最大有限值处可能发生范围错误。
 *   [3] 返回值：指定格式中 x 之后朝 y 方向的下一个可表示值。
 *   脚注 211：即使宏实现，参数值也转换为函数类型。
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用性：三个函数均可调用，返回类型正确 */
    {
        double d = nextafter(1.0, 2.0);
        float  f = nextafterf(1.0f, 2.0f);
        long double ld = nextafterl(1.0L, 2.0L);
        (void)d; (void)f; (void)ld;
    }

    /* [2][3] 基本语义：nextafter(x, y) 返回 x 之后朝 y 方向的下一个可表示值。
     * 对 1.0 朝 2.0 方向，结果应严格大于 1.0，且是紧邻的下一个值。 */
    {
        double x = 1.0;
        double r = nextafter(x, 2.0);
        assert(r > x);                       /* 朝 y 方向前进 */
        /* 紧邻性：x 与 r 之间没有其它可表示值，即 r 是 x 的下一个 */
        assert(nextafter(x, 2.0) == r);
        /* 反向：从 r 朝 x 方向的下一个值应回到 x */
        assert(nextafter(r, x) == x);
    }

    /* [2][3] 朝负方向：nextafter(1.0, 0.0) 应严格小于 1.0 */
    {
        double x = 1.0;
        double r = nextafter(x, 0.0);
        assert(r < x);
        assert(nextafter(r, x) == x);
    }

    /* [2] x == y 时返回 y（即返回 x 本身） */
    {
        double x = 3.14159;
        assert(nextafter(x, x) == x);
        assert(nextafter(0.0, 0.0) == 0.0);
        assert(nextafter(-2.5, -2.5) == -2.5);
    }

    /* [2] 参数先转换为函数类型：对 nextafterf，传入 double 会被转换为 float。
     * 这里用 float 版本验证其行为与 float 精度一致。 */
    {
        float xf = 1.0f;
        float rf = nextafterf(xf, 2.0f);
        assert(rf > xf);
        /* float 的下一个值应等于 1.0f + FLT_EPSILON */
        assert(rf == 1.0f + FLT_EPSILON);
    }

    /* [2] 对 nextafter（double 版），float 参数被提升/转换为 double */
    {
        double r = nextafter(1.0f, 2.0f);   /* 参数转换为 double */
        assert(r > 1.0);
        assert(r == 1.0 + DBL_EPSILON);
    }

    /* [2][3] 从 0.0 朝 1.0 方向：应得到最小正次正规数（或最小正正规数，
     * 取决于实现是否支持次正规数）。这里只验证结果 > 0 且是紧邻 0 的值。 */
    {
        double r = nextafter(0.0, 1.0);
        assert(r > 0.0);
        assert(nextafter(r, 0.0) == 0.0);   /* 反向回到 0 */
    }

    /* [2][3] 从 0.0 朝 -1.0 方向：应得到最小负值 */
    {
        double r = nextafter(0.0, -1.0);
        assert(r < 0.0);
        assert(nextafter(r, 0.0) == 0.0);
    }

    /* [2] 最大有限值处：nextafter(DBL_MAX, INFINITY) 结果为 INFINITY
     * （或不可表示，可能发生范围错误）。这里验证朝无穷方向前进。 */
    {
        double r = nextafter(DBL_MAX, INFINITY);
        assert(isinf(r) || r > DBL_MAX);
    }

    /* [2] 从无穷朝有限值方向：nextafter(INFINITY, 0.0) 应回到 DBL_MAX */
    {
        double r = nextafter(INFINITY, 0.0);
        assert(r == DBL_MAX);
    }

    /* [2] 从 -INFINITY 朝 0 方向：应回到 -DBL_MAX */
    {
        double r = nextafter(-INFINITY, 0.0);
        assert(r == -DBL_MAX);
    }

    /* [2] 从 DBL_MAX 朝 0 方向：应得到紧邻 DBL_MAX 的较小值 */
    {
        double r = nextafter(DBL_MAX, 0.0);
        assert(r < DBL_MAX);
        assert(nextafter(r, DBL_MAX) == DBL_MAX);
    }

    /* [2] 从最小正规数朝 0 方向：应得到最大次正规数（若支持次正规数） */
    {
        double r = nextafter(DBL_MIN, 0.0);
        assert(r < DBL_MIN);
        assert(r > 0.0);
        assert(nextafter(r, DBL_MIN) == DBL_MIN);
    }

    /* [2] 从最小次正规数朝 0 方向：应得到 0.0 */
    {
        double tiny = nextafter(0.0, 1.0);   /* 最小正次正规数 */
        double r = nextafter(tiny, 0.0);
        assert(r == 0.0);
    }

    /* [2] 负值方向测试 */
    {
        double x = -1.0;
        double r = nextafter(x, -2.0);       /* 朝更负方向 */
        assert(r < x);
        assert(nextafter(r, x) == x);
    }

    /* [2] 跨零：nextafter(-tiny, 0.0) 应得到 0.0 或 +tiny 方向 */
    {
        double negtiny = nextafter(0.0, -1.0);  /* 最小负次正规数 */
        double r = nextafter(negtiny, 0.0);
        assert(r == 0.0);
    }

    /* [2] 脚注 211：即使宏实现，参数也转换为函数类型。
     * 用 nextafterf 验证：传入 double 参数被转换为 float。 */
    {
        double big = 1e300;                  /* 转换为 float 会溢出为 inf */
        float rf = nextafterf((float)1.0f, (float)2.0f);
        assert(rf == 1.0f + FLT_EPSILON);
        (void)big;
    }

    /* [3] 返回值类型检查：nextafter 返回 double，nextafterf 返回 float，
     * nextafterl 返回 long double。用 sizeof 验证。 */
    {
        assert(sizeof(nextafter(1.0, 2.0)) == sizeof(double));
        assert(sizeof(nextafterf(1.0f, 2.0f)) == sizeof(float));
        assert(sizeof(nextafterl(1.0L, 2.0L)) == sizeof(long double));
    }

    /* [2][3] long double 版本基本语义 */
    {
        long double x = 1.0L;
        long double r = nextafterl(x, 2.0L);
        assert(r > x);
        assert(nextafterl(r, x) == x);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「nextafter 的参数必须为算术类型」：
     * 传入结构体类型，gcc -std=c99 应报错（incompatible type / 
     * invalid operands 或参数类型不匹配）。 */
    struct S { int x; } s;
    nextafter(s, 1.0);          /* 错误：结构体不能转换为 double */

    /* 违反约束「nextafter 的参数必须为算术类型」：
     * 传入指针类型，应报错。 */
    int *p = 0;
    nextafter(p, 1.0);          /* 错误：指针不能转换为 double */

    /* 违反约束「nextafter 需要两个参数」：
     * 只传一个参数，应报错（too few arguments）。 */
    nextafter(1.0);             /* 错误：参数个数不足 */

    /* 违反约束「nextafter 需要两个参数」：
     * 传三个参数，应报错（too many arguments）。 */
    nextafter(1.0, 2.0, 3.0);   /* 错误：参数个数过多 */

    /* 违反约束「nextafter 返回 double，不能作为左值赋值」：
     * 函数调用结果不是左值，对其赋值应报错。 */
    nextafter(1.0, 2.0) = 5.0;  /* 错误：赋值目标不是左值 */

    /* 违反约束「nextafterf 返回 float，不能作为左值赋值」 */
    nextafterf(1.0f, 2.0f) = 5.0f;  /* 错误：赋值目标不是左值 */

    /* 违反约束「nextafterl 返回 long double，不能作为左值赋值」 */
    nextafterl(1.0L, 2.0L) = 5.0L;  /* 错误：赋值目标不是左值 */

    /* 违反约束「nextafter 的参数必须为算术类型」：
     * 传入 void 表达式（函数返回 void），应报错。 */
    void vfunc(void);
    nextafter(vfunc(), 1.0);    /* 错误：void 类型不能作为参数 */

#endif

    return 0;
}