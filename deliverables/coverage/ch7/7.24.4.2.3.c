/*
 * 测试目标：C99 7.24.4.2.3  wmemcpy 函数
 *
 * 预期行为：
 *   正向测试：包含 <wchar.h>，调用 wmemcpy 复制 n 个宽字符，
 *             返回值等于 s1，目标缓冲区内容与源一致，应能编译并运行通过。
 *   负向测试：违反约束的代码（如参数类型错误、缺少原型声明等）应编译报错。
 *
 * 条款结构：
 *   [1] Synopsis: wchar_t *wmemcpy(wchar_t * restrict s1,
 *                                 const wchar_t * restrict s2, size_t n);
 *   [2] Description: 从 s2 指向的对象复制 n 个宽字符到 s1 指向的对象。
 *   [3] Returns: 返回 s1 的值。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 头文件与原型：包含 <wchar.h> 后 wmemcpy 可用，签名匹配 */
static void test_synopsis(void)
{
    /* 通过函数指针验证原型签名（restrict 限定符不影响类型兼容性） */
    wchar_t *(*fp)(wchar_t * restrict, const wchar_t * restrict, size_t) = wmemcpy;
    assert(fp != NULL);
    printf("[1] wmemcpy 原型可用，函数指针赋值成功\n");
}

/* [2] 复制 n 个宽字符：目标内容应与源前 n 个宽字符一致 */
static void test_copy_n(void)
{
    wchar_t src[8] = L"ABCDEFG";   /* 含结尾 L'\0'，共 8 个宽字符 */
    wchar_t dst[8];
    size_t i;

    /* 先填充 dst 为哨兵值，便于验证只复制了 n 个 */
    for (i = 0; i < 8; ++i)
        dst[i] = L'#';

    /* 复制前 4 个宽字符 */
    wmemcpy(dst, src, 4);

    /* 前 4 个应与 src 一致 */
    for (i = 0; i < 4; ++i)
        assert(dst[i] == src[i]);

    /* 第 5 个及之后应保持哨兵值，未被修改 */
    for (i = 4; i < 8; ++i)
        assert(dst[i] == L'#');

    printf("[2] 复制 n=4 个宽字符，目标前 4 个正确，其余未改动\n");
}

/* [2] 复制 0 个宽字符：目标不应被修改 */
static void test_copy_zero(void)
{
    wchar_t src[4] = L"XYZ";
    wchar_t dst[4] = L"abc";
    size_t i;

    wmemcpy(dst, src, 0);

    /* n=0 时不应写入任何内容 */
    assert(dst[0] == L'a');
    assert(dst[1] == L'b');
    assert(dst[2] == L'c');
    assert(dst[3] == L'\0');
    (void)i;

    printf("[2] n=0 时目标缓冲区未被修改\n");
}

/* [2] 复制整个宽字符串（含结尾空宽字符） */
static void test_copy_full_string(void)
{
    wchar_t src[] = L"Hello";
    wchar_t dst[16];
    size_t n = sizeof(src) / sizeof(src[0]); /* 含 L'\0' */

    wmemcpy(dst, src, n);

    /* 应得到完整字符串，包括结尾 L'\0' */
    assert(wcscmp(dst, L"Hello") == 0);
    assert(dst[5] == L'\0');

    printf("[2] 复制整个宽字符串（含结尾空宽字符）成功\n");
}

/* [3] 返回值应等于 s1 */
static void test_return_value(void)
{
    wchar_t src[4] = L"abc";
    wchar_t dst[4];
    wchar_t *ret;

    ret = wmemcpy(dst, src, 4);

    /* 返回值必须等于第一个实参 s1 */
    assert(ret == dst);

    printf("[3] 返回值等于 s1（%p == %p）\n", (void *)ret, (void *)dst);
}

/* [3] 返回值可用于链式调用 */
static void test_return_chaining(void)
{
    wchar_t a[4] = L"123";
    wchar_t b[4];
    wchar_t c[4];
    wchar_t *ret;

    /* wmemcpy 返回 s1，可继续作为下一次调用的目标 */
    ret = wmemcpy(c, wmemcpy(b, a, 4), 4);

    assert(ret == c);
    assert(wcscmp(b, L"123") == 0);
    assert(wcscmp(c, L"123") == 0);

    printf("[3] 返回值支持链式调用\n");
}

/* [2] 复制二进制宽字符数据（含非可打印宽字符） */
static void test_binary_data(void)
{
    wchar_t src[5] = { 0x0000, 0x0041, 0x4E2D, 0xFFFF, 0x0000 };
    wchar_t dst[5];
    size_t i;

    wmemcpy(dst, src, 5);

    for (i = 0; i < 5; ++i)
        assert(dst[i] == src[i]);

    printf("[2] 复制含非可打印宽字符的二进制数据成功\n");
}

int main(void)
{
    printf("=== C99 7.24.4.2.3 wmemcpy 正向测试 ===\n");

    test_synopsis();
    test_copy_n();
    test_copy_zero();
    test_copy_full_string();
    test_return_value();
    test_return_chaining();
    test_binary_data();

    printf("=== 全部正向测试通过 ===\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「实参类型必须与原型匹配」：
 * wmemcpy 的第一个参数应为 wchar_t *，传入 int * 应报错。
 * 期望：gcc -std=c99 报 incompatible pointer type 警告/错误。 */
void neg_wrong_first_arg(void)
{
    int a[4];
    wchar_t b[4];
    wmemcpy(a, b, 4);   /* int* 与 wchar_t* 不兼容 */
}

/* 违反约束「实参类型必须与原型匹配」：
 * 第二个参数应为 const wchar_t *，传入 char * 应报错。
 * 期望：gcc -std=c99 报 incompatible pointer type。 */
void neg_wrong_second_arg(void)
{
    wchar_t a[4];
    char b[4];
    wmemcpy(a, b, 4);   /* char* 与 const wchar_t* 不兼容 */
}

/* 违反约束「实参个数必须与原型一致」：
 * wmemcpy 需要 3 个实参，只传 2 个应报错。
 * 期望：gcc -std=c99 报 too few arguments to function 'wmemcpy'。 */
void neg_too_few_args(void)
{
    wchar_t a[4], b[4];
    wmemcpy(a, b);      /* 缺少第三个参数 n */
}

/* 违反约束「实参个数必须与原型一致」：
 * 传入 4 个实参应报错。
 * 期望：gcc -std=c99 报 too many arguments to function 'wmemcpy'。 */
void neg_too_many_args(void)
{
    wchar_t a[4], b[4];
    wmemcpy(a, b, 4, 0); /* 多传一个参数 */
}

/* 违反约束「第三个参数必须为 size_t 类型」：
 * 传入结构体类型应报错。
 * 期望：gcc -std=c99 报 incompatible type for argument 3。 */
struct NotSize { int x; };
void neg_wrong_third_arg(void)
{
    wchar_t a[4], b[4];
    struct NotSize s;
    wmemcpy(a, b, s);   /* 结构体不能转换为 size_t */
}

/* 违反约束「返回值类型为 wchar_t *」：
 * 将返回值赋给不兼容的指针类型应报错。
 * 期望：gcc -std=c99 报 incompatible pointer type。 */
void neg_wrong_return_type(void)
{
    wchar_t a[4], b[4];
    int *p = wmemcpy(a, b, 4);  /* wchar_t* 赋给 int* 不兼容 */
    (void)p;
}

/* 违反约束「第一个参数不能为 const 限定」：
 * s1 为 wchar_t *（非 const），传入 const wchar_t * 应报错。
 * 期望：gcc -std=c99 报 discards 'const' qualifier。 */
void neg_const_first_arg(void)
{
    const wchar_t a[4] = L"abc";
    wchar_t b[4];
    wmemcpy(a, b, 4);   /* 向 const 目标写入，丢弃 const 限定 */
}

#endif /* 负向测试结束 */