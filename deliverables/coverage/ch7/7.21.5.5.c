/*
 * 测试条款：C99 7.21.5.5  strrchr 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型：char *strrchr(const char *s, int c);
 *   [2] 语义：定位 c（转换为 char）在 s 所指字符串中最后一次出现的位置；
 *       终止空字符被视为字符串的一部分。
 *   [3] 返回值：指向该字符的指针；若 c 未出现则返回空指针。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：返回值类型为 char *，参数为 (const char *, int) */
static char *(*proto_check)(const char *, int) = strrchr;

int main(void)
{
    /* [1] 原型可用性：通过函数指针调用，验证签名匹配 */
    {
        const char *s = "hello";
        char *p = proto_check(s, 'l');
        assert(p != NULL);
        assert(*p == 'l');
    }

    /* [2] 定位最后一次出现：字符串 "abcabc" 中 'a' 最后一次出现在下标 3 */
    {
        const char *s = "abcabc";
        char *p = strrchr(s, 'a');
        assert(p != NULL);
        assert(p == s + 3);
        assert(*p == 'a');
        /* 其后应只剩 "bc" */
        assert(strcmp(p, "abc") == 0);
    }

    /* [2] 定位最后一次出现：'b' 在 "abcabc" 中最后一次出现在下标 4 */
    {
        const char *s = "abcabc";
        char *p = strrchr(s, 'b');
        assert(p != NULL);
        assert(p == s + 4);
        assert(*p == 'b');
    }

    /* [2] 只出现一次：'x' 在 "axbc" 中唯一出现在下标 1 */
    {
        const char *s = "axbc";
        char *p = strrchr(s, 'x');
        assert(p != NULL);
        assert(p == s + 1);
    }

    /* [2] 终止空字符被视为字符串的一部分：
     *     strrchr(s, '\0') 应返回指向终止空字符的指针，即 s + strlen(s) */
    {
        const char *s = "hello";
        char *p = strrchr(s, '\0');
        assert(p != NULL);
        assert(p == s + strlen(s));
        assert(*p == '\0');
    }

    /* [2] 空字符串：终止空字符是唯一字符，strrchr("", '\0') 返回指向它的指针 */
    {
        const char *s = "";
        char *p = strrchr(s, '\0');
        assert(p != NULL);
        assert(p == s);
        assert(*p == '\0');
    }

    /* [2] c 被转换为 char：传入 int 值 0x100 + 'a'，转换后为 'a' */
    {
        const char *s = "banana";
        char *p = strrchr(s, 0x100 + 'a');
        assert(p != NULL);
        assert(*p == 'a');
        assert(p == s + 5); /* "banana" 中最后一个 'a' 在下标 5 */
    }

    /* [2] c 被转换为 char：传入 'a' + 256 的等价形式，仍匹配 'a' */
    {
        const char *s = "xyzabc";
        char *p = strrchr(s, 'a' + 256);
        assert(p != NULL);
        assert(*p == 'a');
    }

    /* [3] c 未出现：返回空指针 */
    {
        const char *s = "hello world";
        char *p = strrchr(s, 'z');
        assert(p == NULL);
    }

    /* [3] c 未出现（非空字符串中查找不存在的字符） */
    {
        const char *s = "abcdef";
        char *p = strrchr(s, 'q');
        assert(p == NULL);
    }

    /* [3] 返回的指针可用于修改（s 指向可修改数组时） */
    {
        char buf[] = "hello";
        char *p = strrchr(buf, 'l');
        assert(p != NULL);
        assert(p == buf + 3);
        *p = 'L';
        assert(strcmp(buf, "helLo") == 0);
    }

    /* [2][3] 综合：多次出现，验证返回的是最后一次出现的位置 */
    {
        const char *s = "aXbXcXdXe";
        char *p = strrchr(s, 'X');
        assert(p != NULL);
        assert(p == s + 7);
        assert(strcmp(p, "Xe") == 0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */

#if 0

/* 违反约束「strrchr 的声明必须可见」：
 * 未包含 <string.h> 且未声明 strrchr，调用隐式声明在 C99 中为约束违反，
 * gcc -std=c99 应报错（implicit declaration of function 'strrchr'）。
 * 注意：此处故意不包含 <string.h>。 */
void test_no_decl(void)
{
    char *p = strrchr("abc", 'a'); /* 期望：编译报错，隐式声明 */
    (void)p;
}

/* 违反约束「实参类型必须与原型兼容」：
 * 第一个参数应为 const char *，传入 int 属于约束违反，
 * gcc -std=c99 应报错（incompatible type / passing argument 1）。
 * 注意：需在包含 <string.h> 的前提下测试。 */
void test_bad_arg1(void)
{
    char *p = strrchr(42, 'a'); /* 期望：编译报错，int 不能转为 const char * */
    (void)p;
}

/* 违反约束「实参个数必须与原型一致」：
 * 原型要求 2 个参数，只传 1 个属于约束违反，
 * gcc -std=c99 应报错（too few arguments to function 'strrchr'）。 */
void test_too_few_args(void)
{
    char *p = strrchr("abc"); /* 期望：编译报错，参数太少 */
    (void)p;
}

/* 违反约束「实参个数必须与原型一致」：
 * 原型要求 2 个参数，传 3 个属于约束违反，
 * gcc -std=c99 应报错（too many arguments to function 'strrchr'）。 */
void test_too_many_args(void)
{
    char *p = strrchr("abc", 'a', 'b'); /* 期望：编译报错，参数太多 */
    (void)p;
}

/* 违反约束「返回值不可赋值给不兼容类型」：
 * strrchr 返回 char *，赋给 int 属于约束违反（需显式转换），
 * gcc -std=c99 应报错（incompatible types / initialization）。 */
void test_bad_return_assign(void)
{
    int x = strrchr("abc", 'a'); /* 期望：编译报错，char * 赋给 int */
    (void)x;
}

/* 违反约束「非左值不可赋值」：
 * strrchr 的返回值是右值（非左值），对其赋值属于约束违反，
 * gcc -std=c99 应报错（lvalue required as left operand of assignment）。 */
void test_assign_to_return(void)
{
    strrchr("abc", 'a') = (char *)0; /* 期望：编译报错，非左值赋值 */
}

#endif /* 负向测试结束 */