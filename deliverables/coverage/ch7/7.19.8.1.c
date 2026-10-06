/*
 * 测试条款：C99 7.19.8.1  The fread function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，gcc -std=c99 应编译报错。
 *
 * 覆盖段落：
 *   [1] 函数原型（restrict 限定、返回 size_t、参数类型）
 *   [2] 语义：读取至多 nmemb 个元素、每个元素 size 字节、按 fgetc 顺序
 *       存入 unsigned char 覆盖数组、文件位置指示器前进、部分元素值不确定
 *   [3] 返回值：成功读取的元素个数；size 或 nmemb 为 0 时返回 0 且
 *       数组内容与流状态不变
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型兼容 */
static size_t (*fp_fread)(void * restrict, size_t, size_t, FILE * restrict) = fread;

static void make_file(const char *name, const void *data, size_t n)
{
    FILE *f = fopen(name, "wb");
    assert(f != NULL);
    if (n) assert(fwrite(data, 1, n, f) == n);
    assert(fclose(f) == 0);
}

int main(void)
{
    const char *path = "fread_test.bin";

    /* ---------- [2][3] 基本读取：整元素读取，返回元素个数 ---------- */
    {
        unsigned char src[10] = {0,1,2,3,4,5,6,7,8,9};
        unsigned char dst[10];
        memset(dst, 0xAA, sizeof dst);
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        size_t n = fread(dst, 1, 10, f);
        assert(n == 10);                       /* [3] 返回成功读取的元素数 */
        assert(memcmp(dst, src, 10) == 0);     /* [2] 内容按顺序存入 */
        assert(fclose(f) == 0);
    }

    /* ---------- [2] size > 1：按元素为单位读取 ---------- */
    {
        int src[4] = { 0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00 };
        int dst[4] = {0,0,0,0};
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        size_t n = fread(dst, sizeof(int), 4, f);
        assert(n == 4);
        assert(memcmp(dst, src, sizeof src) == 0);
        assert(fclose(f) == 0);
    }

    /* ---------- [2] 部分元素读取：返回完整元素个数 ---------- */
    {
        unsigned char src[7] = {1,2,3,4,5,6,7};
        unsigned char dst[8];
        memset(dst, 0xEE, sizeof dst);
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        /* 请求 4 个元素，每个 2 字节，但文件只有 7 字节 => 只能读 3 个完整元素 */
        size_t n = fread(dst, 2, 4, f);
        assert(n == 3);                        /* [3] 部分元素不计入返回值 */
        assert(dst[0]==1 && dst[1]==2 && dst[2]==3 && dst[3]==4 && dst[4]==5 && dst[5]==6);
        assert(fclose(f) == 0);
    }

    /* ---------- [2] 文件位置指示器前进 ---------- */
    {
        unsigned char src[8] = {10,20,30,40,50,60,70,80};
        unsigned char dst[4];
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        size_t n = fread(dst, 1, 4, f);
        assert(n == 4);
        assert(dst[0]==10 && dst[1]==20 && dst[2]==30 && dst[3]==40);
        /* [2] 位置指示器前进 4 字节：下一次读取应从第 5 字节开始 */
        long pos = ftell(f);
        assert(pos == 4);
        n = fread(dst, 1, 4, f);
        assert(n == 4);
        assert(dst[0]==50 && dst[1]==60 && dst[2]==70 && dst[3]==80);
        assert(fclose(f) == 0);
    }

    /* ---------- [3] size == 0：返回 0，数组与流状态不变 ---------- */
    {
        unsigned char src[4] = {1,2,3,4};
        unsigned char dst[4];
        memset(dst, 0x5A, sizeof dst);
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        long pos_before = ftell(f);
        size_t n = fread(dst, 0, 4, f);        /* size == 0 */
        assert(n == 0);                        /* [3] 返回 0 */
        assert(dst[0]==0x5A && dst[1]==0x5A && dst[2]==0x5A && dst[3]==0x5A); /* 数组不变 */
        assert(ftell(f) == pos_before);        /* 流状态不变 */
        assert(fclose(f) == 0);
    }

    /* ---------- [3] nmemb == 0：返回 0，数组与流状态不变 ---------- */
    {
        unsigned char src[4] = {1,2,3,4};
        unsigned char dst[4];
        memset(dst, 0x3C, sizeof dst);
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        long pos_before = ftell(f);
        size_t n = fread(dst, 1, 0, f);        /* nmemb == 0 */
        assert(n == 0);                        /* [3] 返回 0 */
        assert(dst[0]==0x3C && dst[1]==0x3C && dst[2]==0x3C && dst[3]==0x3C); /* 数组不变 */
        assert(ftell(f) == pos_before);        /* 流状态不变 */
        assert(fclose(f) == 0);
    }

    /* ---------- [3] 空文件：返回 0 ---------- */
    {
        unsigned char dst[4];
        memset(dst, 0x77, sizeof dst);
        make_file(path, NULL, 0);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        size_t n = fread(dst, 1, 4, f);
        assert(n == 0);                        /* EOF 时返回 0 */
        assert(fclose(f) == 0);
    }

    /* ---------- [1] restrict 限定：两个 restrict 指针可指向不同对象 ---------- */
    {
        unsigned char src[3] = {7,8,9};
        unsigned char dst[3];
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        size_t n = fread(dst, 1, 3, f);        /* ptr 与 stream 为不同对象 */
        assert(n == 3);
        assert(dst[0]==7 && dst[1]==8 && dst[2]==9);
        assert(fclose(f) == 0);
    }

    /* ---------- [1] 函数指针调用（验证原型兼容） ---------- */
    {
        unsigned char src[2] = {0xAB, 0xCD};
        unsigned char dst[2] = {0,0};
        make_file(path, src, sizeof src);

        FILE *f = fopen(path, "rb");
        assert(f != NULL);
        size_t n = fp_fread(dst, 1, 2, f);
        assert(n == 2);
        assert(dst[0]==0xAB && dst[1]==0xCD);
        assert(fclose(f) == 0);
    }

    remove(path);
    printf("All positive tests for C99 7.19.8.1 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fread 的第一个参数类型为 void *」：
 * 传入非指针类型（如 int）应报错。
 * 期望：gcc -std=c99 报 incompatible type / passing argument 1 ... */
{
    FILE *f = fopen("x", "rb");
    int not_a_pointer = 0;
    fread(not_a_pointer, 1, 1, f);   /* 错误：第一个实参不是指针 */
}

/* 违反约束「fread 的第二个参数类型为 size_t」：
 * 传入结构体类型（非算术/非整数类型）应报错。
 * 期望：gcc -std=c99 报 incompatible type for argument 2 */
{
    struct S { int a; } s;
    FILE *f = fopen("x", "rb");
    char buf[4];
    fread(buf, s, 1, f);             /* 错误：size 参数为结构体 */
}

/* 违反约束「fread 的第三个参数类型为 size_t」：
 * 传入指针类型应报错。
 * 期望：gcc -std=c99 报 incompatible type for argument 3 */
{
    FILE *f = fopen("x", "rb");
    char buf[4];
    int *p = 0;
    fread(buf, 1, p, f);             /* 错误：nmemb 参数为指针 */
}

/* 违反约束「fread 的第四个参数类型为 FILE *」：
 * 传入 int 应报错。
 * 期望：gcc -std=c99 报 incompatible type for argument 4 */
{
    char buf[4];
    int not_a_file = 0;
    fread(buf, 1, 1, not_a_file);    /* 错误：stream 参数不是 FILE * */
}

/* 违反约束「fread 返回 size_t，不能赋给不兼容的指针类型」：
 * 期望：gcc -std=c99 报 initialization makes pointer from integer */
{
    FILE *f = fopen("x", "rb");
    char buf[4];
    int *p = fread(buf, 1, 1, f);    /* 错误：size_t 赋给 int * */
}

/* 违反约束「fread 需要 4 个实参」：
 * 实参个数不足应报错。
 * 期望：gcc -std=c99 报 too few arguments to function 'fread' */
{
    FILE *f = fopen("x", "rb");
    char buf[4];
    fread(buf, 1, 1);                /* 错误：缺少 stream 实参 */
}

/* 违反约束「fread 需要 4 个实参」：
 * 实参个数过多应报错。
 * 期望：gcc -std=c99 报 too many arguments to function 'fread' */
{
    FILE *f = fopen("x", "rb");
    char buf[4];
    fread(buf, 1, 1, f, f);          /* 错误：多传一个实参 */
}

#endif /* 负向测试结束 */