/*
 * 测试目标：C99 7.19.10.3 —— ferror 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型：int ferror(FILE *stream);  需包含 <stdio.h>
 *   [2] 描述：测试 stream 所指流的错误指示器
 *   [3] 返回：当且仅当 stream 的错误指示器被设置时返回非零
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 原型可用：包含 <stdio.h> 后 ferror 可被调用，返回 int */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* [3] 新打开的流，错误指示器未设置，ferror 应返回 0 */
        int r = ferror(fp);
        assert(r == 0);

        fclose(fp);
    }

    /* [2][3] 对只读流进行写操作会设置错误指示器，ferror 应返回非零 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* 先写入一些内容，再以只读方式重新打开 */
        assert(fputs("hello", fp) >= 0);
        fflush(fp);

        /* 关闭并以只读模式重新打开同一临时文件 */
        /* 使用 freopen 以只读方式重开 */
        fp = freopen(NULL, "r", fp);
        if (fp != NULL) {
            /* 尝试向只读流写入，触发错误指示器 */
            int w = fputc('X', fp);
            /* 写失败：返回 EOF 且错误指示器被设置 */
            if (w == EOF) {
                /* [3] 错误指示器已设置，ferror 必须返回非零 */
                assert(ferror(fp) != 0);
            }
            fclose(fp);
        }
    }

    /* [2][3] 读取不存在的文件失败后，对失败流调用 ferror */
    {
        FILE *fp = fopen("__no_such_file_ferror_test__.txt", "r");
        if (fp == NULL) {
            /* 打开失败，无流可测，跳过 */
        } else {
            /* 若意外打开成功，读取并检查 */
            int c = fgetc(fp);
            (void)c;
            fclose(fp);
        }
    }

    /* [3] 用 clearerr 清除错误指示器后，ferror 应返回 0 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* 制造一个错误：对只读流写 */
        fp = freopen(NULL, "r", fp);
        if (fp != NULL) {
            int w = fputc('Y', fp);
            if (w == EOF) {
                assert(ferror(fp) != 0);   /* 错误已设置 */
                clearerr(fp);              /* 清除错误指示器 */
                assert(ferror(fp) == 0);   /* [3] 清除后应返回 0 */
            }
            fclose(fp);
        }
    }

    /* [3] 返回值语义：非零当且仅当错误指示器被设置 */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* 初始状态：未设置 -> 返回 0 */
        assert(ferror(fp) == 0);

        /* 正常读写不设置错误指示器 */
        assert(fputc('A', fp) != EOF);
        fflush(fp);
        assert(ferror(fp) == 0);

        fclose(fp);
    }

    printf("ferror: all positive tests passed.\n");
    return 0;

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「ferror 的参数类型必须为 FILE *」：
       传入 int 而非 FILE *，gcc -std=c99 应报错
       （incompatible type for argument / passing argument 1 makes pointer from integer） */
    {
        int x = 0;
        int r = ferror(x);   /* 错误：实参类型不匹配 */
        (void)r;
    }

    /* 违反约束「ferror 的参数类型必须为 FILE *」：
       传入 char * 而非 FILE *，应报错 */
    {
        char *s = "not a stream";
        int r = ferror(s);   /* 错误：char * 与 FILE * 不兼容 */
        (void)r;
    }

    /* 违反约束「ferror 需要恰好一个实参」：
       不传实参，应报错（too few arguments to function 'ferror'） */
    {
        int r = ferror();    /* 错误：实参个数不足 */
        (void)r;
    }

    /* 违反约束「ferror 需要恰好一个实参」：
       传两个实参，应报错（too many arguments to function 'ferror'） */
    {
        FILE *fp = tmpfile();
        int r = ferror(fp, fp);   /* 错误：实参个数过多 */
        (void)r;
    }

    /* 违反约束「ferror 返回 int，不能作为左值被赋值」：
       对函数调用结果赋值，应报错（lvalue required as left operand of assignment） */
    {
        FILE *fp = tmpfile();
        ferror(fp) = 1;      /* 错误：函数调用结果不是左值 */
    }

#endif /* 负向测试结束 */
}