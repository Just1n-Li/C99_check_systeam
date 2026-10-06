/*
 * 验证 C99 6.11.4 Character escape sequences
 * [1] Lowercase letters as escape sequences are reserved for future standardization.
 *     Other characters may be used in extensions.
 *
 * 预期行为：
 *   正向测试——程序编译、运行通过，所有 assert 成功。
 *   负向测试——本条款属于「未来方向」（Future language directions），
 *              无 Constraints 节，故无强制编译报错的约束可测。
 *              使用未定义的小写字母转义序列（如 \q）属于未定义行为（UB），
 *              编译器通常仅给出警告而非报错，不构成约束违反。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 标准定义的小写字母转义序列——这些是 C99 已定义的，
           它们占用「保留给未来标准化」的小写字母槽位 */

    /* \a — alert (bell)，ASCII 值 7 */
    {
        char c = '\a';
        printf("\\a = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 7);
    }

    /* \b — backspace，ASCII 值 8 */
    {
        char c = '\b';
        printf("\\b = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 8);
    }

    /* \f — form feed，ASCII 值 12 */
    {
        char c = '\f';
        printf("\\f = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 12);
    }

    /* \n — new line，ASCII 值 10 */
    {
        char c = '\n';
        printf("\\n = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 10);
    }

    /* \r — carriage return，ASCII 值 13 */
    {
        char c = '\r';
        printf("\\r = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 13);
    }

    /* \t — horizontal tab，ASCII 值 9 */
    {
        char c = '\t';
        printf("\\t = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 9);
    }

    /* \v — vertical tab，ASCII 值 11 */
    {
        char c = '\v';
        printf("\\v = %d\n", (int)(unsigned char)c);
        assert((int)(unsigned char)c == 11);
    }

    /* [1] 标准定义的非字母转义序列——这些不属于「小写字母」范畴 */

    /* \\ — backslash，ASCII 值 92 */
    assert('\\' == 92);

    /* \' — single quote，ASCII 值 39 */
    assert('\'' == 39);

    /* \" — double quote，ASCII 值 34 */
    assert('\"' == 34);

    /* \? — question mark，ASCII 值 63 */
    assert('\?' == 63);

    /* \0 — null character，值为 0 */
    assert('\0' == 0);

    /* [1] 十六进制转义序列 \xHH —— 以 'x'（小写字母）开头，属于标准定义 */
    {
        char c = '\x41';  /* 'A' */
        assert(c == 'A');
        assert((int)(unsigned char)c == 65);
    }

    /* [1] 八进制转义序列 \NNN —— 以数字开头，不属于字母 */
    {
        char c = '\101';  /* 'A' in octal */
        assert(c == 'A');
    }

    /* [1] 在字符串字面量中使用所有标准小写字母转义序列 */
    {
        const char *s = "\a\b\f\n\r\t\v";
        /* 字符串长度应为 7（每个转义序列对应一个字符） */
        assert(strlen(s) == 7);
    }

    /* [1] 条款区分：小写字母保留 vs 其他字符可用于扩展
           - 小写字母 a,b,f,n,r,t,v,x 已被标准使用
           - 其他小写字母（c,d,e,g,h,i,j,k,l,m,o,p,q,s,u,w,y,z）保留给未来
           - 非字母字符（数字、标点）可用于扩展 */

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
    /*
     * 本条款 6.11.4 属于「未来方向」（6.11 Future language directions），
     * 仅包含段落 [1]，无 Constraints 节。
     *
     * 条款语义：小写字母转义序列保留给未来标准化；其他字符可用于扩展。
     * 这是一条建议性/方向性条款，不是强制性约束。
     *
     * 使用未定义的小写字母转义序列（如 '\q'）属于未定义行为（UB），
     * 而非约束违反。编译器通常给出警告（warning: unknown escape sequence），
     * 但不拒绝编译。因此不构成有效的负向（约束）测试。
     *
     * 以下 #if 0 块展示 UB 代码，仅供参考，不作为约束违反的负向测试：
     */
#if 0
    /* UB（非约束违反）：未定义的小写字母转义序列 '\q'
       gcc -std=c99 -Wall 通常给出: warning: unknown escape sequence '\q' */
    char ub1 = '\q';

    /* UB（非约束违反）：未定义的小写字母转义序列 '\w'
       gcc -std=c99 -Wall 通常给出: warning: unknown escape sequence '\w' */
    char ub2 = '\w';

    /* UB（非约束违反）：未定义的小写字母转义序列 '\y' */
    char ub3 = '\y';

    /* UB（非约束违反）：字符串中的未定义小写字母转义序列 */
    const char *ub4 = "hello\zworld";
#endif

    return 0;
}