/*
 * 测试 C99 7.20.2.1 —— rand 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int rand(void);  需要 <stdlib.h>
 *   [2] 返回 0..RAND_MAX 范围内的伪随机整数序列
 *   [3] 实现行为如同没有库函数调用 rand（即 rand 不受其它库函数调用影响）
 *   [4] 返回一个伪随机整数
 *   [5] RAND_MAX 至少为 32767
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：rand 是 int(void) 类型，可赋给函数指针 */
static int (*rand_ptr)(void) = rand;

/* [1] 原型检查：rand 不接受参数，调用时不得传参（见负向测试） */
static void test_prototype(void)
{
    int (*p)(void) = rand;   /* [1] 类型匹配 int (void) */
    assert(p == rand_ptr);
    (void)p;
}

/* [2][4] rand 返回 0..RAND_MAX 范围内的伪随机整数 */
static void test_range(void)
{
    int i;
    for (i = 0; i < 1000; ++i) {
        int r = rand();
        assert(r >= 0);          /* [2] 下界 0 */
        assert(r <= RAND_MAX);   /* [2] 上界 RAND_MAX */
    }
}

/* [2] 序列性：多次调用产生一个序列（不要求可预测，但应能产生多个值） */
static void test_sequence(void)
{
    int i;
    int seen_nonzero = 0;
    for (i = 0; i < 100; ++i) {
        if (rand() != 0)
            seen_nonzero = 1;
    }
    /* 100 次调用中至少出现一个非零值（几乎必然成立） */
    assert(seen_nonzero);
}

/* [3] 实现行为如同没有库函数调用 rand：
 *     在两次 rand 调用之间调用其它库函数（如 printf、malloc、strlen 等），
 *     不应改变 rand 序列的“可复现性”——即用相同种子重置后，
 *     插入的库函数调用不应影响后续 rand 序列。
 *     这里用 srand 固定种子，比较“无干扰”与“有干扰”两条序列。 */
static void test_no_library_interference(void)
{
    int seq_plain[16];
    int seq_interfered[16];
    int i;

    /* 无干扰序列 */
    srand(12345u);
    for (i = 0; i < 16; ++i)
        seq_plain[i] = rand();

    /* 有干扰序列：在每次 rand 之间调用其它库函数 */
    srand(12345u);
    for (i = 0; i < 16; ++i) {
        /* 调用若干其它库函数，不应影响 rand 的行为 */
        char buf[32];
        (void)snprintf(buf, sizeof buf, "%d", i);
        (void)strlen(buf);
        {
            void *m = malloc(8);
            free(m);
        }
        seq_interfered[i] = rand();
    }

    /* [3] 两条序列应完全一致 */
    for (i = 0; i < 16; ++i)
        assert(seq_plain[i] == seq_interfered[i]);
}

/* [5] RAND_MAX 至少为 32767 */
static void test_rand_max(void)
{
    assert(RAND_MAX >= 32767);   /* [5] 环境限制 */
    /* RAND_MAX 是整数常量表达式，可用于数组维度等常量上下文 */
    {
        char arr[RAND_MAX >= 32767 ? 1 : -1];  /* 若 RAND_MAX < 32767 则编译失败 */
        (void)arr;
    }
}

/* [4] 返回值类型为 int */
static void test_return_type(void)
{
    int r = rand();          /* [4] 返回 int */
    long lr = (long)rand();  /* 可隐式/显式转换 */
    assert(r >= 0 && r <= RAND_MAX);
    assert(lr >= 0 && lr <= (long)RAND_MAX);
}

int main(void)
{
    test_prototype();
    test_range();
    test_sequence();
    test_no_library_interference();
    test_rand_max();
    test_return_type();

    printf("C99 7.20.2.1 rand: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「rand 的原型为 int rand(void)，不接受参数」：
 * 调用 rand 时传入实参，gcc -std=c99 应报错
 *   error: too many arguments to function 'rand' */
{
    int x = rand(42);
    (void)x;
}

/* 违反约束「rand 的原型为 int rand(void)，不接受参数」：
 * 用带参数的原型声明 rand，与 <stdlib.h> 中的声明冲突，
 * gcc -std=c99 应报错
 *   error: conflicting types for 'rand' */
int rand(int seed);

/* 违反约束「rand 返回 int，不是指针」：
 * 把 rand 的返回值当作指针使用（类型不兼容），应报错
 *   error: invalid type argument of unary '*' (have 'int') */
{
    int v = *rand();
    (void)v;
}

/* 违反约束「rand 返回 int，不能直接作为结构体/数组使用」：
 * 对 int 返回值做成员访问，应报错
 *   error: request for member 'x' in something not a structure or union */
{
    int v = rand().x;
    (void)v;
}

/* 违反约束「rand 是函数，不是对象」：
 * 对函数名取地址后再解引用赋值（函数不是左值），应报错
 *   error: lvalue required as left operand of assignment */
{
    rand = 0;
}

/* 违反约束「rand 的返回值不是左值」：
 * 对函数调用结果赋值，应报错
 *   error: lvalue required as left operand of assignment */
{
    rand() = 5;
}

/* 违反约束「rand 的返回值不是左值」：
 * 对函数调用结果取地址，应报错
 *   error: lvalue required as unary '&' operand */
{
    int *p = &rand();
    (void)p;
}

#endif