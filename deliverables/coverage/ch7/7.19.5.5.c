/*
 * 测试 C99 7.19.5.5 —— setbuf 函数
 *
 * 预期行为：
 *   正向测试：包含 <stdio.h> 后，setbuf 的声明与语义可用；
 *             setbuf(stream, buf) 等价于 setvbuf(stream, buf, _IOFBF, BUFSIZ)；
 *             setbuf(stream, NULL) 等价于 setvbuf(stream, NULL, _IONBF, 0)；
 *             setbuf 返回 void（无返回值）。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：[1] 原型/头文件、[2] 语义等价、[3] 无返回值。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件 <stdio.h> 提供 setbuf 的声明，原型为：
 *     void setbuf(FILE * restrict stream, char * restrict buf);
 *     下面通过取函数指针来静态验证返回类型为 void、参数类型匹配。 */
static void (*fp_setbuf)(FILE * restrict, char * restrict) = setbuf;

/* [2] 语义：setbuf(stream, buf) 等价于 setvbuf(stream, buf, _IOFBF, BUFSIZ)。
 *     这里用一个临时文件流，分别用两种方式设置缓冲，行为应一致（都能成功）。 */
static void test_equivalence_full_buffer(void)
{
    FILE *f1 = tmpfile();
    FILE *f2 = tmpfile();
    assert(f1 != NULL);
    assert(f2 != NULL);

    static char buf1[BUFSIZ];
    static char buf2[BUFSIZ];

    /* setbuf 形式 */
    setbuf(f1, buf1);
    /* 等价的 setvbuf 形式 */
    int r = setvbuf(f2, buf2, _IOFBF, BUFSIZ);
    assert(r == 0);

    /* 两者都应能正常写入并读回 */
    assert(fputs("hello setbuf\n", f1) >= 0);
    assert(fputs("hello setbuf\n", f2) >= 0);
    assert(fflush(f1) == 0);
    assert(fflush(f2) == 0);

    fclose(f1);
    fclose(f2);
}

/* [2] 语义：setbuf(stream, NULL) 等价于 setvbuf(stream, NULL, _IONBF, 0)，
 *     即关闭缓冲（无缓冲）。 */
static void test_equivalence_unbuffered(void)
{
    FILE *f1 = tmpfile();
    FILE *f2 = tmpfile();
    assert(f1 != NULL);
    assert(f2 != NULL);

    /* setbuf 形式：buf 为 NULL -> 无缓冲 */
    setbuf(f1, NULL);
    /* 等价的 setvbuf 形式 */
    int r = setvbuf(f2, NULL, _IONBF, 0);
    assert(r == 0);

    assert(fputs("unbuffered\n", f1) >= 0);
    assert(fputs("unbuffered\n", f2) >= 0);

    fclose(f1);
    fclose(f2);
}

/* [3] 返回值：setbuf 返回 void，不能用于赋值或取值。
 *     正向：仅作为语句调用，合法。 */
static void test_returns_void(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);

    /* 合法：作为表达式语句调用，忽略“无返回值” */
    setbuf(f, NULL);

    fclose(f);
}

/* [1] 原型参数带 restrict 限定：传入普通指针合法（restrict 只影响别名语义，
 *     不改变调用形式）。这里验证普通 char* 与 FILE* 实参可正常传入。 */
static void test_restrict_params_accept_plain_pointers(void)
{
    FILE *f = tmpfile();
    assert(f != NULL);
    char mybuf[BUFSIZ];
    char *p = mybuf;      /* 普通指针，非 restrict 限定 */
    setbuf(f, p);
    fclose(f);
}

int main(void)
{
    /* 引用函数指针，确保 [1] 的原型被实例化检查 */
    assert(fp_setbuf == setbuf);

    test_equivalence_full_buffer();          /* [2] */
    test_equivalence_unbuffered();           /* [2] */
    test_returns_void();                     /* [3] */
    test_restrict_params_accept_plain_pointers(); /* [1] */

    printf("C99 7.19.5.5 setbuf: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「setbuf 返回 void，无返回值」：
 * 试图把 setbuf 的“返回值”赋给变量，void 表达式不能用作赋值右操作数。
 * 期望：gcc -std=c99 报错，如 "void value not ignored as it ought to be"。 */
void neg_assign_void_result(FILE *f, char *buf)
{
    int x = setbuf(f, buf);   /* 错误：void 不能赋给 int */
    (void)x;
}

/* 违反约束「setbuf 返回 void」：
 * 试图对 void 表达式取地址/解引用，void 不是对象类型。
 * 期望：编译报错。 */
void neg_deref_void_result(FILE *f, char *buf)
{
    *setbuf(f, buf);          /* 错误：不能解引用 void 表达式 */
}

/* 违反约束「setbuf 返回 void」：
 * 在需要值的上下文中使用 void 表达式（如作为函数实参）。
 * 期望：编译报错。 */
void neg_use_void_as_argument(FILE *f, char *buf)
{
    printf("%d\n", setbuf(f, buf));  /* 错误：void 不能作为 %d 的实参 */
}

/* 违反约束「原型要求第一个参数为 FILE *」：
 * 传入 int，类型不匹配。
 * 期望：编译报错（incompatible type / 参数类型不兼容）。 */
void neg_wrong_first_arg_type(void)
{
    int not_a_file = 0;
    setbuf(not_a_file, NULL);  /* 错误：第一个实参应为 FILE* */
}

/* 违反约束「原型要求第二个参数为 char *」：
 * 传入 int*，类型不匹配。
 * 期望：编译报错。 */
void neg_wrong_second_arg_type(FILE *f)
{
    int ibuf[BUFSIZ];
    setbuf(f, ibuf);           /* 错误：第二个实参应为 char*，int* 不兼容 */
}

/* 违反约束「调用前需有 setbuf 的可见声明」：
 * 若未包含 <stdio.h>，C99 中隐式函数声明已不再是合法形式（C99 移除了隐式声明）。
 * 期望：gcc -std=c99 报错/警告为错误，如 "implicit declaration of function 'setbuf'"。
 * 注意：此处仅为示意，实际测试需在无 <stdio.h> 的翻译单元中验证。 */
void neg_no_declaration(void)
{
    /* 假设本文件未包含 <stdio.h>：
     * setbuf(0, 0);  // 错误：隐式声明在 C99 中不允许 */
}

#endif