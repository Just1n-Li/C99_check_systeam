/*
 * 验证 C99 条款 6.3.1.3 Signed and unsigned integers
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <limits.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 当一个整数类型的值转换为另一个整数类型（非 _Bool）时，如果该值能被新类型表示，则值不变。 */
    {
        int i = 100;
        long l = i; /* int -> long, 值在 long 范围内，值不变 */
        assert(l == 100);

        int ni = -100;
        short s = ni; /* int -> short, 值在 short 范围内，值不变 */
        assert(s == -100);

        unsigned int ui = 200U;
        int si = ui; /* unsigned int -> int, 值在 int 范围内，值不变 */
        assert(si == 200);
    }

    /* [2] 否则，如果新类型是无符号的，则通过反复加或减（新类型能表示的最大值 + 1）直到值在新类型的范围内。 */
    {
        int ni = -1;
        unsigned int ui = ni; /* 数学值 -1，加 UINT_MAX+1，结果为 UINT_MAX */
        assert(ui == UINT_MAX);

        int i257 = 257;
        unsigned char uc = i257; /* 数学值 257，减 256，结果为 1 */
        assert(uc == 1);

        int ni129 = -129;
        unsigned char uc2 = ni129; /* 数学值 -129，加 256，结果为 127 */
        assert(uc2 == 127);
    }

    /* [3] 否则，新类型是有符号的且值无法表示；结果是实现定义的，或者引发实现定义的信号。 */
    {
        /* 此处结果为实现定义，不使用 assert 断言特定值，仅打印结果 */
        int i200 = 200;
        signed char sc = i200; /* 200 超出 SCHAR_MAX，实现定义 */
        printf("[3] (signed char)200 = %d (implementation-defined)\n", (int)sc);

        int ni200 = -200;
        signed char sc2 = ni200; /* -200 小于 SCHAR_MIN，实现定义 */
        printf("[3] (signed char)(-200) = %d (implementation-defined)\n", (int)sc2);
    }

    /* Footnote 49: 规则描述的是数学值上的算术，而不是给定类型表达式的值。 */
    {
        /* 即使 -1 在 int 中以补码表示，转换为 unsigned long 时也是基于数学值 -1 计算 */
        int ni = -1;
        unsigned long ul = ni;
        assert(ul == ULONG_MAX);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 6.3.1.3 描述整数到整数的转换。条款本身无直接约束，
   但强制转换要求操作数必须是标量类型（6.5.4）。
   以下代码试图将结构体强制转换为整数类型，违反 6.5.4 约束。 */
struct S { int x; } s;
int i = (int)s; /* 期望编译报错：操作数不是标量类型，不能转换为整数 */
#endif