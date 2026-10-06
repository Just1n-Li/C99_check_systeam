/*
 * 测试条款：C99 7.19.7.5  The getc function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件本身仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int getc(FILE *stream);
 *   [2] 等价于 fgetc；若实现为宏，可能多次求值 stream，故实参不应有副作用。
 *   [3] 返回下一个字符；到达 EOF 时设置 eof 指示器并返回 EOF；
 *       读错误时设置 error 指示器并返回 EOF。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：getc 接受 FILE* 并返回 int。
 *     用函数指针类型强制匹配原型，若原型不符则编译报错。 */
static int (*getc_proto_check)(FILE *) = getc;

static void test_prototype(void)
{
    /* [1] 原型为 int getc(FILE *stream); */
    assert(getc_proto_check != NULL);
}

static void test_basic_read(void)
{
    FILE *fp;
    int c;

    /* [3] 从流中读取下一个字符 */
    fp = tmpfile();
    assert(fp != NULL);

    assert(fputs("ABC", fp) >= 0);
    rewind(fp);

    c = getc(fp);
    assert(c == 'A');           /* [3] 返回下一个字符 */
    c = getc(fp);
    assert(c == 'B');
    c = getc(fp);
    assert(c == 'C');

    /* [3] 到达文件末尾：设置 eof 指示器并返回 EOF */
    c = getc(fp);
    assert(c == EOF);
    assert(feof(fp) != 0);      /* eof 指示器被设置 */

    fclose(fp);
}

static void test_eof_indicator(void)
{
    FILE *fp;
    int c;

    /* [3] 空文件：第一次 getc 即返回 EOF 并设置 eof 指示器 */
    fp = tmpfile();
    assert(fp != NULL);

    assert(feof(fp) == 0);      /* 初始未设置 */
    c = getc(fp);
    assert(c == EOF);
    assert(feof(fp) != 0);      /* eof 指示器被设置 */

    /* 再次调用仍返回 EOF，eof 指示器保持设置 */
    c = getc(fp);
    assert(c == EOF);
    assert(feof(fp) != 0);

    fclose(fp);
}

static void test_equivalence_with_fgetc(void)
{
    FILE *fp1, *fp2;
    int c1, c2;
    int i;

    /* [2] getc 等价于 fgetc：对相同输入应产生相同结果 */
    fp1 = tmpfile();
    fp2 = tmpfile();
    assert(fp1 != NULL && fp2 != NULL);

    assert(fputs("Hello, getc!", fp1) >= 0);
    assert(fputs("Hello, getc!", fp2) >= 0);
    rewind(fp1);
    rewind(fp2);

    for (i = 0; i < 20; i++) {
        c1 = getc(fp1);
        c2 = fgetc(fp2);
        assert(c1 == c2);
        if (c1 == EOF)
            break;
    }

    fclose(fp1);
    fclose(fp2);
}

static void test_read_error_indicator(void)
{
    FILE *fp;
    int c;

    /* [3] 读错误：设置 error 指示器并返回 EOF。
     *     以只写方式打开文件后尝试读取，触发读错误。 */
    fp = tmpfile();
    assert(fp != NULL);

    /* 以只写模式重新打开同一底层文件不可行，改用 fopen 只写模式 */
    fclose(fp);

    fp = fopen("getc_test_tmp.txt", "w");
    assert(fp != NULL);
    assert(fputs("data", fp) >= 0);
    fclose(fp);

    /* 以只写模式打开，读取应失败 */
    fp = fopen("getc_test_tmp.txt", "w");
    assert(fp != NULL);

    errno = 0;
    c = getc(fp);
    /* 只写流上读取：返回 EOF 并设置 error 指示器 */
    assert(c == EOF);
    assert(ferror(fp) != 0);    /* error 指示器被设置 */

    fclose(fp);
    remove("getc_test_tmp.txt");
}

static void test_no_side_effect_requirement(void)
{
    FILE *fp;
    int c;

    /* [2] 实参不应有副作用：这里使用无副作用的简单 FILE* 变量。
     *     若实现为宏，多次求值该实参也不会改变程序语义。 */
    fp = tmpfile();
    assert(fp != NULL);
    assert(fputs("X", fp) >= 0);
    rewind(fp);

    c = getc(fp);               /* 实参 fp 无副作用 */
    assert(c == 'X');

    fclose(fp);
}

int main(void)
{
    test_prototype();
    test_basic_read();
    test_eof_indicator();
    test_equivalence_with_fgetc();
    test_read_error_indicator();
    test_no_side_effect_requirement();

    printf("C99 7.19.7.5 getc: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型为 int getc(FILE *stream)」：
 * 实参类型不匹配——传入 int 而非 FILE*。
 * gcc -std=c99 应报错：passing argument 1 of 'getc' makes pointer
 * from integer without a cast（或 incompatible type）。 */
void neg_wrong_arg_type(void)
{
    int c = getc(42);           /* 42 不是 FILE* */
    (void)c;
}

/* 违反约束「[1] 原型为 int getc(FILE *stream)」：
 * 实参个数错误——不传参数。
 * gcc -std=c99 应报错：too few arguments to function 'getc'。 */
void neg_too_few_args(void)
{
    int c = getc();             /* 缺少 stream 实参 */
    (void)c;
}

/* 违反约束「[1] 原型为 int getc(FILE *stream)」：
 * 实参个数错误——传入过多参数。
 * gcc -std=c99 应报错：too many arguments to function 'getc'。 */
void neg_too_many_args(void)
{
    FILE *fp = 0;
    int c = getc(fp, fp);       /* 多余实参 */
    (void)c;
}

/* 违反约束「[1] 原型为 int getc(FILE *stream)」：
 * 实参类型不兼容——传入 const char*。
 * gcc -std=c99 应报错：incompatible type for argument 1 of 'getc'。 */
void neg_incompatible_arg(void)
{
    const char *s = "not a FILE";
    int c = getc(s);            /* const char* 不是 FILE* */
    (void)c;
}

/* 违反约束「[1] 原型为 int getc(FILE *stream)」：
 * 对 getc 的返回值赋值（getc 返回 int 右值，非左值）。
 * gcc -std=c99 应报错：lvalue required as left operand of assignment。 */
void neg_assign_to_result(void)
{
    FILE *fp = 0;
    getc(fp) = 0;               /* 不能给函数返回值赋值 */
}

/* 违反约束「[1] 原型为 int getc(FILE *stream)」：
 * 取 getc 返回值的地址（非左值）。
 * gcc -std=c99 应报错：lvalue required as unary '&' operand。 */
void neg_address_of_result(void)
{
    FILE *fp = 0;
    int *p = &getc(fp);         /* 不能取函数返回值的地址 */
    (void)p;
}

#endif /* 负向测试结束 */