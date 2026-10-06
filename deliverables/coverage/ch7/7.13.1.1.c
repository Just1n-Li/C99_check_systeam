/*
 * 测试 C99 7.13.1.1 —— setjmp 宏
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反 [4] 环境限制的调用上下文，编译器应报错（本文件用 #if 0 隔离，
 *             保证整体仍可编译运行；把片段取出单独编译应报错）。
 *
 * 覆盖段落：
 *   [1] 头文件 <setjmp.h> 与原型 int setjmp(jmp_buf env);
 *   [2] setjmp 把调用环境保存到 jmp_buf 参数中，供 longjmp 使用
 *   [3] 直接调用返回 0；由 longjmp 返回时返回非零值
 *   [4] 环境限制：setjmp 只能出现在 4 种上下文之一
 *   [5] 其他上下文为未定义行为（UB，不作为负向测试）
 */

#include <setjmp.h>
#include <stdio.h>
#include <assert.h>

/* 全局 jmp_buf，供跨函数 longjmp 使用 */
static jmp_buf env_global;

/* 用于验证 [2]：在另一个函数里 longjmp 回 setjmp 保存的环境 */
static void do_longjmp(int val)
{
    longjmp(env_global, val);
}

/* 用于验证 [3]：longjmp 返回值被 setjmp 接收 */
static int test_return_value(void)
{
    int r = setjmp(env_global);   /* [4] 整个表达式语句（可强转 void） */
    if (r == 0) {
        /* [3] 直接调用返回 0 */
        do_longjmp(42);           /* 触发 longjmp，控制流回到上面的 setjmp */
        /* 不会到达这里 */
        return -1;
    }
    /* [3] 由 longjmp 返回时返回非零值 */
    return r;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 头文件与原型：能声明 jmp_buf 并调用 setjmp */
    jmp_buf env;
    int r;

    /* [2] setjmp 保存调用环境到 jmp_buf 参数中 */
    /* [4] 上下文①：整个表达式语句（可强转 void） */
    r = setjmp(env);
    assert(r == 0);               /* [3] 直接调用返回 0 */

    /* [2] 保存的环境可被 longjmp 使用：longjmp 后回到此处 */
    if (r == 0) {
        longjmp(env, 1);          /* 使用刚才保存的环境 */
        assert(0 && "longjmp 不应返回");
    }
    assert(r == 1);               /* [3] 由 longjmp 返回，值为传入的非零值 */

    /* [3] longjmp 传入不同的非零值，setjmp 返回该值 */
    r = setjmp(env);
    if (r == 0) {
        longjmp(env, 7);
    }
    assert(r == 7);

    /* [2][3] 跨函数 longjmp：环境在 main 中保存，在 do_longjmp 中恢复 */
    assert(test_return_value() == 42);

    /* [4] 上下文②：作为选择语句的整个控制表达式 */
    if (setjmp(env) == 0) {
        longjmp(env, 2);
    }
    /* 到达此处说明 setjmp 返回非零 */

    /* [4] 上下文②：与整数常量表达式做关系/相等比较，且整体为选择语句控制表达式 */
    if (setjmp(env) != 0) {
        /* 第一次不会进入；下面触发 longjmp 后进入 */
    } else {
        longjmp(env, 3);
    }

    /* [4] 上下文③：一元 ! 运算符的操作数，整体为选择语句控制表达式 */
    if (!setjmp(env)) {
        longjmp(env, 4);
    }

    /* [4] 上下文①：作为迭代语句的整个控制表达式 */
    {
        int count = 0;
        while (setjmp(env) == 0) {
            count++;
            if (count < 3) {
                longjmp(env, 5);  /* 回到 while 控制表达式 */
            }
            break;
        }
        assert(count == 3);
    }

    /* [4] 上下文①：for 语句的整个控制表达式 */
    {
        int n = 0;
        for (; setjmp(env) == 0; ) {
            n++;
            if (n < 2) {
                longjmp(env, 6);
            }
            break;
        }
        assert(n == 2);
    }

    /* [4] 上下文④：整个表达式语句，可强转 void */
    (void)setjmp(env);

    printf("正向测试全部通过：setjmp/longjmp 语义符合 C99 7.13.1.1\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
    /*
     * 违反 [4] 环境限制：setjmp 出现在赋值语句的右操作数中，
     * 不是「整个表达式语句」，也不是选择/迭代语句的整个控制表达式。
     * 期望：gcc -std=c99 -pedantic-errors 报错
     *   （如 "the address of 'env' will always evaluate as 'true'" 之外，
     *     GCC 会给出 setjmp 使用位置错误相关诊断）。
     */
    {
        jmp_buf env2;
        int x;
        x = setjmp(env2);   /* 违反 [4]：不是允许的上下文 */
    }

    /*
     * 违反 [4] 环境限制：setjmp 作为函数实参的一部分，
     * 不是整个表达式语句，也不是控制表达式。
     * 期望：编译报错。
     */
    {
        jmp_buf env3;
        printf("%d\n", setjmp(env3));   /* 违反 [4] */
    }

    /*
     * 违反 [4] 环境限制：setjmp 作为二元 + 的操作数，
     * 且整体不是选择/迭代语句的整个控制表达式。
     * 期望：编译报错。
     */
    {
        jmp_buf env4;
        int y = setjmp(env4) + 1;   /* 违反 [4] */
    }

    /*
     * 违反 [4] 环境限制：setjmp 出现在 return 语句中，
     * 不是允许的上下文。
     * 期望：编译报错。
     */
    {
        jmp_buf env5;
        return setjmp(env5);   /* 违反 [4] */
    }
#endif

    return 0;
}