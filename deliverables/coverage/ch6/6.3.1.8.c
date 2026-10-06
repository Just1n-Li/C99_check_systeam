/*
 * 测试条款：C99 6.3.1.8 Usual arithmetic conversions（通常算术转换）
 *
 * 预期行为：
 *   - 正向测试：以下代码应能编译并运行通过（assert 全部成立），
 *     验证 [1] 中各种操作数组合下公共实数类型/结果类型的确定规则，
 *     以及 [2] 浮点操作数/表达式结果可以更高精度表示但类型不变。
 *   - 负向测试：违反约束的片段（放在 #if 0 中）应导致编译报错。
 *
 * 说明：6.3.1.8 本身主要是语义（Semantics）条款，其“约束”体现在
 *       通常算术转换只适用于算术类型操作数；对非算术类型（结构体、
 *       指针等）使用要求算术操作数的运算符即违反约束。此外，转换结果
 *       不是左值，对其赋值违反赋值运算符约束（6.5.16）。
 */

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <complex.h>
#include <float.h>

/* 用于检查表达式类型的辅助宏（编译期） */
#define TYPEOF_OK(expr, type) \
    do { type _t = (expr); (void)_t; } while (0)

int main(void)
{
    /* ================================================================
     * 正向测试：以下代码应能编译并运行通过
     * ================================================================ */

    /* ---------- [1] 浮点优先：long double 优先 ---------- */
    {
        /* long double 与 double 运算 -> 公共实数类型 long double */
        long double ld = 1.5L;
        double d = 2.5;
        long double r = ld + d;
        assert(r == 4.0L);
        /* 结果类型应为 long double */
        TYPEOF_OK(ld + d, long double);
    }

    /* ---------- [1] double 优先于 float ---------- */
    {
        double d = 1.25;
        float f = 2.5f;
        double r = d + f;
        assert(r == 3.75);
        TYPEOF_OK(d + f, double);
    }

    /* ---------- [1] float 优先于整数 ---------- */
    {
        float f = 1.5f;
        int i = 2;
        float r = f + i;
        assert(r == 3.5f);
        TYPEOF_OK(f + i, float);
    }

    /* ---------- [1] 整数提升：char/short 提升为 int ---------- */
    {
        char c = 10;
        short s = 20;
        /* 两者都提升为 int，结果类型 int */
        int r = c + s;
        assert(r == 30);
        TYPEOF_OK(c + s, int);
    }

    /* ---------- [1] 同类型：无需进一步转换 ---------- */
    {
        int a = 3, b = 4;
        int r = a + b;
        assert(r == 7);
        TYPEOF_OK(a + b, int);
    }

    /* ---------- [1] 同为有符号：低秩转高秩 ---------- */
    {
        /* int 与 long：int 转 long，结果 long */
        int i = 5;
        long l = 6L;
        long r = i + l;
        assert(r == 11L);
        TYPEOF_OK(i + l, long);
    }

    /* ---------- [1] 同为无符号：低秩转高秩 ---------- */
    {
        unsigned int ui = 5u;
        unsigned long ul = 6ul;
        unsigned long r = ui + ul;
        assert(r == 11ul);
        TYPEOF_OK(ui + ul, unsigned long);
    }

    /* ---------- [1] 无符号秩 >= 有符号秩：有符号转无符号 ---------- */
    {
        /* unsigned int 与 int：int 转 unsigned int，结果 unsigned int */
        unsigned int u = 1u;
        int i = -1;
        unsigned int r = u + i;   /* -1 转成 UINT_MAX，加 1 回绕为 0 */
        assert(r == 0u);
        TYPEOF_OK(u + i, unsigned int);
    }

    /* ---------- [1] 有符号类型能表示无符号所有值：无符号转有符号 ---------- */
    {
        /* long 能表示 unsigned int 的所有值（在常见平台上），
           故 unsigned int 转 long，结果 long */
        long l = 10L;
        unsigned int u = 20u;
        long r = l + u;
        assert(r == 30L);
        TYPEOF_OK(l + u, long);
    }

    /* ---------- [1] 否则：都转为有符号类型对应的无符号类型 ---------- */
    {
        /* 在 long 与 unsigned long 同秩且 long 无法表示 unsigned long
           全部值的平台上，两者都转为 unsigned long。
           这里用 unsigned long 与 long 验证结果类型为 unsigned long。 */
        unsigned long ul = 1ul;
        long l = -1L;
        unsigned long r = ul + l;  /* l 转为 unsigned long，回绕 */
        assert(r == 0ul);
        TYPEOF_OK(ul + l, unsigned long);
    }

    /* ---------- [1] 类型域：同为实数 -> 结果实数；含复数 -> 结果复数 ---------- */
    {
        double d = 1.0;
        double _Complex z = 2.0 + 3.0 * I;
        double _Complex r = d + z;   /* 结果类型域为复数 */
        assert(creal(r) == 3.0 && cimag(r) == 3.0);
        TYPEOF_OK(d + z, double _Complex);
    }
    {
        /* 两个实数操作数 -> 结果类型域为实数 */
        float f = 1.0f;
        double d = 2.0;
        double r = f + d;
        assert(r == 3.0);
        TYPEOF_OK(f + d, double);
    }

    /* ---------- [1] 脚注 51：double _Complex 与 float -> float 转 double ---------- */
    {
        double _Complex z = 1.0 + 1.0 * I;
        float f = 2.0f;
        double _Complex r = z + f;
        assert(creal(r) == 3.0 && cimag(r) == 1.0);
        TYPEOF_OK(z + f, double _Complex);
    }

    /* ---------- [2] 浮点操作数/结果可更高精度表示，但类型不变 ---------- */
    {
        float f = 1.0f / 3.0f;
        /* 即使中间以更高精度计算，f 的类型仍是 float */
        TYPEOF_OK(f, float);
        /* 结果类型仍为 float */
        float g = f * 3.0f;
        TYPEOF_OK(f * 3.0f, float);
        (void)g;
    }

    /* ---------- [1] 关系/相等运算符也使用通常算术转换 ---------- */
    {
        int i = -1;
        unsigned int u = 1u;
        /* i 转为 unsigned int，故 -1 < 1u 为假 */
        assert((i < u) == 0);
    }

    printf("正向测试全部通过。\n");

    /* ================================================================
     * 负向测试：以下代码违反 C99 约束，应编译报错
     * ================================================================ */
#if 0

    /* 违反约束「通常算术转换只适用于算术类型操作数」：
       结构体不是算术类型，不能参与 + 运算。
       gcc -std=c99 应报错：invalid operands to binary + */
    {
        struct S { int x; } a, b;
        a + b;
    }

    /* 违反约束「通常算术转换只适用于算术类型操作数」：
       指针不是算术类型，不能参与 * 运算。
       gcc -std=c99 应报错：invalid operands to binary * */
    {
        int arr[2];
        int *p = arr;
        int *q = arr + 1;
        p * q;
    }

    /* 违反约束「转换结果不是左值，不能赋值」（6.5.16 赋值运算符约束）：
       强制转换的结果不是左值，对其赋值应报错。
       gcc -std=c99 应报错：lvalue required as left operand of assignment */
    {
        int i = 0;
        (int)i = 5;
    }

    /* 违反约束「函数返回结构体的成员不是左值，不能赋值」：
       f().x 不是左值，对其赋值应报错。
       gcc -std=c99 应报错：lvalue required as left operand of assignment */
    {
        struct S { int x; };
        struct S f(void);
        f().x = 1;
    }

    /* 违反约束「条件表达式结果不是左值，不能赋值」：
       (1 ? a : b) 不是左值，对其赋值应报错。
       gcc -std=c99 应报错：lvalue required as left operand of assignment */
    {
        int a = 0, b = 0;
        (1 ? a : b) = 5;
    }

    /* 违反约束「逗号表达式结果不是左值，不能赋值」：
       (a, b) 不是左值，对其赋值应报错。
       gcc -std=c99 应报错：lvalue required as left operand of assignment */
    {
        int a = 0, b = 0;
        (a, b) = 5;
    }

    /* 违反约束「const 限定类型不可修改」：
       const int 对象不能赋值，应报错。
       gcc -std=c99 应报错：assignment of read-only variable */
    {
        const int ci = 0;
        ci = 1;
    }

    /* 违反约束「const 限定结构体成员不可修改」：
       const 结构体对象的成员不能赋值，应报错。
       gcc -std=c99 应报错：assignment of read-only member */
    {
        struct S { int x; };
        const struct S s = { 0 };
        s.x = 1;
    }

    /* 违反约束「通过 const 指针解引用不可修改」：
       const int * 解引用结果不是可修改左值，应报错。
       gcc -std=c99 应报错：assignment of read-only location */
    {
        int v = 0;
        const int *p = &v;
        *p = 1;
    }

#endif /* 负向测试结束 */

    return 0;
}