/*
 * 测试 C99 7.19.8.2 —— fwrite 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdio.h>
 *   [2] 语义：从 ptr 指向的数组写出至多 nmemb 个 size 字节的元素；
 *       每个对象按 unsigned char 数组逐字节（size 次 fputc）写出；
 *       文件位置指示器前进已成功写出的字符数；出错时位置指示器值不确定。
 *   [3] 返回值：成功写出的元素个数；仅当写错误时小于 nmemb；
 *       size 或 nmemb 为 0 时返回 0 且流状态不变。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* 辅助：读取整个文件内容到 buf，返回读到的字节数 */
static size_t slurp(const char *path, unsigned char *buf, size_t cap)
{
    FILE *f = fopen(path, "rb");
    size_t n;
    assert(f != NULL);
    n = fread(buf, 1, cap, f);
    fclose(f);
    return n;
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：包含 <stdio.h> 后 fwrite 声明可见，参数为
     *     (const void * restrict, size_t, size_t, FILE * restrict)，
     *     返回 size_t。这里通过取函数指针类型来静态验证签名。 */
    {
        size_t (*fp)(const void * restrict, size_t, size_t, FILE * restrict) = fwrite;
        assert(fp != NULL);
    }

    /* [2][3] 基本写出：写 3 个元素，每个 4 字节，应返回 3 */
    {
        const char *path = "fw_basic.bin";
        int data[3] = { 0x11223344, 0x55667788, 0x0A0B0C0D };
        FILE *f = fopen(path, "wb");
        size_t r;
        unsigned char buf[64];
        size_t n;

        assert(f != NULL);
        r = fwrite(data, sizeof(int), 3, f);
        assert(r == 3);                 /* [3] 全部成功 */
        fclose(f);

        /* [2] 逐字节按 unsigned char 覆盖写出：验证字节内容 */
        n = slurp(path, buf, sizeof buf);
        assert(n == 3 * sizeof(int));
        assert(memcmp(buf, data, 3 * sizeof(int)) == 0);
        remove(path);
    }

    /* [2] 文件位置指示器前进已成功写出的字符数 */
    {
        const char *path = "fw_pos.bin";
        FILE *f = fopen(path, "wb");
        long pos;
        size_t r;

        assert(f != NULL);
        r = fwrite("ABCDE", 1, 5, f);
        assert(r == 5);
        pos = ftell(f);                 /* [2] 前进 5 个字符 */
        assert(pos == 5);
        r = fwrite("XY", 1, 2, f);
        assert(r == 2);
        pos = ftell(f);
        assert(pos == 7);
        fclose(f);
        remove(path);
    }

    /* [2] 元素大小 > 1 时，位置指示器按“字符数”前进（size*nmemb） */
    {
        const char *path = "fw_pos2.bin";
        FILE *f = fopen(path, "wb");
        double d[2] = { 1.5, 2.5 };
        size_t r;
        long pos;

        assert(f != NULL);
        r = fwrite(d, sizeof(double), 2, f);
        assert(r == 2);
        pos = ftell(f);
        assert(pos == (long)(2 * sizeof(double)));
        fclose(f);
        remove(path);
    }

    /* [3] size == 0：返回 0，流状态不变（位置指示器不变） */
    {
        const char *path = "fw_zero_size.bin";
        FILE *f = fopen(path, "wb");
        size_t r;
        long pos_before, pos_after;

        assert(f != NULL);
        assert(fwrite("hello", 1, 5, f) == 5);
        pos_before = ftell(f);
        r = fwrite("ignored", 0, 10, f);   /* size == 0 */
        assert(r == 0);                    /* [3] 返回 0 */
        pos_after = ftell(f);
        assert(pos_after == pos_before);   /* [3] 流状态不变 */
        fclose(f);
        remove(path);
    }

    /* [3] nmemb == 0：返回 0，流状态不变 */
    {
        const char *path = "fw_zero_nmemb.bin";
        FILE *f = fopen(path, "wb");
        size_t r;
        long pos_before, pos_after;

        assert(f != NULL);
        assert(fwrite("hello", 1, 5, f) == 5);
        pos_before = ftell(f);
        r = fwrite("ignored", 4, 0, f);    /* nmemb == 0 */
        assert(r == 0);                    /* [3] 返回 0 */
        pos_after = ftell(f);
        assert(pos_after == pos_before);   /* [3] 流状态不变 */
        fclose(f);
        remove(path);
    }

    /* [3] size == 0 且 nmemb == 0：仍返回 0 */
    {
        const char *path = "fw_zero_both.bin";
        FILE *f = fopen(path, "wb");
        size_t r;

        assert(f != NULL);
        r = fwrite("x", 0, 0, f);
        assert(r == 0);
        fclose(f);
        remove(path);
    }

    /* [2] 写出到只读流（"rb"）应触发写错误，返回值 < nmemb */
    {
        const char *path = "fw_err.bin";
        FILE *f = fopen(path, "wb");
        FILE *rf;
        size_t r;

        assert(f != NULL);
        assert(fwrite("data", 1, 4, f) == 4);
        fclose(f);

        rf = fopen(path, "rb");            /* 只读打开 */
        assert(rf != NULL);
        r = fwrite("X", 1, 1, rf);         /* 写只读流：错误 */
        assert(r < 1);                     /* [3] 出错时 < nmemb */
        fclose(rf);
        remove(path);
    }

    /* [2] 二进制数据（含 0 字节）应被完整写出，不被截断 */
    {
        const char *path = "fw_bin.bin";
        unsigned char data[5] = { 0x00, 0xFF, 0x00, 0x7F, 0x80 };
        FILE *f = fopen(path, "wb");
        unsigned char buf[8];
        size_t n, r;

        assert(f != NULL);
        r = fwrite(data, 1, 5, f);
        assert(r == 5);
        fclose(f);

        n = slurp(path, buf, sizeof buf);
        assert(n == 5);
        assert(memcmp(buf, data, 5) == 0);
        remove(path);
    }

    /* [2] 结构体对象按 unsigned char 覆盖逐字节写出 */
    {
        const char *path = "fw_struct.bin";
        struct S { int a; char b; } s = { 0x01020304, 'Z' };
        FILE *f = fopen(path, "wb");
        unsigned char buf[sizeof(struct S)];
        size_t n, r;

        assert(f != NULL);
        r = fwrite(&s, sizeof(struct S), 1, f);
        assert(r == 1);
        fclose(f);

        n = slurp(path, buf, sizeof buf);
        assert(n == sizeof(struct S));
        assert(memcmp(buf, &s, sizeof(struct S)) == 0);
        remove(path);
    }

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「fwrite 的第一个参数类型为 const void *」：
     * 传入 FILE* 作为数据指针，类型不兼容，gcc -std=c99 应报错
     * （incompatible pointer type / passing argument 1）。 */
    {
        FILE *f = fopen("x", "wb");
        fwrite(f, 1, 1, f);   /* 第一个实参应为 const void*，此处为 FILE* */
    }

    /* 违反约束「fwrite 的第四个参数类型为 FILE *」：
     * 传入 int 作为流指针，类型不兼容，应报错。 */
    {
        int x = 0;
        fwrite("a", 1, 1, x); /* 第四个实参应为 FILE*，此处为 int */
    }

    /* 违反约束「fwrite 的第二个参数类型为 size_t」：
     * 传入结构体，无法转换为 size_t，应报错。 */
    {
        struct T { int v; } t;
        FILE *f = fopen("x", "wb");
        fwrite("a", t, 1, f); /* 第二个实参应为 size_t，此处为 struct T */
    }

    /* 违反约束「fwrite 的第三个参数类型为 size_t」：
     * 传入指针，无法转换为 size_t，应报错。 */
    {
        FILE *f = fopen("x", "wb");
        int *p = 0;
        fwrite("a", 1, p, f); /* 第三个实参应为 size_t，此处为 int* */
    }

    /* 违反约束「fwrite 返回 size_t，调用需 4 个实参」：
     * 实参个数不足，应报错（too few arguments to function 'fwrite'）。 */
    {
        FILE *f = fopen("x", "wb");
        fwrite("a", 1, f);    /* 缺少第 4 个实参 */
    }

    /* 违反约束「fwrite 调用需 4 个实参」：
     * 实参个数过多，应报错（too many arguments to function 'fwrite'）。 */
    {
        FILE *f = fopen("x", "wb");
        fwrite("a", 1, 1, f, 0); /* 多出第 5 个实参 */
    }

    /* 违反约束「fwrite 的返回值为 size_t，不可作为左值赋值」：
     * 对函数调用结果赋值，应报错（lvalue required as left operand of assignment）。 */
    {
        FILE *f = fopen("x", "wb");
        fwrite("a", 1, 1, f) = 0; /* 函数返回值非左值 */
    }

#endif /* 负向测试结束 */

    return 0;
}