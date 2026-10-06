/*
 * 验证 C99 6.4.4.3 Enumeration constants
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <assert.h>
#include <stdio.h>

enum Color { RED, GREEN, BLUE };

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 语法：enumeration-constant 必须是 identifier */
    /* RED, GREEN, BLUE 均为合法标识符，作为枚举常量声明成功 */

    /* [2] 语义：An identifier declared as an enumeration constant has type int. */
    /* 验证枚举常量的 sizeof 等于 sizeof(int) */
    assert(sizeof(RED) == sizeof(int));
    assert(sizeof(GREEN) == sizeof(int));
    assert(sizeof(BLUE) == sizeof(int));

    /* 验证枚举常量可赋值给 int 变量 */
    int i = RED;
    assert(i == 0);

    /* 验证枚举常量是常量表达式，可用于数组维度声明 */
    int arr[RED + 3];
    arr[RED] = 10;
    assert(arr[RED] == 10);

    /* 验证枚举常量参与算术运算，结果类型仍为 int */
    assert(sizeof(RED + 1) == sizeof(int));

    printf("6.4.4.3 Enumeration constants: 正向测试通过\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* 违反语法 [1] enumeration-constant 必须是 identifier：数字字面量不能作为枚举常量 */
    enum E1 { 123 };

    /* 违反语法 [1] enumeration-constant 必须是 identifier：字符串字面量不能作为枚举常量 */
    enum E2 { "abc" };

    /* 违反语法 [1] enumeration-constant 必须是 identifier：关键字不能作为标识符 */
    enum E3 { int };
    #endif

    return 0;
}