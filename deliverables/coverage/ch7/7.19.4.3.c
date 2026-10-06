/*
 * 测试条款：C99 7.19.4.3  The tmpfile function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型声明 FILE *tmpfile(void);
 *   [2] 创建临时二进制文件；与任何已存在文件不同；关闭或程序终止时自动删除；
 *       以 "wb+" 模式打开（可读可写、二进制、截断/新建）。
 *   [3] 推荐实践：程序生命周期内至少能打开 TMP_MAX 个临时文件。
 *   [4] 成功返回指向该文件流的指针；失败返回空指针。
 *   Forward references: fopen (7.19.5.3)
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：tmpfile 无参数，返回 FILE*。
 * 通过取函数指针类型来静态验证原型签名。 */
static FILE *(*tmpfile_proto_check)(void) = tmpfile;

int main(void)
{
    /* [1] 原型：调用时不带参数，返回 FILE *。 */
    FILE *fp = tmpfile();
    assert(tmpfile_proto_check == tmpfile);

    /* [4] 成功时应返回非空指针。若返回 NULL 则无法继续测试。 */
    if (fp == NULL) {
        /* 环境无法创建临时文件时，仅验证返回空指针这一合法分支。 */
        printf("tmpfile() returned NULL (creation failed) - acceptable per [4]\n");
        return 0;
    }
    assert(fp != NULL);

    /* [2] 以 "wb+" 模式打开：应可写、可读、二进制。
     * 写入二进制数据（含 '\0' 与高位字节），验证二进制模式。 */
    {
        const unsigned char out[] = { 0x00, 0x01, 0x7F, 0x80, 0xFF, 'A', '\n' };
        size_t n = sizeof out;
        size_t w = fwrite(out, 1, n, fp);
        assert(w == n);                 /* 可写 */
        assert(ferror(fp) == 0);
    }

    /* [2] "wb+" 允许读回：定位到开头后读取，内容应一致。 */
    {
        unsigned char in[sizeof((unsigned char[]){0x00,0x01,0x7F,0x80,0xFF,'A','\n'})];
        const unsigned char expect[] = { 0x00, 0x01, 0x7F, 0x80, 0xFF, 'A', '\n' };
        int rc = fseek(fp, 0L, SEEK_SET);
        assert(rc == 0);                /* 可定位 */
        size_t r = fread(in, 1, sizeof in, fp);
        assert(r == sizeof in);         /* 可读 */
        assert(memcmp(in, expect, sizeof in) == 0);
    }

    /* [2] 文件应可被更新（update）：再次写入并读回。 */
    {
        int rc = fseek(fp, 0L, SEEK_SET);
        assert(rc == 0);
        assert(fputc('Z', fp) == 'Z');
        assert(fflush(fp) == 0);
        rc = fseek(fp, 0L, SEEK_SET);
        assert(rc == 0);
        assert(fgetc(fp) == 'Z');
    }

    /* [2] 关闭文件：关闭后该临时文件应被自动删除。
     * 通过再次尝试打开同名文件不可行（名字未知），
     * 因此这里只验证 fclose 成功，删除行为由实现保证。 */
    assert(fclose(fp) == 0);

    /* [3] 推荐实践：程序生命周期内至少能打开 TMP_MAX 个临时文件。
     * 逐个打开并保持打开，直到达到 TMP_MAX 或失败。
     * 注意：TMP_MAX 可能很大，这里只验证“至少能打开若干个”，
     * 并检查 TMP_MAX 宏存在且为正。 */
    assert(TMP_MAX > 0);

    {
        /* 同时打开多个临时文件，验证可同时打开（受 FOPEN_MAX 限制）。 */
        enum { N = 8 };
        FILE *files[N];
        int i, opened = 0;
        for (i = 0; i < N; i++) {
            files[i] = tmpfile();
            if (files[i] == NULL) break;
            opened++;
            /* 每个文件应彼此不同：写入不同内容互不影响。 */
            assert(fputc('a' + i, files[i]) == 'a' + i);
            assert(fflush(files[i]) == 0);
        }
        /* 至少应能打开一个（通常远多于一个）。 */
        assert(opened >= 1);

        /* 验证各文件内容互不干扰（不同文件）。 */
        for (i = 0; i < opened; i++) {
            int rc = fseek(files[i], 0L, SEEK_SET);
            assert(rc == 0);
            assert(fgetc(files[i]) == 'a' + i);
        }
        for (i = 0; i < opened; i++) {
            assert(fclose(files[i]) == 0);
        }
    }

    /* [2] 程序终止时自动删除：创建一个临时文件后不关闭，
     * 依赖程序正常终止时自动清理。这里仅创建，不关闭。 */
    {
        FILE *leak = tmpfile();
        if (leak != NULL) {
            assert(fputc('x', leak) == 'x');
            /* 故意不 fclose：程序正常终止时应自动删除。 */
        }
    }

    printf("All positive tests for C99 7.19.4.3 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「tmpfile 的原型为 FILE *tmpfile(void)」：
 * 以参数调用 tmpfile 应编译报错（too many arguments to function 'tmpfile'）。 */
void bad_call_with_arg(void)
{
    FILE *fp = tmpfile(1);   /* 错误：tmpfile 不接受参数 */
    (void)fp;
}

/* 违反约束「tmpfile 返回 FILE *」：
 * 将返回值赋给不兼容的指针类型（如 int *）在严格编译下应报错/警告为约束违反。 */
void bad_return_type(void)
{
    int *p = tmpfile();      /* 错误：FILE * 不能隐式转换为 int * */
    (void)p;
}

/* 违反约束「使用 tmpfile 前需包含 <stdio.h>」：
 * 未声明 tmpfile 而调用，在 C99 中为约束违反（隐式声明被禁止）。 */
void bad_no_declaration(void)
{
    /* 假设此处未包含 <stdio.h>，调用未声明的 tmpfile 应报错。 */
    /* FILE *fp = tmpfile(); */
}

#endif /* 负向测试结束 */