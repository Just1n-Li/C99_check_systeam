/*
 * 验证 C99 6.8.5.3 The for statement
 * 预期行为：正向测试运行通过，负向测试编译报错
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    int i;
    int count = 0;

    /* [1] 基本行为：clause-1 声明，expression-2 控制表达式，expression-3 作为 void 表达式在每次循环后求值 */
    for (int j = 0; j < 3; j++) {
        count++;
    }
    assert(count == 3);

    /* [1] clause-1 是表达式：作为 void 表达式在第一次求值控制表达式前求值 */
    count = 0;
    for (i = 0; i < 3; i++) {
        count++;
    }
    assert(count == 3);

    /* [1] clause-1 是声明：其作用域包含整个循环，包括另外两个表达式 (expression-2 和 expression-3) */
    count = 0;
    for (int k = 0; k < 3; k = k + 1) {
        count += k;
    }
    assert(count == 3); /* 0 + 1 + 2 = 3 */

    /* [2] 省略 clause-1 */
    count = 0;
    i = 0;
    for (; i < 3; i++) {
        count++;
    }
    assert(count == 3);

    /* [2] 省略 expression-3 */
    count = 0;
    for (i = 0; i < 3;) {
        count++;
        i++;
    }
    assert(count == 3);

    /* [2] 省略 expression-2：被替换为非零常量，需用 break 跳出 */
    count = 0;
    for (i = 0; ; i++) {
        if (i >= 3) break;
        count++;
    }
    assert(count == 3);

    /* [2] 省略所有可省略的部分 */
    count = 0;
    for (;;) {
        if (count >= 3) break;
        count++;
    }
    assert(count == 3);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反 6.8.5 约束（控制表达式必须为标量类型）：结构体不能作为控制表达式，gcc -std=c99 应报错 */
struct S { int x; } s;
for (; s; ) { break; }

/* 违反 6.8.5.3 [1] 语法约束：clause-1 只能是表达式或声明，不能是语句（如 if 语句），gcc -std=c99 应报错 */
for (if (1); ; ) { break; }

/* 违反 6.8.5.3 [1] 作用域约束：clause-1 声明的标识符作用域仅限于 for 循环内部，外部不可见，gcc -std=c99 应报错 */
for (int i = 0; i < 1; i++) { }
i = 5; /* 'i' undeclared */

/* 违反 6.8.5.3 [1] 语法约束：clause-1 作为声明时，不能包含多个独立的声明语句，gcc -std=c99 应报错 */
for (int i = 0, int j = 0; ; ) { break; }

#endif