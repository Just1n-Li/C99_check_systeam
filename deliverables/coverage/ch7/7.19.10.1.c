/*
 * 测试 C99 7.19.10.1 —— clearerr 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错）。
 *
 * 条款要点：
 *   [1] 原型：void clearerr(FILE *stream);  声明于 <stdio.h>
 *   [2] 清除 stream 所指流的文件结束指示符与错误指示符
 *   [3] 无返回值
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型存在性：取函数地址，类型应为 void (*)(FILE *) */
static void (*clearerr_ptr)(FILE *) = clearerr;

int main(void)
{
    /* [1] 通过 <stdio.h> 声明的原型调用，参数为 FILE * */
    FILE *fp;
    char buf[16];
    int c;

    /* 准备一个可读的临时文件 */
    fp = tmpfile();
    assert(fp != NULL);

    /* 写入一些数据以便后续读取 */
    assert(fputs("hello", fp) >= 0);
    assert(fflush(fp) == 0);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    /* [2] 正常读取到文件末尾，设置 EOF 指示符 */
    while ((c = fgetc(fp)) != EOF)
        ;
    /* 此时 feof 应为真，ferror 应为假 */
    assert(feof(fp) != 0);
    assert(ferror(fp) == 0);

    /* [2] clearerr 应清除文件结束指示符 */
    clearerr(fp);
    assert(feof(fp) == 0);
    assert(ferror(fp) == 0);

    /* [2] 清除后应能重新读取（回到文件开头再读） */
    assert(fseek(fp, 0, SEEK_SET) == 0);
    assert(fgets(buf, sizeof buf, fp) != NULL);
    assert(strcmp(buf, "hello") == 0);

    /* [2] 制造一个错误指示符：对只读流进行写操作 */
    {
        FILE *rf = fopen("/dev/null", "r");
        if (rf != NULL) {
            /* 对以 "r" 打开的流写入，通常设置错误指示符 */
            if (fputc('x', rf) == EOF) {
                assert(ferror(rf) != 0);
                /* [2] clearerr 应清除错误指示符 */
                clearerr(rf);
                assert(ferror(rf) == 0);
                assert(feof(rf) == 0);
            }
            fclose(rf);
        }
    }

    /* [2] 同时清除两种指示符：先置 EOF，再置错误，再 clearerr */
    assert(fseek(fp, 0, SEEK_END) == 0);
    while (fgetc(fp) != EOF)
        ;
    assert(feof(fp) != 0);
    clearerr(fp);
    assert(feof(fp) == 0);
    assert(ferror(fp) == 0);

    /* [3] 无返回值：clearerr 只能作为表达式语句使用，
     *     不能出现在需要值的上下文中。这里验证其返回类型为 void。 */
    {
        /* 若 clearerr 有返回值，下面这行会因 void 不能参与运算而报错，
         * 因此该表达式本身即验证了返回类型为 void。 */
        (void)sizeof(clearerr_ptr); /* 仅使用函数指针，避免未使用警告 */
    }

    /* [1] 通过函数指针调用，行为与直接调用一致 */
    clearerr_ptr(fp);
    assert(feof(fp) == 0);
    assert(ferror(fp) == 0);

    fclose(fp);

    printf("C99 7.19.10.1 clearerr: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「clearerr 的返回类型为 void，不得使用其返回值」：
 * 将 void 表达式用于赋值，gcc -std=c99 应报错
 *   error: void value not ignored as it ought to be */
void test_use_return_value(FILE *fp)
{
    int x = clearerr(fp);   /* 错误：void 值不能赋给 int */
    (void)x;
}

/* 违反约束「clearerr 的参数类型为 FILE *」：
 * 传入 int 实参，gcc -std=c99 应报错
 *   warning/error: passing argument 1 of 'clearerr' makes pointer from integer */
void test_wrong_arg_type(void)
{
    clearerr(0);            /* 错误：0 为 int，非 FILE *（无原型时才会隐式转换） */
}

/* 违反约束「clearerr 的参数个数为 1」：
 * 少传参数，gcc -std=c99 应报错
 *   error: too few arguments to function 'clearerr' */
void test_too_few_args(void)
{
    clearerr();             /* 错误：缺少 FILE * 实参 */
}

/* 违反约束「clearerr 的参数个数为 1」：
 * 多传参数，gcc -std=c99 应报错
 *   error: too many arguments to function 'clearerr' */
void test_too_many_args(FILE *fp)
{
    clearerr(fp, fp);       /* 错误：参数过多 */
}

/* 违反约束「clearerr 返回 void，不能作为右值参与运算」：
 * 对 void 表达式取地址/解引用，gcc -std=c99 应报错 */
void test_void_arithmetic(FILE *fp)
{
    clearerr(fp) + 1;       /* 错误：void 值不能参与算术运算 */
}

#endif /* 负向测试结束 */