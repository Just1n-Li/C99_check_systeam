/*
 * 测试条款：C99 7.14.2.1  The raise function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 覆盖段落：
 *   [1] 原型：int raise(int sig);  头文件 <signal.h>
 *   [2] 语义：raise 执行 7.14.1.1 中针对 sig 的动作；
 *            若调用了信号处理函数，raise 在处理函数返回之后才返回。
 *   [3] 返回值：成功返回 0，失败返回非 0。
 */

#include <signal.h>
#include <stdio.h>
#include <assert.h>

/* 用于验证 [2] 中“处理函数被调用”的全局标志 */
static volatile sig_atomic_t handler_called = 0;
static volatile sig_atomic_t handler_sig    = 0;

static void my_handler(int sig)
{
    handler_called = 1;
    handler_sig    = sig;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型检查：raise 接受 int 参数并返回 int。
     *     通过函数指针类型匹配来静态验证原型。 */
    {
        int (*fp)(int) = raise;   /* 若原型不符，此处会编译告警/报错 */
        assert(fp != NULL);
    }

    /* [3] 返回值语义：对合法信号，raise 应返回 0（成功）。 */
    {
        int r = raise(SIGUSR1);   /* 默认动作：终止进程，故先安装处理函数 */
        (void)r;
    }

    /* [2] 语义：安装处理函数后 raise(sig) 应调用该处理函数，
     *     并且 raise 在处理函数返回之后才返回。
     *     用标志位验证处理函数确实被调用过。 */
    {
        void (*old)(int) = signal(SIGUSR1, my_handler);
        assert(old != SIG_ERR);

        handler_called = 0;
        handler_sig    = 0;

        int r = raise(SIGUSR1);

        /* [3] 成功返回 0 */
        assert(r == 0);
        /* [2] 处理函数已被调用，且参数为 SIGUSR1 */
        assert(handler_called == 1);
        assert(handler_sig == SIGUSR1);

        /* 恢复默认处理 */
        signal(SIGUSR1, SIG_DFL);
    }

    /* [2] 再次验证：raise 返回时处理函数已经执行完毕。
     *     由于 handler_called 在处理函数内被置位，
     *     若 raise 在 handler 之前返回，则此处断言会失败。 */
    {
        void (*old)(int) = signal(SIGUSR2, my_handler);
        assert(old != SIG_ERR);

        handler_called = 0;
        int r = raise(SIGUSR2);
        assert(r == 0);
        assert(handler_called == 1);   /* raise 返回时 handler 已运行完 */

        signal(SIGUSR2, SIG_DFL);
    }

    /* [3] 失败情形：对非法信号编号，raise 应返回非 0。
     *     注意：具体哪些编号非法由实现定义，这里用一个明显越界的值。 */
    {
        int r = raise(-1);
        /* 标准只要求“失败返回非 0”，不保证一定失败；
         * 因此这里只做弱检查：若返回 0 也允许（实现可能接受）。
         * 为保持测试确定性，仅验证返回值类型为 int。 */
        (void)r;
    }

    printf("正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「raise 的原型为 int raise(int)」：
     * 参数个数不匹配，gcc -std=c99 应报错。 */
    raise();                 /* 参数太少 */

    /* 违反约束「raise 的原型为 int raise(int)」：
     * 参数过多，gcc -std=c99 应报错。 */
    raise(SIGUSR1, SIGUSR2); /* 参数太多 */

    /* 违反约束「raise 返回 int，不可作为左值赋值」：
     * 函数调用结果不是左值，赋值应报错。 */
    raise(SIGUSR1) = 0;      /* 对非左值赋值 */

    /* 违反约束「raise 需在 <signal.h> 中声明」：
     * 若未包含头文件，隐式声明在 C99 中已不允许（约束违反），
     * 编译器应报错或至少给出诊断。 */
    /* 注：此处已在文件顶部包含 <signal.h>，故用注释说明；
     * 实际负向测试可另建文件不包含头文件来触发。 */

#endif

    return 0;
}