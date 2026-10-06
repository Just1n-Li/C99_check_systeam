/*
 * 验证 C99 6.5.7 Bitwise shift operators
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <assert.h>
#include <limits.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 语法与结合性：左移和右移运算符，从左到右结合 */
    assert((1 << 2 << 1) == 8);   /* (1 << 2) << 1 == 4 << 1 == 8 */
    assert((16 >> 2 >> 1) == 2);  /* (16 >> 2) >> 1 == 4 >> 1 == 2 */

    /* [2] 约束：操作数为整数类型 */
    unsigned int u_val = 10u;
    int i_val = 10;
    long l_val = 10L;
    assert((u_val << 1) == 20u);
    assert((i_val >> 1) == 5);
    assert((l_val << 1) == 20L);

    /* [3] 整数提升：结果类型是提升后的左操作数的类型 */
    char ch = 1;
    unsigned char uch = 1;
    /* char 和 unsigned char 在移位前提升为 int */
    assert(sizeof(ch << 1) == sizeof(int));
    assert(sizeof(uch << 1) == sizeof(int));
    /* 提升后可以表示更大的值 */
    assert((ch << 8) == 256);
    assert((uch << 8) == 256);
    /* 注意：如果右操作数为负或大于等于左操作数宽度，行为未定义(UB)，此处不测试 UB */

    /* [4] 左移语义：E1 << E2 */
    /* 无符号类型：E1 x 2^E2 mod 2^N */
    int width = sizeof(int) * CHAR_BIT;
    unsigned int u_ovf = (1u << (width - 1));
    assert((u_ovf << 1) == 0u); /* 2^(width-1) * 2 mod 2^width = 0 */
    assert((5u << 2) == 20u);   /* 5 * 2^2 = 20 */

    /* 有符号非负且 E1 x 2^E2 可表示 */
    int s_val = 1;
    assert((s_val << 4) == 16); /* 1 * 2^4 = 16 */

    /* [5] 右移语义：E1 >> E2 */
    /* 无符号类型：E1 / 2^E2 的整数部分 */
    unsigned int u_rshift = 100u;
    assert((u_rshift >> 2) == 25); /* 100 / 4 = 25 */

    /* 有符号非负：E1 / 2^E2 的整数部分 */
    int s_rshift = 100;
    assert((s_rshift >> 2) == 25);

    /* 有符号负数：结果由实现定义 (implementation-defined) */
    int s_neg = -100;
    int res_neg = s_neg >> 2;
    (void)res_neg; /* 不做断言，仅验证能编译运行 */

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* [2] 违反约束「操作数必须为整数类型」：浮点数不能作为左操作数，gcc -std=c99 应报错 */
    float f1 = 1.0f;
    f1 << 1;

    /* [2] 违反约束「操作数必须为整数类型」：浮点数不能作为右操作数，gcc -std=c99 应报错 */
    int i1 = 1;
    i1 >> 1.0;

    /* [2] 违反约束「操作数必须为整数类型」：指针不能移位，gcc -std=c99 应报错 */
    int x = 0;
    int *p = &x;
    p << 1;

    /* [2] 违反约束「操作数必须为整数类型」：数组不能移位，gcc -std=c99 应报错 */
    int arr[3] = {0, 1, 2};
    arr >> 1;
#endif

    return 0;
}