/*
 * 验证 C99 6.5.10 Bitwise AND operator
 * 预期行为：正向测试运行通过，负向测试编译报错
 */
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 语法：AND-expression & equality-expression，支持连续按位与 */
    {
        int a = 0xF0;
        int b = 0x0F;
        int c = 0xFF;
        int d = a & b & c; /* 连续按位与 */
        assert(d == 0x00);
    }

    /* [3] 语义：常规算术转换在操作数上执行 */
    {
        int i = 0xFF;
        unsigned char c = 0x0F;
        /* c 被提升为 int，然后与 i 进行按位与 */
        int r = i & c;
        assert(r == 0x0F);
        
        unsigned int ui = 0xFFFFFFFFu;
        int si = 0x0F;
        /* si 转换为 unsigned int，结果为 unsigned int */
        unsigned int ur = ui & si;
        assert(ur == 0x0000000Fu);
    }

    /* [4] 语义：按位与结果，当且仅当对应位均为 1 时结果位置 1 */
    {
        int a = 0xAB; /* 1010 1011 */
        int b = 0xF0; /* 1111 0000 */
        int r = a & b; /* 1010 0000 = 0xA0 */
        assert(r == 0xA0);
        
        /* 逐一验证四种位组合 */
        assert((0x1 & 0x1) == 0x1);
        assert((0x1 & 0x0) == 0x0);
        assert((0x0 & 0x1) == 0x0);
        assert((0x0 & 0x0) == 0x0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束「操作数必须为整数类型」：浮点数不能进行按位与，gcc -std=c99 应报错 */
{
    float f = 1.0f;
    int i = 1;
    f & i;
}

/* [2] 违反约束「操作数必须为整数类型」：指针不能进行按位与，gcc -std=c99 应报错 */
{
    int *p = 0;
    int i = 1;
    p & i;
}

/* [2] 违反约束「操作数必须为整数类型」：结构体不能进行按位与，gcc -std=c99 应报错 */
{
    struct S { int x; } a, b;
    a & b;
}

/* [2] 违反约束「操作数必须为整数类型」：数组不能进行按位与，gcc -std=c99 应报错 */
{
    int arr1[5] = {0};
    int arr2[5] = {0};
    arr1 & arr2;
}
#endif