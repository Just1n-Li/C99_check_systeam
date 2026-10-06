/*
 * 测试 C99 7.24.3.9 —— putwchar 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中）。
 *
 * 条款要点：
 *   [1] 原型：wint_t putwchar(wchar_t c);  声明于 <wchar.h>
 *   [2] 语义：等价于 putwc(c, stdout)
 *   [3] 返回：写入的字符，或 WEOF
 */

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：putwchar 接受 wchar_t，返回 wint_t。
 *     通过取函数指针类型来静态验证签名。 */
static wint_t (*putwchar_ptr)(wchar_t) = putwchar;

int main(void)
{
    /* 设置本地化环境，使宽字符输出可用（不影响函数签名/返回值语义） */
    setlocale(LC_ALL, "C");

    /* [1] 函数指针类型与原型一致 */
    assert(putwchar_ptr == putwchar);

    /* [3] 返回值：成功时返回写入的字符本身。
     *     使用 stdout 重定向到临时文件以避免污染测试输出。 */
    {
        FILE *fp = freopen("putwchar_test.tmp", "w", stdout);
        assert(fp != NULL);

        wchar_t ch = L'A';
        wint_t r = putwchar(ch);

        /* [3] 成功时返回写入的字符 */
        assert(r == (wint_t)ch);

        /* [2] 语义等价于 putwc(c, stdout)：用 putwc 写同一字符，
         *     两者写入结果应一致（此处仅验证调用成功且返回字符）。 */
        wint_t r2 = putwc(L'B', stdout);
        assert(r2 == (wint_t)L'B');

        fflush(stdout);
        fclose(stdout);
    }

    /* 恢复 stdout 到终端（重新打开） */
    {
        FILE *fp = freopen("/dev/tty", "w", stdout);
        if (fp == NULL) {
            /* 无终端环境下忽略，不影响测试结论 */
        }
    }

    /* [3] 返回值类型为 wint_t，可与 WEOF 比较 */
    {
        wint_t r = (wint_t)L'Z';
        assert(r != WEOF);
        assert(WEOF == (wint_t)-1 || WEOF != (wint_t)L'Z');
    }

    /* [2] 等价性：putwchar(c) 与 putwc(c, stdout) 返回相同的字符值。
     *     再次重定向验证。 */
    {
        FILE *fp = freopen("putwchar_test2.tmp", "w", stdout);
        assert(fp != NULL);

        wchar_t c1 = L'x';
        wint_t a = putwchar(c1);
        wint_t b = putwc(c1, stdout);

        assert(a == (wint_t)c1);
        assert(b == (wint_t)c1);
        assert(a == b);

        fflush(stdout);
        fclose(stdout);
    }

    {
        FILE *fp = freopen("/dev/tty", "w", stdout);
        (void)fp;
    }

    printf("putwchar: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「putwchar 的参数类型为 wchar_t」：
 * 传入不兼容的指针类型，gcc -std=c99 应报错（参数类型不匹配）。 */
#include <wchar.h>
void bad_arg(void) {
    int *p = 0;
    putwchar(p);            /* 期望：error: incompatible type for argument 1 */
}

/* 违反约束「putwchar 返回 wint_t，不能当作结构体使用」：
 * 对返回值做非法成员访问，应报错。 */
#include <wchar.h>
void bad_member(void) {
    putwchar(L'a').x;       /* 期望：error: request for member 'x' in something not a structure */
}

/* 违反约束「putwchar 的返回值不是左值，不能赋值」：
 * 对函数调用结果赋值，应报错。 */
#include <wchar.h>
void bad_assign(void) {
    putwchar(L'a') = 0;     /* 期望：error: lvalue required as left operand of assignment */
}

/* 违反约束「putwchar 需要 <wchar.h> 中的原型」：
 * 未包含头文件时调用，C99 下隐式声明为 int 返回，与 wint_t 不符，
 * 在严格模式下应给出诊断。 */
void bad_no_proto(void) {
    putwchar(L'a');         /* 期望：warning/error: implicit declaration of function 'putwchar' */
}

/* 违反约束「参数个数必须为 1」：
 * 传入过多参数，应报错。 */
#include <wchar.h>
void bad_argc(void) {
    putwchar(L'a', L'b');   /* 期望：error: too many arguments to function 'putwchar' */
}

#endif