/*
 * 验证 C99 6.5 Expressions
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 表达式计算值、指定对象或函数、产生副作用 */
void test_6_5_1(void) {
    int x = 0;
    x = 5; /* 指定对象并产生副作用 */
    int y = x + 3; /* 计算值 */
    void (*fp)(void) = test_6_5_1; /* 指定函数 */
    assert(y == 8);
    assert(fp != NULL);
    (void)fp;
}

/* [2] 序列点之间对象最多修改一次，且读取仅用于确定要存储的值 */
void test_6_5_2(void) {
    int i = 1;
    i = i + 1; /* 允许：读取 i 仅为确定要存储的值 */
    assert(i == 2);

    int a[3] = {0, 0, 0};
    i = 0;
    a[i] = i; /* 允许：读取 i 仅为确定要存储的值 */
    assert(a[0] == 0);
}

/* [3] 求值顺序未指定（除特定操作符外） */
void test_6_5_3(void) {
    int a = 1, b = 2;
    /* 加法操作数求值顺序未指定，但结果确定 */
    int c = a + b;
    assert(c == 3);
}

/* [4] 位运算符操作数必须为整数类型 */
void test_6_5_4(void) {
    unsigned int ui = 0xF0F0;
    int i = 0x0F0F;
    
    assert((~ui) == 0x0F0F); /* 按位取反 */
    assert((i << 4) == 0xF0F0); /* 左移 */
    assert((ui >> 4) == 0x0F0F); /* 右移 */
    assert((ui & i) == 0x0); /* 按位与 */
    assert((ui ^ i) == 0xFFFF); /* 按位异或 */
    assert((ui | i) == 0xFFFF); /* 按位或 */
}

/* [5] 异常条件（如溢出）导致未定义行为，不进行正向或负向测试 */

/* [6] 有效类型：通过左值赋予动态分配的对象有效类型 */
void test_6_5_6_lvalue(void) {
    void *ptr = malloc(sizeof(int));
    assert(ptr != NULL);
    
    /* 分配的对象没有声明类型，通过 int 类型的左值存储值，使有效类型变为 int */
    int *ip = (int *)ptr;
    *ip = 42;
    
    /* 后续访问不修改值，有效类型仍为 int */
    assert(*ip == 42);
    
    free(ptr);
}

/* [6] 有效类型：使用 memcpy 复制值到无声明类型的对象 */
void test_6_5_6_memcpy(void) {
    int src = 99;
    void *ptr = malloc(sizeof(int));
    assert(ptr != NULL);
    
    /* 使用 memcpy 复制值到无声明类型的对象，有效类型变为源对象的有效类型 */
    memcpy(ptr, &src, sizeof(int));
    
    int *ip = (int *)ptr;
    assert(*ip == 99);
    
    free(ptr);
}

/* [7] 别名规则：允许使用字符类型访问任何对象 */
void test_6_5_7_char(void) {
    int x = 0x41424344;
    char *cp = (char *)&x;
    
    /* 字符类型可以访问任何对象的有效类型 */
    char c0 = cp[0];
    char c1 = cp[1];
    char c2 = cp[2];
    char c3 = cp[3];
    
    (void)c0; (void)c1; (void)c2; (void)c3;
    assert(cp != NULL);
}

/* [7] 别名规则：聚合类型包含兼容类型 */
void test_6_5_7_aggregate(void) {
    struct S { int x; } s = {42};
    int *ip = &s.x; /* 通过聚合类型的成员访问 */
    assert(*ip == 42);
}

/* [8] 浮点表达式收缩 */
void test_6_5_8(void) {
    double x = 3.0;
    double y = 4.0;
    double z = 5.0;
    
    /* 允许收缩 */
    #pragma STDC FP_CONTRACT ON
    double res1 = x * y + z;
    
    /* 禁止收缩 */
    #pragma STDC FP_CONTRACT OFF
    double res2 = x * y + z;
    
    /* 默认实现定义 */
    #pragma STDC FP_CONTRACT DEFAULT
    double res3 = x * y + z;
    
    assert(res1 == 17.0);
    assert(res2 == 17.0);
    assert(res3 == 17.0);
}

int main(void) {
    test_6_5_1();
    test_6_5_2();
    test_6_5_3();
    test_6_5_4();
    test_6_5_6_lvalue();
    test_6_5_6_memcpy();
    test_6_5_7_char();
    test_6_5_7_aggregate();
    test_6_5_8();
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [4] 违反约束「位运算符操作数必须为整数类型」：浮点数不能用于位运算，gcc -std=c99 应报错 */
void test_negative_6_5_4(void) {
    float f = 1.0f;
    double d = 2.0;
    
    /* 按位取反 ~ 要求整数类型 */
    ~f;
    
    /* 左移 << 要求整数类型 */
    f << 1;
    
    /* 右移 >> 要求整数类型 */
    f >> 1;
    
    /* 按位与 & 要求整数类型 */
    f & d;
    
    /* 按位异或 ^ 要求整数类型 */
    f ^ d;
    
    /* 按位或 | 要求整数类型 */
    f | d;
}
#endif