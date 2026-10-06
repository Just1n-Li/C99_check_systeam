/* 验证 C99 6.8.3 Expression and null statements */
/* 预期行为：正向测试运行通过，负向测试编译报错 */

#include <stdio.h>
#include <assert.h>

int side_effect_counter = 0;
int p(int x) {
    side_effect_counter += x;
    return x * 2;
}

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [2] 表达式语句作为 void 表达式求值，为了其副作用（如赋值和函数调用） */
    int a = 5;
    a = a + 1; /* 表达式语句，副作用是修改 a */
    assert(a == 6);

    side_effect_counter = 0;
    p(10); /* 函数调用作为表达式语句，丢弃返回值，副作用是修改 counter */
    assert(side_effect_counter == 10);

    /* [3] 空语句（仅含一个分号）不执行任何操作 */
    ; /* 空语句 */
    assert(side_effect_counter == 10); /* 状态未变 */

    /* [4] EXAMPLE 1: 显式转换为 void 丢弃返回值 */
    side_effect_counter = 0;
    (void)p(20); /* 显式丢弃返回值 */
    assert(side_effect_counter == 20);

    /* [5] EXAMPLE 2: 空语句作为循环体 */
    char s[] = "hello";
    char *ptr = s;
    while (*ptr++ != '\0') ; /* 空语句作为空循环体 */
    assert(*ptr == '\0'); /* ptr 指向字符串结尾的 '\0' */

    /* [6] EXAMPLE 3: 空语句带标签，位于复合语句的 } 之前 */
    int loop1 = 1, loop2 = 1, want_out = 1;
    while (loop1) {
        while (loop2) {
            if (want_out) goto end_loop1;
        }
    end_loop1: ; /* 空语句带标签 */
        break;
    }
    assert(loop1 == 1);

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* 违反语法约束 [1]：表达式语句必须以分号结尾，缺少分号应报错 */
    int b = 5
    b = b + 1;

    /* 违反语法约束 [1]：表达式语句必须以分号结尾，函数调用缺少分号应报错 */
    p(0)
    #endif

    return 0;
}