/*
 * 验证 C99 6.8.5.2 The do statement
 * 预期行为：正向测试运行通过，负向测试编译报错（语法错误）。
 */
#include <stdio.h>
#include <assert.h>

int main(void) {
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 控制表达式在循环体每次执行之后求值 */
    {
        /* 测试1：循环体至少执行一次，即使控制表达式初始为假。
           这验证了控制表达式不是在循环体之前求值。 */
        int i = 10;
        do {
            i++;
        } while (i < 10);
        assert(i == 11); /* 循环体执行一次后，i=11，控制表达式 11<10 为假，结束 */
    }

    {
        /* 测试2：循环体执行多次，每次执行后求值控制表达式 */
        int count = 0;
        do {
            count++;
        } while (count < 5);
        assert(count == 5); /* 执行5次后，count=5，控制表达式 5<5 为假，结束 */
    }

    {
        /* 测试3：在循环体中修改控制变量，验证控制表达式使用修改后的值（即之后求值） */
        int flag = 0;
        int loop_count = 0;
        do {
            loop_count++;
            flag = 1; /* 在循环体中修改 flag */
        } while (flag == 0 && loop_count < 10); 
        /* 如果控制表达式在循环体之前求值，flag==0 为真，会继续循环（死循环或达到 loop_count 上限）。
           因为在之后求值，flag==0 为假，循环立即结束。 */
        assert(loop_count == 1);
        assert(flag == 1);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /* 6.8.5.2 条款本身未定义额外约束，但 do-while 语句语法要求 while(expression); 结尾必须有分号。
       缺少分号属于语法错误，编译器应报错。 */
    int i = 0;
    do {
        i++;
    } while (i < 3) /* 缺少分号，期望编译器报错：expected ';' after 'while' condition */
#endif

    return 0;
}