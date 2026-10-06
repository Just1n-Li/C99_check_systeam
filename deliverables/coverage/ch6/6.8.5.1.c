/*
 * 验证 C99 6.8.5.1 The while statement
 * [1] The evaluation of the controlling expression takes place before each execution of the loop body.
 *
 * 正向测试：验证 while 控制表达式在每次循环体执行前求值，预期运行通过。
 * 负向测试：验证 while 控制表达式的类型约束（应为标量类型），预期编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] Semantics: 控制表达式在第一次循环体执行前求值——若初始为 false，循环体不执行 */
    {
        int x = 0;
        int body_executed = 0;
        while (x > 0) {
            body_executed = 1;
        }
        assert(body_executed == 0);  /* 循环体从未执行 */
    }

    /* [1] Semantics: 控制表达式在每次循环体执行前求值——通过副作用验证求值时机 */
    {
        int count = 0;
        int iterations = 0;
        while (count++ < 3) {
            iterations++;
        }
        /* count++ 在每次循环体执行前求值：
           第1次: count=0<3→true,  count变为1, body执行
           第2次: count=1<3→true,  count变为2, body执行
           第3次: count=2<3→true,  count变为3, body执行
           第4次: count=3<3→false, count变为4, body不执行 */
        assert(count == 4);
        assert(iterations == 3);
    }

    /* [1] Semantics: 控制表达式在每次循环体执行前求值——逗号表达式精确统计求值次数 */
    {
        int i = 0;
        int eval_count = 0;
        while ((eval_count++, i < 3)) {
            i++;
        }
        /* 控制表达式求值 4 次：3 次 true + 1 次 false */
        assert(eval_count == 4);
        assert(i == 3);
    }

    /* [1] Semantics: 控制表达式在每次循环体执行前求值——循环体内修改控制变量 */
    {
        int n = 5;
        int sum = 0;
        while (n > 0) {
            sum += n;
            n--;
        }
        /* 每次进入循环体前都重新求值 n > 0 */
        assert(sum == 15);  /* 5+4+3+2+1 */
        assert(n == 0);
    }

    /* [1] Semantics: 控制表达式在每次循环体执行前求值——空循环体 */
    {
        int i = 0;
        while (i++ < 10)
            ;  /* 空语句：控制表达式仍然在每次"执行"前求值 */
        assert(i == 11);  /* 最后一次 i++=10<10→false, i 变为 11 */
    }

    /* [1] Semantics: 控制表达式在每次循环体执行前求值——break 时不再求值 */
    {
        int i = 0;
        int eval_count = 0;
        while ((eval_count++, i < 100)) {
            if (i == 2)
                break;
            i++;
        }
        /* break 跳出后不再求值控制表达式：求值 3 次（i=0,1,2） */
        assert(eval_count == 3);
        assert(i == 2);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* 违反约束 6.8.5「控制表达式应为标量类型」：结构体不是标量类型，
   不能作为 while 控制表达式，gcc -std=c99 应报错 */
{
    struct S { int x; } s = {1};
    while (s) {   /* error: used type 'struct S' where scalar is required */
        break;
    }
}

/* 违反约束 6.8.5「控制表达式应为标量类型」：函数类型不是标量类型，
   不能作为 while 控制表达式，gcc -std=c99 应报错 */
{
    while (main) {   /* error: function type used as scalar */
        break;
    }
}
#endif