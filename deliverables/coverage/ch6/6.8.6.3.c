/* 验证 C99 6.8.6.3 The break statement
 * 预期行为：正向测试运行通过，负向测试编译报错 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    int count = 0;
    int i, j;

    /* [2] 语义：break 终止 while 循环 */
    while (1) {
        count++;
        break;
    }
    assert(count == 1);

    /* [2] 语义：break 终止 do-while 循环 */
    count = 0;
    do {
        count++;
        break;
    } while (1);
    assert(count == 1);

    /* [2] 语义：break 终止 for 循环 */
    count = 0;
    for (;;) {
        count++;
        break;
    }
    assert(count == 1);

    /* [2] 语义：break 终止 switch 语句 */
    count = 0;
    switch (1) {
        case 1:
            count = 10;
            break;
        case 2:
            count = 20;
            break;
    }
    assert(count == 10);

    /* [2] 语义：break 只终止最小封闭的循环或 switch（嵌套循环测试） */
    count = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (j == 1) {
                break; /* 只终止内层 for，不影响外层 */
            }
            count++;
        }
    }
    /* 外层循环执行 3 次，每次内层 j=0 时 count++，j=1 时 break */
    assert(count == 3);

    /* [2] 语义：switch 嵌套在循环中，break 只终止最小封闭的 switch */
    count = 0;
    for (i = 0; i < 3; i++) {
        switch (i) {
            case 1:
                break; /* 只终止 switch，不终止外层 for */
            default:
                count++;
                break;
        }
    }
    /* i=0: default, count=1; i=1: case 1, break; i=2: default, count=2 */
    assert(count == 2);

    printf("6.8.6.3 正向测试通过\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* [1] 约束：break 只能出现在 switch 体或循环体中。
     * 此处 break 直接出现在函数体中，不在任何循环或 switch 内，应报错 */
    break;

    /* [1] 约束：此处 break 出现在 if 语句中，但 if 不在循环或 switch 内，应报错 */
    if (1) {
        break;
    }
    #endif

    return 0;
}