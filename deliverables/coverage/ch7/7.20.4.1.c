/*
 * 测试 C99 7.20.4.1 —— abort 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *             通过 fork 子进程调用 abort()，父进程用 waitpid 检查子进程
 *             是否因 SIGABRT 而异常终止（WIFSIGNALED 且 WTERMSIG==SIGABRT），
 *             以此验证 [2] 中“abort 通过 raise(SIGABRT) 向宿主环境报告
 *             不成功终止”以及 [3] 中“abort 不返回给调用者”。
 *   负向测试：违反约束的代码应编译报错（见文件末尾 #if 0 块）。
 *
 * 说明：abort 的 Synopsis [1] 声明为 void abort(void)，无参数、无返回值。
 *       本测试使用 POSIX 的 fork/waitpid 来观察进程终止状态，这是验证
 *       “不返回给调用者”与“SIGABRT”语义的标准手段。
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：abort 声明为 void abort(void)。
 *     验证：可以取函数地址、可以以无参形式调用（在子进程中调用）。
 *     这里仅做编译期/链接期检查，不真正在主进程调用。 */
static void (*abort_ptr)(void) = abort;

/* [2] abort 导致异常程序终止，并通过 raise(SIGABRT) 报告不成功终止。
 *     验证：子进程调用 abort() 后，父进程观察到子进程被 SIGABRT 信号终止。 */
static void test_abort_raises_sigabrt(void)
{
    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        /* 子进程：调用 abort，正常情况下不会返回。 */
        abort();
        /* 如果 abort 返回了（违反 [3]），用 _exit 报告异常。 */
        _exit(42);
    } else {
        int status = 0;
        pid_t r = waitpid(pid, &status, 0);
        assert(r == pid);

        /* [2] 异常终止：子进程应被信号终止，且信号为 SIGABRT。 */
        assert(WIFSIGNALED(status));
        assert(WTERMSIG(status) == SIGABRT);

        /* [3] abort 不返回给调用者：子进程不应以正常退出码 42 结束。 */
        assert(!(WIFEXITED(status) && WEXITSTATUS(status) == 42));
    }
}

/* [2] 若 SIGABRT 被捕获且处理函数不返回，则 abort 不导致进程终止。
 *     验证：安装一个不返回的 SIGABRT 处理函数（内部 _exit），
 *           子进程调用 abort 后，父进程观察到子进程以处理函数指定的
 *           退出码正常退出，而不是被 SIGABRT 终止。 */
static void sigabrt_handler(int sig)
{
    (void)sig;
    /* 处理函数不返回：直接 _exit，避免 abort 继续终止进程。 */
    _exit(7);
}

static void test_abort_with_caught_sigabrt(void)
{
    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        /* 安装不返回的 SIGABRT 处理函数。 */
        if (signal(SIGABRT, sigabrt_handler) == SIG_ERR) {
            _exit(99);
        }
        abort();          /* 处理函数不返回，进程由 _exit(7) 结束 */
        _exit(42);        /* 不应到达 */
    } else {
        int status = 0;
        pid_t r = waitpid(pid, &status, 0);
        assert(r == pid);

        /* 处理函数不返回，因此进程以 _exit(7) 正常退出。 */
        assert(WIFEXITED(status));
        assert(WEXITSTATUS(status) == 7);
    }
}

/* [1] 函数指针类型检查：abort 的类型为 void (*)(void)。 */
static void test_abort_signature(void)
{
    assert(abort_ptr != NULL);
    /* 赋值给正确类型的函数指针，编译期验证签名。 */
    void (*p)(void) = abort_ptr;
    assert(p == abort);
}

int main(void)
{
    /* [1] Synopsis */
    test_abort_signature();

    /* [2] 异常终止 + raise(SIGABRT) */
    test_abort_raises_sigabrt();

    /* [2] SIGABRT 被捕获且处理函数不返回的情形 */
    test_abort_with_caught_sigabrt();

    /* [3] abort 不返回给调用者（由上面子进程测试间接验证） */

    printf("C99 7.20.4.1 abort: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「abort 的 Synopsis 为 void abort(void)」：
 * abort 不接受参数，传入实参应编译报错。
 * 期望：gcc -std=c99 报 “too many arguments to function 'abort'”。 */
void bad_call_with_arg(void)
{
    abort(1);
}

/* 违反约束「abort 返回类型为 void」：
 * 不能把 void 表达式用作赋值右值，也不能对其取“值”。
 * 期望：gcc -std=c99 报 “void value not ignored as it ought to be”。 */
void bad_use_return_value(void)
{
    int x = abort();   /* void 不能赋给 int */
    (void)x;
}

/* 违反约束「abort 返回类型为 void」：
 * 不能对 void 表达式做算术运算。
 * 期望：gcc -std=c99 报 “invalid use of void expression”。 */
void bad_arithmetic_on_void(void)
{
    int y = abort() + 1;
    (void)y;
}

/* 违反约束「abort 的 Synopsis 为 void abort(void)」：
 * 不能把 abort 当作非函数对象使用（例如取它的“值”参与运算）。
 * 期望：gcc -std=c99 报 “invalid use of void expression” 或类似错误。 */
void bad_use_as_object(void)
{
    int z = (int)abort;
    (void)z;
}

#endif /* 负向测试结束 */