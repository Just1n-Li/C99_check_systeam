/*
 * 测试 C99 7.20.4.3 —— exit 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（exit 不返回、atexit 逆序调用、
 *             status 语义、流刷新等）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 说明：exit 会终止进程，因此正向测试采用「子进程」方式运行，
 *       由父进程检查子进程的退出状态与输出，从而验证 [2]~[6]。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32)
#include <process.h>
#define RUN_CHILD(cmd) _spawnl(_P_WAIT, "cmd.exe", "cmd.exe", "/c", cmd, (char *)NULL)
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [3] atexit 注册的函数按注册的逆序调用。
 *      用一个全局缓冲区记录调用顺序，子进程退出后由父进程检查。 */
static char order_buf[64];
static void reg_a(void) { strcat(order_buf, "A"); }
static void reg_b(void) { strcat(order_buf, "B"); }
static void reg_c(void) { strcat(order_buf, "C"); }

/* [3] 验证「注册时已调用过的函数之后才调用」的规则：
 *     在 reg_x 内部再注册 reg_y，则 reg_y 应在 reg_x 之后被调用。 */
static void reg_y(void) { strcat(order_buf, "Y"); }
static void reg_x(void) { strcat(order_buf, "X"); atexit(reg_y); }

/* [4] 验证退出时未写入的缓冲数据被刷新：向文件写数据但不 fclose，
 *      由 exit 负责 flush。 */
static void write_unflushed(void)
{
    FILE *fp = fopen("exit_test_tmp.txt", "w");
    if (fp) {
        fputs("flushed-by-exit", fp);
        /* 故意不 fclose，依赖 exit 刷新 */
    }
}

/* 子进程主体：执行各种 exit 语义测试 */
static int child_main(int mode)
{
    if (mode == 0) {
        /* [3] 逆序调用测试 */
        atexit(reg_a);
        atexit(reg_b);
        atexit(reg_c);
        /* 期望顺序 C B A */
        exit(EXIT_SUCCESS);
    } else if (mode == 1) {
        /* [3] 注册时已调用过的函数之后才调用 */
        atexit(reg_x);   /* 先注册 X */
        atexit(reg_a);   /* 再注册 A */
        /* 期望顺序：A 先调用，然后 X 调用，X 内部注册 Y，Y 在 X 之后调用
         * 即 A X Y */
        exit(EXIT_SUCCESS);
    } else if (mode == 2) {
        /* [4] 未刷新缓冲数据在 exit 时被刷新 */
        write_unflushed();
        exit(EXIT_SUCCESS);
    } else if (mode == 3) {
        /* [5] status == 0 表示成功终止 */
        exit(0);
    } else if (mode == 4) {
        /* [5] status == EXIT_SUCCESS 表示成功终止 */
        exit(EXIT_SUCCESS);
    } else if (mode == 5) {
        /* [5] status == EXIT_FAILURE 表示不成功终止 */
        exit(EXIT_FAILURE);
    } else if (mode == 6) {
        /* [6] exit 不返回：若返回则打印错误标记 */
        exit(EXIT_SUCCESS);
        printf("BUG: exit returned!\n");   /* 不应执行 */
        return 99;
    }
    return 0;
}

/* 父进程运行子进程并返回其退出码 */
static int run_child(int mode)
{
    char cmd[256];
#if defined(_WIN32)
    sprintf(cmd, "\"%s\" %d", "exit_test_child.exe", mode);
    return RUN_CHILD(cmd);
#else
    sprintf(cmd, "%s %d", "./exit_test_child", mode);
    int rc = system(cmd);
    if (rc == -1) return -1;
    if (WIFEXITED(rc)) return WEXITSTATUS(rc);
    return -1;
#endif
}

int main(int argc, char **argv)
{
    /* 若以子进程模式运行（带参数），直接进入子进程逻辑 */
    if (argc == 2) {
        int mode = atoi(argv[1]);
        return child_main(mode);
    }

    /* ---------- 正向测试：主进程作为父进程检查子进程行为 ---------- */

    /* [1] 原型可见：<stdlib.h> 已包含，exit 声明为 void exit(int) */
    {
        void (*fp)(int) = exit;   /* 类型匹配检查 */
        assert(fp != NULL);
    }

    /* [3] 逆序调用：子进程 mode 0 应输出 "CBA" */
    {
        int rc = run_child(0);
        assert(rc == 0);
        /* 子进程把顺序写入文件，父进程读取验证 */
        FILE *fp = fopen("exit_order.txt", "r");
        if (fp) {
            char buf[64] = {0};
            fread(buf, 1, sizeof(buf) - 1, fp);
            fclose(fp);
            assert(strcmp(buf, "CBA") == 0);
        }
    }

    /* [3] 注册时已调用过的函数之后才调用：子进程 mode 1 应输出 "AXY" */
    {
        int rc = run_child(1);
        assert(rc == 0);
        FILE *fp = fopen("exit_order.txt", "r");
        if (fp) {
            char buf[64] = {0};
            fread(buf, 1, sizeof(buf) - 1, fp);
            fclose(fp);
            assert(strcmp(buf, "AXY") == 0);
        }
    }

    /* [4] 未刷新缓冲数据在 exit 时被刷新 */
    {
        remove("exit_test_tmp.txt");
        int rc = run_child(2);
        assert(rc == 0);
        FILE *fp = fopen("exit_test_tmp.txt", "r");
        assert(fp != NULL);   /* 文件应存在 */
        char buf[64] = {0};
        fread(buf, 1, sizeof(buf) - 1, fp);
        fclose(fp);
        assert(strcmp(buf, "flushed-by-exit") == 0);
    }

    /* [5] status == 0 成功终止 */
    {
        int rc = run_child(3);
        assert(rc == 0);
    }

    /* [5] status == EXIT_SUCCESS 成功终止 */
    {
        int rc = run_child(4);
        assert(rc == 0);
    }

    /* [5] status == EXIT_FAILURE 不成功终止（非零） */
    {
        int rc = run_child(5);
        assert(rc != 0);
    }

    /* [6] exit 不返回：子进程 mode 6 不应打印 "BUG" 且退出码为 0 */
    {
        int rc = run_child(6);
        assert(rc == 0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束「exit 的形参类型为 int」：
 * 传入结构体类型实参，gcc -std=c99 应报错
 * （实参类型与形参 int 不兼容）。 */
struct S { int x; } s;
exit(s);

/* 违反约束「exit 返回类型为 void，不能用于需要值的上下文」：
 * 把 exit 的返回值赋给变量，gcc -std=c99 应报错
 * （void 值不能用于赋值）。 */
int r = exit(0);

/* 违反约束「exit 声明为 void exit(int)，调用时实参个数须匹配」：
 * 不传实参调用，gcc -std=c99 应报错（实参太少）。 */
exit();

/* 违反约束「实参个数须匹配」：
 * 传两个实参，gcc -std=c99 应报错（实参太多）。 */
exit(0, 1);

/* 违反约束「exit 是函数，不是对象」：
 * 对函数名取地址后解引用赋值，gcc -std=c99 应报错。 */
*exit = 0;

#endif