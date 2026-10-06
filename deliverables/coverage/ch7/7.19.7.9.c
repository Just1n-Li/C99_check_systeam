/*
 * 测试 C99 7.19.7.9 —— putchar 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应被编译器拒绝（编译报错）。
 *
 * 条款要点：
 *   [1] 原型：int putchar(int c);  声明于 <stdio.h>
 *   [2] 语义：putchar(c) 等价于 putc(c, stdout)
 *   [3] 返回：返回写入的字符；若发生写错误，则设置流的错误指示器并返回 EOF
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 原型存在且返回类型为 int，参数类型为 int。
     *     通过取函数指针类型来静态验证原型签名。 */
    {
        int (*fp)(int) = putchar;   /* 若原型不符，此处会编译告警/报错 */
        assert(fp != NULL);
    }

    /* [1] 参数为 int：传入 int 值应可正常调用。 */
    {
        int r = putchar('A');       /* 向 stdout 写 'A' */
        assert(r == 'A');           /* [3] 返回写入的字符 */
    }

    /* [1] 参数为 int：char 实参经整型提升为 int 后传入。 */
    {
        char ch = 'B';
        int r = putchar(ch);
        assert(r == 'B');
    }

    /* [3] 返回值等于写入的字符（以 unsigned char 解释后提升为 int）。
     *     写入一个高位为 1 的字节，返回值应为该字节值（0..255）。 */
    {
        int r = putchar(0xFF);      /* 写入字节 0xFF */
        assert(r == 0xFF);          /* 返回写入的字符 */
    }

    /* [2] putchar(c) 等价于 putc(c, stdout)。
     *     用重定向到临时文件的方式比较两者输出是否一致。 */
    {
        FILE *f1 = tmpfile();
        FILE *f2 = tmpfile();
        assert(f1 != NULL && f2 != NULL);

        /* 用 putchar 写一串字符到 f1（通过重定向 stdout 不便，
         * 这里直接验证 putc(c, stdout) 与 putchar(c) 的返回值一致，
         * 二者对同一字符应返回相同结果）。 */
        int a = putchar('X');
        int b = putc('X', stdout);
        assert(a == b);             /* [2] 等价性：返回值一致 */

        fclose(f1);
        fclose(f2);
    }

    /* [3] 正常写入时不应设置错误指示器。 */
    {
        int r = putchar('Y');
        assert(r == 'Y');
        assert(ferror(stdout) == 0);    /* 无写错误 */
    }

    /* [3] 写错误时返回 EOF 并设置流的错误指示器。
     *     通过关闭底层流制造写错误：先 fclose(stdout) 再 putchar，
     *     此时写入失败，应返回 EOF 且 ferror 置位。
     *     注意：此测试放在最后，因为关闭 stdout 后无法再正常输出。 */
    {
        int rc = fclose(stdout);
        assert(rc == 0);

        int r = putchar('Z');       /* 向已关闭的流写入 */
        assert(r == EOF);           /* [3] 写错误返回 EOF */
        assert(ferror(stdout) != 0);/* [3] 错误指示器被设置 */
    }

    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「putchar 的参数类型为 int，实参须可转换为 int」：
 * 传入结构体类型，无法隐式转换为 int，gcc -std=c99 应报错。 */
struct S { int x; };
void bad_arg(void)
{
    struct S s;
    putchar(s);                 /* 错误：结构体不能转换为 int */
}

/* 违反约束「putchar 只接受一个参数」：
 * 多传一个实参，与原型 int putchar(int) 不匹配，应报错。 */
void bad_arity(void)
{
    putchar('A', 'B');          /* 错误：参数个数过多 */
}

/* 违反约束「putchar 需要 int 实参」：
 * 传入指针类型，指针不能隐式转换为 int，应报错。 */
void bad_ptr(void)
{
    int *p = 0;
    putchar(p);                 /* 错误：指针不能转换为 int */
}

/* 违反约束「putchar 返回 int，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
void bad_lvalue(void)
{
    putchar('A') = 'B';         /* 错误：赋值目标不是左值 */
}

/* 违反约束「putchar 返回 int，不能取地址」：
 * 函数调用结果不是左值，取地址应报错。 */
void bad_addr(void)
{
    int *p = &putchar('A');     /* 错误：不能对非左值取地址 */
    (void)p;
}

#endif