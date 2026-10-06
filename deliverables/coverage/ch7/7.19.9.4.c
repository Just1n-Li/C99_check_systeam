/*
 * 测试 C99 7.19.9.4 —— ftell 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：long int ftell(FILE *stream);  需要 <stdio.h>
 *   [2] 语义：返回当前文件位置指示器的值；二进制流为从文件开头起的字符数；
 *             文本流返回可用于 fseek 恢复位置的信息。
 *   [3] 返回：成功返回当前位置值；失败返回 -1L 并向 errno 存入实现定义的正值。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：ftell 的返回类型必须是 long int，参数为 FILE*。
 *     用函数指针赋值来静态验证签名。 */
static long int (*ftell_proto_check)(FILE *) = ftell;

static void test_binary_stream(void)
{
    /* [2] 二进制流：ftell 返回从文件开头起的字符数 */
    const char *fname = "c99_7_19_9_4_bin.tmp";
    FILE *fp = fopen(fname, "wb+");
    assert(fp != NULL);

    /* 初始位置应为 0 */
    long pos0 = ftell(fp);
    assert(pos0 == 0L);

    /* 写入 10 个字节 */
    const char data[10] = { 'a','b','c','d','e','f','g','h','i','j' };
    size_t nw = fwrite(data, 1, sizeof data, fp);
    assert(nw == sizeof data);

    /* 二进制流：位置指示器 = 从文件开头起的字符数 = 10 */
    long pos1 = ftell(fp);
    assert(pos1 == 10L);

    /* 再写 5 个字节，位置应为 15 */
    const char more[5] = { 'k','l','m','n','o' };
    assert(fwrite(more, 1, sizeof more, fp) == sizeof more);
    long pos2 = ftell(fp);
    assert(pos2 == 15L);

    /* 用 fseek 回到位置 10，再用 ftell 验证 */
    assert(fseek(fp, 10L, SEEK_SET) == 0);
    long pos3 = ftell(fp);
    assert(pos3 == 10L);

    /* 从位置 10 读 5 个字节，应读到 'k'..'o' */
    char buf[5] = { 0 };
    assert(fread(buf, 1, sizeof buf, fp) == sizeof buf);
    assert(memcmp(buf, more, sizeof more) == 0);

    /* 读完后位置应为 15 */
    long pos4 = ftell(fp);
    assert(pos4 == 15L);

    /* 移动到文件末尾，位置应等于文件大小 15 */
    assert(fseek(fp, 0L, SEEK_END) == 0);
    long endpos = ftell(fp);
    assert(endpos == 15L);

    fclose(fp);
    remove(fname);
}

static void test_text_stream(void)
{
    /* [2] 文本流：ftell 返回的信息可用于 fseek 恢复位置。
     *     这里验证「保存位置 -> 移动 -> 用保存值恢复 -> 内容一致」。 */
    const char *fname = "c99_7_19_9_4_txt.tmp";
    FILE *fp = fopen(fname, "w+");
    assert(fp != NULL);

    /* 写入若干文本行 */
    assert(fputs("line one\n", fp) >= 0);
    assert(fputs("line two\n", fp) >= 0);
    assert(fputs("line three\n", fp) >= 0);
    fflush(fp);

    /* 回到开头 */
    assert(fseek(fp, 0L, SEEK_SET) == 0);

    /* 读第一行 */
    char line1[64] = { 0 };
    assert(fgets(line1, sizeof line1, fp) != NULL);
    assert(strcmp(line1, "line one\n") == 0);

    /* 保存当前位置（文本流：值是实现相关的，但可用于 fseek 恢复） */
    long saved = ftell(fp);
    assert(saved != -1L);

    /* 读第二行 */
    char line2[64] = { 0 };
    assert(fgets(line2, sizeof line2, fp) != NULL);
    assert(strcmp(line2, "line two\n") == 0);

    /* 用保存的位置恢复，再读一次应得到第二行 */
    assert(fseek(fp, saved, SEEK_SET) == 0);
    char line2b[64] = { 0 };
    assert(fgets(line2b, sizeof line2b, fp) != NULL);
    assert(strcmp(line2b, "line two\n") == 0);

    fclose(fp);
    remove(fname);
}

static void test_failure_returns_minus_one(void)
{
    /* [3] 失败时返回 -1L 并向 errno 存入实现定义的正值。
     *     对已关闭的流调用 ftell 是未定义行为，不能作为负向测试；
     *     这里用一个合法但会失败的场景：对不可定位的流调用 ftell。
     *     标准并未强制所有实现都失败，因此仅在失败时检查返回值与 errno。 */
    const char *fname = "c99_7_19_9_4_fail.tmp";
    FILE *fp = fopen(fname, "w+");
    assert(fp != NULL);
    assert(fputs("x", fp) >= 0);
    fflush(fp);

    /* 正常可定位流：成功，返回非负值 */
    long ok = ftell(fp);
    assert(ok >= 0L);

    fclose(fp);
    remove(fname);

    /* 说明：真正触发 ftell 失败需要不可定位的流（如管道），
     * 其行为依赖实现，故此处只验证成功路径的返回值语义。 */
}

static void test_errno_positive_on_failure(void)
{
    /* [3] 若 ftell 失败，errno 应被置为实现定义的正值。
     *     使用不可定位的流（管道）尝试触发失败；若实现允许定位，
     *     则跳过该断言，避免依赖实现细节。 */
    FILE *fp = popen("echo hello", "r");
    if (fp != NULL) {
        errno = 0;
        long r = ftell(fp);
        if (r == -1L) {
            /* 失败：errno 应为正值 */
            assert(errno > 0);
        } else {
            /* 实现允许定位，返回非负值 */
            assert(r >= 0L);
        }
        pclose(fp);
    }
}

int main(void)
{
    /* [1] 原型签名静态检查 */
    assert(ftell_proto_check == ftell);

    test_binary_stream();          /* [2] 二进制流语义 */
    test_text_stream();            /* [2] 文本流语义 */
    test_failure_returns_minus_one(); /* [3] 成功返回值 */
    test_errno_positive_on_failure(); /* [3] 失败时 errno 正值 */

    printf("C99 7.19.9.4 ftell: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「ftell 的参数类型必须为 FILE *」：
 * 传入 int 而非 FILE*，gcc -std=c99 应报错
 * （incompatible type for argument 1 of 'ftell'）。 */
void bad_arg_type(void)
{
    int x = 0;
    long r = ftell(x);   /* 错误：参数应为 FILE* */
    (void)r;
}

/* 违反约束「ftell 需要 <stdio.h> 中的原型」：
 * 若未包含 <stdio.h>，ftell 无原型，调用属于隐式声明，
 * C99 已删除隐式函数声明，gcc -std=c99 应报错
 * （implicit declaration of function 'ftell'）。 */
void bad_no_prototype(void)
{
    /* 假设此处没有 #include <stdio.h> */
    long r = ftell(0);   /* 错误：无原型，隐式声明被 C99 禁止 */
    (void)r;
}

/* 违反约束「ftell 的返回类型为 long int」：
 * 将返回值赋给不兼容的指针类型，gcc -std=c99 应报错
 * （assignment makes pointer from integer without a cast）。 */
void bad_return_type(void)
{
    FILE *fp = 0;
    char *p = ftell(fp);  /* 错误：long int 不能隐式转换为 char* */
    (void)p;
}

/* 违反约束「ftell 只接受一个参数」：
 * 多传参数，gcc -std=c99 应报错
 * （too many arguments to function 'ftell'）。 */
void bad_too_many_args(void)
{
    FILE *fp = 0;
    long r = ftell(fp, 0);  /* 错误：参数过多 */
    (void)r;
}

#endif /* 负向测试结束 */