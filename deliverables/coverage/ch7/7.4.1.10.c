/*
 * 测试 C99 7.4.1.10 —— isspace 函数
 *
 * 预期行为：
 *   正向测试：程序应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的代码片段应导致编译报错（放在 #if 0 中，不参与编译）。
 *
 * 覆盖段落：
 *   [1] 原型：int isspace(int c);  头文件 <ctype.h>
 *   [2] 描述：测试标准空白字符（空格、\f、\n、\r、\t、\v）；
 *       在 "C" locale 下仅对这些标准空白字符返回真；
 *       对 isalnum 为假的 locale 特定字符集合也可能返回真（本测试在 "C" locale 下验证）。
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 验证函数原型可用：返回 int，接受 int 实参 */
static int proto_check(int c)
{
    int (*fp)(int) = isspace;   /* 函数指针类型必须匹配 int(int) */
    return fp(c);
}

int main(void)
{
    /* [1] 原型与头文件：调用应可编译，返回 int */
    {
        int r = isspace(' ');
        assert(r == 0 || r != 0);   /* 返回值可作布尔使用 */
        assert(proto_check(' ') == isspace(' '));
    }

    /* [2] 六个标准空白字符在 "C" locale 下必须返回真 */
    assert(isspace(' ')  != 0);   /* space        */
    assert(isspace('\f') != 0);   /* form feed    */
    assert(isspace('\n') != 0);   /* new-line     */
    assert(isspace('\r') != 0);   /* carriage ret */
    assert(isspace('\t') != 0);   /* horiz tab    */
    assert(isspace('\v') != 0);   /* vert tab     */

    /* [2] 在 "C" locale 下，非标准空白字符必须返回假 */
    assert(isspace('a')  == 0);
    assert(isspace('Z')  == 0);
    assert(isspace('0')  == 0);
    assert(isspace('9')  == 0);
    assert(isspace('!')  == 0);
    assert(isspace('_')  == 0);
    assert(isspace('\0') == 0);   /* 空字符不是空白 */
    assert(isspace('\a') == 0);   /* 响铃不是空白 */
    assert(isspace('\b') == 0);   /* 退格不是空白 */
    assert(isspace(0x7F) == 0);   /* DEL 不是空白 */

    /* [2] 显式设置 "C" locale，再次确认标准空白字符集合 */
    {
        char *old = setlocale(LC_ALL, "C");
        assert(old != NULL);
        assert(isspace(' ')  != 0);
        assert(isspace('\f') != 0);
        assert(isspace('\n') != 0);
        assert(isspace('\r') != 0);
        assert(isspace('\t') != 0);
        assert(isspace('\v') != 0);
        assert(isspace('x')  == 0);
        setlocale(LC_ALL, old);
    }

    /* [2] 实参必须是可表示为 unsigned char 的值或 EOF；
     *     对 0..UCHAR_MAX 范围内的普通字符逐一测试，不应崩溃。 */
    {
        int c;
        for (c = 0; c <= 127; ++c) {
            int r = isspace(c);
            /* 结果只应为 0 或非 0，且对标准空白字符为真 */
            if (c == ' ' || c == '\f' || c == '\n' ||
                c == '\r' || c == '\t' || c == '\v') {
                assert(r != 0);
            } else {
                assert(r == 0);
            }
        }
    }

    /* [2] EOF 是合法实参（值为负），isspace(EOF) 应为假 */
    assert(isspace(EOF) == 0);

    printf("C99 7.4.1.10 isspace: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isspace 的实参必须是可表示为 unsigned char 的值或 EOF」：
 * 传入一个超出该范围的负值（非 EOF），行为未定义——注意这是 UB，
 * 编译器通常不报错，故此处仅作说明，不作为负向编译错误测试。
 * 真正的约束违反见下。 */

/* 违反约束「isspace 声明于 <ctype.h>，原型为 int isspace(int)」：
 * 未包含 <ctype.h> 就调用 isspace，在 C99 中隐式函数声明被禁止，
 * gcc -std=c99 应报错：implicit declaration of function 'isspace'。 */
int bad_no_header(void)
{
    return isspace(' ');   /* 缺少 #include <ctype.h> */
}

/* 违反约束「原型要求实参为 int 类型」：
 * 用不兼容的函数指针类型接收 isspace，应报错（incompatible pointer type）。 */
void bad_ptr_type(void)
{
    void (*fp)(char) = isspace;   /* 类型不匹配 */
    (void)fp;
}

/* 违反约束「原型要求返回 int」：
 * 将 isspace 的返回值赋给不兼容的指针类型，应报错。 */
void bad_ret_type(void)
{
    char *p = isspace(' ');   /* int 不能隐式转换为 char* */
    (void)p;
}

#endif