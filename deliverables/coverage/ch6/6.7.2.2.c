/*
 * 验证 C99 6.7.2.2 Enumeration specifiers
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法测试：尾随逗号、enum identifier 引用 */
void test_syntax(void) {
    /* [1] enumerator-list 后可以有尾随逗号 */
    enum E1 { A1, B1, };
    assert(A1 == 0);
    assert(B1 == 1);

    /* [1] enum identifier 作为类型说明符引用已定义的枚举 */
    enum E1 e1;
    e1 = B1;
    assert(e1 == 1);
}

/* [3] 语义测试：默认值、= 指定值、重复值、类型为 int */
void test_semantics_3(void) {
    /* [5] 示例代码 */
    enum hue { chartreuse, burgundy, claret = 20, winedark };
    enum hue col, *cp;

    col = claret;
    cp = &col;

    /* [3] 第一个枚举常量默认为 0 */
    assert(chartreuse == 0);
    /* [3] 后续无 = 的枚举常量为前一个 + 1 */
    assert(burgundy == 1);
    /* [3] 使用 = 定义枚举常量的值 */
    assert(claret == 20);
    /* [3] 后续无 = 的枚举常量为前一个 + 1 */
    assert(winedark == 21);

    /* [3] 枚举常量具有 int 类型 */
    int i = burgundy;
    assert(i == 1);
    assert(sizeof(burgundy) == sizeof(int));

    /* [3] 使用 = 可能产生重复的值 */
    enum dup { D_A = 10, D_B = 10, D_C = 5, D_D = 5 };
    assert(D_A == D_B);
    assert(D_C == D_D);

    /* [5] 示例逻辑验证 */
    if (*cp != burgundy) {
        /* col = claret (20), burgundy = 1, 20 != 1 为真，进入此分支 */
        assert(*cp == claret);
    } else {
        assert(0); /* 不应到达 */
    }
}

/* [4] 语义测试：兼容性与完整类型 */
void test_semantics_4(void) {
    enum E { X = 0, Y = 255 };
    enum E e = Y;

    /* [4] 枚举类型在 } 之后是完整类型，可以求 sizeof */
    size_t sz = sizeof(enum E);
    /* [4] 与 char, signed int, 或 unsigned int 兼容，大小应为 1, 2, 4 或 8 */
    assert(sz == 1 || sz == 2 || sz == 4 || sz == 8);

    /* [4] 可以隐式转换为整数类型 */
    int val = e;
    assert(val == 255);
}

int main(void) {
    test_syntax();
    test_semantics_3();
    test_semantics_4();
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [2] 违反约束「必须是整型常量表达式」：使用变量初始化枚举常量 */
void test_neg_var(void) {
    int v = 5;
    enum E { A = v }; /* v 不是常量表达式 */
}

/* [2] 违反约束「必须是整型常量表达式」：使用浮点常量初始化 */
void test_neg_float(void) {
    enum E { A = 1.5 }; /* 1.5 不是整型常量表达式 */
}

/* [2] 违反约束「必须是整型常量表达式」：使用字符串常量初始化 */
void test_neg_string(void) {
    enum E { A = "hello" }; /* 字符串不是整型常量表达式 */
}

/* [2] 违反约束「值必须能表示为 int」：超出 int 范围的常量 (假设 int 为 32 位) */
void test_neg_range(void) {
    enum E { A = 2147483648 }; /* 2^31 超出 32 位 int 的最大值 2^31-1 */
}

/* [2] 违反约束「必须是整型常量表达式」：包含逗号运算符 */
void test_neg_comma(void) {
    enum E { A = (1, 2) }; /* 逗号表达式不是整型常量表达式 */
}
#endif