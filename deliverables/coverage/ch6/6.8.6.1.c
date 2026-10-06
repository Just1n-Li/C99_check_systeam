/*
 * 验证 C99 6.8.6.1 The goto statement
 *
 * 正向测试：应能编译（gcc -std=c99 -pedantic）并运行通过，所有 assert 成立。
 * 负向测试：违反 6.8.6.1 约束，应编译报错（放在 #if 0 块中以保证文件可编译）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [2] 语义：goto 语句无条件跳转到同一函数内带命名标签的语句 */
static void test_semantics_unconditional_jump(void)
{
    int x = 0;
    goto target;
    x = 999;        /* 此行应被跳过 */
target:
    x = 42;
    assert(x == 42);
}

/* [3] EXAMPLE 1：跳入复杂语句集合的中间（跳入 for 循环体内部） */
static void test_example1_jump_into_middle(void)
{
    int count = 0;
    int need_reinit = 1;
    int iterations = 0;

    goto first_time;
    for (;;) {
        /* determine next operation */
        iterations++;
        if (iterations > 10) break;
        if (need_reinit) {
            /* reinitialize-only code */
            need_reinit = 0;
first_time:
            /* general initialization code */
            count = 100;
            continue;
        }
        /* handle other operations */
        count++;
    }
    /* first_time 跳转后 count 被设为 100，随后 continue 回到循环头，
       iterations 递增到超过 10 后 break 退出 */
    assert(count == 100);
}

/* [4] EXAMPLE 2 合法部分：在 VLA 作用域内跳转（不跨越 VLA 声明） */
static void test_example2_jump_within_vla_scope(int n, int j)
{
    {
        double a[n];          /* VLA 声明 */
        a[j] = 4.4;
        goto lab3;            /* valid: going WITHIN scope of VLA */
        a[j] = 5.5;           /* 被跳过 */
lab3:
        a[j] = 6.6;
        assert(a[j] == 6.6);  /* 跳转后 a[j] 应为 6.6 */
    }
}

int main(void)
{
    test_semantics_unconditional_jump();
    test_example1_jump_into_middle();
    test_example2_jump_within_vla_scope(5, 2);
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 6.8.6.1 约束，应编译报错 ========== */
#if 0

/* [1] 违反约束「goto 的标识符必须命名同一函数（enclosing function）内的标签」：
   目标标签定义在另一个函数中，gcc -std=c99 应报错 "label not found" 或类似错误 */
static void other_function(void)
{
other_label:
    return;
}
static void test_goto_label_in_other_function(void)
{
    goto other_label;  /* other_label 不在本函数内 */
}

/* [1] 违反约束「goto 的标识符必须命名一个标签」：
   跳转到根本不存在的标签，gcc -std=c99 应报错 */
static void test_goto_nonexistent_label(void)
{
    goto nonexistent_label;
}

/* [1] 违反约束「不得从 VLA 作用域外跳入 VLA 作用域内」：
   EXAMPLE 2 第一个非法示例 —— goto lab3 从块外跳入 VLA a 的作用域，
   gcc -std=c99 应报错 "jump into scope of variably modified type" */
static void test_goto_into_vla_scope_1(int n, int j)
{
    goto lab3;  /* invalid: going INTO scope of VLA */
    {
        double a[n];
        a[j] = 4.4;
lab3:
        a[j] = 3.3;
    }
}

/* [1] 违反约束「不得从 VLA 作用域外跳入 VLA 作用域内」：
   EXAMPLE 2 第二个非法示例 —— 块外的 goto lab4 跳入已离开的 VLA 作用域，
   gcc -std=c99 应报错 "jump into scope of variably modified type" */
static void test_goto_into_vla_scope_2(int n, int j)
{
    {
        double a[n];
        a[j] = 4.4;
        goto lab4;  /* valid: going WITHIN scope of VLA */
        a[j] = 5.5;
lab4:
        a[j] = 6.6;
    }
    goto lab4;  /* invalid: going INTO scope of VLA */
}

#endif