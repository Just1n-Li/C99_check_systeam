/*
 * 验证 C99 6.5.1 Primary expressions
 * 正向测试：应能编译并运行通过（覆盖 [1]~[5] 全部语义）
 * 负向测试：违反语法/约束，应编译报错（脚注 79、[5] 非左值性保持）
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* [2] 函数声明，用于测试 identifier 作为函数指示器 */
static int func(int x) { return x * 2; }
static void void_func(void) { printf("void_func called\n"); }

/* 返回结构体的函数，用于负向测试 [5] 括号不改变非左值性 */
struct S { int a; };
static struct S make_s(void) { struct S s = { 42 }; return s; }

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    /* [1] 语法：primary-expression 的四种形式：
     * identifier, constant, string-literal, ( expression )，以下逐条测试 */

    /* [2] identifier 作为对象（左值）：声明为对象的标识符是左值 */
    int obj = 10;
    obj = 20;  /* obj 是左值，可以被赋值 */
    assert(obj == 20);
    printf("[2] identifier as object (lvalue): obj = %d\n", obj);

    /* [2] identifier 作为函数指示器：声明为函数的标识符是函数指示器 */
    {
        int result = func(5);
        assert(result == 10);
        printf("[2] identifier as function designator: func(5) = %d\n", result);
    }

    /* [3] constant 是 primary expression，类型取决于形式和值（详见 6.4.4） */
    {
        /* 整型常量 */
        int ci = 42;
        assert(ci == 42);
        /* 浮点常量 */
        double cd = 3.14;
        assert(cd > 3.13 && cd < 3.15);
        /* 字符常量 */
        char cc = 'A';
        assert(cc == 65);
        /* 枚举常量 */
        enum Color { RED, GREEN, BLUE };
        int ce = GREEN;
        assert(ce == 1);
        printf("[3] constant: int=%d, double=%.2f, char=%c, enum=%d\n",
               ci, cd, cc, ce);
    }

    /* [4] string literal 是 primary expression，是左值（类型详见 6.4.5） */
    {
        const char *str = "hello";
        assert(strcmp(str, "hello") == 0);
        /* 字符串字面量是左值，可以取地址（类型为 char[6]） */
        const char (*p)[6] = &"hello";
        assert((*p)[0] == 'h');
        printf("[4] string literal is lvalue: \"%s\"\n", str);
    }

    /* [5] 括号表达式保持类型和值，与未括号表达式相同 */
    {
        int val = (100);  /* 常量加括号，值不变 */
        assert(val == 100);
        printf("[5] parenthesized constant preserves value: (100) = %d\n", val);
    }

    /* [5] 括号表达式保持左值性：若未括号表达式是左值，括号后也是左值 */
    {
        int x = 1;
        (x) = 50;  /* (x) 仍然是左值，可以被赋值 */
        assert(x == 50);
        printf("[5] parenthesized lvalue stays lvalue: (x) = %d\n", x);
    }

    /* [5] 括号表达式保持函数指示器性 */
    {
        int result = (func)(7);  /* (func) 仍然是函数指示器，可被调用 */
        assert(result == 14);
        printf("[5] parenthesized function designator: (func)(7) = %d\n", result);
    }

    /* [5] 括号表达式保持 void 表达式性 */
    {
        (void_func)();  /* (void_func)() 仍然是 void 表达式 */
        printf("[5] parenthesized void expression called OK\n");
    }

    /* [5] 括号表达式保持字符串字面量的左值性 */
    {
        const char (*p)[6] = &("hello");  /* ("hello") 仍然是左值 */
        assert((*p)[0] == 'h');
        printf("[5] parenthesized string literal stays lvalue\n");
    }

    printf("\nAll positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 语法/约束，应编译报错 ========== */
#if 0
    /* 脚注 79 / [2]: 未声明的 identifier 违反语法
     * "an undeclared identifier is a violation of the syntax"
     * gcc -std=c99 应报错: 'undeclared_var' undeclared (first use in this function) */
    undeclared_var = 1;

    /* 脚注 79 / [2]: 在表达式中使用未声明的标识符
     * gcc -std=c99 应报错: 'unknown_name' undeclared */
    int y = unknown_name + 1;

    /* [5] 括号表达式保持非左值性：
     * make_s().a 不是左值（函数返回的结构体成员是非左值），
     * 加括号 (make_s().a) 也不应变为左值，对它赋值应报错
     * gcc -std=c99 应报错: lvalue required as left operand of assignment */
    (make_s().a) = 10;

    /* [5] 括号表达式保持非左值性：
     * 强制转换 (int)42 的结果不是左值，
     * 加括号 ((int)42) 也不应变为左值，对它赋值应报错
     * gcc -std=c99 应报错: lvalue required as left operand of assignment */
    ((int)42) = 10;
#endif

    return 0;
}