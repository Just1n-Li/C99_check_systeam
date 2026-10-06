/*
 * 验证 C99 6.8.4.1 The if statement
 * 正向测试：应能编译并运行通过，验证 [2][3] 语义
 * 负向测试：违反 [1] 约束（控制表达式非标量类型），应编译报错
 */

#include <assert.h>
#include <stdio.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [2] 表达式不等于 0 时执行第一个子语句 */
    {
        int executed = 0;
        if (1)
            executed = 1;
        assert(executed == 1);
    }

    /* [2] 表达式等于 0 时不执行第一个子语句 */
    {
        int executed = 0;
        if (0)
            executed = 1;
        assert(executed == 0);
    }

    /* [2] else 形式：表达式不等于 0 时执行第一个子语句，不执行第二个 */
    {
        int first = 0, second = 0;
        if (1)
            first = 1;
        else
            second = 1;
        assert(first == 1 && second == 0);
    }

    /* [2] else 形式：表达式等于 0 时执行第二个子语句 */
    {
        int first = 0, second = 0;
        if (0)
            first = 1;
        else
            second = 1;
        assert(first == 0 && second == 1);
    }

    /* [1] 正向：指针类型是标量类型，可作为控制表达式 */
    {
        int x = 42;
        int *p = &x;
        int hit = 0;
        if (p)
            hit = 1;
        assert(hit == 1);

        int *null_p = NULL;
        hit = 0;
        if (null_p)
            hit = 1;
        else
            hit = 2;
        assert(hit == 2);
    }

    /* [1] 正向：浮点类型是标量类型，可作为控制表达式 */
    {
        double d = 3.14;
        int hit = 0;
        if (d)
            hit = 1;
        assert(hit == 1);

        double zero = 0.0;
        hit = 0;
        if (zero)
            hit = 1;
        else
            hit = 2;
        assert(hit == 2);
    }

    /* [1] 正向：枚举类型是标量类型（整数类型），可作为控制表达式 */
    {
        enum Color { RED, GREEN, BLUE } c = GREEN;
        int hit = 0;
        if (c)
            hit = 1;
        assert(hit == 1);

        c = RED;
        hit = 0;
        if (c)
            hit = 1;
        else
            hit = 2;
        assert(hit == 2);
    }

    /* [3] else 与最近的 if 关联（dangling else）——外层为真、内层为假 */
    {
        int result = 0;
        if (1)
            if (0)
                result = 1;  /* 内层 if 为假，不执行 */
            else
                result = 2;  /* else 与内层 if 关联，执行 */
        assert(result == 2);
    }

    /* [3] else 与最近的 if 关联——外层为假，整体不执行 */
    {
        int result = 0;
        if (0)
            if (1)
                result = 1;
            else
                result = 2;  /* else 与内层 if 关联，但外层 if 为假，整体跳过 */
        assert(result == 0);
    }

    /* [3] 用花括号改变 else 关联：else 与外层 if 关联 */
    {
        int result = 0;
        if (1) {
            if (0)
                result = 1;
        } else {
            result = 2;  /* else 与外层 if 关联，外层为真，不执行 */
        }
        assert(result == 0);
    }

    /* [2] 如果第一个子语句通过标签到达，第二个子语句不执行 */
    {
        int first = 0, second = 0;
        goto target;
        if (0)
        target:
            first = 1;
        else
            second = 1;
        /* goto 跳入 if 的第一个子语句，else 不执行 */
        assert(first == 1 && second == 0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [1] 违反约束「控制表达式必须为标量类型」：结构体不是标量类型，
      gcc -std=c99 应报错 "used type 'struct S' where scalar is required" */
{
    struct S { int x; } s = {1};
    if (s)
        (void)0;
}

/* [1] 违反约束「控制表达式必须为标量类型」：联合体不是标量类型，
      gcc -std=c99 应报错 */
{
    union U { int x; float f; } u;
    u.x = 1;
    if (u)
        (void)0;
}

#endif