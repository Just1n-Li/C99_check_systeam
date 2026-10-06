/*
 * 测试 C99 6.4.9 —— Comments（注释）
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过，验证 /*...*/ 与 // 注释的语义。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中）。
 *
 * 覆盖段落：[1] 块注释、[2] 行注释、[3] EXAMPLE、脚注 71（块注释不嵌套）。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 块注释：/* 引入注释，内容仅用于识别多字节字符并寻找 */ 终止符。
 *     下面这行注释本身就是一个块注释。 */
int block_comment_ok = 1; /* 行尾块注释 */

/* [1] 块注释可以跨多行，且内部内容被忽略。
 *     这里故意放入看起来像代码的东西：
 *     int x = 999; 这行不会生效
 *     结束于下一行 */
int after_multiline = 2;

/* [2] 行注释：// 引入注释，直到（不含）下一个换行符。
 *     下面这行整行都是注释：int never_defined = 3;
 */
int line_comment_ok = 4; // 行尾行注释

/* [3] EXAMPLE: "a//b" 是四字符字符串字面量，// 在字符串内不是注释 */
static const char *ex1 = "a//b";

/* [3] EXAMPLE: f = g/**//h; 等价于 f = g / h;  （块注释把两个 / 分开） */
static int g_val = 12;
static int h_val = 3;

/* [3] EXAMPLE: /*//*/ l(); 等价于 l();  —— 块注释 /*//*/ 被整体忽略 */
static int l_called = 0;
static void l(void) { l_called = 1; }

/* [3] EXAMPLE: m = n//**/o + p; 等价于 m = n + p;
 *     因为 //**/o + p; 整行被行注释吞掉，m = n 后换行结束语句。
 *     注意：这里 n 后面没有分号，靠换行结束赋值语句。 */
static int n_val = 5;
static int p_val = 100;

/* [3] EXAMPLE: glue(/,/) k(); 是语法错误而非注释 —— 见负向测试。
 *     这里演示合法的 glue 宏用于其它用途。 */
#define glue(x,y) x##y
static int glue(ab, cd) = 7; /* 展开为 int abcd = 7; */

/* [3] EXAMPLE: //\ 换行 i(); 是两行注释的一部分（行注释续行）。
 *     下面用块注释形式安全演示等价语义：整段被忽略。 */
/* 模拟： //\
   i();   —— 这两行都属于同一条行注释 */

/* 脚注 71：/* ... */ 注释不嵌套。
 *     下面这个块注释中出现的 /* 只是普通字符，直到第一个 */ 结束。 */
/* 外层开始 /* 内层看起来像开始，但只是文本 */ int nested_demo = 8;

int main(void)
{
    /* [1] 块注释语义 */
    assert(block_comment_ok == 1);
    assert(after_multiline == 2);

    /* [2] 行注释语义 */
    assert(line_comment_ok == 4);

    /* [3] EXAMPLE: "a//b" 是四字符字符串字面量 */
    assert(sizeof("a//b") == 5);          /* 4 字符 + '\0' */
    assert(ex1[0] == 'a' && ex1[1] == '/' && ex1[2] == '/' && ex1[3] == 'b');
    assert(ex1[4] == '\0');

    /* [3] EXAMPLE: f = g/**//h; 等价于 f = g / h; */
    {
        int f = g_val/**//h_val;
        assert(f == 4);                   /* 12 / 3 == 4 */
    }

    /* [3] EXAMPLE: /*//*/ l(); 等价于 l(); */
    /*//*/ l();
    assert(l_called == 1);

    /* [3] EXAMPLE: m = n//**/o + p; 等价于 m = n + p; */
    {
        int m;
        int n = n_val;
        int p = p_val;
        m = n//**/o + p;
        assert(m == 105);                 /* 5 + 100 == 105 */
    }

    /* [3] EXAMPLE: glue(/,/) k(); 是语法错误 —— 见负向测试。
     *     这里验证合法 glue 展开。 */
    assert(abcd == 7);

    /* 脚注 71：块注释不嵌套，nested_demo 正常定义 */
    assert(nested_demo == 8);

    printf("C99 6.4.9 Comments: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [3] EXAMPLE: glue(/,/) k(); 是语法错误，不是注释。
 *     宏展开后得到 / / k(); 即两个除号，缺少左操作数，gcc -std=c99 应报错。 */
#define glue(x,y) x##y
glue(/,/) k();

/* [3] EXAMPLE: #include "//e" 是未定义行为（UB），不是约束违反，
 *     因此不放在这里作为负向测试。UB 代码仍能编译通过。 */

/* 说明：本条款 6.4.9 主要是描述性语义（注释如何被识别），
 * 本身没有形如 "shall" 的约束条款。唯一可视为编译期错误的是
 * EXAMPLE 中 glue(/,/) k(); 展开后产生的语法错误。
 * 其余如 #include "//e" 属于未定义行为，不作为负向测试。 */

#endif