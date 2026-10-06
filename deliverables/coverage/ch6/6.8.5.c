/*
 * 验证 C99 6.8.5 迭代语句
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 语法：覆盖 while, do-while, for(expression), for(declaration) 四种形式 */
    int count = 0;
    while (count < 3) { count++; }
    assert(count == 3);

    count = 0;
    do { count++; } while (count < 3);
    assert(count == 3);

    int i;
    for (i = 0; i < 3; i++) {}
    assert(i == 3);

    for (int j = 0; j < 3; j++) {}
    /* j 的作用域在 for 循环外不可见 */

    /* [4] 语义：重复执行直到控制表达式比较等于 0 */
    int k = 5;
    while (k) { k--; }
    assert(k == 0);

    /* [4] 语义：无论从迭代语句进入还是通过跳转进入，重复都会发生 */
    int m = 0;
    goto inside_loop;
    while (m < 2) {
        inside_loop:
        m++;
    }
    /* 跳转进入循环体后，执行 m++ 变为 1，随后判断 m<2 成立，再执行一次变为 2，最后判断为假退出 */
    assert(m == 2);

    /* [5] 语义：迭代语句是一个块，其作用域是外围块的严格子集 */
    int shadow = 100;
    for (int shadow = 0; shadow < 5; shadow++) {
        /* 内部 shadow 遮蔽外部 shadow */
    }
    assert(shadow == 100); /* 外部 shadow 未被修改 */

    /* Footnote 136: 跳过的代码不执行，for 的 clause-1 不被求值 */
    int init_check = 0;
    goto skip_for_init;
    for (init_check = 99; init_check < 100; init_check++) {
        /* loop body */
    }
    skip_for_init:
    assert(init_check == 0); /* for 的初始化子句被跳过，未执行 */

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* [2] 违反约束「控制表达式必须具有标量类型」：结构体不能作为控制表达式 */
    struct S { int a; } s_var;
    while (s_var) { break; }

    /* [2] 违反约束「控制表达式必须具有标量类型」：联合体不能作为控制表达式 */
    union U { int a; } u_var;
    do { break; } while (u_var);

    /* [2] 违反约束「控制表达式必须具有标量类型」：数组不能作为控制表达式 */
    int arr[5];
    for ( ; arr; ) { break; }

    /* [3] 违反约束「for 语句的声明部分只能声明具有 auto 或 register 存储类的对象」：static 不允许 */
    for (static int i = 0; i < 1; i++) { break; }

    /* [3] 违反约束「for 语句的声明部分只能声明具有 auto 或 register 存储类的对象」：extern 不允许 */
    for (extern int i; i < 1; i++) { break; }
    #endif

    return 0;
}