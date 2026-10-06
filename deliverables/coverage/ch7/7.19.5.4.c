/*
 * 测试 C99 7.19.5.4 —— freopen 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明（restrict 限定、返回 FILE*、三个参数）
 *   [2] 打开 filename 指定的文件并与 stream 关联；mode 用法同 fopen
 *   [3] filename 为 NULL 时尝试改变 stream 的模式（实现定义）
 *   [4] 先尝试关闭 stream 关联的文件（关闭失败被忽略），清除错误/EOF 指示器
 *   [5] 打开失败返回 NULL；否则返回 stream 的值
 *   Footnote 238: 主要用于重定向标准文本流
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型兼容 */
static FILE *(*freopen_proto)(const char *restrict,
                              const char *restrict,
                              FILE *restrict) = freopen;

int main(void)
{
    /* [1] 原型存在且可赋值给兼容的函数指针 */
    assert(freopen_proto == freopen);

    /* [2] 用 freopen 打开一个文件并与 stream 关联，mode 用法同 fopen */
    {
        FILE *fp = fopen("c99_7_19_5_4_tmp.txt", "w");
        assert(fp != NULL);
        assert(fputs("hello\n", fp) >= 0);
        /* 重新以读模式打开同一文件，关联到同一个 stream */
        FILE *r = freopen("c99_7_19_5_4_tmp.txt", "r", fp);
        /* [5] 成功时返回 stream 的值 */
        assert(r == fp);
        /* [4] 错误/EOF 指示器被清除：刚重开，feof/ferror 应为 0 */
        assert(feof(r) == 0);
        assert(ferror(r) == 0);
        /* 能读到之前写入的内容 */
        {
            char buf[32];
            assert(fgets(buf, sizeof buf, r) != NULL);
            assert(strcmp(buf, "hello\n") == 0);
        }
        fclose(r);
    }

    /* [5] 打开失败返回 NULL（打开一个不存在的目录下的文件） */
    {
        FILE *fp = fopen("c99_7_19_5_4_tmp2.txt", "w");
        assert(fp != NULL);
        FILE *bad = freopen("/no_such_dir_xyz/no_such_file_xyz",
                            "r", fp);
        assert(bad == NULL);
        /* 失败后原 stream 状态由实现决定，这里不再使用它 */
    }

    /* [3] filename 为 NULL：尝试改变 stream 的模式（实现定义）。
     *     这里只验证调用不崩溃、返回值要么是 stream 要么是 NULL，
     *     不假设具体实现是否允许该模式改变。 */
    {
        FILE *fp = fopen("c99_7_19_5_4_tmp3.txt", "w+");
        assert(fp != NULL);
        FILE *r = freopen(NULL, "w+", fp);
        /* 实现定义：允许则返回 stream，不允许则返回 NULL */
        assert(r == fp || r == NULL);
        if (r == fp) {
            /* [4] 指示器被清除 */
            assert(feof(r) == 0);
            assert(ferror(r) == 0);
            fclose(r);
        }
    }

    /* [4] 清除错误/EOF 指示器：先制造 EOF，再用 freopen 重开，指示器应清零 */
    {
        FILE *fp = fopen("c99_7_19_5_4_tmp4.txt", "w");
        assert(fp != NULL);
        fputs("x", fp);
        fclose(fp);

        fp = fopen("c99_7_19_5_4_tmp4.txt", "r");
        assert(fp != NULL);
        /* 读到 EOF，设置 EOF 指示器 */
        while (fgetc(fp) != EOF) { }
        assert(feof(fp) != 0);

        /* 用 freopen 重新关联同一文件，指示器应被清除 */
        FILE *r = freopen("c99_7_19_5_4_tmp4.txt", "r", fp);
        assert(r == fp);
        assert(feof(r) == 0);
        assert(ferror(r) == 0);
        fclose(r);
    }

    /* Footnote 238: 主要用于重定向标准文本流。
     * 这里演示把 stdout 重定向到文件，再恢复。
     * 注意：恢复 stdout 到终端在不同平台方式不同，这里只做重定向到文件，
     * 然后通过 freopen 再重定向到另一个文件，验证返回值语义。 */
    {
        FILE *r1 = freopen("c99_7_19_5_4_stdout1.txt", "w", stdout);
        assert(r1 == stdout);
        printf("redirected-1\n");
        fflush(stdout);

        FILE *r2 = freopen("c99_7_19_5_4_stdout2.txt", "w", stdout);
        assert(r2 == stdout);
        printf("redirected-2\n");
        fflush(stdout);
        /* 不再恢复 stdout，程序结束即可 */
    }

    /* 清理临时文件 */
    remove("c99_7_19_5_4_tmp.txt");
    remove("c99_7_19_5_4_tmp2.txt");
    remove("c99_7_19_5_4_tmp3.txt");
    remove("c99_7_19_5_4_tmp4.txt");
    remove("c99_7_19_5_4_stdout1.txt");
    remove("c99_7_19_5_4_stdout2.txt");

    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「freopen 的第一个参数类型为 const char * restrict」：
 * 传入 int 类型实参，gcc -std=c99 应报错（参数类型不兼容）。 */
void bad_arg_type(void)
{
    FILE *fp = fopen("x", "w");
    freopen(123, "r", fp);   /* 错误：第一个实参应为 const char * */
}

/* 违反约束「freopen 的第二个参数类型为 const char * restrict」：
 * 传入 int 类型实参，应报错。 */
void bad_mode_type(void)
{
    FILE *fp = fopen("x", "w");
    freopen("x", 456, fp);   /* 错误：第二个实参应为 const char * */
}

/* 违反约束「freopen 的第三个参数类型为 FILE * restrict」：
 * 传入 int 类型实参，应报错。 */
void bad_stream_type(void)
{
    freopen("x", "r", 789);  /* 错误：第三个实参应为 FILE * */
}

/* 违反约束「freopen 需要三个实参」：
 * 实参个数不足，应报错。 */
void too_few_args(void)
{
    freopen("x", "r");       /* 错误：缺少第三个实参 */
}

/* 违反约束「freopen 需要三个实参」：
 * 实参个数过多，应报错。 */
void too_many_args(void)
{
    FILE *fp = fopen("x", "w");
    freopen("x", "r", fp, fp); /* 错误：实参过多 */
}

/* 违反约束「freopen 返回 FILE *，不能当作其他类型使用」：
 * 把返回值赋给 int，应报错（指针到整数的隐式转换不允许）。 */
void bad_return_use(void)
{
    FILE *fp = fopen("x", "w");
    int n = freopen("x", "r", fp);  /* 错误：FILE* 不能隐式转 int */
    (void)n;
}

#endif /* 负向测试结束 */