/*
 * 测试条款：C99 7.19.5.1  The fclose function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int fclose(FILE *stream);  需要 <stdio.h>
 *   [2] 语义：成功调用会 flush 流并把文件关闭；未写出的缓冲数据交给宿主
 *            环境写入文件；未读的缓冲数据被丢弃；无论成功与否，流与文件
 *            解除关联，setbuf/setvbuf 设置的缓冲区也与流解除关联。
 *   [3] 返回值：成功关闭返回 0，检测到错误返回 EOF。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：fclose 的声明必须与标准一致。
 *     通过取函数指针类型来静态验证签名（不匹配则编译报错）。 */
static int (*fclose_proto_check)(FILE *) = fclose;

int main(void)
{
    /* [1] 原型：返回类型为 int，参数为 FILE * */
    assert(fclose_proto_check == fclose);

    /* ---------- [2][3] 成功关闭：返回 0，数据被 flush 到文件 ---------- */
    {
        const char *path = "c99_7_19_5_1_a.txt";
        FILE *fp = fopen(path, "w");
        assert(fp != NULL);

        /* 写入数据，此时可能仍在缓冲区中 */
        assert(fputs("hello fclose\n", fp) >= 0);

        /* [2] 成功调用 fclose：flush 缓冲并关闭文件 */
        /* [3] 成功关闭返回 0 */
        int rc = fclose(fp);
        assert(rc == 0);

        /* [2] 验证未写出的缓冲数据确实被交付给宿主环境写入文件 */
        fp = fopen(path, "r");
        assert(fp != NULL);
        char buf[64];
        memset(buf, 0, sizeof buf);
        assert(fgets(buf, sizeof buf, fp) != NULL);
        assert(strcmp(buf, "hello fclose\n") == 0);
        assert(fclose(fp) == 0);

        remove(path);
    }

    /* ---------- [2] 未读缓冲数据被丢弃；流与文件解除关联 ---------- */
    {
        const char *path = "c99_7_19_5_1_b.txt";
        FILE *fp = fopen(path, "w+");
        assert(fp != NULL);

        assert(fputs("ABCDEFGHIJ", fp) >= 0);
        assert(fflush(fp) == 0);   /* 确保数据落到文件，便于读取 */

        /* 读入部分数据，剩余数据留在读缓冲区中 */
        int c = fgetc(fp);
        assert(c == 'A');

        /* [2] fclose 丢弃未读缓冲数据并关闭文件 */
        /* [3] 返回 0 */
        assert(fclose(fp) == 0);

        /* [2] 流已与文件解除关联：再次使用该 FILE* 是未定义行为，
         *     这里只验证文件内容仍为完整写入的数据（未被破坏）。 */
        fp = fopen(path, "r");
        assert(fp != NULL);
        char buf[64];
        memset(buf, 0, sizeof buf);
        assert(fgets(buf, sizeof buf, fp) != NULL);
        assert(strcmp(buf, "ABCDEFGHIJ") == 0);
        assert(fclose(fp) == 0);

        remove(path);
    }

    /* ---------- [2] setvbuf 设置的缓冲区随 fclose 解除关联 ---------- */
    {
        const char *path = "c99_7_19_5_1_c.txt";
        FILE *fp = fopen(path, "w");
        assert(fp != NULL);

        /* 使用自动分配的缓冲区（buf 为 NULL，size 非 0） */
        assert(setvbuf(fp, NULL, _IOFBF, 1024) == 0);

        assert(fputs("buffered\n", fp) >= 0);

        /* [2] fclose 使 setvbuf 设置的缓冲区与流解除关联
         *     （自动分配的缓冲区被释放） */
        assert(fclose(fp) == 0);

        fp = fopen(path, "r");
        assert(fp != NULL);
        char buf[64];
        memset(buf, 0, sizeof buf);
        assert(fgets(buf, sizeof buf, fp) != NULL);
        assert(strcmp(buf, "buffered\n") == 0);
        assert(fclose(fp) == 0);

        remove(path);
    }

    /* ---------- [2] setbuf 设置的缓冲区随 fclose 解除关联 ---------- */
    {
        const char *path = "c99_7_19_5_1_d.txt";
        FILE *fp = fopen(path, "w");
        assert(fp != NULL);

        static char mybuf[BUFSIZ];
        setbuf(fp, mybuf);   /* 用户提供的缓冲区 */

        assert(fputs("setbuf data\n", fp) >= 0);

        /* [2] fclose 使 setbuf 设置的缓冲区与流解除关联 */
        assert(fclose(fp) == 0);

        fp = fopen(path, "r");
        assert(fp != NULL);
        char buf[64];
        memset(buf, 0, sizeof buf);
        assert(fgets(buf, sizeof buf, fp) != NULL);
        assert(strcmp(buf, "setbuf data\n") == 0);
        assert(fclose(fp) == 0);

        remove(path);
    }

    /* ---------- [3] 错误情形：对已关闭的流再次 fclose 是 UB，
     *            不作为负向测试。这里用「关闭一个只读流」等正常路径
     *            验证返回值语义：正常关闭返回 0。 ---------- */
    {
        const char *path = "c99_7_19_5_1_e.txt";
        FILE *fp = fopen(path, "w");
        assert(fp != NULL);
        assert(fclose(fp) == 0);   /* [3] 成功返回 0 */

        /* 清理 */
        remove(path);
    }

    printf("C99 7.19.5.1 fclose: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fclose 的参数类型必须为 FILE *」：
 * 传入 int 实参，gcc -std=c99 应报 incompatible type / 参数类型不匹配错误。 */
void bad_arg_type(void)
{
    int x = 0;
    fclose(x);              /* 错误：int 不能转换为 FILE * */
}

/* 违反约束「fclose 的参数个数必须为 1」：
 * 少传参数，gcc -std=c99 应报 too few arguments 错误。 */
void bad_too_few_args(void)
{
    fclose();               /* 错误：缺少 FILE * 实参 */
}

/* 违反约束「fclose 的参数个数必须为 1」：
 * 多传参数，gcc -std=c99 应报 too many arguments 错误。 */
void bad_too_many_args(void)
{
    FILE *fp = 0;
    fclose(fp, fp);         /* 错误：参数过多 */
}

/* 违反约束「fclose 的返回类型为 int，不可作为左值赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报 lvalue required 错误。 */
void bad_assign_to_call(void)
{
    FILE *fp = 0;
    fclose(fp) = 0;         /* 错误：函数调用结果不是左值 */
}

/* 违反约束「fclose 的返回类型为 int，不可取地址」：
 * 对函数调用结果取地址，gcc -std=c99 应报 lvalue required 错误。 */
void bad_addr_of_call(void)
{
    FILE *fp = 0;
    int *p = &fclose(fp);   /* 错误：不能对非左值取地址 */
    (void)p;
}

/* 违反约束「fclose 的返回类型为 int，不可用于需要结构体/数组的上下文」：
 * 将返回值赋给结构体，gcc -std=c99 应报类型不兼容错误。 */
struct S { int a; };
void bad_return_type_use(void)
{
    FILE *fp = 0;
    struct S s;
    s = fclose(fp);         /* 错误：int 不能赋给 struct S */
}

#endif /* 负向测试结束 */