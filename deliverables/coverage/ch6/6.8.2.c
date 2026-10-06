/*
 * 验证 C99 条款 6.8.2 Compound statement (复合语句)
 * 预期行为：
 * 正向测试：能编译并运行通过，验证复合语句的语法结构及“复合语句是一个块”的语义。
 * 负向测试：违反语法规则和块作用域约束，应编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void) {
    int x = 10;

    /* [1] Syntax: 空复合语句（没有任何 block-item） */
    {
    }

    /* [1] Syntax: 包含 block-item-list 的复合语句，block-item 可以是 declaration 或 statement */
    {
        int a = 1;      /* declaration */
        printf("a = %d\n", a); /* statement (expression statement) */
        a++;            /* statement */
        int b = a + 1;  /* declaration (C99 允许混合声明和语句) */
        assert(b == 3);
    }

    /* [2] Semantics: A compound statement is a block. 
       验证复合语句作为一个块，具有独立的作用域，内部声明的变量会遮蔽外部同名变量 */
    {
        int x = 20;     /* 遮蔽外层的 x */
        assert(x == 20);
        
        /* 嵌套的复合语句也是一个块 */
        {
            assert(x == 20);
            int x = 30; /* 遮蔽上一层块的 x */
            assert(x == 30);
        }
        
        assert(x == 20); /* 离开内层块后，恢复上一层块的 x */
    }
    
    assert(x == 10); /* 离开复合语句后，外层 x 不受影响 */

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* [1] Syntax 违反：复合语句必须由大括号包围，缺少右大括号 */
{
    int a = 1;
    a++;
/* 缺少 }，gcc -std=c99 应报语法错误：expected '}' at end of input */

/* [2] Semantics 违反：复合语句是一个块，块内声明的变量在块外不可见（超出作用域） */
{
    int block_var = 100;
}
block_var = 200; 
/* 错误: 'block_var' undeclared，gcc -std=c99 应报错：block_var 未声明 */

#endif