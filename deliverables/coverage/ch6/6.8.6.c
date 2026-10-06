/*
 * 验证 C99 6.8.6 Jump statements
 * 预期行为：正向测试运行通过，负向测试编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法测试：goto identifier ; */
/* [2] 语义测试：无条件跳转到另一处 */
static int test_goto(void) {
    int x = 0;
    x = 10;
    goto target;
    x = 20; /* 此行应被跳过 */
target:
    return x; /* 预期返回 10 */
}

/* [1] 语法测试：continue ; */
/* [2] 语义测试：无条件跳转到循环迭代的另一处（循环末尾） */
static int test_continue(void) {
    int sum = 0;
    for (int i = 0; i < 5; ++i) {
        if (i == 2) {
            continue; /* 跳过 i==2 时的累加 */
        }
        sum += i;
    }
    return sum; /* 0+1+3+4 = 8 */
}

/* [1] 语法测试：break ; */
/* [2] 语义测试：无条件跳出循环到另一处（循环外） */
static int test_break(void) {
    int sum = 0;
    for (int i = 0; i < 5; ++i) {
        if (i == 3) {
            break; /* i==3 时跳出 */
        }
        sum += i;
    }
    return sum; /* 0+1+2 = 3 */
}

/* [1] 语法测试：return expression ; */
/* [2] 语义测试：无条件跳转回调用者 */
static int test_return(void) {
    return 42;
}

int main(void) {
    assert(test_goto() == 10);
    assert(test_continue() == 8);
    assert(test_break() == 3);
    assert(test_return() == 42);
    
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反语法规则：goto 后面必须是 identifier，不能是整数字面量 */
void bad_goto(void) {
    goto 123;
}

/* [1] 违反语法规则：continue 后面不能有 expression */
void bad_continue(void) {
    for (;;) {
        continue 1;
    }
}

/* [1] 违反语法规则：break 后面不能有 expression */
void bad_break(void) {
    for (;;) {
        break 1;
    }
}

/* [1] 违反语法规则：return 后面必须是 expression 或为空，不能是类型声明 */
int bad_return(void) {
    return int x;
}
#endif