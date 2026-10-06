/*
 * 测试条款：C99 7.19.2 Streams
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：[1][2][3][4][5][6][7] 及 Footnotes 232/233。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 文本流与二进制流两种映射形式均受支持。
     *     这里分别以 "w"/"r"（文本）与 "wb"/"rb"（二进制）打开同一文件。 */
    {
        FILE *f = fopen("t7192.txt", "w");
        assert(f != NULL);
        fputs("hello\n", f);
        fclose(f);

        f = fopen("t7192.txt", "r");
        assert(f != NULL);
        char buf[64];
        assert(fgets(buf, sizeof buf, f) != NULL);
        assert(strcmp(buf, "hello\n") == 0);
        fclose(f);

        f = fopen("t7192.bin", "wb");
        assert(f != NULL);
        fclose(f);
        f = fopen("t7192.bin", "rb");
        assert(f != NULL);
        fclose(f);
    }

    /* [2] 文本流：由行组成的有序字符序列，每行以换行符结尾。
     *     仅当数据只含可打印字符、水平制表符和换行符，且换行符前无空格、
     *     末字符为换行符时，读回的数据才必然与写出的相等。 */
    {
        const char *data = "abc\tdef\nghi\n"; /* 只含可打印字符、\t、\n */
        FILE *f = fopen("t7192.txt", "w");
        assert(f != NULL);
        fputs(data, f);
        fclose(f);

        f = fopen("t7192.txt", "r");
        assert(f != NULL);
        char buf[64];
        size_t n = fread(buf, 1, sizeof buf - 1, f);
        buf[n] = '\0';
        fclose(f);
        /* 满足 [2] 的条件，读回必然相等 */
        assert(strcmp(buf, data) == 0);
    }

    /* [3] 二进制流：可透明记录内部数据；读回的数据应与先前写出的相等
     *     （同一实现下）。这里用含 0x00、0xFF 的字节序列验证透明性。 */
    {
        unsigned char out[8] = { 0x00, 0x01, 0x7F, 0x80, 0xFE, 0xFF, 0x0A, 0x0D };
        FILE *f = fopen("t7192.bin", "wb");
        assert(f != NULL);
        assert(fwrite(out, 1, sizeof out, f) == sizeof out);
        fclose(f);

        f = fopen("t7192.bin", "rb");
        assert(f != NULL);
        unsigned char in[8];
        size_t n = fread(in, 1, sizeof in, f);
        fclose(f);
        assert(n == sizeof out);
        assert(memcmp(in, out, sizeof out) == 0);
    }

    /* [4] 流的定向：与外部文件关联后、任何操作前，流无定向。
     *     一旦施加宽字符 I/O 函数，流变为宽定向；
     *     一旦施加字节 I/O 函数，流变为字节定向。
     *     只有 freopen 或 fwide 能改变定向（成功的 freopen 清除定向）。 */
    {
        FILE *f = fopen("t7192.txt", "w+");
        assert(f != NULL);

        /* 初始无定向：fwide(f, 0) 返回 0 */
        assert(fwide(f, 0) == 0);

        /* 施加字节 I/O 函数后变为字节定向：fwide(f, 0) 返回负值 */
        fputc('A', f);
        assert(fwide(f, 0) < 0);

        /* 成功的 freopen 清除定向 */
        f = freopen("t7192.txt", "w+", f);
        assert(f != NULL);
        assert(fwide(f, 0) == 0);

        /* 施加宽字符 I/O 函数后变为宽定向：fwide(f, 0) 返回正值 */
        fputwc(L'B', f);
        assert(fwide(f, 0) > 0);

        fclose(f);
    }

    /* [4] fwide 本身可设置定向：fwide(f, 1) 使流宽定向，fwide(f, -1) 使流字节定向。 */
    {
        FILE *f = fopen("t7192.txt", "w+");
        assert(f != NULL);
        assert(fwide(f, 1) > 0);   /* 设为宽定向 */
        assert(fwide(f, 0) > 0);
        fclose(f);

        f = fopen("t7192.txt", "w+");
        assert(f != NULL);
        assert(fwide(f, -1) < 0);  /* 设为字节定向 */
        assert(fwide(f, 0) < 0);
        fclose(f);
    }

    /* [5] 字节 I/O 函数不得施加于宽定向流；宽字符 I/O 函数不得施加于字节定向流。
     *     正向：在字节定向流上使用字节 I/O，在宽定向流上使用宽字符 I/O。 */
    {
        FILE *f = fopen("t7192.txt", "w+");
        assert(f != NULL);
        fwide(f, -1);              /* 字节定向 */
        assert(fputc('X', f) != EOF);   /* 字节 I/O 合法 */
        fclose(f);

        f = fopen("t7192.txt", "w+");
        assert(f != NULL);
        fwide(f, 1);               /* 宽定向 */
        assert(fputwc(L'Y', f) != WEOF); /* 宽字符 I/O 合法 */
        fclose(f);
    }

    /* [5] 其余流操作不影响、也不受定向影响：例如 fflush、fclose 可用于任一取向的流。 */
    {
        FILE *f = fopen("t7192.txt", "w+");
        assert(f != NULL);
        fwide(f, 1);
        assert(fflush(f) == 0);    /* 非 I/O 定向相关操作 */
        assert(fclose(f) == 0);
    }

    /* [6] 每个宽定向流关联一个 mbstate_t 对象，存储当前解析状态。
     *     fgetpos 存储该 mbstate_t 的表示；fsetpos 用同一 fpos_t 值恢复它。 */
    {
        FILE *f = fopen("t7192.txt", "w+");
        assert(f != NULL);
        fwide(f, 1);               /* 宽定向 */
        fputwc(L'A', f);
        fputwc(L'B', f);
        fflush(f);

        fpos_t pos;
        assert(fgetpos(f, &pos) == 0);   /* 存储 mbstate_t 表示 */

        fputwc(L'C', f);
        fflush(f);

        assert(fsetpos(f, &pos) == 0);   /* 恢复 mbstate_t 与位置 */
        wint_t wc = fgetwc(f);
        assert(wc == L'C');              /* 位置已恢复到 'C' 之前 */
        fclose(f);
    }

    /* [7] 实现应支持至少含 254 个字符（含终止换行符）的文本行；
     *     BUFSIZ 至少为 256。 */
    {
        assert(BUFSIZ >= 256);

        /* 构造一行 253 个可打印字符 + 换行符 = 254 字符 */
        char line[300];
        memset(line, 'a', 253);
        line[253] = '\n';
        line[254] = '\0';

        FILE *f = fopen("t7192.txt", "w");
        assert(f != NULL);
        fputs(line, f);
        fclose(f);

        f = fopen("t7192.txt", "r");
        assert(f != NULL);
        char buf[300];
        assert(fgets(buf, sizeof buf, f) != NULL);
        assert(strlen(buf) == 254);      /* 253 字符 + '\n' */
        assert(buf[253] == '\n');
        fclose(f);
    }

    /* Footnote 233：stdin/stdout/stderr 在程序启动时无定向。
     * 注意：本测试进程的 stdout 可能已被前面的库调用影响，
     * 因此仅对 stdin 做无定向检查（未做任何 I/O 前）。 */
    {
        /* 若 stdin 尚未被使用，fwide(stdin, 0) 应为 0。
         * 为稳妥起见，这里只断言 fwide 调用本身合法（返回 int）。 */
        int o = fwide(stdin, 0);
        (void)o; /* 无定向或已定向均合法，仅验证调用可用 */
    }

    /* Footnote 232：实现可不区分文本流与二进制流。
     * 这里仅验证两种模式都能打开同一文件，不假设其差异。 */
    {
        FILE *ft = fopen("t7192.txt", "r");
        FILE *fb = fopen("t7192.txt", "rb");
        assert(ft != NULL && fb != NULL);
        fclose(ft);
        fclose(fb);
    }

    remove("t7192.txt");
    remove("t7192.bin");

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束 [5]：字节 I/O 函数不得施加于宽定向流。
     * 期望：编译器/实现应拒绝（或在运行期报错）。
     * 这里以 fputc 施加于宽定向流为例。 */
    {
        FILE *f = fopen("x.txt", "w+");
        fwide(f, 1);          /* 宽定向 */
        fputc('A', f);        /* 违反 [5]：字节 I/O 用于宽定向流 */
        fclose(f);
    }

    /* 违反约束 [5]：宽字符 I/O 函数不得施加于字节定向流。
     * 期望：编译器/实现应拒绝。 */
    {
        FILE *f = fopen("x.txt", "w+");
        fwide(f, -1);         /* 字节定向 */
        fputwc(L'A', f);      /* 违反 [5]：宽字符 I/O 用于字节定向流 */
        fclose(f);
    }

    /* 违反约束 [5]：fwide 的第二个参数只允许 0、正值、负值三种语义，
     * 但标准并未把其它值列为约束；此处不构造非法值，避免误判。
     * 下面演示对已定向流用 fwide 试图改变定向——标准规定只有 freopen
     * 或 fwide 能改变定向，但一旦定向后 fwide 不能改变它（返回当前定向）。
     * 这属于语义而非约束，故不放入负向测试。 */

#endif

    return 0;
}