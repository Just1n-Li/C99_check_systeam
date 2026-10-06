/*
 * 测试 C99 7.20.6.1 —— abs / labs / llabs 函数
 *
 * 预期行为：
 *   正向测试：包含 <stdlib.h> 后调用 abs/labs/llabs，返回值等于参数的绝对值，
 *             程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码（如参数个数错误、未包含头文件而隐式声明等）
 *             应导致编译报错；这些片段放在 #if 0 中，不影响本文件编译。
 *
 * 说明：条款 [2] 指出“若结果无法表示，行为未定义”，这属于 UB，
 *       不作为负向测试（UB 代码仍能编译通过），仅在注释中说明。
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型声明检查：三个函数均声明于 <stdlib.h>，返回类型分别为
 *     int / long int / long long int。通过取函数指针类型来静态验证签名。 */
static int  (*p_abs)(int)                = abs;
static long (*p_labs)(long)              = labs;
static long long (*p_llabs)(long long)   = llabs;

int main(void)
{
    /* [1] 基本调用：验证三个函数可被正常调用 */
    int        i;
    long       l;
    long long  ll;

    /* [2][3] abs：计算整数 j 的绝对值并返回 */
    i = abs(0);
    assert(i == 0);                 /* |0| == 0 */

    i = abs(42);
    assert(i == 42);                /* |42| == 42 */

    i = abs(-42);
    assert(i == 42);                /* |-42| == 42 */

    i = abs(INT_MAX);
    assert(i == INT_MAX);           /* |INT_MAX| == INT_MAX */

    /* [2][3] labs：long 版本 */
    l = labs(0L);
    assert(l == 0L);

    l = labs(123456789L);
    assert(l == 123456789L);

    l = labs(-123456789L);
    assert(l == 123456789L);

    l = labs(LONG_MAX);
    assert(l == LONG_MAX);

    /* [2][3] llabs：long long 版本 */
    ll = llabs(0LL);
    assert(ll == 0LL);

    ll = llabs(1234567890123LL);
    assert(ll == 1234567890123LL);

    ll = llabs(-1234567890123LL);
    assert(ll == 1234567890123LL);

    ll = llabs(LLONG_MAX);
    assert(ll == LLONG_MAX);

    /* [3] 返回值类型检查：结果应能赋给对应宽度的整型而不丢失信息 */
    {
        int        ri  = abs(-7);
        long       rl  = labs(-7L);
        long long  rll = llabs(-7LL);
        assert(ri  == 7);
        assert(rl  == 7L);
        assert(rll == 7LL);
    }

    /* [1] 通过函数指针调用，验证原型与签名一致 */
    assert(p_abs(-5)   == 5);
    assert(p_labs(-5L) == 5L);
    assert(p_llabs(-5LL) == 5LL);

    /* [2] 边界：绝对值可表示的最大正数（非 UB 情形） */
    assert(abs(-(INT_MAX)) == INT_MAX);
    assert(labs(-(LONG_MAX)) == LONG_MAX);
    assert(llabs(-(LLONG_MAX)) == LLONG_MAX);

    /*
     * 注意（UB，不作为负向测试）：
     *   abs(INT_MIN) / labs(LONG_MIN) / llabs(LLONG_MIN)
     * 的结果无法用补码表示（见 Footnote 265），属于 [2] 所述
     * “结果无法表示时行为未定义”，因此不在此处断言其值。
     */

    printf("C99 7.20.6.1 abs/labs/llabs: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「函数调用实参个数必须与原型一致」：
 * abs 原型为 int abs(int)，此处传 0 个实参，gcc -std=c99 应报错
 *   error: too few arguments to function 'abs' */
int bad1(void) { return abs(); }

/* 违反约束「函数调用实参个数必须与原型一致」：
 * labs 原型为 long int labs(long int)，此处传 2 个实参，应报错
 *   error: too many arguments to function 'labs' */
long bad2(void) { return labs(1L, 2L); }

/* 违反约束「函数调用实参个数必须与原型一致」：
 * llabs 原型为 long long int llabs(long long int)，传 3 个实参，应报错 */
long long bad3(void) { return llabs(1LL, 2LL, 3LL); }

/* 违反约束「使用函数前必须有可见声明（C99 取消隐式声明）」：
 * 未包含 <stdlib.h> 且未声明 abs，直接调用，gcc -std=c99 应报错
 *   error: implicit declaration of function 'abs' */
int bad4(void) { return abs(-1); }

/* 违反约束「赋值目标必须是可修改的左值」：
 * abs 的返回值不是左值，对其赋值应报错
 *   error: lvalue required as left operand of assignment */
int bad5(void) { return (abs(-1) = 5); }

/* 违反约束「取地址操作数必须是左值/函数指示符」：
 * 对函数返回值取地址应报错
 *   error: lvalue required as unary '&' operand */
int *bad6(void) { return &abs(-1); }

#endif