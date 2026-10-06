/*
 * 测试 C99 7.19.7.2 —— fgets 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型（含 restrict 限定符、返回 char *）
 *   [2] 语义：最多读 n-1 个字符；遇换行符（保留）或 EOF 停止；
 *       在最后读入字符之后写入 '\0'
 *   [3] 返回值：成功返回 s；EOF 且未读入任何字符时数组不变并返回 NULL；
 *       读错误时数组内容不确定并返回 NULL
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：fgets 的签名应为
 *     char *fgets(char * restrict s, int n, FILE * restrict stream);
 * 通过取函数指针类型来静态验证返回类型与参数类型。 */
static char *(*fp_fgets)(char * restrict, int, FILE * restrict) = fgets;

static void make_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    assert(f != NULL);
    fwrite(content, 1, strlen(content), f);
    fclose(f);
}

int main(void)
{
    const char *path = "c99_7_19_7_2_tmp.txt";
    char buf[64];
    char *ret;
    FILE *f;

    /* 确认函数指针赋值成功（[1]） */
    assert(fp_fgets == fgets);

    /* ---------- [2] 基本读取：读入整行，换行符被保留 ---------- */
    make_file(path, "hello\nworld\n");
    f = fopen(path, "rb");
    assert(f != NULL);

    memset(buf, 'X', sizeof buf);
    ret = fgets(buf, sizeof buf, f);
    assert(ret == buf);                 /* [3] 成功返回 s */
    assert(strcmp(buf, "hello\n") == 0);/* [2] 换行符被保留 */
    /* [2] 最后读入字符之后立即写入 '\0' */
    assert(buf[5] == '\n');
    assert(buf[6] == '\0');

    /* 第二次读取：读入 "world\n" */
    ret = fgets(buf, sizeof buf, f);
    assert(ret == buf);
    assert(strcmp(buf, "world\n") == 0);

    /* ---------- [2] 最多读 n-1 个字符 ---------- */
    /* 文件内容为 "abcdefghij"（无换行），n=5 时最多读 4 个字符 */
    fclose(f);
    make_file(path, "abcdefghij");
    f = fopen(path, "rb");
    assert(f != NULL);

    memset(buf, 'X', sizeof buf);
    ret = fgets(buf, 5, f);
    assert(ret == buf);
    assert(strcmp(buf, "abcd") == 0);   /* 只读入 4 个字符 */
    assert(buf[4] == '\0');             /* 第 5 个位置是 '\0' */

    /* 继续读取剩余部分 */
    ret = fgets(buf, sizeof buf, f);
    assert(ret == buf);
    assert(strcmp(buf, "efghij") == 0);

    /* ---------- [2][3] 到达 EOF 且未读入任何字符 ---------- */
    /* 此时已到文件末尾，再读一次应返回 NULL 且数组内容不变 */
    memset(buf, 'Z', sizeof buf);
    ret = fgets(buf, sizeof buf, f);
    assert(ret == NULL);                /* [3] 返回空指针 */
    /* [3] 数组内容保持不变 */
    assert(buf[0] == 'Z');
    assert(buf[1] == 'Z');
    assert(buf[63] == 'Z');

    fclose(f);

    /* ---------- [2] 文件末尾无换行符时也能正常读取 ---------- */
    make_file(path, "no-newline");
    f = fopen(path, "rb");
    assert(f != NULL);
    memset(buf, 'X', sizeof buf);
    ret = fgets(buf, sizeof buf, f);
    assert(ret == buf);
    assert(strcmp(buf, "no-newline") == 0);
    assert(buf[10] == '\0');

    /* 再次读取应到 EOF，返回 NULL */
    ret = fgets(buf, sizeof buf, f);
    assert(ret == NULL);
    fclose(f);

    /* ---------- [2] 空文件：立即 EOF，返回 NULL，数组不变 ---------- */
    make_file(path, "");
    f = fopen(path, "rb");
    assert(f != NULL);
    memset(buf, 'Q', sizeof buf);
    ret = fgets(buf, sizeof buf, f);
    assert(ret == NULL);
    assert(buf[0] == 'Q');              /* 数组未被修改 */
    fclose(f);

    /* ---------- [2] 恰好读满 n-1 个字符后停止 ---------- */
    /* 文件为 "1234567890"，n=6 时读入 "12345" */
    make_file(path, "1234567890");
    f = fopen(path, "rb");
    assert(f != NULL);
    memset(buf, 'X', sizeof buf);
    ret = fgets(buf, 6, f);
    assert(ret == buf);
    assert(strcmp(buf, "12345") == 0);
    assert(buf[5] == '\0');
    fclose(f);

    /* ---------- [2] 换行符出现在 n-1 个字符之内时提前停止 ---------- */
    /* 文件为 "ab\ncd"，n=10 时读到换行符即停止，保留换行符 */
    make_file(path, "ab\ncd");
    f = fopen(path, "rb");
    assert(f != NULL);
    memset(buf, 'X', sizeof buf);
    ret = fgets(buf, 10, f);
    assert(ret == buf);
    assert(strcmp(buf, "ab\n") == 0);
    assert(buf[3] == '\0');
    fclose(f);

    /* ---------- [2] 换行符恰好是第 n-1 个字符 ---------- */
    /* 文件为 "abc\ndef"，n=4 时读入 "abc"，换行符未读入 */
    make_file(path, "abc\ndef");
    f = fopen(path, "rb");
    assert(f != NULL);
    memset(buf, 'X', sizeof buf);
    ret = fgets(buf, 4, f);
    assert(ret == buf);
    assert(strcmp(buf, "abc") == 0);
    assert(buf[3] == '\0');
    /* 下一次读取从换行符开始 */
    ret = fgets(buf, sizeof buf, f);
    assert(ret == buf);
    assert(strcmp(buf, "\n") == 0);
    fclose(f);

    /* ---------- [3] 读错误：返回 NULL ---------- */
    /* 以只写方式打开文件后尝试读取，触发读错误 */
    f = fopen(path, "w");
    assert(f != NULL);
    ret = fgets(buf, sizeof buf, f);    /* 对只写流读取，应失败 */
    assert(ret == NULL);                /* [3] 读错误返回空指针 */
    fclose(f);

    remove(path);

    printf("C99 7.19.7.2 fgets: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fgets 的第一个参数类型为 char *」：
 * 传入 const char * 会丢弃 const 限定符，gcc -std=c99 应报错
 * （discards qualifiers / incompatible pointer type）。 */
#include <stdio.h>
void neg1(FILE *stream)
{
    const char *s = "abc";
    fgets(s, 10, stream);   /* 错误：第一个实参应为 char *，不能是 const char * */
}

/* 违反约束「fgets 的第二个参数类型为 int」：
 * 传入指针类型无法隐式转换为 int，gcc -std=c99 应报错。 */
void neg2(FILE *stream)
{
    char buf[10];
    int *p = 0;
    fgets(buf, p, stream);  /* 错误：第二个实参应为 int，不能是指针 */
}

/* 违反约束「fgets 的第三个参数类型为 FILE *」：
 * 传入 int 无法隐式转换为 FILE *，gcc -std=c99 应报错。 */
void neg3(void)
{
    char buf[10];
    fgets(buf, 10, 0);      /* 错误：第三个实参应为 FILE *，不能是 int */
}

/* 违反约束「fgets 返回 char *」：
 * 将返回值赋给不兼容的指针类型（如 int *）应报错。 */
void neg4(FILE *stream)
{
    char buf[10];
    int *p = fgets(buf, 10, stream);  /* 错误：char * 不能赋给 int * */
    (void)p;
}

/* 违反约束「fgets 需要 3 个实参」：
 * 实参个数不匹配应报错。 */
void neg5(FILE *stream)
{
    char buf[10];
    fgets(buf, 10);         /* 错误：实参个数不足 */
    (void)stream;
}

/* 违反约束「fgets 需要 3 个实参」：
 * 实参个数过多应报错。 */
void neg6(FILE *stream)
{
    char buf[10];
    fgets(buf, 10, stream, buf);  /* 错误：实参个数过多 */
}

#endif /* 负向测试结束 */