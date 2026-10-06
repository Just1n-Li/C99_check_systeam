/*
 * 测试条款：C99 7.19.4.1  The remove function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明 int remove(const char *filename);
 *   [2] 语义：使文件不再能通过该名字访问；后续用该名字打开应失败（除非重新创建）；
 *       文件处于打开状态时行为由实现定义（此处只做“可调用、不崩溃”的弱验证）。
 *   [3] 返回值：成功返回 0，失败返回非 0。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：remove 的返回类型为 int，参数为 const char *。
 *     通过取函数指针类型来静态验证原型签名。 */
static int (*remove_proto_check)(const char *) = remove;

int main(void)
{
    /* [1] 原型：返回类型 int，参数 const char * */
    {
        int (*fp)(const char *) = remove;
        assert(fp == remove_proto_check);
    }

    /* [3] 成功路径：创建一个文件，remove 应返回 0 */
    {
        const char *fname = "c99_7_19_4_1_tmp.txt";
        FILE *fp = fopen(fname, "w");
        assert(fp != NULL);
        fputs("hello c99 remove test\n", fp);
        assert(fclose(fp) == 0);

        /* 文件存在，可被打开 */
        fp = fopen(fname, "r");
        assert(fp != NULL);
        assert(fclose(fp) == 0);

        /* [3] 成功时返回 0 */
        int rc = remove(fname);
        assert(rc == 0);

        /* [2] 删除后，用同一名字打开应失败 */
        fp = fopen(fname, "r");
        assert(fp == NULL);
    }

    /* [2] 重新创建后，同一名字又可访问 */
    {
        const char *fname = "c99_7_19_4_1_tmp2.txt";
        FILE *fp = fopen(fname, "w");
        assert(fp != NULL);
        assert(fclose(fp) == 0);

        assert(remove(fname) == 0);

        /* 删除后打开失败 */
        fp = fopen(fname, "r");
        assert(fp == NULL);

        /* 重新创建后打开成功 */
        fp = fopen(fname, "w");
        assert(fp != NULL);
        assert(fclose(fp) == 0);

        /* 清理 */
        assert(remove(fname) == 0);
    }

    /* [3] 失败路径：删除一个不存在的文件，应返回非 0 */
    {
        const char *fname = "c99_7_19_4_1_no_such_file_xyz.txt";
        /* 确保它确实不存在 */
        (void)remove(fname);

        int rc = remove(fname);
        assert(rc != 0);
    }

    /* [2] 文件处于打开状态时调用 remove：行为由实现定义。
     *     这里只验证“调用本身可编译、可执行、不崩溃”，不断言具体结果。 */
    {
        const char *fname = "c99_7_19_4_1_open.txt";
        FILE *fp = fopen(fname, "w");
        assert(fp != NULL);
        fputs("open file\n", fp);

        /* 实现定义行为：只要求可调用，不检查返回值 */
        (void)remove(fname);

        assert(fclose(fp) == 0);

        /* 清理（若上面 remove 未成功，这里再删一次） */
        (void)remove(fname);
    }

    printf("C99 7.19.4.1 remove: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「remove 的参数类型为 const char *」：
 * 传入 int 实参，gcc -std=c99 应报错（参数类型不兼容 / 指针与整数比较）。
 * 期望：error: passing argument 1 of 'remove' makes pointer from integer without a cast */
int bad_arg_type(void)
{
    int x = 42;
    return remove(x);
}

/* 违反约束「remove 的参数个数为 1」：
 * 不传参数，gcc -std=c99 应报错（参数太少）。
 * 期望：error: too few arguments to function 'remove' */
int bad_too_few_args(void)
{
    return remove();
}

/* 违反约束「remove 的参数个数为 1」：
 * 传两个参数，gcc -std=c99 应报错（参数太多）。
 * 期望：error: too many arguments to function 'remove' */
int bad_too_many_args(void)
{
    return remove("a.txt", "b.txt");
}

/* 违反约束「remove 的返回类型为 int」：
 * 把返回值当作结构体使用，gcc -std=c99 应报错。
 * 期望：error: invalid use of void expression / incompatible types */
struct S { int x; };
int bad_return_use(void)
{
    struct S s;
    s = remove("a.txt");   /* int 不能赋给 struct S */
    return s.x;
}

/* 违反约束「remove 的参数类型为 const char *」：
 * 传入 double 实参，gcc -std=c99 应报错。
 * 期望：error: incompatible type for argument 1 of 'remove' */
int bad_arg_double(void)
{
    double d = 3.14;
    return remove(d);
}

#endif