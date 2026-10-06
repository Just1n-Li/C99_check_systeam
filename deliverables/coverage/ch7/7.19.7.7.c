/*
 * 测试条款：C99 7.19.7.7  The gets function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *             验证 gets 的声明、从 stdin 读字符、丢弃换行、写 '\0'、
 *             成功返回 s、EOF 且未读入任何字符时返回 NULL 且数组不变。
 *   负向测试：违反约束的代码片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 说明：gets 在 C99 中仍是标准库函数（在 C11 中被移除），
 *       因此用 -std=c99 编译时 <stdio.h> 必须声明 char *gets(char *s);
 *       本程序通过重定向 stdin 来驱动 gets 的行为。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] Synopsis：<stdio.h> 中声明 char *gets(char *s);
 *     这里用一个函数指针来静态验证其原型（返回 char*，参数 char*）。 */
static char *(*gets_proto)(char *) = gets;

/* 辅助：把字符串写入临时文件，再重定向 stdin 读取它 */
static FILE *make_stdin(const char *content)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);
    if (content && *content)
        fwrite(content, 1, strlen(content), fp);
    rewind(fp);
    return fp;
}

int main(void)
{
    /* [1] 原型检查：gets 的返回类型为 char *，参数为 char * */
    assert(gets_proto == gets);

    /* [2] 读取字符直到换行，丢弃换行，并在最后写入 '\0' */
    {
        FILE *fp = make_stdin("hello\nworld\n");
        char buf[64];
        char *r;

        /* 保存/替换 stdin */
        FILE *saved = stdin;
        stdin = fp;

        memset(buf, 'X', sizeof buf);
        r = gets(buf);
        /* [3] 成功时返回 s（即 buf） */
        assert(r == buf);
        /* [2] 换行被丢弃，'\0' 写在最后读入字符之后 */
        assert(strcmp(buf, "hello") == 0);
        assert(buf[5] == '\0');

        /* 第二次调用读取第二行 */
        r = gets(buf);
        assert(r == buf);
        assert(strcmp(buf, "world") == 0);
        assert(buf[5] == '\0');

        /* 第三次：已到 EOF 且未读入任何字符 -> 返回 NULL，数组不变 */
        memset(buf, 'Z', sizeof buf);
        r = gets(buf);
        /* [3] EOF 且未读入字符：返回空指针，数组内容保持不变 */
        assert(r == NULL);
        assert(buf[0] == 'Z');   /* 数组未被改动 */

        stdin = saved;
        fclose(fp);
    }

    /* [2] 无换行结尾的输入：读到 EOF 停止，仍写 '\0' */
    {
        FILE *fp = make_stdin("abc");   /* 无换行 */
        char buf[16];
        char *r;
        FILE *saved = stdin;
        stdin = fp;

        memset(buf, 'Q', sizeof buf);
        r = gets(buf);
        /* [3] 成功（读入了字符）返回 s */
        assert(r == buf);
        /* [2] 读到 EOF 停止，'\0' 写在最后字符之后 */
        assert(strcmp(buf, "abc") == 0);
        assert(buf[3] == '\0');

        /* 再次调用：EOF 且未读入字符 -> NULL，数组不变 */
        memset(buf, 'W', sizeof buf);
        r = gets(buf);
        assert(r == NULL);
        assert(buf[0] == 'W');

        stdin = saved;
        fclose(fp);
    }

    /* [2] 空行输入：立即读到换行，丢弃换行，写入 '\0'（空串） */
    {
        FILE *fp = make_stdin("\n");
        char buf[8];
        char *r;
        FILE *saved = stdin;
        stdin = fp;

        memset(buf, 'A', sizeof buf);
        r = gets(buf);
        /* [3] 成功返回 s */
        assert(r == buf);
        /* [2] 换行被丢弃，'\0' 立即写入 */
        assert(buf[0] == '\0');

        stdin = saved;
        fclose(fp);
    }

    /* [2] 空输入（立即 EOF）：未读入任何字符 */
    {
        FILE *fp = make_stdin("");
        char buf[8];
        char *r;
        FILE *saved = stdin;
        stdin = fp;

        memset(buf, 'M', sizeof buf);
        r = gets(buf);
        /* [3] EOF 且未读入字符：返回 NULL，数组不变 */
        assert(r == NULL);
        assert(buf[0] == 'M');

        stdin = saved;
        fclose(fp);
    }

    printf("C99 7.19.7.7 gets: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型为 char *gets(char *s)」：
 * 参数类型不匹配——传入 int* 而非 char*，且无隐式转换，
 * gcc -std=c99 应报错（incompatible pointer type / 参数类型不兼容）。 */
void neg_wrong_arg_type(void)
{
    int arr[16];
    gets(arr);          /* 期望：编译报错，int* 不能传给 char* */
}

/* 违反约束「[1] 返回类型为 char *」：
 * 把 gets 的返回值赋给不兼容的指针类型（无强制转换），
 * gcc -std=c99 应报错（assignment from incompatible pointer type）。 */
void neg_wrong_return_use(void)
{
    char buf[16];
    int *p = gets(buf); /* 期望：编译报错，char* 赋给 int* */
    (void)p;
}

/* 违反约束「[1] 参数个数为 1」：
 * 调用 gets 时传入过多实参，gcc -std=c99 应报错（too many arguments）。 */
void neg_too_many_args(void)
{
    char buf[16];
    gets(buf, buf);     /* 期望：编译报错，实参个数过多 */
}

/* 违反约束「[1] 参数个数为 1」：
 * 调用 gets 时未提供实参，gcc -std=c99 应报错（too few arguments）。 */
void neg_too_few_args(void)
{
    gets();             /* 期望：编译报错，实参个数过少 */
}

/* 违反约束「[1] 返回类型为 char *」：
 * 对 gets 的返回值（非左值）赋值，gcc -std=c99 应报错（lvalue required）。 */
void neg_assign_to_result(void)
{
    char buf[16];
    gets(buf) = buf;    /* 期望：编译报错，函数调用结果不是左值 */
}

#endif /* 负向测试结束 */