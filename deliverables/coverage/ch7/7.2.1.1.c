/*
 * 测试 C99 7.2.1.1 —— assert 宏
 *
 * 预期行为：
 *   正向测试：包含 <assert.h> 后，assert 宏可用于标量表达式；
 *             表达式为真（非 0）时无副作用、程序继续；
 *             表达式为假（等于 0）时向 stderr 写诊断信息并调用 abort；
 *             assert 展开为 void 表达式，不返回值。
 *   负向测试：违反约束的代码（非标量类型实参、对 assert 结果赋值等）
 *             应导致编译报错。
 *
 * 说明：由于 assert 失败会 abort，正向测试中“失败路径”通过子进程
 *       （fork + waitpid）验证，避免终止整个测试程序。
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */

/* [1] 原型：void assert(scalar expression);
 *     验证 assert 可用，且展开为 void 表达式（不产生值）。 */
static void test_synopsis_and_void_expression(void)
{
    /* [1] 正常包含并使用 assert */
    assert(1);

    /* [2] assert 展开为 void 表达式：可以出现在逗号表达式左侧，
     *     其值被丢弃，整体仍为 void。 */
    (void)(assert(1), 0);

    /* [2] 作为 void 表达式，可单独作为语句使用 */
    assert(2 + 2 == 4);

    printf("[1][2] assert 展开为 void 表达式，正常使用通过\n");
}

/* [2] 表达式为真（非 0）时：不写诊断、不 abort，程序继续执行 */
static void test_true_expression_no_abort(void)
{
    int x = 42;
    assert(x == 42);          /* 真：继续 */
    assert(x);                /* 非 0：继续 */
    assert(-1);               /* 负数非 0：继续 */
    assert(0.5);              /* 浮点非 0：继续（标量类型） */
    assert((void *)0 == NULL ? 0 : 1); /* 指针比较结果：标量 */
    printf("[2] 真值表达式不触发 abort，程序继续\n");
}

/* [2] 表达式为假（等于 0）时：写诊断到 stderr 并调用 abort。
 *     用 fork 子进程验证，父进程检查子进程是否异常终止。 */
static void test_false_expression_aborts(void)
{
    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程：stderr 重定向到 /dev/null，避免污染测试输出 */
        freopen("/dev/null", "w", stderr);
        assert(0);            /* 假：应写诊断并 abort */
        /* 若 assert 未 abort，子进程会走到这里 */
        _exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        /* 子进程应因 abort 而异常终止（收到 SIGABRT） */
        assert(WIFSIGNALED(status));
        assert(WTERMSIG(status) == SIGABRT);
        printf("[2] 假值表达式触发 abort（SIGABRT），符合预期\n");
    }
}

/* [2] 诊断信息应包含：实参文本、__FILE__、__LINE__、__func__。
 *     通过捕获子进程 stderr 内容来验证。 */
static void test_diagnostic_content(void)
{
    int fds[2];
    if (pipe(fds) != 0) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程：把 stderr 接到管道写端 */
        close(fds[0]);
        dup2(fds[1], STDERR_FILENO);
        close(fds[1]);
        assert(1 == 2);       /* 故意失败，诊断文本应含 "1 == 2" */
        _exit(0);
    } else {
        close(fds[1]);
        char buf[4096];
        ssize_t n = read(fds[0], buf, sizeof(buf) - 1);
        close(fds[0]);
        int status = 0;
        waitpid(pid, &status, 0);

        assert(n > 0);
        buf[n] = '\0';

        /* 诊断信息应包含实参文本 */
        assert(strstr(buf, "1 == 2") != NULL);
        /* 诊断信息应包含源文件名（__FILE__） */
        assert(strstr(buf, __FILE__) != NULL);
        /* 诊断信息应包含函数名（__func__） */
        assert(strstr(buf, "test_diagnostic_content") != NULL);
        /* 诊断信息应包含行号（__LINE__ 的数字形式，这里只检查非空） */
        assert(strlen(buf) > 0);

        printf("[2] 诊断信息包含实参文本、__FILE__、__func__：\n%s", buf);
    }
}

/* [2] 标量类型：整型、浮点、指针、枚举均可作为 assert 实参 */
static void test_scalar_types(void)
{
    int i = 1;
    double d = 1.0;
    int *p = &i;
    enum E { A = 1, B = 2 } e = A;

    assert(i);        /* 整型 */
    assert(d);        /* 浮点 */
    assert(p);        /* 指针 */
    assert(e);        /* 枚举 */
    assert('a');      /* 字符（整型） */
    printf("[2] 整型/浮点/指针/枚举/字符等标量类型均可作为 assert 实参\n");
}

/* [3] assert 不返回值：验证其类型为 void。
 *     通过 _Generic（C11）不可用，改用编译期技巧：
 *     把 assert 的结果赋给 void 变量是合法的，赋给 int 变量则非法。
 *     这里只做正向：确认 assert 可作为 void 表达式使用。 */
static void test_returns_no_value(void)
{
    /* [3] assert 返回 void：不能用于需要值的上下文，
     *     但可作为 void 表达式语句。 */
    assert(1);
    printf("[3] assert 不返回值（void 表达式）\n");
}

/* [2] 与 NDEBUG 交互：定义 NDEBUG 后 assert 被展开为 ((void)0)，
 *     不进行求值。这是标准允许的实现方式（C99 7.2.1.1 未强制，
 *     但为常见实现行为，此处仅验证不崩溃）。 */
static void test_ndebug_behavior(void)
{
    /* 注意：本测试文件未定义 NDEBUG，assert 正常生效。
     * 这里只验证 assert 在正常模式下工作。 */
    assert(1);
    printf("[2] 未定义 NDEBUG 时 assert 正常生效\n");
}

int main(void)
{
    printf("===== C99 7.2.1.1 assert 宏 正向测试 =====\n");

    test_synopsis_and_void_expression();
    test_true_expression_no_abort();
    test_false_expression_aborts();
    test_diagnostic_content();
    test_scalar_types();
    test_returns_no_value();
    test_ndebug_behavior();

    printf("===== 全部正向测试通过 =====\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* 违反约束 [1]「expression shall have a scalar type」：
 * 结构体类型不是标量类型，不能作为 assert 实参。
 * gcc -std=c99 应报错：incompatible type for argument / 
 *   "struct S" is not a scalar type。 */
struct S { int x; };
struct S s;
assert(s);   /* 错误：结构体非标量 */

/* 违反约束 [1]「expression shall have a scalar type」：
 * 数组类型不是标量类型。 */
int arr[3];
assert(arr); /* 错误：数组非标量 */

/* 违反约束 [1]「expression shall have a scalar type」：
 * 联合体类型不是标量类型。 */
union U { int a; double b; };
union U u;
assert(u);   /* 错误：联合体非标量 */

/* 违反约束 [1]「expression shall have a scalar type」：
 * void 类型不是标量类型。 */
void f(void);
assert(f()); /* 错误：void 非标量 */

/* 违反约束 [3]「assert 不返回值」：
 * assert 展开为 void 表达式，不能把它的“值”赋给 int 变量。
 * gcc -std=c99 应报错：void value not ignored as it ought to be。 */
int n = assert(1);   /* 错误：void 表达式不能初始化 int */

/* 违反约束 [3]「assert 不返回值」：
 * 不能对 assert 的结果做算术运算。 */
int m = assert(1) + 1;  /* 错误：void 表达式参与算术 */

/* 违反约束 [3]「assert 不返回值」：
 * 不能把 assert 的结果作为函数实参传递。 */
void g(int);
g(assert(1));   /* 错误：void 表达式作为 int 实参 */

#endif