/*
 * C99 7.3.2 —— <math.h> 中三角函数族的 Conventions（约定）
 *
 * 条款原文：
 *   [1] Values are interpreted as radians, not degrees.
 *       An implementation may set errno but is not required to.
 *
 * 预期行为：
 *   - 正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *     验证 sin/cos/tan 等函数把实参当作「弧度」而非「角度」来解释；
 *     并验证实现「可以」设置 errno，但「不要求」设置 errno（即两种行为都合法）。
 *   - 负向测试：违反约束的代码应编译报错（放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：7.3.2 本身是 Conventions（约定）段落，不含显式 Constraints 编号，
 *       因此负向测试针对的是「调用这些函数时必须满足的通用约束」，
 *       即实参必须为算术类型（7.3 中函数原型要求 double 形参）。
 */

#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <assert.h>

/* 用于比较浮点数的容差 */
static int close_to(double a, double b, double eps)
{
    double d = a - b;
    if (d < 0) d = -d;
    return d <= eps;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 值按「弧度」解释，而不是「角度」。
     *     若实现错误地把实参当作角度，则 sin(3.14159265358979323846)
     *     会被算成 sin(π 度) ≈ 0.0548，而不是 ≈ 0。
     *     这里用 π 弧度验证 sin(π) ≈ 0、cos(π) ≈ -1。 */
    {
        const double PI = 3.14159265358979323846;
        double s = sin(PI);
        double c = cos(PI);
        double t = tan(PI / 4.0);   /* tan(π/4 弧度) = 1 */

        /* 若按角度解释，sin(π 度) 会明显偏离 0，断言会失败 */
        assert(close_to(s, 0.0, 1e-12));
        assert(close_to(c, -1.0, 1e-12));
        assert(close_to(t, 1.0, 1e-12));

        printf("[1] sin(PI)=%.17g  cos(PI)=%.17g  tan(PI/4)=%.17g\n", s, c, t);
    }

    /* [1] 再验证一个非特殊角：sin(π/6 弧度) = 0.5，cos(π/3 弧度) = 0.5。
     *     若按角度解释，sin(30 度) 恰好也是 0.5，所以这里特意用
     *     sin(π/6) 与 sin(30) 的差异来区分「弧度 vs 角度」：
     *     sin(π/6 弧度) ≈ 0.5，而 sin(π/6 度) ≈ 0.00914。 */
    {
        const double PI = 3.14159265358979323846;
        double a = sin(PI / 6.0);   /* 弧度解释 → 0.5 */
        double b = sin(PI / 6.0 * 180.0 / PI); /* 等价于 sin(30 度) → 0.5 */

        assert(close_to(a, 0.5, 1e-12));
        assert(close_to(b, 0.5, 1e-12));

        /* 关键区分：sin(0.5235987755982988 弧度) 与 sin(0.5235987755982988 度) 不同 */
        double as_radians = sin(0.5235987755982988);          /* ≈ 0.5 */
        double as_degrees = sin(0.5235987755982988 * PI / 180.0); /* ≈ 0.00914 */
        assert(close_to(as_radians, 0.5, 1e-12));
        assert(!close_to(as_degrees, 0.5, 1e-3));  /* 角度解释会得到完全不同的值 */

        printf("[1] sin(PI/6)=%.17g  sin(30deg)=%.17g\n", a, b);
    }

    /* [1] 反三角函数同样以弧度返回：asin(1) = π/2，atan(1) = π/4。 */
    {
        const double PI = 3.14159265358979323846;
        double as = asin(1.0);
        double at = atan(1.0);
        double ac = acos(0.0);

        assert(close_to(as, PI / 2.0, 1e-12));
        assert(close_to(at, PI / 4.0, 1e-12));
        assert(close_to(ac, PI / 2.0, 1e-12));

        printf("[1] asin(1)=%.17g  atan(1)=%.17g  acos(0)=%.17g\n", as, at, ac);
    }

    /* [1] "An implementation may set errno but is not required to."
     *     即：实现可以设置 errno，也可以不设置。两种行为都符合标准。
     *     因此我们不能断言 errno 一定被设置，也不能断言一定不被设置。
     *     这里只验证：调用这些函数不会导致程序崩溃，且返回值在合法范围内；
     *     同时演示「允许但不要求」的语义——无论 errno 是否改变都合法。 */
    {
        int saved = errno;
        errno = 0;
        double v = sin(1.0);   /* 正常输入，errno 可能被设置也可能不被设置 */
        /* 不 assert errno 的具体值，因为标准允许两种行为 */
        (void)v;
        (void)saved;
        printf("[1] errno after sin(1.0) = %d (may be 0 or unchanged; both allowed)\n", errno);
    }

    /* [1] 定义域外的输入（如 asin(2.0)）属于 7.12.1 的 domain error，
     *     实现可以设置 errno = EDOM，也可以不设置。这里同样只验证
     *     「允许但不要求」——不断言 errno 一定为 EDOM。 */
    {
        errno = 0;
        double v = asin(2.0);   /* 定义域外 */
        (void)v;
        /* 标准允许 errno 被设为 EDOM，也允许不设；两种都合法 */
        printf("[1] errno after asin(2.0) = %d (EDOM=%d; setting it is optional)\n",
               errno, EDOM);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「实参必须为算术类型」：
     * sin/cos/tan 等函数的原型为 double sin(double)，
     * 传入结构体类型无法隐式转换为 double，gcc -std=c99 应报错。 */
    {
        struct S { int x; } s;
        double r = sin(s);   /* 错误：结构体不能转换为 double */
        (void)r;
    }

    /* 违反约束「实参必须为算术类型」：
     * 传入指针类型，指针不能隐式转换为 double，应报错。 */
    {
        int *p = 0;
        double r = cos(p);   /* 错误：指针不能转换为 double */
        (void)r;
    }

    /* 违反约束「实参必须为算术类型」：
     * 传入数组类型（退化为指针），同样不能转换为 double，应报错。 */
    {
        int arr[3];
        double r = tan(arr); /* 错误：数组/指针不能转换为 double */
        (void)r;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * sin 原型只接受 1 个参数，传 2 个应报错。 */
    {
        double r = sin(1.0, 2.0);  /* 错误：参数过多 */
        (void)r;
    }

    /* 违反约束「函数调用实参个数必须与原型一致」：
     * atan2 原型接受 2 个参数，只传 1 个应报错。 */
    {
        double r = atan2(1.0);     /* 错误：参数过少 */
        (void)r;
    }

#endif

    return 0;
}