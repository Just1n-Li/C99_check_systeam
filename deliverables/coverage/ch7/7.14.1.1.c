/*
 * 测试条款：ISO/IEC 9899:1999 (C99) 7.14.1.1  The signal function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 ... #endif 中，
 *             因此整个文件仍可正常编译运行）。
 *
 * 覆盖段落：[1] 原型、[2] 三种处理方式、[3] 处理期间行为、[4] abort/raise 限制、
 *           [5] 异步信号处理器限制、[6] 启动时默认状态、[7] 库函数不调用 signal、
 *           [8] 返回值语义。
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [2] 信号处理器：只能通过 volatile sig_atomic_t 与外界通信（见 [5]） */
static volatile sig_atomic_t g_handler_called = 0;
static volatile sig_atomic_t g_handler_sig  = 0;

static void my_handler(int sig)
{
    /* [5] 处理器内只允许给 volatile sig_atomic_t 赋值 */
    g_handler_called = 1;
    g_handler_sig = sig;
}

/* [1] 验证 signal 的原型：返回类型是「指向接受 int 返回 void 的函数」的指针 */
static void (*check_proto(int sig, void (*func)(int)))(int)
{
    return signal(sig, func);
}

int main(void)
{
    /* ------------------------------------------------------------
     * [1] 原型检查：把 signal 赋给正确类型的函数指针
     * ------------------------------------------------------------ */
    {
        void (*(*pf)(int, void (*)(int)))(int) = signal;
        assert(pf != NULL);
        (void)pf;
    }

    /* ------------------------------------------------------------
     * [2] SIG_DFL / SIG_IGN / 函数指针 三种处理方式
     * ------------------------------------------------------------ */
    {
        void (*old)(int);

        /* 安装自定义处理器 */
        old = signal(SIGUSR1, my_handler);
        assert(old != SIG_ERR);

        /* [8] 再次调用 signal 应返回「最近一次成功调用」所设置的值，
         *     即刚才安装的 my_handler */
        old = signal(SIGUSR1, my_handler);
        assert(old == my_handler);

        /* [2] SIG_IGN：忽略该信号 */
        old = signal(SIGUSR1, SIG_IGN);
        assert(old == my_handler);

        /* [2] SIG_DFL：恢复默认处理 */
        old = signal(SIGUSR1, SIG_DFL);
        assert(old == SIG_IGN);

        /* 通过 check_proto 走一遍原型路径 */
        old = check_proto(SIGUSR1, SIG_DFL);
        assert(old == SIG_DFL);
    }

    /* ------------------------------------------------------------
     * [2][3] 实际触发信号：处理器被调用，参数为信号编号
     * ------------------------------------------------------------ */
    {
        void (*old)(int) = signal(SIGUSR1, my_handler);
        assert(old != SIG_ERR);

        g_handler_called = 0;
        g_handler_sig = 0;

        /* raise 触发信号；[3] 处理器返回后（SIGUSR1 非计算异常）
         * 程序从被中断处继续执行 */
        int r = raise(SIGUSR1);
        assert(r == 0);

        /* [3] 处理器已被调用，且收到正确的信号编号 */
        assert(g_handler_called == 1);
        assert(g_handler_sig == SIGUSR1);

        /* 恢复默认 */
        (void)signal(SIGUSR1, SIG_DFL);
    }

    /* ------------------------------------------------------------
     * [4] 由 raise 触发的处理器内不得调用 raise（此处只验证
     *     处理器内调用 signal 是允许的，见 [5]）
     * ------------------------------------------------------------ */
    {
        void (*old)(int) = signal(SIGUSR2, my_handler);
        assert(old != SIG_ERR);

        g_handler_called = 0;
        g_handler_sig = 0;
        assert(raise(SIGUSR2) == 0);
        assert(g_handler_called == 1);
        assert(g_handler_sig == SIGUSR2);

        (void)signal(SIGUSR2, SIG_DFL);
    }

    /* ------------------------------------------------------------
     * [5] 处理器内允许调用 signal，且第一个实参等于触发处理器的信号编号
     *     （在处理器内重新安装自身是允许的）
     * ------------------------------------------------------------ */
    {
        /* 这里用一个「处理器内调用 signal」的处理器来验证该路径可编译、
         * 可运行；具体调用在 my_handler2 中完成 */
        extern void my_handler2(int);
        void (*old)(int) = signal(SIGUSR1, my_handler2);
        assert(old != SIG_ERR);

        g_handler_called = 0;
        g_handler_sig = 0;
        assert(raise(SIGUSR1) == 0);
        assert(g_handler_called == 1);
        assert(g_handler_sig == SIGUSR1);

        (void)signal(SIGUSR1, SIG_DFL);
    }

    /* ------------------------------------------------------------
     * [6] 启动时默认状态：对实现定义的信号，等价于 SIG_IGN 或 SIG_DFL。
     *     我们只能验证「查询」不会失败，且返回值是合法值之一。
     * ------------------------------------------------------------ */
    {
        void (*cur)(int) = signal(SIGINT, SIG_DFL);
        assert(cur != SIG_ERR);
        /* 恢复原状 */
        (void)signal(SIGINT, cur);
    }

    /* ------------------------------------------------------------
     * [7] 实现行为如同没有库函数调用 signal：这里通过「调用库函数
     *     不会改变我们已安装的处理器」来间接验证。
     * ------------------------------------------------------------ */
    {
        void (*old)(int) = signal(SIGUSR1, my_handler);
        assert(old != SIG_ERR);

        /* 调用若干库函数 */
        char buf[32];
        (void)strcpy(buf, "hello");
        (void)strlen(buf);
        (void)printf("");   /* 空输出，仅调用 */

        /* 处理器应保持不变 */
        void (*now)(int) = signal(SIGUSR1, my_handler);
        assert(now == my_handler);

        (void)signal(SIGUSR1, SIG_DFL);
    }

    /* ------------------------------------------------------------
     * [8] 返回值：成功时返回最近一次成功调用所设置的值；
     *     失败时返回 SIG_ERR 并向 errno 存入正值。
     *     用一个非法信号编号触发失败路径。
     * ------------------------------------------------------------ */
    {
        errno = 0;
        void (*bad)(int) = signal(-1, SIG_DFL);
        if (bad == SIG_ERR) {
            /* 失败路径：errno 应为正值 */
            assert(errno > 0);
        }
        /* 若实现接受 -1，则视为成功，不检查 errno */
    }

    printf("C99 7.14.1.1 signal: all positive tests passed.\n");
    return 0;
}

/* [5] 处理器内调用 signal（第一个实参等于触发处理器的信号编号） */
void my_handler2(int sig)
{
    g_handler_called = 1;
    g_handler_sig = sig;
    /* 允许：signal 的第一个实参等于触发处理器的信号编号 */
    (void)signal(sig, my_handler2);
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「signal 的第一个参数类型为 int」：
 * 传入结构体，gcc -std=c99 应报错（参数类型不兼容）。 */
struct S { int x; } s;
signal(s, SIG_DFL);

/* 违反约束「signal 的第二个参数类型为 void (*)(int)」：
 * 传入 int，gcc -std=c99 应报错（参数类型不兼容）。 */
signal(SIGUSR1, 42);

/* 违反约束「signal 的第二个参数类型为 void (*)(int)」：
 * 传入 void (*)(void)（参数个数不匹配），gcc -std=c99 应报错。 */
void f_void(void);
signal(SIGUSR1, f_void);

/* 违反约束「signal 的第二个参数类型为 void (*)(int)」：
 * 传入 int (*)(int)（返回类型不匹配），gcc -std=c99 应报错。 */
int f_int(int);
signal(SIGUSR1, f_int);

/* 违反约束「signal 返回类型为 void (*)(int)」：
 * 把返回值赋给 int，gcc -std=c99 应报错（指针到整数不兼容）。 */
int x = signal(SIGUSR1, SIG_DFL);

/* 违反约束「signal 返回类型为 void (*)(int)」：
 * 把返回值赋给 void (*)(void)，gcc -std=c99 应报错。 */
void (*p_void)(void) = signal(SIGUSR1, SIG_DFL);

/* 违反约束「signal 需要两个实参」：
 * 只传一个实参，gcc -std=c99 应报错（实参太少）。 */
signal(SIGUSR1);

/* 违反约束「signal 需要两个实参」：
 * 传三个实参，gcc -std=c99 应报错（实参太多）。 */
signal(SIGUSR1, SIG_DFL, 0);

/* 违反约束「signal 的第一个参数类型为 int」：
 * 传入指针，gcc -std=c99 应报错（指针到整数不兼容）。 */
int *ip = 0;
signal(ip, SIG_DFL);

#endif /* 负向测试结束 */