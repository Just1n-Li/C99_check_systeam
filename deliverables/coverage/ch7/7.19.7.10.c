/*
 * 测试 C99 7.19.7.10 —— puts 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int puts(const char *s);  声明于 <stdio.h>
 *   [2] 把 s 指向的字符串写到 stdout，并追加一个换行符；
 *       结尾的空字符 '\0' 不写出。
 *   [3] 写错误时返回 EOF；否则返回一个非负值。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：puts 的返回类型为 int，参数为 const char *。
 *     通过取函数指针类型来静态验证原型签名。 */
static int (*puts_proto_check)(const char *) = puts;

int main(void)
{
    /* [1] 原型存在且可调用，返回类型为 int */
    int ret;

    /* [2] 基本调用：写入字符串并追加换行。
     *     这里把 stdout 重定向到临时文件，以便检查实际写出的字节。 */
    {
        FILE *fp;
        char buf[64];
        size_t n;

        fp = freopen("puts_test_tmp.txt", "w+", stdout);
        assert(fp != NULL);

        /* [2] 写入 "hello"：应输出 "hello\n"，不含结尾 '\0' */
        ret = puts("hello");

        /* [3] 成功时返回非负值 */
        assert(ret >= 0);

        fflush(stdout);

        /* 读回文件内容验证 [2]：追加了换行，且未写出 '\0' */
        rewind(stdout);
        n = fread(buf, 1, sizeof(buf) - 1, stdout);
        buf[n] = '\0';

        /* 期望内容恰好是 "hello\n"（5 个字符 + 换行 = 6 字节） */
        assert(n == 6);
        assert(strcmp(buf, "hello\n") == 0);
        /* 确认没有把结尾空字符写进流 */
        assert(buf[5] == '\n');
        assert(buf[6] == '\0'); /* 这是我们自己补的终止符，不是 puts 写的 */

        /* [2] 空字符串：应只输出一个换行符 */
        rewind(stdout);
        /* 清空文件以便重新检查 */
        {
            FILE *tmp = freopen("puts_test_tmp.txt", "w+", stdout);
            assert(tmp != NULL);
        }
        ret = puts("");
        assert(ret >= 0); /* [3] */
        fflush(stdout);
        rewind(stdout);
        n = fread(buf, 1, sizeof(buf) - 1, stdout);
        buf[n] = '\0';
        assert(n == 1);
        assert(buf[0] == '\n');

        /* [2] 含内嵌可打印字符的字符串 */
        {
            FILE *tmp = freopen("puts_test_tmp.txt", "w+", stdout);
            assert(tmp != NULL);
        }
        ret = puts("a\tb");
        assert(ret >= 0); /* [3] */
        fflush(stdout);
        rewind(stdout);
        n = fread(buf, 1, sizeof(buf) - 1, stdout);
        buf[n] = '\0';
        assert(n == 4);
        assert(strcmp(buf, "a\tb\n") == 0);

        /* 恢复 stdout 到终端/原流，避免影响后续输出 */
        fclose(stdout);
        stdout = fdopen(1, "w");
        assert(stdout != NULL);
    }

    /* [1] 通过函数指针调用，验证原型签名一致 */
    {
        FILE *fp;
        char buf[32];
        size_t n;

        fp = freopen("puts_test_tmp2.txt", "w+", stdout);
        assert(fp != NULL);

        ret = puts_proto_check("ptr");
        assert(ret >= 0); /* [3] */

        fflush(stdout);
        rewind(stdout);
        n = fread(buf, 1, sizeof(buf) - 1, stdout);
        buf[n] = '\0';
        assert(strcmp(buf, "ptr\n") == 0); /* [2] */

        fclose(stdout);
        stdout = fdopen(1, "w");
        assert(stdout != NULL);
    }

    /* [3] 返回值语义：成功路径返回非负值（此处再确认一次） */
    {
        FILE *fp;
        fp = freopen("puts_test_tmp3.txt", "w+", stdout);
        assert(fp != NULL);
        ret = puts("ok");
        assert(ret >= 0);
        fclose(stdout);
        stdout = fdopen(1, "w");
        assert(stdout != NULL);
    }

    printf("All positive tests for C99 7.19.7.10 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「puts 的参数类型为 const char *」：
 * 传入 int 实参，且无隐式转换可行（int 不能隐式转为指针），
 * gcc -std=c99 应报错：passing argument 1 of 'puts' makes pointer
 * from integer without a cast / incompatible type。 */
void neg_wrong_arg_type(void)
{
    puts(42);
}

/* 违反约束「puts 的参数类型为 const char *」：
 * 传入 double 实参，double 不能隐式转为指针，应编译报错。 */
void neg_wrong_arg_type_double(void)
{
    puts(3.14);
}

/* 违反约束「puts 的参数类型为 const char *」：
 * 传入结构体实参，结构体不能隐式转为指针，应编译报错。 */
struct NegS { int x; };
void neg_wrong_arg_type_struct(void)
{
    struct NegS s;
    puts(s);
}

/* 违反约束「puts 的参数个数为 1」：
 * 传入两个实参，应编译报错：too many arguments to function 'puts'。 */
void neg_too_many_args(void)
{
    puts("a", "b");
}

/* 违反约束「puts 的参数个数为 1」：
 * 不传实参，应编译报错：too few arguments to function 'puts'。 */
void neg_too_few_args(void)
{
    puts();
}

/* 违反约束「puts 的返回类型为 int，不可作为左值赋值」：
 * 对函数调用结果赋值，应编译报错：lvalue required as left operand
 * of assignment。 */
void neg_assign_to_call(void)
{
    puts("x") = 0;
}

/* 违反约束「puts 的返回类型为 int，不可取地址」：
 * 对函数调用结果取地址，应编译报错：lvalue required as unary '&'
 * operand。 */
void neg_addr_of_call(void)
{
    int *p = &puts("x");
    (void)p;
}

#endif /* 负向测试结束 */