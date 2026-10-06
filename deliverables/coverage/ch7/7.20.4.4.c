/*
 * 测试条款：C99 7.20.4.4  The _Exit function
 *
 * 预期行为：
 *   正向测试：包含 <stdlib.h> 后声明 void _Exit(int status); 可编译；
 *             调用 _Exit 使程序正常终止，控制权返回宿主环境；
 *             atexit 注册的函数与 signal 注册的信号处理器均不被调用；
 *             返回给宿主环境的状态与 exit 的确定方式相同（低 8 位有效）；
 *             _Exit 不返回给调用者。
 *   负向测试：违反约束的代码（如 _Exit 返回值被使用、参数类型错误、
 *             未包含 <stdlib.h> 而隐式声明等）应导致编译报错。
 *
 * 说明：由于 _Exit 会终止进程，正向测试通过 fork() 子进程来观察其行为，
 *       父进程检查子进程的退出状态，从而验证 [2][3] 的语义。
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* 用于验证 atexit 是否被调用的标志（通过文件传递，因为进程会终止） */
static const char *atexit_marker = "/tmp/c99_7_20_4_4_atexit_marker";
static const char *signal_marker = "/tmp/c99_7_20_4_4_signal_marker";

static void atexit_handler(void)
{
    FILE *f = fopen(atexit_marker, "w");
    if (f) { fputs("called", f); fclose(f); }
}

static void sig_handler(int sig)
{
    (void)sig;
    FILE *f = fopen(signal_marker, "w");
    if (f) { fputs("called", f); fclose(f); }
}

/* 辅助：删除标记文件 */
static void remove_markers(void)
{
    remove(atexit_marker);
    remove(signal_marker);
}

/* 辅助：检查标记文件是否存在 */
static int marker_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f) { fclose(f); return 1; }
    return 0;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型声明：包含 <stdlib.h> 后 _Exit 可用，且签名为 void _Exit(int) */
    {
        /* 通过取函数指针类型来静态验证签名 */
        void (*fp)(int) = _Exit;
        assert(fp != NULL);
        /* 验证返回类型为 void：不能把 _Exit 的“返回值”赋给变量（见负向测试） */
    }

    /* [2] _Exit 使程序正常终止，控制权返回宿主环境；
     *     返回状态与 exit 的确定方式相同（低 8 位有效）。
     *     用 fork 子进程验证：子进程调用 _Exit(42)，父进程应看到退出码 42。
     */
    {
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            /* 子进程 */
            _Exit(42);
            /* [3] _Exit 不能返回给调用者：若返回则说明实现错误 */
            _exit(99); /* 不应到达 */
        } else {
            int status = 0;
            pid_t w = waitpid(pid, &status, 0);
            assert(w == pid);
            assert(WIFEXITED(status));
            assert(WEXITSTATUS(status) == 42);
        }
    }

    /* [2] 状态确定方式与 exit 相同：验证低 8 位截断语义。
     *     传入 0x1234，宿主环境应看到 0x34（低 8 位）。
     */
    {
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            _Exit(0x1234);
            _exit(99);
        } else {
            int status = 0;
            pid_t w = waitpid(pid, &status, 0);
            assert(w == pid);
            assert(WIFEXITED(status));
            assert(WEXITSTATUS(status) == (0x1234 & 0xFF));
        }
    }

    /* [2] atexit 注册的函数不被调用 */
    {
        remove_markers();
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            /* 注册 atexit 处理器 */
            int r = atexit(atexit_handler);
            (void)r;
            /* 调用 _Exit，atexit 处理器不应被调用 */
            _Exit(0);
            _exit(99);
        } else {
            int status = 0;
            pid_t w = waitpid(pid, &status, 0);
            assert(w == pid);
            assert(WIFEXITED(status));
            assert(WEXITSTATUS(status) == 0);
            /* 关键断言：atexit 处理器未被调用 */
            assert(!marker_exists(atexit_marker));
        }
    }

    /* [2] signal 注册的信号处理器不被调用。
     *     注意：_Exit 本身不触发信号；这里验证的是 _Exit 不会像 exit 那样
     *     在终止过程中调用已注册的信号处理器。我们注册一个处理器，
     *     然后调用 _Exit，处理器不应被调用。
     */
    {
        remove_markers();
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            /* 注册信号处理器 */
            void (*old)(int) = signal(SIGUSR1, sig_handler);
            (void)old;
            /* 调用 _Exit，信号处理器不应被调用 */
            _Exit(0);
            _exit(99);
        } else {
            int status = 0;
            pid_t w = waitpid(pid, &status, 0);
            assert(w == pid);
            assert(WIFEXITED(status));
            assert(WEXITSTATUS(status) == 0);
            /* 关键断言：信号处理器未被调用 */
            assert(!marker_exists(signal_marker));
        }
    }

    /* [3] _Exit 不能返回给调用者：上面的子进程测试已经隐含验证——
     *     若 _Exit 返回，子进程会执行 _exit(99)，父进程将看到 99 而非 0/42。
     *     这里再显式验证一次：子进程调用 _Exit(7)，父进程看到 7 而非 99。
     */
    {
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            _Exit(7);
            _exit(99); /* 若 _Exit 返回，退出码将是 99 */
        } else {
            int status = 0;
            pid_t w = waitpid(pid, &status, 0);
            assert(w == pid);
            assert(WIFEXITED(status));
            assert(WEXITSTATUS(status) == 7);
        }
    }

    /* [2] 与 exit 的对比：exit 会调用 atexit 处理器，_Exit 不会。
     *     这是一个对照测试，确认我们的测试方法有效。
     */
    {
        remove_markers();
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            int r = atexit(atexit_handler);
            (void)r;
            exit(0); /* exit 应调用 atexit 处理器 */
            _exit(99);
        } else {
            int status = 0;
            pid_t w = waitpid(pid, &status, 0);
            assert(w == pid);
            assert(WIFEXITED(status));
            assert(WEXITSTATUS(status) == 0);
            /* 对照：exit 确实调用了 atexit 处理器 */
            assert(marker_exists(atexit_marker));
        }
    }

    remove_markers();

    printf("C99 7.20.4.4 _Exit: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「_Exit 的返回类型为 void，不能使用其返回值」：
     * _Exit 返回 void，不能把它的“返回值”赋给变量。
     * gcc -std=c99 应报错：void value not ignored as it ought to be。
     */
    {
        int x = _Exit(0);   /* 错误：void 值不能用于初始化 int */
        (void)x;
    }

    /* 违反约束「_Exit 的参数类型为 int」：
     * 传入不兼容的指针类型，应报错。
     * gcc -std=c99 应报错：incompatible type for argument 1。
     */
    {
        char *p = "hello";
        _Exit(p);           /* 错误：参数应为 int，传入了 char* */
    }

    /* 违反约束「_Exit 的参数个数为 1」：
     * 传入两个参数，应报错。
     * gcc -std=c99 应报错：too many arguments to function '_Exit'。
     */
    {
        _Exit(0, 1);        /* 错误：参数过多 */
    }

    /* 违反约束「_Exit 的参数个数为 1」：
     * 不传参数，应报错。
     * gcc -std=c99 应报错：too few arguments to function '_Exit'。
     */
    {
        _Exit();            /* 错误：参数过少 */
    }

    /* 违反约束「_Exit 声明于 <stdlib.h>」：
     * 未包含 <stdlib.h> 时调用 _Exit，在 C99 中隐式声明被移除，
     * 应报错（或至少警告）。这里演示不包含头文件直接调用。
     * 注意：本文件顶部已包含 <stdlib.h>，故此片段仅作示意。
     * gcc -std=c99 应报错：implicit declaration of function '_Exit'。
     */
    {
        /* 假设此处未包含 <stdlib.h> */
        /* _Exit(0); */
    }

    /* 违反约束「_Exit 不能作为左值被赋值」：
     * 函数指示符不是左值，不能赋值。
     * gcc -std=c99 应报错：lvalue required as left operand of assignment。
     */
    {
        _Exit = 0;          /* 错误：函数指示符不是左值 */
    }

    /* 违反约束「_Exit 不能取地址后解引用赋值」：
     * 函数指示符不是左值，*_Exit 也不是左值。
     * gcc -std=c99 应报错：lvalue required as left operand of assignment。
     */
    {
        *_Exit = 0;         /* 错误：解引用函数指示符不是左值 */
    }

#endif /* 负向测试结束 */
}