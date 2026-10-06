/*
 * 测试目标：C99 7.19.5.2  The fflush function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明 int fflush(FILE *stream);
 *   [2] 输出流/更新流（最近操作非输入）时刷新未写数据；否则行为未定义（UB，不作负向测试）。
 *   [3] stream 为 NULL 时刷新所有“行为已定义”的流。
 *   [4] 写错误时置错误指示器并返回 EOF，否则返回 0。
 *   Forward references: fopen (7.19.5.3)
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：fflush 的签名必须是 int fflush(FILE *stream);
 *     通过取函数指针类型来静态验证返回类型与参数类型。 */
static int (*fflush_proto_check)(FILE *) = fflush;

int main(void)
{
    /* [1] 原型存在且可调用，返回类型为 int */
    {
        int (*p)(FILE *) = fflush;
        assert(p == fflush_proto_check);
    }

    /* [2] 对输出流刷新：写入数据后 fflush 应返回 0（成功）。
     *     使用临时文件，验证数据确实被交付给宿主环境（写入文件）。 */
    {
        const char *path = "fflush_test_tmp.txt";
        FILE *fp = fopen(path, "w");   /* Forward reference: fopen (7.19.5.3) */
        assert(fp != NULL);

        assert(fputs("hello fflush\n", fp) >= 0);

        /* [2] 输出流：刷新未写数据 */
        int r = fflush(fp);
        assert(r == 0);                /* [4] 无写错误时返回 0 */

        /* 刷新后数据应已到达文件：用另一个句柄读取验证 */
        {
            FILE *rd = fopen(path, "r");
            char buf[64];
            assert(rd != NULL);
            assert(fgets(buf, sizeof buf, rd) != NULL);
            assert(strcmp(buf, "hello fflush\n") == 0);
            fclose(rd);
        }

        fclose(fp);
        remove(path);
    }

    /* [2] 对更新流（update stream）且最近操作不是输入时刷新。
     *     以 "w+" 打开，先写后 fflush，属于“最近操作非输入”的情形。 */
    {
        const char *path = "fflush_test_upd.txt";
        FILE *fp = fopen(path, "w+");
        assert(fp != NULL);

        assert(fputc('A', fp) != EOF);
        /* 最近操作是输出（非输入），fflush 行为已定义 */
        assert(fflush(fp) == 0);

        /* 回绕读取，确认写入内容 */
        assert(fseek(fp, 0L, SEEK_SET) == 0);
        {
            int c = fgetc(fp);
            assert(c == 'A');
        }
        fclose(fp);
        remove(path);
    }

    /* [3] stream 为 NULL：刷新所有行为已定义的流，应返回 0 */
    {
        /* 先向 stdout 写点东西，再对 NULL 调用 fflush */
        assert(fputs("flush-all\n", stdout) >= 0);
        int r = fflush(NULL);
        assert(r == 0);                /* [4] 无写错误时返回 0 */
    }

    /* [4] 正常情况返回 0：对已打开的输出流多次刷新仍返回 0 */
    {
        const char *path = "fflush_test_ret.txt";
        FILE *fp = fopen(path, "w");
        assert(fp != NULL);
        assert(fflush(fp) == 0);
        assert(fflush(fp) == 0);
        fclose(fp);
        remove(path);
    }

    /* [4] 写错误时置错误指示器并返回 EOF。
     *     以只读模式打开文件后尝试写入会失败，随后 fflush 应返回 EOF
     *     并置该流的错误指示器（ferror 为真）。 */
    {
        const char *path = "fflush_test_err.txt";
        FILE *fp = fopen(path, "w");
        assert(fp != NULL);
        assert(fputs("x", fp) >= 0);
        fclose(fp);

        FILE *ro = fopen(path, "r");   /* 只读流 */
        assert(ro != NULL);

        /* 对只读流写入：fputc 失败，置错误指示器 */
        errno = 0;
        int wc = fputc('Z', ro);
        assert(wc == EOF);
        assert(ferror(ro) != 0);

        /* [4] 已处于错误状态，fflush 返回 EOF 并保持/置错误指示器 */
        int fr = fflush(ro);
        assert(fr == EOF);
        assert(ferror(ro) != 0);

        fclose(ro);
        remove(path);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fflush 的原型为 int fflush(FILE *stream)」：
 * 参数类型不匹配，传入 int 而非 FILE *，gcc -std=c99 应报错
 * （incompatible type for argument 1 / passing argument 1 ... makes pointer from integer）。 */
void neg_wrong_arg_type(void)
{
    fflush(42);            /* 42 不是 FILE *，应编译报错 */
}

/* 违反约束「fflush 的原型为 int fflush(FILE *stream)」：
 * 参数个数不匹配，gcc -std=c99 应报错（too few arguments to function 'fflush'）。 */
void neg_too_few_args(void)
{
    fflush();              /* 缺少必需参数，应编译报错 */
}

/* 违反约束「fflush 的原型为 int fflush(FILE *stream)」：
 * 参数个数过多，gcc -std=c99 应报错（too many arguments to function 'fflush'）。 */
void neg_too_many_args(FILE *fp)
{
    fflush(fp, fp);        /* 多余参数，应编译报错 */
}

/* 违反约束「fflush 返回 int」：
 * 将返回值赋给不兼容的指针类型，gcc -std=c99 应报错
 * （assignment makes pointer from integer without a cast）。 */
void neg_bad_return_use(FILE *fp)
{
    FILE *p = fflush(fp);  /* int 赋给 FILE *，应编译报错 */
    (void)p;
}

#endif /* 负向测试结束 */