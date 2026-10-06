/*
 * 验证 C99 条款 6.5.11 Bitwise exclusive OR operator
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 语法：exclusive-OR-expression ^ AND-expression */
    {
        int a = 5, b = 3;
        int c = a ^ b;
        assert(c == 6); /* 0101 ^ 0011 = 0110 */
    }

    /* [3] 语义：常规算术转换 */
    {
        char c = 5;
        int i = 3;
        /* char 提升为 int */
        assert((c ^ i) == 6);
        
        unsigned int u = 0xFFFFFFFFu;
        int si = 0;
        /* int 转换为 unsigned int */
        assert((u ^ si) == 0xFFFFFFFFu);
        
        /* 测试有符号与无符号运算时的转换 */
        unsigned int u2 = 1u;
        int si2 = -1;
        /* -1 转换为 unsigned int 为 0xFFFFFFFF */
        assert((u2 ^ si2) == 0xFFFFFFFEu);
    }

    /* [4] 语义：按位异或结果 */
    {
        unsigned int a = 0xF0F0u;
        unsigned int b = 0xFF00u;
        /* 1111000011110000 ^ 1111111100000000 = 0000111111110000 */
        assert((a ^ b) == 0x0FF0u);
        
        /* a ^ a == 0 */
        assert((a ^ a) == 0);
        
        /* a ^ 0 == a */
        assert((a ^ 0u) == a);
        
        /* 交换律 */
        assert((a ^ b) == (b ^ a));
        
        /* 结合律 */
        unsigned int c = 0x00FFu;
        assert(((a ^ b) ^ c) == (a ^ (b ^ c)));
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束「操作数必须为整数类型」：浮点数不能使用 ^ 运算符 */
void test_float(void) {
    double a = 1.0, b = 2.0;
    double c = a ^ b; /* gcc -std=c99 应报错: invalid operands to binary ^ */
}

/* [2] 违反约束「操作数必须为整数类型」：指针不能使用 ^ 运算符 */
void test_pointer(void) {
    int x = 1, y = 2;
    int *p = &x, *q = &y;
    int *r = p ^ q; /* gcc -std=c99 应报错: invalid operands to binary ^ */
}

/* [2] 违反约束「操作数必须为整数类型」：结构体不能使用 ^ 运算符 */
void test_struct(void) {
    struct S { int x; } a, b;
    struct S c = a ^ b; /* gcc -std=c99 应报错: invalid operands to binary ^ */
}
#endif