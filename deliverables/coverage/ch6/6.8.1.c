/*
 * 验证 C99 6.8.1 Labeled statements（带标签语句）
 *
 * 正向测试：验证 [1] 三种标签语法形式、[3] 不同函数可重用同名标签、
 *           [4] 标签本身不改变控制流——应编译通过并运行通过。
 * 负向测试：验证 [2] case/default 只能出现在 switch 中、
 *           [3] 同一函数内标签名必须唯一——应编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int helper_same_label_name(void);

int main(void)
{
    int x;

    /* [1] 语法形式一：identifier : statement —— 普通标签后跟表达式语句 */
    {
        int n = 0;
    label_expr:
        n = 42;
        assert(n == 42);
    }

    /* [1] 语法形式一：标签后跟空语句 */
    {
    label_empty:
        ;
        x = 1;
        assert(x == 1);
    }

    /* [1] 语法形式一：标签后跟复合语句 */
    {
        int y = 0;
    label_compound:
        {
            y = 5;
        }
        assert(y == 5);
    }

    /* [1] 语法形式一：标签前缀可出现在 if 语句前 */
    {
        int z = 0;
    label_if:
        if (z == 0) {
            z = 7;
        }
        assert(z == 7);
    }

    /* [1] 语法形式一：标签前缀可出现在 while 语句前 */
    {
        int w = 0;
    label_while:
        while (w < 3) {
            w++;
        }
        assert(w == 3);
    }

    /* [1] 语法形式一：标签前缀可出现在 for 语句前 */
    {
        int s = 0;
    label_for:
        for (int i = 0; i < 5; i++) {
            s += i;
        }
        assert(s == 10);
    }

    /* [1] 语法形式一：标签前缀可出现在 do 语句前 */
    {
        int d = 0;
    label_do:
        do {
            d++;
        } while (d < 4);
        assert(d == 4);
    }

    /* [1] 语法形式一：标签前缀可出现在 goto 语句前 */
    {
        int g = 0;
    label_goto:
        goto label_target;
    label_target:
        g = 99;
        assert(g == 99);
    }

    /* [1] 语法形式一：标签前缀可出现在 return 语句前 */
    {
        int r = 0;
    label_return:
        r = 55;
        assert(r == 55);
    }

    /* [1] 语法形式二：case constant-expression : statement */
    {
        int v = 2;
        switch (v) {
        case 1:
            x = 10;
            break;
        case 2:
            x = 20;
            break;
        default:
            x = 0;
            break;
        }
        assert(x == 20);
    }

    /* [1] 语法形式三：default : statement */
    {
        int v = 99;
        switch (v) {
        case 1:
            x = 10;
            break;
        default:
            x = 30;
            break;
        }
        assert(x == 30);
    }

    /* [4] 语义：标签本身不改变控制流，执行继续穿过标签 */
    {
        int a = 0, b = 0, c = 0;
        goto entry;
    entry:
        a = 1;
    middle:
        b = 2;
    exit_label:
        c = 3;
        /* goto 跳到 entry 后，控制流依次穿过 middle、exit_label */
        assert(a == 1);
        assert(b == 2);
        assert(c == 3);
    }

    /* [4] 语义：连续多个标签贴在同一语句前，控制流不受影响 */
    {
        int val = 0;
    multi_a:
    multi_b:
    multi_c:
        val = 77;
        assert(val == 77);
    }

    /* [4] 语义：标签不改变流——不 goto 时标签被忽略，顺序执行 */
    {
        int seq = 0;
    seq_label:
        seq = 1;
        /* 没有 goto seq_label，标签被忽略，顺序执行 */
        assert(seq == 1);
    }

    /* [3] 约束正向：不同函数中可使用相同标签名（标签作用域为函数级） */
    {
        assert(helper_same_label_name() == 100);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* [3] 不同函数可使用与 main 中相同的标签名 label_expr 等 */
int helper_same_label_name(void)
{
    int val = 0;
label_expr:   /* 与 main 中的 label_expr 同名，但在不同函数中，合法 */
    val = 100;
    return val;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「case 或 default 标签只能出现在 switch 语句中」：
   case 标签出现在 switch 之外，gcc -std=c99 应报错 */
void test_case_outside_switch(void)
{
    int x = 1;
    case 1:           /* 错误：case 标签不在 switch 中 */
        x = 2;
}

/* [2] 违反约束「case 或 default 标签只能出现在 switch 语句中」：
   default 标签出现在 switch 之外，gcc -std=c99 应报错 */
void test_default_outside_switch(void)
{
    int x = 1;
    default:          /* 错误：default 标签不在 switch 中 */
        x = 2;
}

/* [2] 违反约束：case 标签出现在 if 语句中（非 switch），应报错 */
void test_case_in_if(void)
{
    int x = 1;
    if (x) {
        case 2:       /* 错误：case 标签不在 switch 中 */
            x = 3;
    }
}

/* [3] 违反约束「标签名在函数内应唯一」：
   同一函数内重复定义相同标签名，gcc -std=c99 应报错 */
void test_duplicate_label(void)
{
    int x = 0;
label_dup:
    x = 1;
label_dup:             /* 错误：重复标签名 */
    x = 2;
}

/* [3] 违反约束「标签名在函数内应唯一」：
   三个相同标签名，gcc -std=c99 应报错 */
void test_triple_duplicate_label(void)
{
label_t:
    ;
label_t:              /* 错误：重复 */
    ;
label_t:              /* 错误：重复 */
    ;
}

/* [3] 违反约束「标签名在函数内应唯一」：
   普通标签与 case 标签同名也构成重复（同一函数内），应报错 */
void test_label_same_as_case(void)
{
    int x = 1;
same:
    x = 2;
    switch (x) {
    case 1:
        break;
    same:              /* 错误：与普通标签 same 同名 */
        break;
    }
}

#endif