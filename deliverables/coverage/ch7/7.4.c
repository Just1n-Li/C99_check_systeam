/*
 * 测试 C99 7.4 <ctype.h> 字符处理
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 头文件声明字符分类/映射函数；实参为 int，其值须可表示为
 *       unsigned char 或等于 EOF；否则行为未定义（UB，不作为负向测试）。
 *   [2] 行为受当前 locale 影响；仅在非 "C" locale 下才有 locale 相关
 *       方面的函数在下方注明。
 *   [3] printing character / control character 的定义；所有字母和数字
 *       都是 printing character。
 *   Forward references: EOF (7.19.1), localization (7.11)。
 *   Footnotes 172/173。
 */

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] <ctype.h> 声明了字符分类与映射函数。
     *     这里逐一取函数地址，验证声明存在且类型为 int(int)。 */
    {
        int (*p_isalnum)(int)  = isalnum;
        int (*p_isalpha)(int)  = isalpha;
        int (*p_isblank)(int)  = isblank;
        int (*p_iscntrl)(int)  = iscntrl;
        int (*p_isdigit)(int)  = isdigit;
        int (*p_isgraph)(int)  = isgraph;
        int (*p_islower)(int)  = islower;
        int (*p_isprint)(int)  = isprint;
        int (*p_ispunct)(int)  = ispunct;
        int (*p_isspace)(int)  = isspace;
        int (*p_isupper)(int)  = isupper;
        int (*p_isxdigit)(int) = isxdigit;
        int (*p_tolower)(int)  = tolower;
        int (*p_toupper)(int)  = toupper;

        assert(p_isalnum && p_isalpha && p_isblank && p_iscntrl);
        assert(p_isdigit && p_isgraph && p_islower && p_isprint);
        assert(p_ispunct && p_isspace && p_isupper && p_isxdigit);
        assert(p_tolower && p_toupper);
    }

    /* [1] 实参为 int，其值须可表示为 unsigned char 或等于 EOF。
     *     在 "C" locale 下测试典型字符的分类结果。 */
    {
        /* 字母 */
        assert(isalpha('A') && isalpha('z'));
        assert(isupper('A') && !isupper('a'));
        assert(islower('a') && !islower('A'));

        /* 数字 */
        assert(isdigit('0') && isdigit('9') && !isdigit('a'));
        assert(isxdigit('0') && isxdigit('F') && isxdigit('a'));
        assert(!isxdigit('g'));

        /* 字母或数字 */
        assert(isalnum('A') && isalnum('5') && !isalnum(' '));

        /* 空白 */
        assert(isspace(' ') && isspace('\t') && isspace('\n'));
        assert(isspace('\v') && isspace('\f') && isspace('\r'));
        assert(!isspace('A'));

        /* blank：空格与水平制表符 */
        assert(isblank(' ') && isblank('\t'));
        assert(!isblank('\n'));

        /* 标点 */
        assert(ispunct('.') && ispunct(',') && ispunct('!'));
        assert(!ispunct('A') && !ispunct(' '));

        /* 可打印字符：字母、数字、标点、空格都是 printing character */
        assert(isprint('A') && isprint('0') && isprint(' ') && isprint('~'));
        assert(!isprint('\n') && !isprint('\t'));

        /* 图形字符：可打印且非空格 */
        assert(isgraph('A') && isgraph('!') && !isgraph(' '));
        assert(!isgraph('\n'));

        /* 控制字符 */
        assert(iscntrl('\n') && iscntrl('\t') && iscntrl('\0'));
        assert(!iscntrl('A') && !iscntrl(' '));
    }

    /* [1] EOF 是合法实参（值等于 EOF），函数应接受它。 */
    {
        /* 对 EOF 调用分类函数是合法的（结果由实现定义，但不得 UB）。 */
        (void)isalpha(EOF);
        (void)isdigit(EOF);
        (void)isspace(EOF);
        (void)isprint(EOF);
        (void)tolower(EOF);
        (void)toupper(EOF);
        /* 只要不崩溃即通过；这里断言 EOF 是负值（7.19.1）。 */
        assert(EOF < 0);
    }

    /* [1] 映射函数 tolower / toupper 的语义。 */
    {
        assert(tolower('A') == 'a');
        assert(tolower('Z') == 'z');
        assert(tolower('a') == 'a');   /* 已是小写，不变 */
        assert(tolower('5') == '5');   /* 非字母，不变 */

        assert(toupper('a') == 'A');
        assert(toupper('z') == 'Z');
        assert(toupper('A') == 'A');   /* 已是大写，不变 */
        assert(toupper('5') == '5');   /* 非字母，不变 */
    }

    /* [1] 实参值可表示为 unsigned char：遍历 0..UCHAR_MAX 全部合法，
     *     不应触发 UB（这里只验证不崩溃并保持一致性）。 */
    {
        for (int c = 0; c <= UCHAR_MAX; ++c) {
            int a = isalpha(c);
            int d = isdigit(c);
            int al = isalnum(c);
            /* 字母或数字 => alnum 为真 */
            assert(al == (a || d));
            /* 字母必为可打印（[3]：所有字母和数字都是 printing character） */
            if (a || d) {
                assert(isprint(c));
            }
        }
    }

    /* [3] 所有字母和数字都是 printing character。 */
    {
        for (int c = 'A'; c <= 'Z'; ++c) assert(isprint(c));
        for (int c = 'a'; c <= 'z'; ++c) assert(isprint(c));
        for (int c = '0'; c <= '9'; ++c) assert(isprint(c));
    }

    /* [3] printing character 与 control character 互补（在 "C" locale 下
     *     对 0..UCHAR_MAX 的每个值，二者恰有一个为真）。 */
    {
        for (int c = 0; c <= UCHAR_MAX; ++c) {
            int pr = isprint(c);
            int ct = iscntrl(c);
            assert(pr != ct);   /* 恰有一个成立 */
        }
    }

    /* [2] 行为受当前 locale 影响。在 "C" locale 下，locale 相关函数
     *     的行为是确定的；这里用 setlocale 显式设为 "C" 并复测。 */
    {
        /* 注意：setlocale 声明于 <locale.h>（7.11）。 */
        /* 为不引入额外头文件依赖，这里仅说明：在 "C" locale 下，
         * 上述断言均成立。若实现支持 setlocale，可如下验证： */
        /* #include <locale.h>; setlocale(LC_ALL, "C"); */
        assert(isalpha('A'));   /* "C" locale 下 'A' 是字母 */
        assert(isdigit('0'));   /* "C" locale 下 '0' 是数字 */
    }

    /* [Footnote 173] 在七位 US ASCII 实现中：
     *   printing characters: 0x20 (space) .. 0x7E (tilde)
     *   control characters : 0x00 (NUL) .. 0x1F (US) 以及 0x7F (DEL)
     * 若实现使用 ASCII，则以下断言成立。 */
    {
        if ('A' == 0x41 && '~' == 0x7E) {   /* 检测 ASCII 实现 */
            assert(isprint(0x20) && isprint(0x7E));
            assert(!isprint(0x1F) && !isprint(0x7F));
            assert(iscntrl(0x00) && iscntrl(0x1F) && iscntrl(0x7F));
            assert(!iscntrl(0x20) && !iscntrl(0x7E));
        }
    }

    printf("C99 7.4 <ctype.h> 正向测试全部通过。\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* 违反约束「<ctype.h> 函数实参为 int」：
     * 传入结构体类型，无法隐式转换为 int，gcc -std=c99 应报错。 */
    struct S { int x; } s;
    isalpha(s);          /* 期望：error: incompatible type for argument 1 */

    /* 违反约束「实参为 int」：
     * 传入指针类型，指针不能隐式转换为 int，应报错。 */
    char *p = "x";
    isdigit(p);          /* 期望：error: incompatible type for argument 1 */

    /* 违反约束「函数返回 int」：
     * 把 isalpha 的返回值赋给结构体，类型不兼容，应报错。 */
    struct S t;
    t = isalpha('A');    /* 期望：error: incompatible types in assignment */

    /* 违反约束「函数返回 int」：
     * 对函数调用结果取成员（int 无成员），应报错。 */
    isalpha('A').x;      /* 期望：error: request for member 'x' in something
                            not a structure or union */

    /* 违反约束「函数返回 int，非左值」：
     * 对函数调用结果赋值，应报错。 */
    isalpha('A') = 1;    /* 期望：error: lvalue required as left operand
                            of assignment */

    /* 违反约束「函数返回 int，非左值」：
     * 对 tolower 调用结果取地址，应报错。 */
    int *q = &tolower('A');  /* 期望：error: lvalue required as unary '&'
                                operand */

    /* 违反约束「函数返回 int」：
     * 用函数返回值初始化数组，应报错。 */
    int arr[isalpha('A')];   /* 期望：error: variably modified 'arr' at file
                                scope（或类似） */

#endif

    return 0;
}