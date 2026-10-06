/*
 * 验证 C99 条款 6.11.7 Function definitions
 * 预期行为：
 * 正向测试：K&R 风格（非原型格式）的函数定义能编译并运行通过（编译器可能会发出过时警告，但不应报错）。
 * 负向测试：本条款仅声明该特性为 obsolescent（过时），未定义任何约束，故无负向测试。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证带有单独参数标识符和声明列表的函数定义（非原型格式）仍然可用 */
int k_and_r_add(a, b)
int a;
int b;
{
    return a + b;
}

/* [1] 验证无参数的 K&R 风格函数定义 */
int k_and_r_no_params()
{
    return 42;
}

int main(void)
{
    /* 测试 K&R 风格带参数函数 */
    assert(k_and_r_add(3, 4) == 7);
    printf("k_and_r_add(3, 4) = %d\n", k_and_r_add(3, 4));

    /* 测试 K&R 风格无参数函数 */
    assert(k_and_r_no_params() == 42);
    printf("k_and_r_no_params() = %d\n", k_and_r_no_params());

    printf("All positive tests passed.\n");

    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 
 * 6.11.7 条款仅指出使用单独参数标识符和声明列表的函数定义是 obsolescent（过时特性），
 * 并未将其列为约束。因此，使用 K&R 风格函数定义不会导致编译报错（编译器可能会发出警告）。
 * 故本条款无对应的负向测试。
 */
#endif