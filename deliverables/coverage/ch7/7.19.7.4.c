/*
 * 测试 C99 7.19.7.4 —— fputs 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdio.h>
 *   [2] 语义：把 s 指向的字符串写入 stream，不写入结尾的 '\0'
 *   [3] 返回值：写错误返回 EOF；否则返回非负值
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：包含 <stdio.h> 后 fputs 可被调用，
     *     参数为 (const char * restrict, FILE * restrict)。
     *     这里用函数指针验证签名兼容性（restrict 不改变类型兼容性）。 */
    {
        int (*fp)(const char *restrict, FILE *restrict) = fputs;
        assert(fp != NULL);
    }

    /* [2] 语义：写入字符串内容，且不写入结尾的 '\0'。
     *     用临时文件验证写入的字节序列。 */
    {
        const char *msg = "hello";   /* 5 个字符 + '\0' */
        FILE *f = tmpfile();
        assert(f != NULL);

        int r = fputs(msg, f);       /* [2] 写入 */
        assert(r != EOF);            /* [3] 成功时非负 */
        assert(r >= 0);              /* [3] 非负值 */

        /* 回读文件内容，验证恰好写入 5 字节，且不含 '\0' */
        assert(fflush(f) == 0);
        assert(fseek(f, 0L, SEEK_END) == 0);
        long len = ftell(f);
        assert(len == 5);            /* [2] 结尾 '\0' 未被写入 */

        assert(fseek(f, 0L, SEEK_SET) == 0);
        char buf[16];
        size_t n = fread(buf, 1, sizeof buf, f);
        assert(n == 5);
        assert(memcmp(buf, "hello", 5) == 0);
        /* 确认第 6 个字节不是 '\0'（即 '\0' 未被写入） */
        assert(buf[5] != '\0' || n < 6);

        fclose(f);
    }

    /* [2] 空字符串：只写入 0 个字符（'\0' 不写入） */
    {
        FILE *f = tmpfile();
        assert(f != NULL);
        int r = fputs("", f);
        assert(r != EOF);
        assert(fflush(f) == 0);
        assert(fseek(f, 0L, SEEK_END) == 0);
        assert(ftell(f) == 0);       /* [2] 空串不写入任何字节 */
        fclose(f);
    }

    /* [2] 字符串中含内嵌字符（非 '\0'）应原样写入 */
    {
        const char *msg = "a\nb\tc";  /* 5 个可见字符 */
        FILE *f = tmpfile();
        assert(f != NULL);
        int r = fputs(msg, f);
        assert(r != EOF);
        assert(fflush(f) == 0);
        assert(fseek(f, 0L, SEEK_END) == 0);
        assert(ftell(f) == 5);
        fclose(f);
    }

    /* [3] 返回值非负：多次调用均返回 >= 0 */
    {
        FILE *f = tmpfile();
        assert(f != NULL);
        for (int i = 0; i < 3; i++) {
            int r = fputs("x", f);
            assert(r != EOF);
            assert(r >= 0);
        }
        fclose(f);
    }

    /* [3] 写错误时返回 EOF：对只读流写入应失败并返回 EOF。
     *     用 fopen 以 "r" 打开一个已存在的文件，然后尝试 fputs。 */
    {
        /* 先创建一个文件 */
        const char *path = "fputs_test_tmp.txt";
        FILE *w = fopen(path, "w");
        assert(w != NULL);
        assert(fputs("data", w) != EOF);
        fclose(w);

        /* 以只读方式打开，写入应失败 */
        FILE *r = fopen(path, "r");
        assert(r != NULL);
        int rc = fputs("should fail", r);
        assert(rc == EOF);           /* [3] 写错误返回 EOF */
        fclose(r);

        remove(path);
    }

    /* [2] 写入标准输出流（stdout）应成功，返回非负值 */
    {
        int r = fputs("", stdout);   /* 空串，不产生可见输出 */
        assert(r != EOF);
        assert(r >= 0);
    }

    printf("All positive tests for C99 7.19.7.4 fputs passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「fputs 的第一个参数类型为 const char *」：
     * 传入 int 类型实参，gcc -std=c99 应报错
     * （int 不能隐式转换为 const char *）。 */
    {
        FILE *f = tmpfile();
        int x = 42;
        fputs(x, f);                 /* 错误：第一个实参应为 const char * */
    }

    /* 违反约束「fputs 的第二个参数类型为 FILE *」：
     * 传入 int 类型实参，gcc -std=c99 应报错。 */
    {
        fputs("hi", 0);              /* 错误：第二个实参应为 FILE * */
    }

    /* 违反约束「fputs 需要两个实参」：
     * 实参个数不匹配，gcc -std=c99 应报错。 */
    {
        FILE *f = tmpfile();
        fputs("hi");                 /* 错误：实参太少 */
    }

    /* 违反约束「fputs 需要两个实参」：
     * 实参过多，gcc -std=c99 应报错。 */
    {
        FILE *f = tmpfile();
        fputs("hi", f, f);           /* 错误：实参太多 */
    }

    /* 违反约束「fputs 返回 int，不能作为函数指针赋给不兼容类型」：
     * 将 fputs 赋给返回 void 的函数指针，gcc -std=c99 应报错。 */
    {
        void (*fp)(const char *, FILE *) = fputs;  /* 错误：返回类型不兼容 */
        (void)fp;
    }

#endif /* 负向测试结束 */
}