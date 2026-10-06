/*
 * 测试条款：C99 7.19.5.3  The fopen function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明
 *   [2] 打开文件并关联流
 *   [3] mode 字符串的合法取值
 *   [4] 读模式打开不存在的文件失败
 *   [5] 追加模式：写总在文件末尾
 *   [6] 更新模式：输入/输出之间需要 fflush 或定位函数
 *   [7] 打开时清错误/EOF 指示器
 *   [8] 成功返回流指针，失败返回 NULL
 *   Footnote 237：mode 首字符匹配后剩余字符可被忽略
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* 用于测试的临时文件名 */
#define TMPFILE "c99_7_19_5_3_tmp.txt"
#define TMPFILE2 "c99_7_19_5_3_tmp2.txt"

static void cleanup(void)
{
    remove(TMPFILE);
    remove(TMPFILE2);
}

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型：fopen 返回 FILE*，两个参数均为 const char * restrict */
    {
        FILE *(*fp)(const char * restrict, const char * restrict) = fopen;
        assert(fp != NULL);
    }

    cleanup();

    /* [2] 打开文件并关联流：写模式创建文件 */
    {
        FILE *f = fopen(TMPFILE, "w");
        assert(f != NULL);                 /* [8] 成功返回非空指针 */
        assert(fputs("hello\n", f) >= 0);
        assert(fclose(f) == 0);
    }

    /* [3] mode 合法取值：r / w / a / rb / wb / ab / r+ / w+ / a+ /
     *     r+b / rb+ / w+b / wb+ / a+b / ab+ 均应被接受 */
    {
        const char *modes[] = {
            "r", "w", "a", "rb", "wb", "ab",
            "r+", "w+", "a+",
            "r+b", "rb+", "w+b", "wb+", "a+b", "ab+"
        };
        size_t i;
        for (i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
            FILE *f = fopen(TMPFILE, modes[i]);
            assert(f != NULL);             /* [8] 合法 mode 应成功 */
            assert(fclose(f) == 0);
        }
    }

    /* [4] 读模式打开不存在的文件应失败，返回 NULL */
    {
        FILE *f = fopen("c99_no_such_file_xyz_12345.txt", "r");
        assert(f == NULL);                 /* [8] 失败返回空指针 */
    }

    /* [5] 追加模式：所有写被强制到当前文件末尾 */
    {
        FILE *f = fopen(TMPFILE, "w");
        assert(f != NULL);
        assert(fputs("AAA", f) >= 0);
        assert(fclose(f) == 0);

        f = fopen(TMPFILE, "a");
        assert(f != NULL);
        /* 即使 fseek 到开头，追加模式写仍应落在末尾 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fputs("BBB", f) >= 0);
        assert(fclose(f) == 0);

        /* 读回验证内容为 "AAABBB" */
        f = fopen(TMPFILE, "r");
        assert(f != NULL);
        char buf[64];
        size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        assert(strcmp(buf, "AAABBB") == 0);
        assert(fclose(f) == 0);
    }

    /* [6] 更新模式：输入/输出之间需 fflush 或定位函数 */
    {
        FILE *f = fopen(TMPFILE, "w+");
        assert(f != NULL);
        /* 写 -> fflush -> 读 */
        assert(fputs("XYZ", f) >= 0);
        assert(fflush(f) == 0);
        assert(fseek(f, 0, SEEK_SET) == 0);
        int c = fgetc(f);
        assert(c == 'X');
        /* 读 -> 定位函数 -> 写 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fputc('Q', f) == 'Q');
        assert(fclose(f) == 0);

        f = fopen(TMPFILE, "r");
        assert(f != NULL);
        char buf[64];
        size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        buf[n] = '\0';
        assert(buf[0] == 'Q');
        assert(fclose(f) == 0);
    }

    /* [7] 打开时清错误/EOF 指示器：先制造 EOF，再重新打开应清除 */
    {
        FILE *f = fopen(TMPFILE, "r");
        assert(f != NULL);
        /* 读到 EOF，设置 EOF 指示器 */
        while (fgetc(f) != EOF) { }
        assert(feof(f) != 0);
        assert(fclose(f) == 0);

        /* 重新打开，EOF 指示器应被清除 */
        f = fopen(TMPFILE, "r");
        assert(f != NULL);
        assert(feof(f) == 0);              /* [7] 打开时清 EOF 指示器 */
        assert(ferror(f) == 0);            /* [7] 打开时清错误指示器 */
        assert(fclose(f) == 0);
    }

    /* [8] 返回值：成功非空，失败为空 */
    {
        FILE *ok = fopen(TMPFILE, "r");
        assert(ok != NULL);
        assert(fclose(ok) == 0);

        FILE *bad = fopen("c99_definitely_missing_file_98765.txt", "r");
        assert(bad == NULL);
    }

    /* Footnote 237：mode 首字符匹配后，剩余字符可被忽略或用于选择文件种类。
     * 这里只验证实现不会因额外字符而崩溃（行为由实现定义，不做强断言）。 */
    {
        FILE *f = fopen(TMPFILE, "r");
        assert(f != NULL);
        assert(fclose(f) == 0);
    }

    cleanup();
    printf("C99 7.19.5.3 fopen: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束 [1]：fopen 的第一个参数类型为 const char *，
     * 传入 int 类型实参，gcc -std=c99 应报错
     * （incompatible integer to pointer conversion / 参数类型不匹配）。 */
    {
        FILE *f = fopen(42, "r");
        (void)f;
    }

    /* 违反约束 [1]：fopen 的第二个参数类型为 const char *，
     * 传入 int 类型实参，应报错。 */
    {
        FILE *f = fopen("x", 7);
        (void)f;
    }

    /* 违反约束 [1]：fopen 需要两个参数，只传一个应报错
     * （too few arguments to function 'fopen'）。 */
    {
        FILE *f = fopen("x");
        (void)f;
    }

    /* 违反约束 [1]：fopen 需要两个参数，传三个应报错
     * （too many arguments to function 'fopen'）。 */
    {
        FILE *f = fopen("x", "r", "extra");
        (void)f;
    }

    /* 违反约束 [1]：fopen 返回 FILE *，将其赋给不兼容的指针类型
     * 在严格模式下应给出警告/错误（-Werror 下报错）。 */
    {
        int *p = fopen("x", "r");
        (void)p;
    }

#endif

    return 0;
}