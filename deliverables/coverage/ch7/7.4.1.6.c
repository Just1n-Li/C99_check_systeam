/*
 * 测试 C99 7.4.1.6 —— isgraph 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 条款要点：
 *   [1] 原型：int isgraph(int c);  声明于 <ctype.h>
 *   [2] 语义：测试任何可打印字符，但空格 (' ') 除外。
 *       即：isgraph(c) 为真  <=>  c 是可打印字符且 c != ' '
 *       等价于 isprint(c) && c != ' '
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：isgraph 接受 int 参数并返回 int。
 *     通过函数指针类型匹配来验证原型签名。 */
static int (*isgraph_ptr)(int) = isgraph;

int main(void)
{
    /* [1] 头文件 <ctype.h> 提供 isgraph 声明，可正常调用。 */
    int r = isgraph('A');
    assert(r != 0);

    /* [2] 空格 ' ' 不是图形字符：isgraph(' ') 必须为 0。 */
    assert(isgraph(' ') == 0);

    /* [2] 可打印且非空格的字符应为真。
     *     覆盖：数字、大写字母、小写字母、标点、符号。 */
    assert(isgraph('0') != 0);
    assert(isgraph('9') != 0);
    assert(isgraph('A') != 0);
    assert(isgraph('Z') != 0);
    assert(isgraph('a') != 0);
    assert(isgraph('z') != 0);
    assert(isgraph('!') != 0);
    assert(isgraph('~') != 0);
    assert(isgraph('@') != 0);
    assert(isgraph('#') != 0);
    assert(isgraph('+') != 0);
    assert(isgraph('{') != 0);

    /* [2] 非可打印字符应为假：控制字符。 */
    assert(isgraph('\0') == 0);   /* NUL */
    assert(isgraph('\n') == 0);   /* 换行 */
    assert(isgraph('\t') == 0);   /* 制表符 */
    assert(isgraph('\r') == 0);   /* 回车 */
    assert(isgraph('\v') == 0);   /* 垂直制表 */
    assert(isgraph('\f') == 0);   /* 换页 */
    assert(isgraph(0x01) == 0);   /* SOH */
    assert(isgraph(0x1F) == 0);   /* US */
    assert(isgraph(0x7F) == 0);   /* DEL */

    /* [2] 与 isprint 的关系：isgraph(c) == (isprint(c) && c != ' ')
     *     对可打印字符逐一验证。 */
    {
        int c;
        for (c = 0; c <= 0x7F; ++c) {
            int expect = (isprint(c) != 0) && (c != ' ');
            int got = (isgraph(c) != 0);
            assert(got == expect);
        }
    }

    /* [2] 空格是唯一被排除的可打印字符：
     *     对 0..127 中所有可打印字符，只有 ' ' 使 isgraph 为假。 */
    {
        int c;
        for (c = 0; c <= 0x7F; ++c) {
            if (isprint(c)) {
                if (c == ' ') {
                    assert(isgraph(c) == 0);
                } else {
                    assert(isgraph(c) != 0);
                }
            }
        }
    }

    /* [1] 参数为 int：传入 EOF 应返回 0（EOF 不是图形字符）。 */
    assert(isgraph(EOF) == 0);

    /* [1] 参数为 int：传入字符值（已提升为 int）正常工作。 */
    {
        char ch = 'Q';
        assert(isgraph(ch) != 0);   /* char 提升为 int */
    }

    /* [1] 通过函数指针调用，验证原型签名一致。 */
    assert(isgraph_ptr('x') != 0);
    assert(isgraph_ptr(' ') == 0);

    printf("C99 7.4.1.6 isgraph: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isgraph 的实参必须为 int 类型（算术类型可隐式转换）」：
 * 传入结构体类型，无法转换为 int，gcc -std=c99 应报错。 */
struct S { int x; } s;
isgraph(s);   /* error: incompatible type for argument 1 of 'isgraph' */

/* 违反约束「isgraph 的实参必须为 int 类型」：
 * 传入指针类型，指针不能隐式转换为 int，gcc -std=c99 应报错。 */
int *p = 0;
isgraph(p);   /* error: incompatible type for argument 1 of 'isgraph' */

/* 违反约束「isgraph 的实参必须为 int 类型」：
 * 传入 double，浮点不能隐式转换为 int（无原型时才会默认提升，
 * 但有原型时实参类型必须可赋值给 int），gcc -std=c99 应报错。 */
isgraph(3.14);   /* error: incompatible type for argument 1 of 'isgraph' */

/* 违反约束「isgraph 只接受一个实参」：
 * 传入两个实参，gcc -std=c99 应报错。 */
isgraph('a', 'b');   /* error: too many arguments to function 'isgraph' */

/* 违反约束「isgraph 需要一个实参」：
 * 不传实参，gcc -std=c99 应报错。 */
isgraph();   /* error: too few arguments to function 'isgraph' */

/* 违反约束「isgraph 返回 int，不能作为函数被赋值调用」：
 * 对函数返回值赋值，gcc -std=c99 应报错。 */
isgraph('a') = 5;   /* error: lvalue required as left operand of assignment */

#endif