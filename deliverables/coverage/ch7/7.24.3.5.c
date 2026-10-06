/*
 * 测试目标：C99 7.24.3.5 The fwide function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 函数原型 int fwide(FILE *stream, int mode);
 *   [2] mode > 0 尝试设为宽定向；mode < 0 尝试设为字节定向；mode == 0 不改变定向。
 *   [3] 返回值：>0 宽定向；<0 字节定向；==0 无定向。
 *   Footnote 293：若定向已确定，fwide 不改变它。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 函数原型检查：fwide 接受 (FILE *, int) 并返回 int */
static int (*fwide_proto_check)(FILE *, int) = fwide;

int main(void)
{
    FILE *fp;
    int r;

    /* 确保原型类型正确 */
    assert(fwide_proto_check == fwide);

    /* ---------- 测试 1：新打开的流，mode == 0 不改变定向，返回 0 ---------- */
    /* [2] mode 为 0 时不改变定向；[3] 无定向时返回 0 */
    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, 0);
    assert(r == 0);                 /* 新流无定向 */
    /* 再次用 mode==0 调用，仍应返回 0 */
    r = fwide(fp, 0);
    assert(r == 0);
    fclose(fp);

    /* ---------- 测试 2：mode > 0 使流变为宽定向，返回 > 0 ---------- */
    /* [2] mode > 0 尝试设为宽定向；[3] 宽定向返回 > 0 */
    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, 1);
    assert(r > 0);                  /* 宽定向 */
    /* 再次查询（mode==0）应仍报告宽定向 */
    r = fwide(fp, 0);
    assert(r > 0);
    fclose(fp);

    /* ---------- 测试 3：mode < 0 使流变为字节定向，返回 < 0 ---------- */
    /* [2] mode < 0 尝试设为字节定向；[3] 字节定向返回 < 0 */
    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, -1);
    assert(r < 0);                  /* 字节定向 */
    /* 再次查询（mode==0）应仍报告字节定向 */
    r = fwide(fp, 0);
    assert(r < 0);
    fclose(fp);

    /* ---------- 测试 4：Footnote 293 —— 定向已确定后 fwide 不改变它 ---------- */
    /* 先设为宽定向 */
    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, 1);
    assert(r > 0);
    /* 再尝试用 mode < 0 改为字节定向：应失败，仍为宽定向 */
    r = fwide(fp, -1);
    assert(r > 0);                  /* 定向未被改变 */
    r = fwide(fp, 0);
    assert(r > 0);
    fclose(fp);

    /* 先设为字节定向 */
    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, -1);
    assert(r < 0);
    /* 再尝试用 mode > 0 改为宽定向：应失败，仍为字节定向 */
    r = fwide(fp, 1);
    assert(r < 0);                  /* 定向未被改变 */
    r = fwide(fp, 0);
    assert(r < 0);
    fclose(fp);

    /* ---------- 测试 5：mode 的符号决定行为，非零值大小无关 ---------- */
    /* [2] 只关心 mode 是否大于/小于/等于 0 */
    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, 1000);            /* 任意正数 */
    assert(r > 0);
    fclose(fp);

    fp = tmpfile();
    assert(fp != NULL);
    r = fwide(fp, -1000);           /* 任意负数 */
    assert(r < 0);
    fclose(fp);

    /* ---------- 测试 6：标准流也可使用 fwide ---------- */
    /* [1] stream 参数为 FILE *，标准流同样适用 */
    r = fwide(stdout, 0);           /* 仅查询，不改变定向 */
    /* 返回值只要求是 >0 / <0 / ==0 之一，不强制具体值 */
    assert(r > 0 || r < 0 || r == 0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「fwide 的第一个参数类型为 FILE *」：
 * 传入 int 而非 FILE *，gcc -std=c99 应报错
 * （incompatible type for argument 1 of 'fwide' / passing argument 1 ... makes pointer from integer） */
void bad_arg1(void)
{
    int x = 0;
    fwide(x, 0);            /* 错误：第一个实参应为 FILE * */
}

/* 违反约束「fwide 的第二个参数类型为 int」：
 * 传入 struct 类型，gcc -std=c99 应报错
 * （incompatible type for argument 2 of 'fwide'） */
struct NotInt { int a; };
void bad_arg2(FILE *fp)
{
    struct NotInt n;
    fwide(fp, n);           /* 错误：第二个实参应为 int */
}

/* 违反约束「fwide 返回 int」：
 * 将返回值赋给不兼容的指针类型，gcc -std=c99 应报错
 * （assignment makes pointer from integer without a cast） */
void bad_ret(FILE *fp)
{
    char *p = fwide(fp, 0); /* 错误：int 不能隐式转换为 char * */
    (void)p;
}

/* 违反约束「fwide 需要两个实参」：
 * 实参个数不匹配，gcc -std=c99 应报错
 * （too few arguments to function 'fwide'） */
void bad_argc(FILE *fp)
{
    fwide(fp);              /* 错误：缺少第二个实参 */
}

#endif