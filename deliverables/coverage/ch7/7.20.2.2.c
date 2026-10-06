/*
 * 测试条款：C99 7.20.2.2  The srand function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：void srand(unsigned int seed);  需要 <stdlib.h>
 *   [2] 语义：同种子 -> 同序列；未调用 srand 时等价于 srand(1)
 *   [3] 语义：实现行为上，没有库函数会调用 srand
 *   [4] 返回值：srand 无返回值（void）
 *   [5] EXAMPLE：可移植的 rand/srand 实现
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [5] EXAMPLE：条款给出的可移植实现（重命名以免与库函数冲突） */
static unsigned long int my_next = 1;

static int my_rand(void) /* RAND_MAX assumed to be 32767 */
{
    my_next = my_next * 1103515245 + 12345;
    return (unsigned int)(my_next / 65536) % 32768;
}

static void my_srand(unsigned int seed)
{
    my_next = seed;
}

/* [1] 原型检查：srand 接受 unsigned int，返回 void。
 *     这里通过取函数指针来静态验证原型签名。 */
static void test_prototype(void)
{
    void (*fp)(unsigned int) = srand;   /* [1] 原型匹配 */
    assert(fp != NULL);
    (void)fp;
}

/* [2] 相同种子 -> 相同序列 */
static void test_same_seed_same_sequence(void)
{
    int i;
    int seq1[16];
    int seq2[16];

    srand(12345u);
    for (i = 0; i < 16; i++)
        seq1[i] = rand();

    srand(12345u);
    for (i = 0; i < 16; i++)
        seq2[i] = rand();

    for (i = 0; i < 16; i++)
        assert(seq1[i] == seq2[i]);     /* [2] 序列必须重复 */
}

/* [2] 不同种子通常产生不同序列（此处只验证“可重复性”是强制的，
 *     不同种子不同序列并非标准强制，但作为合理性检查） */
static void test_different_seed(void)
{
    int i;
    int seq1[16];
    int seq2[16];
    int same = 1;

    srand(1u);
    for (i = 0; i < 16; i++)
        seq1[i] = rand();

    srand(999999u);
    for (i = 0; i < 16; i++)
        seq2[i] = rand();

    for (i = 0; i < 16; i++)
        if (seq1[i] != seq2[i]) { same = 0; break; }

    /* 不强制，但几乎必然不同；仅打印，不做断言 */
    printf("different seed produced %s sequence\n",
           same ? "the SAME" : "a different");
}

/* [2] 未调用 srand 时，等价于 srand(1)。
 *     注意：rand 在程序开始时可能已被其它代码调用过，
 *     因此这里用“先 srand(1) 取序列 A，再 srand(1) 取序列 B”来验证
 *     与“首次调用 srand(1)”的一致性语义。
 *     更严格的验证：在子进程中不调用 srand 直接 rand，
 *     与 srand(1) 后 rand 比较。这里用标准可移植方式：
 *     由于无法重置库内部状态，我们验证 srand(1) 的序列可重复，
 *     并说明“未调用 srand 等价于 srand(1)”由实现保证。 */
static void test_default_seed_equivalence(void)
{
    int i;
    int a[8];
    int b[8];

    srand(1u);
    for (i = 0; i < 8; i++)
        a[i] = rand();

    srand(1u);
    for (i = 0; i < 8; i++)
        b[i] = rand();

    for (i = 0; i < 8; i++)
        assert(a[i] == b[i]);           /* [2] srand(1) 可重复 */
}

/* [4] srand 返回 void：不能把它的“结果”用于需要值的上下文。
 *     正向：作为表达式语句调用是合法的。 */
static void test_void_return(void)
{
    srand(42u);                         /* [4] 合法：表达式语句 */
    srand(42u);
}

/* [5] EXAMPLE 实现的自洽性：my_srand/my_rand 满足 [2] 的语义 */
static void test_example_impl(void)
{
    int i;
    int s1[8];
    int s2[8];

    my_srand(7u);
    for (i = 0; i < 8; i++)
        s1[i] = my_rand();

    my_srand(7u);
    for (i = 0; i < 8; i++)
        s2[i] = my_rand();

    for (i = 0; i < 8; i++)
        assert(s1[i] == s2[i]);         /* [5] 示例实现可重复 */

    /* 示例实现中 RAND_MAX 假定为 32767，返回值应在 [0, 32767] */
    my_srand(1u);
    for (i = 0; i < 100; i++) {
        int r = my_rand();
        assert(r >= 0 && r <= 32767);
    }
}

/* [3] “实现行为上，没有库函数会调用 srand”：
 *     这是一个实现约束，无法在可移植代码中直接观测。
 *     我们通过“调用其它库函数不会改变 rand 序列”来间接验证。 */
static void test_no_library_calls_srand(void)
{
    int i;
    int before[8];
    int after[8];

    srand(2024u);
    for (i = 0; i < 8; i++)
        before[i] = rand();

    /* 调用若干常见库函数，它们不应重置 rand 序列 */
    {
        char buf[32];
        sprintf(buf, "%d", 12345);
        (void)buf;
        (void)malloc(16);
        (void)strlen("hello");
    }

    for (i = 0; i < 8; i++)
        after[i] = rand();

    /* 序列应继续，而不是从头开始 */
    {
        int j;
        int restart[8];
        srand(2024u);
        for (j = 0; j < 8; j++)
            restart[j] = rand();
        /* before 应与 restart 相同（同种子） */
        for (j = 0; j < 8; j++)
            assert(before[j] == restart[j]);
        /* after 是 before 之后的延续，不应等于 before（除非周期极短） */
        {
            int all_same = 1;
            for (j = 0; j < 8; j++)
                if (after[j] != before[j]) { all_same = 0; break; }
            /* 不强制断言，仅说明库函数未重置序列 */
            printf("library calls did%s reset rand sequence\n",
                   all_same ? "" : " NOT");
        }
    }
}

int main(void)
{
    test_prototype();
    test_same_seed_same_sequence();
    test_different_seed();
    test_default_seed_equivalence();
    test_void_return();
    test_example_impl();
    test_no_library_calls_srand();

    printf("All positive tests for C99 7.20.2.2 passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * （统一放在 #if 0 中，保证本文件整体可编译运行）
 * ============================================================ */
#if 0

/* 违反约束「srand 的原型为 void srand(unsigned int)」：
 * 用错误的参数类型调用（传指针），gcc -std=c99 应报错
 * （在 C99 中，无原型声明已不允许；此处 srand 有原型，
 *   传不兼容类型属于约束违反）。 */
void bad_call_wrong_arg(void)
{
    int x = 0;
    srand(&x);          /* 错误：期望 unsigned int，得到 int* */
}

/* 违反约束「srand 返回 void」：
 * 把 void 表达式用于需要值的上下文（赋值），应编译报错。 */
void bad_use_void_value(void)
{
    int r;
    r = srand(1u);      /* 错误：void 值不能赋给 int */
}

/* 违反约束「srand 返回 void」：
 * 对 void 表达式做算术运算，应编译报错。 */
void bad_arithmetic_on_void(void)
{
    int r = srand(1u) + 1;   /* 错误：void 不能参与算术 */
}

/* 违反约束「srand 的原型为 void srand(unsigned int)」：
 * 参数个数错误，应编译报错。 */
void bad_call_too_many_args(void)
{
    srand(1u, 2u);      /* 错误：参数过多 */
}

/* 违反约束「srand 的原型为 void srand(unsigned int)」：
 * 参数个数错误（缺少参数），应编译报错。 */
void bad_call_too_few_args(void)
{
    srand();            /* 错误：参数过少（有原型时不允许省略实参） */
}

/* 违反约束「srand 返回 void」：
 * 对 void 表达式取地址，应编译报错。 */
void bad_take_address_of_void(void)
{
    void *p = &srand(1u);   /* 错误：不能对 void 表达式取地址 */
}

#endif /* 负向测试结束 */