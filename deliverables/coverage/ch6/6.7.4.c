/*
 * 测试 C99 6.7.4 Function specifiers (inline)
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，故意违反 6.7.4 的 Constraints，
 *             使用 gcc -std=c99 -pedantic-errors 编译时应报错。
 *
 * 覆盖段落：[1] 语法、[2][3][4] Constraints、[5][6] Semantics、[7][8] EXAMPLE。
 */

#include <stdio.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 语法：function-specifier 就是关键字 inline。
 * [5] 语义：inline 函数说明符可以出现多次，行为等同于只出现一次。 */
inline inline int add(int a, int b)   /* 重复 inline，合法 */
{
    return a + b;
}

/* [6] 内部链接的函数可以是 inline 函数（static inline）。 */
static inline int square(int x)
{
    return x * x;
}

/* [6] 外部链接 + inline：本翻译单元内必须也有定义。
 * 这里所有文件作用域声明都带 inline 且不带 extern，
 * 因此这是一个 inline definition（不提供外部定义）。 */
inline double fahr(double t)
{
    return (9.0 * t) / 5.0 + 32.0;
}

/* [7] EXAMPLE：cels 也是 inline definition。 */
inline double cels(double t)
{
    return (5.0 * (t - 32.0)) / 9.0;
}

/* [7] EXAMPLE：带 extern 的文件作用域声明会创建外部定义。
 * 注意：为了让本测试程序能独立链接，这里给出一个外部定义
 * （在真实的多翻译单元场景中，外部定义可位于另一翻译单元）。 */
extern double fahr(double);   /* 使 fahr 成为外部定义 */

/* [7] EXAMPLE：convert 中调用 cels / fahr，翻译器可做 inline 替换。 */
double convert(int is_fahr, double temp)
{
    return is_fahr ? cels(temp) : fahr(temp);
}

/* [6] 内部链接 inline 函数，用于验证调用结果。 */
static inline int twice(int x)
{
    return 2 * x;
}

int main(void)
{
    /* [5] 重复 inline 与单个 inline 行为一致。 */
    assert(add(3, 4) == 7);

    /* [6] static inline 内部链接函数可正常调用。 */
    assert(square(5) == 25);
    assert(twice(21) == 42);

    /* [7][8] EXAMPLE：fahr 是外部定义，cels 是 inline definition，
     * 二者都可被调用，结果正确。 */
    assert(fahr(0.0) == 32.0);
    assert(cels(32.0) == 0.0);
    assert(convert(1, 32.0) == 0.0);    /* 用 cels */
    assert(convert(0, 0.0) == 32.0);    /* 用 fahr */

    /* [7] 往返转换验证数值一致性。 */
    assert(cels(fahr(100.0)) == 100.0);

    printf("C99 6.7.4 positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 6.7.4 约束，应编译报错
 * （统一放在 #if 0 中，保证本文件仍能正常编译运行）
 * ============================================================ */
#if 0

/* [2] 违反约束「function specifier 只能用于函数标识符的声明」：
 * inline 用在一个对象（变量）的声明上。
 * 期望：gcc -std=c99 -pedantic-errors 报错，
 *       例如 "inline declaration of 'x' not allowed" / "invalid storage class"。 */
inline int x = 5;

/* [2] 违反约束：inline 用在类型声明（typedef）上，而非函数标识符。 */
inline typedef int myint;

/* [4] 违反约束「在宿主环境中，inline 不得出现在 main 的声明中」：
 * 期望：gcc -std=c99 -pedantic-errors 报错，
 *       例如 "cannot inline function 'main'"。 */
inline int main(void) { return 0; }

/* [3] 违反约束「外部链接的 inline 定义不得包含可修改的静态存储期对象定义」：
 * 期望：gcc -std=c99 -pedantic-errors 报错，
 *       例如 "function 'bad_static' can never be inlined because it uses
 *             static variable 'counter'" 或类似诊断。 */
inline int bad_static(void)
{
    static int counter = 0;   /* 可修改的静态存储期对象定义 */
    counter++;
    return counter;
}

/* [3] 违反约束「外部链接的 inline 定义不得引用内部链接的标识符」：
 * 期望：gcc -std=c99 -pedantic-errors 报错，
 *       例如 "function 'bad_ref' can never be inlined because it uses
 *             internal linkage identifier 'internal_var'"。 */
static int internal_var = 10;
inline int bad_ref(void)
{
    return internal_var;      /* 引用内部链接标识符 */
}

#endif /* 负向测试结束 */