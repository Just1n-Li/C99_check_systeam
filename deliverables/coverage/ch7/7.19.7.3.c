/*
 * 测试条款：C99 7.19.7.3  The fputc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型：int fputc(int c, FILE *stream);
 *   [2] 语义：把 c 转换为 unsigned char 后写入 stream 当前位置，并推进位置指示器；
 *            若流不支持定位或为追加模式，则追加到流末尾。
 *   [3] 返回：成功返回写入的字符；写错误时设置流的错误指示器并返回 EOF。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：fputc 的返回类型为 int，参数为 (int, FILE*)。
 *     通过取函数指针类型来静态验证原型。 */
static int (*fp_fputc)(int, FILE *) = fputc;

int main(void)
{
    /* [1] 原型：确认函数指针类型匹配（编译期检查）。 */
    assert(fp_fputc == fputc);

    /* [2] 语义：写入普通文件，字符被写入并推进位置指示器。 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);

        int r1 = fputc('A', f);          /* [3] 成功返回写入的字符 */
        assert(r1 == 'A');

        int r2 = fputc('B', f);
        assert(r2 == 'B');

        int r3 = fputc('C', f);
        assert(r3 == 'C');

        /* [2] 位置指示器应已推进到 3。 */
        long pos = ftell(f);
        assert(pos == 3L);

        /* 回读验证内容与顺序。 */
        assert(fflush(f) == 0);
        rewind(f);
        assert(fgetc(f) == 'A');
        assert(fgetc(f) == 'B');
        assert(fgetc(f) == 'C');
        assert(fgetc(f) == EOF);

        fclose(f);
    }

    /* [2] 语义：c 被转换为 unsigned char。
     *     传入超出 unsigned char 范围的值（如 0x141），
     *     实际写入的应是 (unsigned char)0x141 == 0x41 == 'A'。 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);

        int r = fputc(0x141, f);         /* 低 8 位为 0x41 */
        assert(r == 0x41);               /* [3] 返回写入的字符（转换后的值） */

        assert(fflush(f) == 0);
        rewind(f);
        assert(fgetc(f) == 0x41);
        assert(fgetc(f) == EOF);

        fclose(f);
    }

    /* [2] 语义：追加模式（"a"）下字符被追加到流末尾。
     *     先写入 "XY"，关闭后以追加模式打开，再写 'Z'，
     *     结果应为 "XYZ"。 */
    {
        const char *path = "fputc_append_test.tmp";

        FILE *f = fopen(path, "w");
        assert(f != NULL);
        assert(fputc('X', f) == 'X');
        assert(fputc('Y', f) == 'Y');
        assert(fclose(f) == 0);

        f = fopen(path, "a");            /* 追加模式 */
        assert(f != NULL);
        assert(fputc('Z', f) == 'Z');    /* [2] 追加到末尾 */
        assert(fclose(f) == 0);

        f = fopen(path, "r");
        assert(f != NULL);
        assert(fgetc(f) == 'X');
        assert(fgetc(f) == 'Y');
        assert(fgetc(f) == 'Z');
        assert(fgetc(f) == EOF);
        assert(fclose(f) == 0);

        remove(path);
    }

    /* [3] 返回：写错误时设置流的错误指示器并返回 EOF。
     *     以只读模式打开文件后尝试写入，应触发写错误。 */
    {
        const char *path = "fputc_err_test.tmp";

        FILE *f = fopen(path, "w");
        assert(f != NULL);
        assert(fputc('Q', f) == 'Q');
        assert(fclose(f) == 0);

        f = fopen(path, "r");            /* 只读，不可写 */
        assert(f != NULL);

        errno = 0;
        int r = fputc('W', f);           /* [3] 写错误 */
        assert(r == EOF);                /* 返回 EOF */
        assert(ferror(f) != 0);          /* 错误指示器被设置 */

        fclose(f);
        remove(path);
    }

    /* [3] 返回：成功时返回写入的字符，且不设置错误指示器。 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);

        int r = fputc('m', f);
        assert(r == 'm');
        assert(ferror(f) == 0);          /* 无错误 */

        fclose(f);
    }

    printf("All positive tests for C99 7.19.7.3 fputc passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fputc 的第一个参数必须为 int（算术类型可隐式转换）」：
 * 传入结构体类型，无法转换为 int，gcc -std=c99 应报错。 */
struct S { int x; } s;
FILE *g;
fputc(s, g);

/* 违反约束「fputc 的第二个参数必须为 FILE *」：
 * 传入 int，类型不兼容，gcc -std=c99 应报错。 */
fputc('a', 42);

/* 违反约束「fputc 需要两个实参」：
 * 实参个数不足，gcc -std=c99 应报错。 */
fputc('a');

/* 违反约束「fputc 需要两个实参」：
 * 实参个数过多，gcc -std=c99 应报错。 */
fputc('a', g, 'b');

/* 违反约束「fputc 的返回值类型为 int，不可作为左值赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报错。 */
fputc('a', g) = 1;

/* 违反约束「fputc 的第二个参数为 FILE *，不能传入 void * 之外的不兼容指针」：
 * 传入 char *，与 FILE * 不兼容，gcc -std=c99 应报错。 */
char *cp;
fputc('a', cp);

#endif