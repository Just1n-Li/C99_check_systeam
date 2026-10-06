/*
 * 测试 C99 7.12.11.4 —— nexttoward / nexttowardf / nexttowardl
 *
 * 预期行为：
 *   正向测试：包含 <math.h>，调用三个函数，验证：
 *     [1] 原型存在且返回类型正确（double / float / long double）
 *     [2] 语义等价于 nextafter，但第二参数为 long double；
 *         当 x == y 时返回 y 转换到函数返回类型后的值。
 *   负向测试：违反约束的调用（参数个数/类型错误）应编译报错。
 *
 * 编译：gcc -std=c99 -Wall -Wextra test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：通过函数指针类型验证签名 */
static double  (*p_nt) (double, long double)       = nexttoward;
static float   (*p_ntf)(float,  long double)       = nexttowardf;
static long double (*p_ntl)(long double, long double) = nexttowardl;

int main(void)
{
    /* [1] 三个函数均可调用，返回类型正确 */
    double  d = nexttoward(1.0, 2.0L);
    float   f = nexttowardf(1.0f, 2.0L);
    long double l = nexttowardl(1.0L, 2.0L);

    /* [2] 语义：nexttoward 等价于 nextafter，但第二参数为 long double。
     *     对同一 x、同一方向，nexttoward(x, y) 应等于 nextafter(x, (double)y)
     *     在 double 精度下（此处 y 可精确表示时）。 */
    assert(d == nextafter(1.0, 2.0));
    assert(f == nextafterf(1.0f, 2.0f));
    assert(l == nextafterl(1.0L, 2.0L));

    /* [2] 方向性：向更大的 y 前进，结果应大于 x */
    assert(d > 1.0);
    assert(f > 1.0f);
    assert(l > 1.0L);

    /* [2] 向更小的 y 前进，结果应小于 x */
    assert(nexttoward(1.0, 0.0L) < 1.0);
    assert(nexttowardf(1.0f, 0.0L) < 1.0f);
    assert(nexttowardl(1.0L, 0.0L) < 1.0L);

    /* [2] 关键语义：当 x == y 时，返回 y 转换到函数返回类型后的值。
     *     注意第二参数是 long double，转换到 double/float 时可能损失精度，
     *     这正是与 nextafter 的差别所在。 */
    {
        /* x == y，且 y 可被 double 精确表示 */
        double r1 = nexttoward(3.5, 3.5L);
        assert(r1 == 3.5);

        /* x == y，y 为 long double 值，转换到 double 后返回 */
        long double yv = 1.0L + LDBL_EPSILON; /* 通常无法被 double 精确表示 */
        double r2 = nexttoward(1.0, yv);
        assert(r2 == (double)yv);   /* 返回的是 y 转换到 double 的值 */

        /* float 版本：x == y 时返回 (float)y */
        float r3 = nexttowardf(2.0f, 2.0L);
        assert(r3 == 2.0f);

        /* long double 版本：x == y 时返回 y 本身 */
        long double r4 = nexttowardl(2.0L, 2.0L);
        assert(r4 == 2.0L);
    }

    /* [2] 与 nextafter 的等价性（第二参数可精确转换时） */
    assert(nexttoward(0.0, 1.0L) == nextafter(0.0, 1.0));
    assert(nexttoward(-1.0, -2.0L) == nextafter(-1.0, -2.0));

    /* [2] 边界：x 为 0，y 为 0，返回 0 */
    assert(nexttoward(0.0, 0.0L) == 0.0);
    assert(nexttowardf(0.0f, 0.0L) == 0.0f);
    assert(nexttowardl(0.0L, 0.0L) == 0.0L);

    /* [2] 无穷方向：向 +inf 前进 */
    {
        double inf = nexttoward(1.0, INFINITY);
        assert(inf > 1.0);
        assert(isfinite(inf));
    }

    /* 使用函数指针，确认签名匹配（编译期检查） */
    (void)p_nt; (void)p_ntf; (void)p_ntl;

    printf("nexttoward  : %a\n", d);
    printf("nexttowardf : %a\n", (double)f);
    printf("nexttowardl : %La\n", l);
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「参数个数必须与原型一致」：
 * nexttoward 需要 2 个参数，只给 1 个，gcc -std=c99 应报错
 *   error: too few arguments to function 'nexttoward' */
void bad_arity_1(void) {
    double r = nexttoward(1.0);
    (void)r;
}

/* 违反约束「参数个数必须与原型一致」：
 * 给 3 个参数，gcc -std=c99 应报错
 *   error: too many arguments to function 'nexttoward' */
void bad_arity_2(void) {
    double r = nexttoward(1.0, 2.0L, 3.0L);
    (void)r;
}

/* 违反约束「实参类型必须可转换为形参类型」：
 * 第一个参数为结构体，无法转换为 double，gcc -std=c99 应报错
 *   error: incompatible type for argument 1 of 'nexttoward' */
struct S { int x; };
void bad_arg_type(void) {
    struct S s;
    double r = nexttoward(s, 1.0L);
    (void)r;
}

/* 违反约束「实参类型必须可转换为形参类型」：
 * 第一个参数为指针，无法隐式转换为 double，gcc -std=c99 应报错
 *   error: incompatible type for argument 1 of 'nexttoward' */
void bad_arg_ptr(void) {
    int *p = 0;
    double r = nexttoward(p, 1.0L);
    (void)r;
}

/* 违反约束「函数返回值不可作为左值赋值」：
 * nexttoward 的返回值是右值，不能赋值，gcc -std=c99 应报错
 *   error: lvalue required as left operand of assignment */
void bad_assign_result(void) {
    nexttoward(1.0, 2.0L) = 3.0;
}

/* 违反约束「函数名必须已声明」：
 * 未包含 <math.h> 且未声明 nexttoward 时调用，C99 隐式声明规则下
 * 若在 C99 严格模式（-Werror=implicit-function-declaration）应报错；
 * 这里演示对未声明标识符的调用（在 C99 中隐式声明已移除）。
 *   error: implicit declaration of function 'nexttoward_undeclared' */
void bad_undeclared(void) {
    double r = nexttoward_undeclared(1.0, 2.0L);
    (void)r;
}

#endif /* 负向测试结束 */