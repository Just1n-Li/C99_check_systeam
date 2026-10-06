/*
 * 验证 C99 6.5.16 Assignment operators
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    /* [1] 语法：简单赋值和复合赋值运算符 */
    int a = 10;
    a += 5; assert(a == 15);
    a -= 5; assert(a == 10);
    a *= 2; assert(a == 20);
    a /= 4; assert(a == 5);
    a %= 3; assert(a == 2);
    a <<= 3; assert(a == 16);
    a >>= 2; assert(a == 4);
    a &= 2; assert(a == 0);
    a |= 5; assert(a == 5);
    a ^= 3; assert(a == 6);

    /* [3] 语义：赋值表达式的值是赋值后左操作数的值 */
    int b;
    int c = (b = 42);
    assert(c == 42 && b == 42);

    /* [3] 语义：赋值表达式的类型是左操作数的类型，限定类型则取非限定版本 */
    volatile int v = 10;
    assert(sizeof(v = 20) == sizeof(int)); /* v 是 volatile int，表达式类型是 int */
    assert(v == 20);

    /* [4] 语义：求值顺序未指定，但副作用在序列点前完成 */
    int x = 0;
    int y = 5;
    x = (y = y + 1); /* y 的求值和赋值在 x 赋值前完成 */
    assert(x == 6 && y == 6);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「左操作数必须是可修改的左值」：算术表达式不是左值，gcc -std=c99 应报错 */
int a = 1, b = 2, c = 3;
a + b = c;

/* [2] 违反约束「左操作数必须是可修改的左值」：const 变量不可修改，gcc -std=c99 应报错 */
const int d = 5;
d = 10;

/* [2] 违反约束「左操作数必须是可修改的左值」：数组名不可修改，gcc -std=c99 应报错 */
int arr[5];
arr = 0;

/* [2] 违反约束「左操作数必须是可修改的左值」：函数名不可修改，gcc -std=c99 应报错 */
void func(void) {}
func = 0;

/* [3] 违反约束「赋值表达式不是左值」：对赋值表达式赋值，gcc -std=c99 应报错 */
int e, f, g;
(e = f) = g;

#endif