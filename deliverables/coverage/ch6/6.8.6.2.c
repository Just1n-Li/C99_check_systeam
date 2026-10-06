/*
 * 验证 C99 6.8.6.2 The continue statement
 * 正向测试：continue 在各种循环中的语义行为，应编译并运行通过
 * 负向测试：continue 出现在循环体外，应编译报错
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [2] continue 在 while 循环中：跳转到循环继续部分（条件判断） */
    {
        int i = 0;
        int count = 0;
        while (i < 10) {
            i++;
            if (i % 2 == 0)
                continue; /* 跳过偶数的计数 */
            count++;
        }
        /* i=1,3,5,7,9 被计数，共 5 个 */
        assert(count == 5);
        assert(i == 10);
    }

    /* [2] continue 在 do-while 循环中：跳转到循环继续部分（条件判断） */
    {
        int i = 0;
        int count = 0;
        do {
            i++;
            if (i % 3 == 0)
                continue; /* 跳过 3 的倍数 */
            count++;
        } while (i < 10);
        /* i=1,2,4,5,7,8,10 被计数，共 7 个 */
        assert(count == 7);
        assert(i == 10);
    }

    /* [2] continue 在 for 循环中：跳转到循环继续部分（即 reinit/condition 部分） */
    {
        int count = 0;
        for (int i = 0; i < 10; i++) {
            if (i % 2 == 0)
                continue; /* 跳过偶数，但 i++ 仍执行 */
            count++;
        }
        /* i=1,3,5,7,9 被计数，共 5 个 */
        assert(count == 5);
    }

    /* [2] continue 跳转到「最小封闭迭代语句」的循环继续部分（嵌套循环） */
    {
        int outer_count = 0;
        int inner_count = 0;
        for (int i = 0; i < 3; i++) {
            outer_count++;
            for (int j = 0; j < 5; j++) {
                if (j == 2)
                    continue; /* 只影响内层 for 循环，不影响外层 */
                inner_count++;
            }
        }
        /* 外层循环完整执行 3 次 */
        assert(outer_count == 3);
        /* 内层每次执行 j=0,1,3,4 共 4 次，3 轮共 12 次 */
        assert(inner_count == 12);
    }

    /* [2] continue 等价于 goto contin; —— 用 goto 模拟 continue 行为 */
    {
        int count = 0;
        int i = 0;
        while (i < 10) {
            i++;
            if (i % 2 == 0)
                goto contin; /* 等价于 continue */
            count++;
        contin: ; /* 跟随 contin: 标签的是空语句（脚注 138） */
        }
        assert(count == 5);
        assert(i == 10);
    }

    /* [1] continue 作为循环体（单独一条 continue 语句构成整个循环体） */
    {
        int i = 0;
        while (i < 5) {
            i++;
            continue; /* continue 作为循环体的最后一条（也是唯一有意义的）语句 */
        }
        assert(i == 5);
    }

    /* [1] continue 在 if 语句中，但 if 本身在循环体内 —— 合法 */
    {
        int sum = 0;
        for (int i = 1; i <= 10; i++) {
            if (i > 5)
                continue;
            sum += i;
        }
        /* sum = 1+2+3+4+5 = 15 */
        assert(sum == 15);
    }

    /* [2] 三层嵌套循环中 continue 只影响最内层（对应条款中 while{do{for{continue}}} 的示例结构） */
    {
        int outer = 0, middle = 0, inner = 0;
        while (outer < 3) {
            int m = 0;
            do {
                int n = 0;
                for (n = 0; n < 4; n++) {
                    if (n == 1)
                        continue; /* 只跳过内层 for 的 n==1 */
                    inner++;
                }
                middle++;
                m++;
            } while (m < 2);
            outer++;
        }
        /* 外层 3 次，中层 3*2=6 次，内层每轮 n=0,2,3 共 3 次，6 轮共 18 次 */
        assert(outer == 3);
        assert(middle == 6);
        assert(inner == 18);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [1] 违反约束「continue 只能出现在循环体内或作为循环体」：
   continue 出现在函数体顶层，不在任何循环中，gcc -std=c99 应报错 */
void test_continue_at_function_scope(void)
{
    continue;
}

/* [1] 违反约束「continue 只能出现在循环体内或作为循环体」：
   continue 出现在 if 语句中且不在循环内，gcc -std=c99 应报错 */
void test_continue_in_if_no_loop(int x)
{
    if (x > 0)
        continue;
}

/* [1] 违反约束「continue 只能出现在循环体内或作为循环体」：
   continue 出现在 switch 语句中且不在循环内，gcc -std=c99 应报错 */
void test_continue_in_switch_no_loop(int x)
{
    switch (x) {
        case 1:
            continue;
        default:
            break;
    }
}

/* [1] 违反约束「continue 只能出现在循环体内或作为循环体」：
   continue 出现在裸复合语句中但不在循环内，gcc -std=c99 应报错 */
void test_continue_in_block_no_loop(void)
{
    {
        continue;
    }
}

/* [1] 违反约束「continue 只能出现在循环体内或作为循环体」：
   continue 出现在标号语句后但不在循环内，gcc -std=c99 应报错 */
void test_continue_after_label_no_loop(void)
{
label:
    continue;
}

#endif