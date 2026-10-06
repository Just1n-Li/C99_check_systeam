/*
 * 测试条款：C99 7.19.10.2  The feof function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 条款要点：
 *   [1] 原型： #include <stdio.h>  int feof(FILE *stream);
 *   [2] 描述：测试 stream 所指向流的文件结束指示符（end-of-file indicator）。
 *   [3] 返回：当且仅当 stream 的文件结束指示符被设置时，返回非零值。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：feof 接受 FILE* 并返回 int。
 *     用函数指针类型匹配来静态验证原型（不匹配则编译报错/警告）。 */
static int (*feof_proto_check)(FILE *) = feof;

int main(void)
{
    /* [1] 头文件 <stdio.h> 已包含，feof 可用；原型返回 int。 */
    (void)feof_proto_check;

    /* ---------- 场景 A：刚打开、尚未读取的文件，EOF 指示符未设置 ---------- */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* [3] 刚打开时文件结束指示符未设置 => feof 返回 0（假）。 */
        assert(feof(fp) == 0);

        /* [2][3] 读取一个字符后仍未到文件尾 => 指示符仍未设置。 */
        {
            int c = fgetc(fp);
            assert(c == EOF);          /* 空文件，读取失败 */
            /* 注意：空文件读取失败会设置 EOF 指示符，见场景 B。 */
        }

        fclose(fp);
    }

    /* ---------- 场景 B：读到文件尾后，EOF 指示符被设置 ---------- */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* 写入 "AB" 并回绕。 */
        assert(fputs("AB", fp) >= 0);
        rewind(fp);

        /* [3] 回绕后 EOF 指示符被清除 => feof 返回 0。 */
        assert(feof(fp) == 0);

        /* 读取两个字符，均成功，EOF 指示符仍未设置。 */
        assert(fgetc(fp) == 'A');
        assert(feof(fp) == 0);         /* [3] 尚未到文件尾 */
        assert(fgetc(fp) == 'B');
        assert(feof(fp) == 0);         /* [3] 尚未到文件尾 */

        /* 再读一次：到达文件尾，读取失败并设置 EOF 指示符。 */
        assert(fgetc(fp) == EOF);
        /* [3] 此时 EOF 指示符已设置 => feof 返回非零。 */
        assert(feof(fp) != 0);

        /* [3] "当且仅当"：再次调用仍返回非零（指示符保持设置）。 */
        assert(feof(fp) != 0);

        /* 清除 EOF 指示符（clearerr），feof 应重新返回 0。 */
        clearerr(fp);
        assert(feof(fp) == 0);         /* [3] 指示符被清除后返回 0 */

        fclose(fp);
    }

    /* ---------- 场景 C：空文件，首次读取即设置 EOF 指示符 ---------- */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        assert(feof(fp) == 0);         /* [3] 初始未设置 */
        assert(fgetc(fp) == EOF);      /* 空文件，读取失败 */
        assert(feof(fp) != 0);         /* [3] 现在已设置 */

        fclose(fp);
    }

    /* ---------- 场景 D：返回值语义 —— 非零即真，0 即假 ---------- */
    {
        FILE *fp = tmpfile();
        assert(fp != NULL);

        /* [3] 未设置时返回 0。 */
        assert(!feof(fp));

        assert(fgetc(fp) == EOF);
        /* [3] 已设置时返回非零（用逻辑取反验证非零）。 */
        assert(feof(fp));
        assert(feof(fp) != 0);

        fclose(fp);
    }

    printf("All positive tests for C99 7.19.10.2 (feof) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「feof 的参数类型必须为 FILE *」：
 * 传入 int 而非 FILE*，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg_type(void)
{
    int x = 0;
    feof(x);                 /* 错误：实参类型 int 与形参 FILE* 不兼容 */
}

/* 违反约束「feof 的参数类型必须为 FILE *」：
 * 传入 char* 而非 FILE*，应报 incompatible type 错误。 */
void bad_arg_ptr(void)
{
    char buf[16];
    feof(buf);               /* 错误：char* 与 FILE* 不兼容 */
}

/* 违反约束「feof 的参数个数必须为 1」：
 * 少传参数，应报 too few arguments 错误。 */
void bad_too_few(void)
{
    feof();                  /* 错误：参数个数不足 */
}

/* 违反约束「feof 的参数个数必须为 1」：
 * 多传参数，应报 too many arguments 错误。 */
void bad_too_many(FILE *fp)
{
    feof(fp, fp);            /* 错误：参数个数过多 */
}

/* 违反约束「feof 返回 int，不能作为函数被调用结果赋值给不兼容类型」：
 * 这里演示把 feof 当作对象使用（缺少调用括号），应报错。 */
void bad_not_called(FILE *fp)
{
    int y = feof;            /* 错误：feof 是函数，不能作为 int 对象使用 */
    (void)y;
}

#endif /* 负向测试结束 */