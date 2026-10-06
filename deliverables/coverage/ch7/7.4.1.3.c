/*
 * 测试条款：C99 7.4.1.3 The isblank function
 *
 * 预期行为：
 *   正向测试：包含 <ctype.h>，调用 isblank(int)，验证：
 *     [1] 原型 int isblank(int c) 可用；
 *     [2] 空格 ' ' 与水平制表符 '\t' 返回真；
 *         在 "C" locale 下，其它字符（含换行、垂直制表、回车、换页、
 *         字母、数字、标点、非空白控制字符）返回假；
 *         参数为 EOF 时返回假。
 *   负向测试：违反约束的代码应编译报错（放在 #if 0 中）。
 *
 * 编译：gcc -std=c99 -Wall -Wextra -pedantic test.c -o test
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <locale.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：isblank 接受 int 参数并返回 int。
 *     通过取函数指针类型来静态验证原型。 */
static int (*isblank_proto)(int) = isblank;

int main(void)
{
    /* 确保处于 "C" locale，以便 [2] 中 "C" locale 的断言成立 */
    setlocale(LC_ALL, "C");

    /* [1] 原型可用性：函数指针非空，且可正常调用 */
    assert(isblank_proto != NULL);

    /* [2] 标准空白字符：空格 ' ' 返回真 */
    assert(isblank(' ') != 0);
    assert(isblank_proto(' ') != 0);

    /* [2] 标准空白字符：水平制表符 '\t' 返回真 */
    assert(isblank('\t') != 0);
    assert(isblank_proto('\t') != 0);

    /* [2] "C" locale 下，isblank 仅对标准空白字符返回真。
     *     以下字符均应为假。 */

    /* 其它空白类字符（isspace 为真但不是标准空白） */
    assert(isblank('\n') == 0);   /* 换行 */
    assert(isblank('\v') == 0);   /* 垂直制表 */
    assert(isblank('\f') == 0);   /* 换页 */
    assert(isblank('\r') == 0);   /* 回车 */

    /* 字母、数字、标点 */
    assert(isblank('a') == 0);
    assert(isblank('Z') == 0);
    assert(isblank('0') == 0);
    assert(isblank('9') == 0);
    assert(isblank('.') == 0);
    assert(isblank(',') == 0);
    assert(isblank('_') == 0);

    /* 非空白控制字符 */
    assert(isblank('\0') == 0);
    assert(isblank('\a') == 0);   /* 响铃 */
    assert(isblank('\b') == 0);   /* 退格 */

    /* [2] 参数为 EOF 时返回假 */
    assert(isblank(EOF) == 0);

    /* [2] 参数必须可表示为 unsigned char 或 EOF；
     *     对 0..UCHAR_MAX 范围内的所有值，结果只应为
     *     ' ' 或 '\t' 时为真。 */
    {
        int c;
        for (c = 0; c <= 255; c++) {
            int r = isblank(c);
            if (c == ' ' || c == '\t') {
                assert(r != 0);
            } else {
                assert(r == 0);
            }
        }
    }

    /* [2] 返回值语义：真/假（非零/零），与具体非零值无关 */
    assert((isblank(' ') != 0) == 1);
    assert((isblank('x') != 0) == 0);

    printf("All positive tests for C99 7.4.1.3 isblank passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isblank 的原型为 int isblank(int)」：
 * 以不兼容的实参类型调用（如传指针），gcc -std=c99 应报错
 * （在 C99 中，无原型声明已不允许；此处直接以错误类型调用原型函数）。 */
{
    char *p = "x";
    isblank(p);   /* 期望：error: incompatible type for argument 1 of 'isblank' */
}

/* 违反约束「isblank 返回 int」：
 * 试图把返回值当作结构体使用，类型不匹配，应报错。 */
{
    struct S { int x; } s;
    s = isblank(' ');   /* 期望：error: incompatible types when assigning to type 'struct S' */
}

/* 违反约束「isblank 接受一个 int 参数」：
 * 参数个数不匹配，应报错。 */
{
    isblank();          /* 期望：error: too few arguments to function 'isblank' */
    isblank(' ', ' ');  /* 期望：error: too many arguments to function 'isblank' */
}

/* 违反约束「isblank 的实参必须可表示为 unsigned char 或 EOF」：
 * 注意：这是运行期语义要求，编译器通常不报错，故不在此作为负向测试。
 * 仅列出以说明为何不放入负向块。 */

#endif /* 负向测试结束 */