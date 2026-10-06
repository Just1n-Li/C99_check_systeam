/*
 * 测试 C99 7.19.10.4 —— perror 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立），
 *             验证 perror 的语义（[2] 输出格式、与 strerror 一致、[3] 无返回值）。
 *   负向测试：违反约束的代码片段应导致编译报错（放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：perror 输出到 stderr，无法在程序内直接捕获比较，
 *       因此正向测试通过重定向 stderr 到临时文件后读取内容来验证格式。
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：void perror(const char *s); —— 验证函数可被调用且返回类型为 void */
static void test_prototype_and_void_return(void)
{
    /* [3] perror 不返回值：以下语句若 perror 有返回值则无法作为 void 表达式使用 */
    /* 这里仅验证可调用性；返回值不可用（见负向测试） */
    errno = 0;
    /* 调用一次，确保原型匹配（参数为 const char *） */
    perror(NULL);
}

/* 辅助：把 stderr 重定向到文件，运行 fn，再把文件内容读回 buf */
static void capture_stderr(void (*fn)(void), char *buf, size_t bufsz)
{
    FILE *tmp = tmpfile();
    assert(tmp != NULL);

    fflush(stderr);
    int saved = dup(fileno(stderr));
    assert(saved != -1);
    assert(dup2(fileno(tmp), fileno(stderr)) != -1);

    fn();
    fflush(stderr);

    /* 恢复 stderr */
    assert(dup2(saved, fileno(stderr)) != -1);
    close(saved);

    /* 读回内容 */
    rewind(tmp);
    size_t n = fread(buf, 1, bufsz - 1, tmp);
    buf[n] = '\0';
    fclose(tmp);
}

/* [2] 当 s 非空且 *s 非 '\0' 时：输出 "s: 错误消息\n" */
static void do_perror_with_prefix(void)
{
    errno = ENOENT;          /* 一个确定存在的错误号 */
    perror("myprog");
}

/* [2] 当 s 为 NULL 时：只输出 "错误消息\n"（无前缀、无冒号空格） */
static void do_perror_null(void)
{
    errno = ENOENT;
    perror(NULL);
}

/* [2] 当 s 指向空字符串时：只输出 "错误消息\n"（无前缀、无冒号空格） */
static void do_perror_empty(void)
{
    errno = ENOENT;
    perror("");
}

/* [2] 错误消息内容应与 strerror(errno) 相同 */
static void do_perror_for_strerror_compare(void)
{
    errno = ENOENT;
    perror(NULL);
}

int main(void)
{
    char buf[1024];

    /* [1] 原型与可调用性 */
    test_prototype_and_void_return();

    /* [2] 带前缀：应以 "myprog: " 开头，并以 '\n' 结尾 */
    capture_stderr(do_perror_with_prefix, buf, sizeof buf);
    assert(strncmp(buf, "myprog: ", 8) == 0);
    assert(buf[strlen(buf) - 1] == '\n');

    /* [2] 前缀后紧跟的错误消息应与 strerror(ENOENT) 一致 */
    {
        const char *msg = strerror(ENOENT);
        assert(msg != NULL);
        /* buf 形如 "myprog: <msg>\n" */
        const char *after = buf + 8; /* 跳过 "myprog: " */
        size_t mlen = strlen(msg);
        assert(strncmp(after, msg, mlen) == 0);
        assert(after[mlen] == '\n');
    }

    /* [2] s == NULL：无前缀，直接是错误消息 + '\n' */
    capture_stderr(do_perror_null, buf, sizeof buf);
    {
        const char *msg = strerror(ENOENT);
        size_t mlen = strlen(msg);
        assert(strncmp(buf, msg, mlen) == 0);
        assert(buf[mlen] == '\n');
        /* 不应出现冒号空格前缀 */
        assert(strncmp(buf, ": ", 2) != 0);
    }

    /* [2] s 指向空字符串：与 NULL 情形相同（无前缀） */
    capture_stderr(do_perror_empty, buf, sizeof buf);
    {
        const char *msg = strerror(ENOENT);
        size_t mlen = strlen(msg);
        assert(strncmp(buf, msg, mlen) == 0);
        assert(buf[mlen] == '\n');
    }

    /* [2] 与 strerror 内容一致（再次独立验证） */
    capture_stderr(do_perror_for_strerror_compare, buf, sizeof buf);
    {
        const char *msg = strerror(ENOENT);
        size_t mlen = strlen(msg);
        assert(strncmp(buf, msg, mlen) == 0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「perror 返回 void，不能使用其返回值」：
 * void 表达式不能作为赋值右操作数，gcc -std=c99 应报错。 */
void bad_use_return_value(void)
{
    int x = perror("x");   /* error: void value not ignored as it ought to be */
    (void)x;
}

/* 违反约束「参数类型为 const char *」：
 * 传入 int 实参，与原型不兼容，应报错。 */
void bad_argument_type(void)
{
    perror(42);            /* error: incompatible type for argument 1 of 'perror' */
}

/* 违反约束「参数个数为 1」：
 * 少传参数，应报错。 */
void bad_too_few_args(void)
{
    perror();              /* error: too few arguments to function 'perror' */
}

/* 违反约束「参数个数为 1」：
 * 多传参数，应报错。 */
void bad_too_many_args(void)
{
    perror("a", "b");      /* error: too many arguments to function 'perror' */
}

/* 违反约束「perror 返回 void，不能取地址后解引用为对象」：
 * 对 void 表达式取地址/解引用，应报错。 */
void bad_deref_void(void)
{
    *perror("x");          /* error: void value not ignored / invalid use of void expression */
}

#endif /* 负向测试结束 */