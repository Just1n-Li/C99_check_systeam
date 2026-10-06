/*
 * 测试条款：C99 7.19.9.3  The fsetpos function
 *
 * 预期行为：
 *   - 正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   - 负向测试：位于 #if 0 块内，故意违反约束，gcc -std=c99 应报错。
 *
 * 覆盖段落：
 *   [1] 原型：int fsetpos(FILE *stream, const fpos_t *pos);
 *   [2] 描述：按 pos 设置 mbstate_t 与文件位置指示器；pos 必须来自同一文件上
 *       先前成功的 fgetpos 调用；读写错误时设置错误指示器并失败。
 *   [3] 语义：成功调用撤销 ungetc 效果、清除 EOF 指示器、建立新解析状态与位置；
 *       成功之后 update stream 上的下一次操作可以是输入或输出。
 *   [4] 返回值：成功返回 0；失败返回非零并把实现定义的正值存入 errno。
 */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：函数指针类型必须与标准原型一致 */
static int (*fp_fsetpos)(FILE *, const fpos_t *) = fsetpos;

int main(void)
{
    const char *path = "c99_7_19_9_3_test.tmp";
    FILE *fp;
    fpos_t pos;
    int ret;
    int c;

    /* [1] 原型可用性 */
    assert(fp_fsetpos == fsetpos);

    /* 准备一个可读写的文件 */
    fp = fopen(path, "w+");
    assert(fp != NULL);

    /* 写入一些内容 */
    assert(fputs("ABCDEFGHIJ", fp) >= 0);
    assert(fflush(fp) == 0);

    /* [2] 用 fgetpos 取得一个位置值（条款要求 pos 来自先前成功的 fgetpos） */
    assert(fseek(fp, 3, SEEK_SET) == 0);
    ret = fgetpos(fp, &pos);
    assert(ret == 0);

    /* 移动位置，稍后用 fsetpos 回到 pos */
    assert(fseek(fp, 8, SEEK_SET) == 0);
    c = fgetc(fp);
    assert(c == 'I');

    /* [2][3][4] 成功调用 fsetpos：返回 0 */
    errno = 0;
    ret = fsetpos(fp, &pos);
    assert(ret == 0);

    /* [3] 位置已恢复到 pos 处（偏移 3，即字符 'D'） */
    c = fgetc(fp);
    assert(c == 'D');

    /* [3] 成功 fsetpos 之后，update stream 上可以继续做输入 */
    assert(fseek(fp, 0, SEEK_SET) == 0);
    assert(fgetc(fp) == 'A');

    /* [3] 成功 fsetpos 之后，update stream 上也可以做输出 */
    assert(fseek(fp, 0, SEEK_SET) == 0);
    assert(fgetc(fp) == 'A');           /* 先读，使流处于读状态 */
    assert(fsetpos(fp, &pos) == 0);     /* 成功 fsetpos 后允许切换为输出 */
    assert(fputc('X', fp) == 'X');      /* 输出操作应成功 */
    assert(fflush(fp) == 0);

    /* [3] 成功 fsetpos 清除 EOF 指示器 */
    assert(fseek(fp, 0, SEEK_END) == 0);
    assert(fgetc(fp) == EOF);           /* 触发 EOF 指示器 */
    assert(feof(fp) != 0);              /* EOF 指示器已置位 */
    assert(fsetpos(fp, &pos) == 0);     /* 成功调用应清除 EOF 指示器 */
    assert(feof(fp) == 0);              /* EOF 指示器已被清除 */

    /* [3] 成功 fsetpos 撤销 ungetc 的效果 */
    assert(fseek(fp, 5, SEEK_SET) == 0);
    assert(fgetc(fp) == 'F');
    assert(ungetc('Z', fp) == 'Z');     /* 压回一个字符 */
    assert(fsetpos(fp, &pos) == 0);     /* 成功调用应撤销 ungetc 效果 */
    c = fgetc(fp);                      /* 应读到 pos 处的 'D'，而不是 'Z' */
    assert(c == 'D');

    /* [4] 失败时返回非零，并在 errno 中存入实现定义的正值 */
    {
        FILE *bad = fopen(path, "r");   /* 只读流 */
        fpos_t badpos;
        assert(bad != NULL);
        assert(fgetpos(bad, &badpos) == 0);
        fclose(bad);                    /* 关闭后使用该流，fsetpos 应失败 */
        errno = 0;
        ret = fsetpos(bad, &badpos);
        assert(ret != 0);               /* 失败返回非零 */
        assert(errno > 0);              /* errno 为实现定义的正值 */
    }

    fclose(fp);
    remove(path);

    printf("C99 7.19.9.3 fsetpos: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fsetpos 的第一个实参类型必须为 FILE *」：
 * 传入 int，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg1(void)
{
    int x = 0;
    fpos_t pos;
    fsetpos(x, &pos);           /* 错误：第一个实参不是 FILE * */
}

/* 违反约束「fsetpos 的第二个实参类型必须为 const fpos_t *」：
 * 传入 int *，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg2(FILE *fp)
{
    int y = 0;
    fsetpos(fp, &y);            /* 错误：第二个实参不是 const fpos_t * */
}

/* 违反约束「fsetpos 的第二个实参类型必须为 const fpos_t *」：
 * 传入 fpos_t（非指针），gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg2b(FILE *fp)
{
    fpos_t pos;
    fsetpos(fp, pos);           /* 错误：第二个实参不是指针 */
}

/* 违反约束「fsetpos 的第二个实参类型必须为 const fpos_t *」：
 * 传入 void *，gcc -std=c99 应报 incompatible type 错误。 */
void bad_arg2c(FILE *fp)
{
    void *p = 0;
    fsetpos(fp, p);             /* 错误：void * 不能隐式转换为 const fpos_t * */
}

/* 违反约束「fsetpos 的实参个数必须为 2」：
 * 只传一个实参，gcc -std=c99 应报 too few arguments 错误。 */
void bad_argc1(FILE *fp)
{
    fsetpos(fp);                /* 错误：实参个数不足 */
}

/* 违反约束「fsetpos 的实参个数必须为 2」：
 * 传三个实参，gcc -std=c99 应报 too many arguments 错误。 */
void bad_argc2(FILE *fp)
{
    fpos_t pos;
    fsetpos(fp, &pos, &pos);    /* 错误：实参个数过多 */
}

/* 违反约束「fsetpos 的返回值类型为 int」：
 * 把返回值赋给结构体，gcc -std=c99 应报 incompatible type 错误。 */
void bad_ret(FILE *fp)
{
    fpos_t pos;
    struct S { int a; } s;
    s = fsetpos(fp, &pos);      /* 错误：int 不能赋给结构体 */
}

#endif