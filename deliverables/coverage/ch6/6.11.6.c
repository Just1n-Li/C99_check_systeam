/*
 * 验证 C99 条款 6.11.6: Function declarators
 * [1] The use of function declarators with empty parentheses (not prototype-format parameter type declarators) is an obsolescent feature.
 *
 * 预期行为：
 * 正向测试：使用空括号的函数声明符（非原型格式）能编译并运行通过，验证其默认实参提升语义。
 * 负向测试：本条款仅为"过时特性"说明，无约束(Constraints)，因此无负向编译报错测试。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 使用带空括号的函数声明符（非原型格式），这是过时特性但在 C99 中合法 */
int old_func(); /* 非原型声明 */

int old_func(int a) {
    return a * 2;
}

/* 验证无原型函数调用的默认实参提升：char -> int, float -> double */
void expects_int(int i) {
    assert(i == (int)'A');
}
void expects_double(double d) {
    assert(d == 1.5);
}

int main(void) {
    /* [1] 调用非原型声明的函数 */
    assert(old_func(21) == 42);

    /* 默认实参提升：通过无原型函数指针调用，char 提升为 int */
    void (*fp_int)() = expects_int;
    fp_int((char)'A');

    /* 默认实参提升：通过无原型函数指针调用，float 提升为 double */
    void (*fp_double)() = expects_double;
    fp_double(1.5f);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 6.11.6 条款本身仅为"过时特性"说明，不包含任何约束(Constraints)。
   使用空括号的函数声明符在 C99 中仍然合法，编译器最多给出警告，不应报错。
   因此此处无对应的负向约束测试。 */
#endif