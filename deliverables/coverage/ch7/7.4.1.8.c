/*
 * 测试条款：C99 7.4.1.8 —— isprint 函数
 *
 * 预期行为：
 *   正向测试：包含 <ctype.h> 后调用 isprint(int)，对可打印字符（含空格 ' '）
 *             返回非零，对不可打印字符返回 0；程序应能编译并运行通过。
 *   负向测试：违反约束的代码应被编译器拒绝（编译报错）。
 *
 * 覆盖段落：
 *   [1] 头文件 <ctype.h> 与原型 int isprint(int c);
 *   [2] 判定“可打印字符（含空格 ' '）”
 */

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件与原型：确认 isprint 可用，且形参为 int、返回 int */
static int (*isprint_proto_check)(int) = isprint;

int main(void)
{
    /* [1] 原型检查：函数指针类型匹配，说明声明为 int isprint(int) */
    assert(isprint_proto_check != NULL);

    /* [2] 空格 ' ' 是可打印字符（条款明确“including space (' ')”） */
    assert(isprint(' ') != 0);

    /* [2] 常见可打印字符：字母、数字、标点、符号 */
    assert(isprint('A') != 0);
    assert(isprint('z') != 0);
    assert(isprint('0') != 0);
    assert(isprint('9') != 0);
    assert(isprint('!') != 0);
    assert(isprint('~') != 0);
    assert(isprint('@') != 0);
    assert(isprint('#') != 0);

    /* [2] 不可打印字符：控制字符应返回 0 */
    assert(isprint('\0') == 0);   /* NUL */
    assert(isprint('\n') == 0);   /* 换行 */
    assert(isprint('\t') == 0);   /* 制表 */
    assert(isprint('\r') == 0);   /* 回车 */
    assert(isprint('\v') == 0);   /* 垂直制表 */
    assert(isprint('\f') == 0);   /* 换页 */
    assert(isprint('\a') == 0);   /* 响铃 */
    assert(isprint('\b') == 0);   /* 退格 */

    /* [2] 遍历整个可打印 ASCII 区间 0x20..0x7E，全部应为可打印 */
    {
        int c;
        for (c = 0x20; c <= 0x7E; ++c) {
            assert(isprint(c) != 0);
        }
    }

    /* [2] 遍历 0x00..0x1F 控制字符区间，全部应为不可打印 */
    {
        int c;
        for (c = 0x00; c <= 0x1F; ++c) {
            assert(isprint(c) == 0);
        }
    }

    /* [2] 0x7F (DEL) 不是可打印字符 */
    assert(isprint(0x7F) == 0);

    /* [1] 参数为 int：传入 EOF 应安全返回 0（EOF 非可打印字符） */
    assert(isprint(EOF) == 0);

    /* [1] 参数为 int：传入 unsigned char 提升后的值应正常工作 */
    {
        unsigned char uc = 'Q';
        assert(isprint((int)uc) != 0);
    }

    /* [1] 参数为 int：传入 char 提升后的值应正常工作 */
    {
        char ch = 'k';
        assert(isprint(ch) != 0);
    }

    /* [2] 返回值语义：非零表示“是可打印字符”，0 表示“不是” */
    assert((isprint(' ') != 0) == 1);
    assert((isprint('\n') != 0) == 0);

    printf("C99 7.4.1.8 isprint: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「isprint 的实参应为 int 类型（可隐式转换为 int 的整型）」：
 * 传入结构体类型实参，无法转换为 int，gcc -std=c99 应报错。 */
struct S { int x; } s;
isprint(s);

/* 违反约束「isprint 的实参应为整型」：
 * 传入指针类型，指针不能隐式转换为 int，gcc -std=c99 应报错。 */
int *p = 0;
isprint(p);

/* 违反约束「isprint 的实参应为整型」：
 * 传入 double 类型，浮点不能隐式转换为 int 作为函数实参（无原型除外），
 * 此处有原型，gcc -std=c99 应报错。 */
isprint(3.14);

/* 违反约束「isprint 的实参个数应为 1」：
 * 传入两个实参，gcc -std=c99 应报错。 */
isprint('a', 'b');

/* 违反约束「isprint 的实参个数应为 1」：
 * 不传实参，gcc -std=c99 应报错。 */
isprint();

/* 违反约束「isprint 的返回值不是左值」：
 * 对函数调用结果赋值，gcc -std=c99 应报错。 */
isprint('a') = 1;

/* 违反约束「isprint 的返回值不是左值」：
 * 对函数调用结果取地址，gcc -std=c99 应报错。 */
int *q = &isprint('a');

#endif