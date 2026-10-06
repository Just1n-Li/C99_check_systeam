/*
 * 测试 C99 7.19.9.5 —— rewind 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 条款要点：
 *   [1] 原型：void rewind(FILE *stream);  需要 <stdio.h>
 *   [2] 将文件位置指示器置到文件开头，等价于 (void)fseek(stream, 0L, SEEK_SET)，
 *       但额外清除流的错误指示器。
 *   [3] 无返回值（void）。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：rewind 接受一个 FILE* 参数，返回 void。
 *     通过取函数指针类型来静态验证原型。 */
static void check_prototype(void)
{
    void (*fp)(FILE *) = rewind;   /* [1] 若原型不符则编译失败 */
    assert(fp != NULL);
}

/* [2] 语义：rewind 把位置指示器置到文件开头 */
static void test_rewind_to_beginning(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    /* 写入一些内容 */
    assert(fputs("Hello, rewind!", fp) >= 0);
    fflush(fp);

    /* 读回全部内容，位置指示器到达末尾 */
    assert(fseek(fp, 0L, SEEK_SET) == 0);
    char buf[64];
    assert(fgets(buf, sizeof buf, fp) != NULL);
    assert(strcmp(buf, "Hello, rewind!") == 0);

    /* 此时位置指示器在文件末尾附近；调用 rewind 应回到开头 */
    rewind(fp);                     /* [2] 置到文件开头 */

    /* 再次读取，应从头开始读到相同内容 */
    char buf2[64];
    assert(fgets(buf2, sizeof buf2, fp) != NULL);
    assert(strcmp(buf2, "Hello, rewind!") == 0);

    fclose(fp);
}

/* [2] 语义：rewind 等价于 (void)fseek(stream, 0L, SEEK_SET) */
static void test_equivalence_with_fseek(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    assert(fputs("ABCDEF", fp) >= 0);
    fflush(fp);

    /* 用 fseek 定位到中间 */
    assert(fseek(fp, 3L, SEEK_SET) == 0);
    int c1 = fgetc(fp);
    assert(c1 == 'D');

    /* 用 rewind 回到开头 */
    rewind(fp);
    int c2 = fgetc(fp);
    assert(c2 == 'A');              /* 与 fseek(fp,0,SEEK_SET) 效果一致 */

    fclose(fp);
}

/* [2] 语义：rewind 清除错误指示器 */
static void test_rewind_clears_error_indicator(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    assert(fputs("data", fp) >= 0);
    fflush(fp);

    /* 制造一个读错误：以写模式打开后尝试读，或对只写流读取。
     * 这里用 tmpfile 是读写模式，改用另一种方式：先关闭底层再读不可行，
     * 因此用 fseek 到负位置触发错误，使错误指示器被设置。 */
    /* 触发错误指示器：对只读流写入 */
    FILE *ro = tmpfile();
    assert(ro != NULL);
    assert(fputs("x", ro) >= 0);
    fflush(ro);
    /* 重新以只读方式打开同一临时文件不可行，改用 freopen 到只读 */
    /* 简化：用 fseek 到非法位置设置错误指示器 */
    assert(fseek(fp, -1L, SEEK_SET) != 0);   /* 失败，设置错误指示器 */
    assert(ferror(fp) != 0);                 /* 错误指示器已设置 */

    /* rewind 应清除错误指示器 */
    rewind(fp);                              /* [2] 同时清除错误指示器 */
    assert(ferror(fp) == 0);                 /* 错误指示器被清除 */

    /* 且位置回到开头 */
    int c = fgetc(fp);
    assert(c == 'd');                        /* "data" 的第一个字符 */

    fclose(fp);
    fclose(ro);
}

/* [3] 语义：rewind 无返回值（void）。
 *     通过把调用结果赋给 void 表达式来验证其返回类型为 void。 */
static void test_returns_void(void)
{
    FILE *fp = tmpfile();
    assert(fp != NULL);

    /* 若 rewind 有非 void 返回值，下面这种用法仍合法，但
     * 我们通过 _Generic 或函数指针类型来静态确认返回类型为 void。 */
    void (*fp_rewind)(FILE *) = rewind;   /* [3] 返回类型必须是 void */
    (void)fp_rewind;

    rewind(fp);                            /* [3] 作为语句使用，无返回值 */

    fclose(fp);
}

int main(void)
{
    check_prototype();                     /* [1] */
    test_rewind_to_beginning();            /* [2] */
    test_equivalence_with_fseek();         /* [2] */
    test_rewind_clears_error_indicator();  /* [2] */
    test_returns_void();                   /* [3] */

    printf("All positive tests for C99 7.19.9.5 (rewind) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「rewind 的原型为 void rewind(FILE *stream)」：
 * 参数类型不匹配，传入 int 而非 FILE*，gcc -std=c99 应报错
 * （incompatible type for argument 1 of 'rewind'）。 */
void bad_arg_type(void)
{
    int x = 0;
    rewind(x);            /* 错误：参数应为 FILE*，不是 int */
}

/* 违反约束「rewind 返回 void」：
 * 试图使用 rewind 的返回值（void 表达式不能用于赋值/取值），
 * gcc -std=c99 应报错（void value not ignored as it ought to be）。 */
void bad_use_return(void)
{
    FILE *fp = tmpfile();
    int r = rewind(fp);   /* 错误：rewind 返回 void，不能赋给 int */
    (void)r;
    fclose(fp);
}

/* 违反约束「rewind 需要 <stdio.h> 中声明的原型」：
 * 若未包含 <stdio.h>，则 rewind 为隐式声明，C99 下应报错
 * （implicit declaration of function 'rewind'）。
 * 此处仅作示意，实际文件中已包含 <stdio.h>。 */
/* rewind(fp); */

#endif