/*
 * 验证 C99 条款 6.7.6 (Type names)
 * 预期行为：
 * - 正向测试：能编译并运行通过，验证类型名的语法和语义。
 * - 负向测试：编译报错，验证违反类型名语法和约束的代码被拒绝。
 */

#include <stdio.h>
#include <assert.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [2] Semantics: type name omits the identifier, used in sizeof and casts */
    /* [3] EXAMPLE (a) int */
    assert(sizeof(int) == sizeof(int));

    /* [3] EXAMPLE (b) pointer to int */
    {
        int x = 10;
        int *p = (int *)&x; /* 使用类型名进行强制转换 */
        assert(*p == 10);
    }

    /* [3] EXAMPLE (c) array of three pointers to int */
    {
        int a = 1, b = 2, c = 3;
        int *arr[3] = {&a, &b, &c};
        int *(*arr_cast)[3] = (int *(*)[3])&arr;
        assert((*arr_cast)[0] == &a);
    }

    /* [3] EXAMPLE (d) pointer to an array of three ints */
    {
        int arr[3] = {1, 2, 3};
        int (*p)[3] = (int (*)[3])&arr;
        assert((*p)[0] == 1);
    }

    /* [3] EXAMPLE (e) pointer to a variable length array of an unspecified number of ints */
    {
        int n = 5;
        int vla[n];
        /* int (*)[*] 只能在块作用域使用，且通常用于 sizeof 或强制转换 */
        int (*p)[n] = (int (*)[*])&vla;
        assert(sizeof(int (*)[*]) == sizeof(int (*)[n]));
    }

    /* [3] EXAMPLE (f) function with no parameter specification returning a pointer to int */
    {
        /* int *() 是函数类型，不能 sizeof，但可以声明指向它的指针 */
        int *(*fp)() = (int *(*)())0;
        assert(fp == 0);
    }

    /* [3] EXAMPLE (g) pointer to function with no parameters returning an int */
    {
        int (*fp)(void) = (int (*)(void))0;
        assert(fp == 0);
    }

    /* [3] EXAMPLE (h) array of an unspecified number of constant pointers to functions */
    {
        int (*const (*ptr)[])(unsigned int, ...) = 0;
        assert(ptr == 0);
    }

    /* [2] Semantics: type name used in compound literal */
    {
        int *p = (int []){1, 2, 3}; /* 类型名 int [] 省略了标识符 */
        assert(p[0] == 1);
    }

    /* 脚注 128: empty parentheses in a type name are interpreted as "function with no parameter specification" */
    {
        /* int () 是函数类型，int (*)() 是指向该函数类型的指针 */
        int (*fp)() = (int (*)())0;
        assert(fp == 0);
    }

    printf("All positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* 违反 type-name 语法：类型名中不能包含标识符 */
    sizeof(int x);

    /* 违反 type-name 语法：类型名中不能包含存储类说明符 */
    sizeof(static int);

    /* 违反 abstract-declarator 语法：指针符号位置错误 */
    sizeof(int [3]*);

    /* 违反 abstract-declarator 语法：函数返回数组是不允许的（6.7.5.3约束），在类型名中表现为语法错误 */
    sizeof(int [3]());

    /* 违反 abstract-declarator 语法：数组大小不能是负数（6.7.5.2约束） */
    sizeof(int [-1]);
    #endif
}