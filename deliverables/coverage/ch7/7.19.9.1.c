/*
 * 测试 C99 7.19.9.1 —— fgetpos 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明：int fgetpos(FILE * restrict stream, fpos_t * restrict pos);
 *   [2] 语义：把当前解析状态与文件位置指示器存入 *pos，值可被 fsetpos 使用以复位。
 *   [3] 返回值：成功返回 0；失败返回非零并在 errno 中存入实现定义的正值。
 *   Forward references: fsetpos (7.19.9.3)
 */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性检查：取函数地址，类型必须匹配。
 *     若 <stdio.h> 未声明 fgetpos，或声明类型不符，此处会编译报错。 */
static int (*fp_fgetpos)(FILE * restrict, fpos_t * restrict) = fgetpos;

int main(void)
{
    /* [1] 原型返回类型为 int */
    {
        int (*p)(FILE * restrict, fpos_t * restrict) = fgetpos;
        assert(p == fgetpos);
        assert(fp_fgetpos == fgetpos);
    }

    /* [2] 基本语义：写入数据后 fgetpos 记录位置，
     *     再用 fsetpos 复位，重新读取应得到相同内容。 */
    {
        const char *path = "fgetpos_test_tmp.txt";
        FILE *f = fopen(path, "w+");
        assert(f != NULL);

        const char *msg = "Hello, fgetpos!";
        size_t n = strlen(msg);
        assert(fwrite(msg, 1, n, f) == n);
        assert(fflush(f) == 0);

        /* 记录当前位置（文件末尾） */
        fpos_t pos;
        errno = 0;
        int rc = fgetpos(f, &pos);
        assert(rc == 0);                 /* [3] 成功返回 0 */

        /* 回到开头，读一部分，再复位到记录的位置 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        char buf[64];
        assert(fread(buf, 1, 5, f) == 5);
        assert(memcmp(buf, "Hello", 5) == 0);

        /* [2] 用 fgetpos 保存的值配合 fsetpos 复位 */
        assert(fsetpos(f, &pos) == 0);

        /* 复位后应位于文件末尾，再读应得到 EOF */
        int c = fgetc(f);
        assert(c == EOF);

        fclose(f);
        remove(path);
    }

    /* [2] 在文件中间记录位置并复位 */
    {
        const char *path = "fgetpos_test_tmp2.txt";
        FILE *f = fopen(path, "w+");
        assert(f != NULL);

        const char *msg = "ABCDEFGHIJ";
        assert(fwrite(msg, 1, 10, f) == 10);
        assert(fflush(f) == 0);

        /* 定位到偏移 4 处并记录 */
        assert(fseek(f, 4, SEEK_SET) == 0);
        fpos_t pos;
        assert(fgetpos(f, &pos) == 0);

        /* 读走 3 个字符 */
        char buf[8];
        assert(fread(buf, 1, 3, f) == 3);
        assert(memcmp(buf, "EFG", 3) == 0);

        /* 复位到偏移 4 */
        assert(fsetpos(f, &pos) == 0);
        assert(fread(buf, 1, 3, f) == 3);
        assert(memcmp(buf, "EFG", 3) == 0);

        fclose(f);
        remove(path);
    }

    /* [3] 失败路径：对已关闭/无效流调用 fgetpos 应返回非零，
     *     并在 errno 中存入一个正值（实现定义）。 */
    {
        const char *path = "fgetpos_test_tmp3.txt";
        FILE *f = fopen(path, "w+");
        assert(f != NULL);
        fclose(f);

        fpos_t pos;
        errno = 0;
        int rc = fgetpos(f, &pos);   /* 对已关闭的流调用 */
        assert(rc != 0);             /* [3] 失败返回非零 */
        assert(errno > 0);           /* [3] errno 为实现定义的正值 */

        remove(path);
    }

    /* [2] 多次调用 fgetpos 记录不同位置，均可被 fsetpos 使用 */
    {
        const char *path = "fgetpos_test_tmp4.txt";
        FILE *f = fopen(path, "w+");
        assert(f != NULL);

        const char *msg = "0123456789";
        assert(fwrite(msg, 1, 10, f) == 10);
        assert(fflush(f) == 0);

        fpos_t p2, p7;
        assert(fseek(f, 2, SEEK_SET) == 0);
        assert(fgetpos(f, &p2) == 0);
        assert(fseek(f, 7, SEEK_SET) == 0);
        assert(fgetpos(f, &p7) == 0);

        char buf[4];
        assert(fsetpos(f, &p7) == 0);
        assert(fread(buf, 1, 3, f) == 3);
        assert(memcmp(buf, "789", 3) == 0);

        assert(fsetpos(f, &p2) == 0);
        assert(fread(buf, 1, 3, f) == 3);
        assert(memcmp(buf, "234", 3) == 0);

        fclose(f);
        remove(path);
    }

    printf("C99 7.19.9.1 fgetpos: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fgetpos 的第一个参数类型为 FILE * restrict」：
 * 传入 int 而非 FILE *，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg1(void)
{
    fpos_t pos;
    int x = 0;
    fgetpos(x, &pos);   /* error: 第一个实参不是 FILE * */
}

/* 违反约束「fgetpos 的第二个参数类型为 fpos_t * restrict」：
 * 传入 int * 而非 fpos_t *，应报 incompatible type 错误。 */
void bad_arg2(FILE *f)
{
    int y = 0;
    fgetpos(f, &y);     /* error: 第二个实参不是 fpos_t * */
}

/* 违反约束「fgetpos 需要两个实参」：
 * 实参个数不足，应报 too few arguments 错误。 */
void bad_argc(FILE *f)
{
    fgetpos(f);         /* error: 实参个数不足 */
}

/* 违反约束「fgetpos 返回 int，不能当作 void 使用于需要值的上下文」：
 * 这里演示把返回值赋给不兼容的指针类型，应报 incompatible 错误。 */
void bad_ret(FILE *f, fpos_t *p)
{
    int *q = fgetpos(f, p);   /* error: int 不能初始化 int * */
    (void)q;
}

/* 违反约束「fgetpos 的 restrict 限定：两个 restrict 指针不得指向同一对象」——
 * 注意：restrict 的违反属于未定义行为而非编译期约束，故此处不列为负向测试。
 * 仅作说明，不放入 #if 0 的报错期望中。 */

#endif /* 负向测试结束 */