/*
 * 验证 C99 条款 6.8 (Statements and blocks)
 * 预期行为：
 * - 正向测试：能编译并运行通过，所有 assert 成立。
 * - 负向测试：违反语法规则，应编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* [4] 辅助函数：验证 return 语句的表达式是完整表达式 */
static int test_return_full_expr(void) {
    int r = 5;
    return (r = 10, r + 1); /* 逗号表达式的副作用在 return 前完成 */
}

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 语法：测试各类语句的合法组合 */
    {
        /* labeled-statement 包含 compound-statement */
        label_block: {
            int expr_stmt_val = 1; /* expression-statement (C99允许声明作为语句) */
            ; /* null statement */
            
            /* selection-statement */
            if (expr_stmt_val) {
                /* empty */
            } else {
                /* empty */
            }
            
            /* iteration-statement */
            while (0) {
                /* empty */
            }
            
            /* jump-statement */
            goto end_label_block;
        }
        end_label_block: ;
    }

    /* [2] 语义：语句按顺序执行 */
    {
        int seq = 0;
        seq = seq + 1;
        seq = seq + 2;
        seq = seq + 3;
        assert(seq == 6); /* 验证顺序累加 */
    }

    /* [3] 语义：块内自动变量初始化器与 VLA 每次到达声明时按顺序求值 */
    {
        int order = 0;
        for (int i = 0; i < 2; i++) {
            /* 每次循环到达此处时，初始化器被求值 */
            int val = (order += 10);
            assert(val == (i + 1) * 10);
            
            /* [3] 变长数组声明符在每次到达声明时求值 */
            int vla_size = val / 5;
            int vla[vla_size];
            vla[0] = i;
            assert(vla[0] == i);
        }
        assert(order == 20);

        /* [3] 无初始化器时存储不确定值（不读取，仅验证分配空间） */
        int uninit;
        uninit = 100; /* 赋予确定值后再读取 */
        assert(uninit == 100);
    }

    /* [4] 语义：完整表达式末尾是序列点，副作用在进入下一语句前完成 */
    {
        /* [4] 完整表达式：初始化器 */
        int init_val = 5;
        int init_test = (init_val = 10, init_val + 1);
        assert(init_test == 11 && init_val == 10); /* 序列点保证了 init_val 已更新 */

        /* [4] 完整表达式：表达式语句 */
        init_val = 20;
        assert(init_val == 20);

        /* [4] 完整表达式：if 控制表达式 */
        if (init_val = 25) { /* 赋值副作用在 if 体执行前完成 */
            assert(init_val == 25);
        }

        /* [4] 完整表达式：while 控制表达式 */
        int while_cnt = 0;
        while (init_val = (while_cnt < 3)) {
            while_cnt++;
        }
        assert(while_cnt == 3 && init_val == 0);

        /* [4] 完整表达式：for 语句的各个表达式 */
        int for_sum = 0;
        int for_init = 0;
        for (for_init = 0; for_init < 3; for_init++) {
            for_sum += for_init;
        }
        assert(for_sum == 3 && for_init == 3);

        /* [4] 完整表达式：return 语句的表达式 */
        assert(test_return_full_expr() == 11);
    }

    printf("All positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    #if 0
    /* 
     * 注意：6.8 条款本身没有 Constraints 段落，但 Syntax [1] 规定了 statement 的合法形式。
     * 以下负向测试针对 Syntax [1] 的语法约束：在需要 statement 的位置使用了 declaration。
     */

    /* 违反 Syntax [1]：if 语句的子语句必须是 statement，不能是 declaration */
    if (1)
        int x = 0; /* 语法错误：期望 statement，但遇到了 declaration */

    /* 违反 Syntax [1]：label 后必须是 statement，不能是 declaration */
    my_label:
        int y = 0; /* 语法错误：期望 statement，但遇到了 declaration */

    #endif
}