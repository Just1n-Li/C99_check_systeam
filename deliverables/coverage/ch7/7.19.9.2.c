/*
 * 测试条款：C99 7.19.9.2  The fseek function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明
 *   [2] 设置文件位置指示器；读写错误时设置错误指示器并失败
 *   [3] 二进制流：SEEK_SET / SEEK_CUR / SEEK_END 语义
 *   [4] 文本流：offset 必须为 0 或先前 ftell 的返回值，且 whence 必须为 SEEK_SET
 *   [5] 成功调用撤销 ungetc 效果、清除 EOF 指示器、建立新位置；更新流下次操作可为输入或输出
 *   [6] 返回值：仅当请求无法满足时返回非零
 *   Forward references: ftell (7.19.9.4)
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型：fseek 返回 int，参数为 (FILE*, long int, int) */
    {
        int (*fp)(FILE *, long int, int) = fseek;
        assert(fp != NULL);
    }

    /* [3] 二进制流：SEEK_SET / SEEK_CUR / SEEK_END 语义 */
    {
        const char *path = "c99_7_19_9_2_bin.dat";
        FILE *f = fopen(path, "wb+");
        assert(f != NULL);

        /* 写入 "ABCDEFGHIJ" (10 字节) */
        assert(fwrite("ABCDEFGHIJ", 1, 10, f) == 10);
        assert(fflush(f) == 0);

        /* SEEK_SET: 从文件开头加 offset */
        assert(fseek(f, 0, SEEK_SET) == 0);          /* [6] 可满足 -> 返回 0 */
        assert(fgetc(f) == 'A');

        assert(fseek(f, 3, SEEK_SET) == 0);          /* 位置 = 0 + 3 */
        assert(fgetc(f) == 'D');

        /* SEEK_CUR: 从当前位置加 offset */
        assert(fseek(f, 0, SEEK_CUR) == 0);          /* 位置不变 */
        assert(fgetc(f) == 'E');

        assert(fseek(f, 2, SEEK_CUR) == 0);          /* 位置 = 5 + 2 = 7 */
        assert(fgetc(f) == 'H');

        /* SEEK_END: 从文件末尾加 offset（二进制流需有意义地支持） */
        assert(fseek(f, 0, SEEK_END) == 0);          /* 位置 = 10 */
        assert(fgetc(f) == EOF);                     /* 已在末尾 */

        assert(fseek(f, -3, SEEK_END) == 0);         /* 位置 = 10 - 3 = 7 */
        assert(fgetc(f) == 'H');

        /* [2] 成功 fseek 后位置指示器被设置：再读一个字符 */
        assert(fseek(f, 1, SEEK_SET) == 0);
        assert(fgetc(f) == 'B');

        fclose(f);
        remove(path);
    }

    /* [4] 文本流：offset 为 0 或先前 ftell 的返回值，且 whence 必须为 SEEK_SET */
    {
        const char *path = "c99_7_19_9_2_txt.dat";
        FILE *f = fopen(path, "w+");
        assert(f != NULL);

        assert(fputs("hello world\n", f) >= 0);
        assert(fflush(f) == 0);

        /* offset == 0, whence == SEEK_SET */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fgetc(f) == 'h');

        /* 读取若干字符后，用 ftell 取得位置，再用该值 fseek 回去 */
        long pos = ftell(f);                         /* Forward reference: ftell */
        assert(pos >= 0);
        assert(fgetc(f) == 'e');
        assert(fgetc(f) == 'l');

        assert(fseek(f, pos, SEEK_SET) == 0);        /* offset 为先前 ftell 的返回值 */
        assert(fgetc(f) == 'e');                     /* 回到 pos 处 */

        fclose(f);
        remove(path);
    }

    /* [5] 成功 fseek 撤销 ungetc 效果、清除 EOF 指示器、建立新位置 */
    {
        const char *path = "c99_7_19_9_2_ungetc.dat";
        FILE *f = fopen(path, "w+");
        assert(f != NULL);

        assert(fputs("XYZ", f) >= 0);
        assert(fflush(f) == 0);

        /* 读到末尾，设置 EOF 指示器 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fgetc(f) == 'X');
        assert(fgetc(f) == 'Y');
        assert(fgetc(f) == 'Z');
        assert(fgetc(f) == EOF);                     /* EOF 指示器被设置 */

        /* ungetc 一个字符 */
        assert(ungetc('Q', f) == 'Q');

        /* 成功 fseek 应撤销 ungetc 效果并清除 EOF 指示器 */
        assert(fseek(f, 0, SEEK_SET) == 0);

        /* 清除 EOF 指示器：现在应能正常读取，而不是立即 EOF */
        assert(fgetc(f) == 'X');
        assert(fgetc(f) == 'Y');
        assert(fgetc(f) == 'Z');
        assert(fgetc(f) == EOF);

        fclose(f);
        remove(path);
    }

    /* [5] 更新流：成功 fseek 后，下一次操作可为输入或输出 */
    {
        const char *path = "c99_7_19_9_2_update.dat";
        FILE *f = fopen(path, "w+");                 /* 更新流 */
        assert(f != NULL);

        assert(fputs("12345", f) >= 0);
        assert(fflush(f) == 0);

        /* 先输出，再 fseek，然后输入 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fgetc(f) == '1');                     /* 输入操作 */

        /* 再 fseek，然后输出 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fputc('9', f) == '9');                /* 输出操作 */

        assert(fflush(f) == 0);
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fgetc(f) == '9');                     /* 验证写入生效 */

        fclose(f);
        remove(path);
    }

    /* [6] 返回值：仅当请求无法满足时返回非零 */
    {
        const char *path = "c99_7_19_9_2_ret.dat";
        FILE *f = fopen(path, "wb+");
        assert(f != NULL);

        assert(fwrite("abc", 1, 3, f) == 3);
        assert(fflush(f) == 0);

        /* 可满足的请求 -> 返回 0 */
        assert(fseek(f, 0, SEEK_SET) == 0);
        assert(fseek(f, 1, SEEK_SET) == 0);
        assert(fseek(f, 0, SEEK_END) == 0);

        /* 无法满足的请求（负位置）-> 返回非零 */
        assert(fseek(f, -1, SEEK_SET) != 0);

        fclose(f);
        remove(path);
    }

    /* [2] 错误指示器：对只读流进行写操作后 fseek 失败并设置错误指示器 */
    {
        const char *path = "c99_7_19_9_2_err.dat";
        FILE *fw = fopen(path, "w");
        assert(fw != NULL);
        assert(fputs("data", fw) >= 0);
        fclose(fw);

        FILE *fr = fopen(path, "r");                 /* 只读 */
        assert(fr != NULL);

        /* 对只读流写 -> 设置错误指示器 */
        (void)fputc('x', fr);
        assert(ferror(fr) != 0);

        /* 错误发生后 fseek 应失败（返回非零） */
        assert(fseek(fr, 0, SEEK_SET) != 0);

        fclose(fr);
        remove(path);
    }

    printf("C99 7.19.9.2 fseek: all positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「fseek 的第一个参数类型为 FILE *」：
     * 传入 int 而非 FILE*，gcc -std=c99 应报错
     * （incompatible type for argument 1 of 'fseek'） */
    {
        int not_a_file = 0;
        fseek(not_a_file, 0, SEEK_SET);
    }

    /* 违反约束「fseek 的第二个参数类型为 long int」：
     * 传入结构体类型，gcc -std=c99 应报错
     * （incompatible type for argument 2 of 'fseek'） */
    {
        struct S { int x; } s;
        FILE *f = 0;
        fseek(f, s, SEEK_SET);
    }

    /* 违反约束「fseek 的第三个参数类型为 int」：
     * 传入指针类型，gcc -std=c99 应报错
     * （incompatible type for argument 3 of 'fseek'） */
    {
        FILE *f = 0;
        int *p = 0;
        fseek(f, 0, p);
    }

    /* 违反约束「fseek 需要 3 个实参」：
     * 实参个数不足，gcc -std=c99 应报错
     * （too few arguments to function 'fseek'） */
    {
        FILE *f = 0;
        fseek(f, 0);
    }

    /* 违反约束「fseek 需要 3 个实参」：
     * 实参个数过多，gcc -std=c99 应报错
     * （too many arguments to function 'fseek'） */
    {
        FILE *f = 0;
        fseek(f, 0, SEEK_SET, 0);
    }

    /* 违反约束「fseek 返回 int，不能作为左值被赋值」：
     * 对函数调用结果赋值，gcc -std=c99 应报错
     * （lvalue required as left operand of assignment） */
    {
        FILE *f = 0;
        fseek(f, 0, SEEK_SET) = 1;
    }

    /* 违反约束「fseek 的返回值类型为 int，不能用于需要结构体的上下文」：
     * 将返回值赋给结构体，gcc -std=c99 应报错
     * （incompatible types when assigning to type 'struct S' from type 'int'） */
    {
        struct S { int x; } s;
        FILE *f = 0;
        s = fseek(f, 0, SEEK_SET);
    }

#endif /* 负向测试结束 */

    return 0;
}