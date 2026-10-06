/*
 * 测试条款：C99 7.24.4.2.4  The wmemmove function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型：wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
 *   [2] 语义：把 s2 指向对象中的 n 个宽字符拷贝到 s1 指向的对象；
 *       拷贝行为如同先复制到不重叠的临时数组，再复制到 s1（因此允许重叠）。
 *   [3] 返回值：返回 s1 的值。
 */

#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stddef.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：确认 wmemmove 的签名与标准一致。
 *     通过取函数指针类型来静态验证返回类型与参数类型。 */
static wchar_t *(*fp_wmemmove)(wchar_t *, const wchar_t *, size_t) = wmemmove;

/* [2] 基本拷贝语义：非重叠区域，n 个宽字符被完整拷贝。 */
static void test_basic_copy(void)
{
    wchar_t src[8] = { L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h' };
    wchar_t dst[8] = { 0 };

    wchar_t *ret = wmemmove(dst, src, 8);

    /* [3] 返回值必须等于 s1（即 dst）。 */
    assert(ret == dst);

    /* [2] 内容被正确拷贝。 */
    for (size_t i = 0; i < 8; ++i) {
        assert(dst[i] == src[i]);
    }
    /* 源对象未被修改。 */
    assert(src[0] == L'a' && src[7] == L'h');
}

/* [2] 部分拷贝：只拷贝前 n 个宽字符，其余保持不变。 */
static void test_partial_copy(void)
{
    wchar_t src[6] = { L'1', L'2', L'3', L'4', L'5', L'6' };
    wchar_t dst[6] = { L'X', L'X', L'X', L'X', L'X', L'X' };

    wchar_t *ret = wmemmove(dst, src, 3);

    assert(ret == dst);          /* [3] */
    assert(dst[0] == L'1');      /* [2] */
    assert(dst[1] == L'2');
    assert(dst[2] == L'3');
    assert(dst[3] == L'X');      /* 未被拷贝的部分保持原值 */
    assert(dst[4] == L'X');
    assert(dst[5] == L'X');
}

/* [2] n == 0：不拷贝任何字符，返回 s1。 */
static void test_zero_length(void)
{
    wchar_t buf[4] = { L'p', L'q', L'r', L's' };
    wchar_t *ret = wmemmove(buf, buf, 0);

    assert(ret == buf);          /* [3] */
    assert(buf[0] == L'p');      /* [2] 内容不变 */
    assert(buf[1] == L'q');
    assert(buf[2] == L'r');
    assert(buf[3] == L's');
}

/* [2] 重叠区域（向后移动，dst > src）：
 *     这正是 wmemmove 与 wmemcpy 的关键区别——wmemmove 允许重叠。
 *     行为如同先复制到临时数组，因此结果等价于“先取快照再写入”。 */
static void test_overlap_forward(void)
{
    wchar_t buf[8] = { L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H' };

    /* 把 buf[0..3] 拷贝到 buf[2..5]（重叠）。 */
    wchar_t *ret = wmemmove(buf + 2, buf, 4);

    assert(ret == buf + 2);      /* [3] */
    /* 期望结果：buf = A B A B C D G H */
    assert(buf[0] == L'A');
    assert(buf[1] == L'B');
    assert(buf[2] == L'A');
    assert(buf[3] == L'B');
    assert(buf[4] == L'C');
    assert(buf[5] == L'D');
    assert(buf[6] == L'G');
    assert(buf[7] == L'H');
}

/* [2] 重叠区域（向前移动，dst < src）：
 *     同样必须得到“先快照后写入”的结果。 */
static void test_overlap_backward(void)
{
    wchar_t buf[8] = { L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H' };

    /* 把 buf[2..5] 拷贝到 buf[0..3]（重叠）。 */
    wchar_t *ret = wmemmove(buf, buf + 2, 4);

    assert(ret == buf);          /* [3] */
    /* 期望结果：buf = C D E F E F G H */
    assert(buf[0] == L'C');
    assert(buf[1] == L'D');
    assert(buf[2] == L'E');
    assert(buf[3] == L'F');
    assert(buf[4] == L'E');
    assert(buf[5] == L'F');
    assert(buf[6] == L'G');
    assert(buf[7] == L'H');
}

/* [2] 完全重叠（s1 == s2）：内容不变，返回 s1。 */
static void test_self_overlap(void)
{
    wchar_t buf[5] = { L'w', L'x', L'y', L'z', L'!' };
    wchar_t *ret = wmemmove(buf, buf, 5);

    assert(ret == buf);          /* [3] */
    assert(buf[0] == L'w');      /* [2] */
    assert(buf[1] == L'x');
    assert(buf[2] == L'y');
    assert(buf[3] == L'z');
    assert(buf[4] == L'!');
}

/* [2] 处理宽字符（含非 ASCII 值），验证按 wchar_t 单位拷贝。 */
static void test_wide_values(void)
{
    wchar_t src[4] = { L'\u4e2d', L'\u6587', L'\u6d4b', L'\u8bd5' }; /* 中文测试 */
    wchar_t dst[4] = { 0 };

    wchar_t *ret = wmemmove(dst, src, 4);

    assert(ret == dst);          /* [3] */
    assert(dst[0] == L'\u4e2d'); /* [2] */
    assert(dst[1] == L'\u6587');
    assert(dst[2] == L'\u6d4b');
    assert(dst[3] == L'\u8bd5');
}

/* [2] 含 L'\0' 的宽字符序列：wmemmove 按 n 拷贝，不因 NUL 停止。 */
static void test_embedded_nul(void)
{
    wchar_t src[5] = { L'a', L'\0', L'b', L'\0', L'c' };
    wchar_t dst[5] = { L'X', L'X', L'X', L'X', L'X' };

    wchar_t *ret = wmemmove(dst, src, 5);

    assert(ret == dst);          /* [3] */
    assert(dst[0] == L'a');      /* [2] 不因 NUL 提前停止 */
    assert(dst[1] == L'\0');
    assert(dst[2] == L'b');
    assert(dst[3] == L'\0');
    assert(dst[4] == L'c');
}

int main(void)
{
    /* 确认函数指针初始化成功（[1] 原型匹配）。 */
    assert(fp_wmemmove == wmemmove);

    test_basic_copy();        /* [2][3] */
    test_partial_copy();      /* [2][3] */
    test_zero_length();       /* [2][3] */
    test_overlap_forward();   /* [2][3] 重叠：向后 */
    test_overlap_backward();  /* [2][3] 重叠：向前 */
    test_self_overlap();      /* [2][3] 完全重叠 */
    test_wide_values();       /* [2] 宽字符值 */
    test_embedded_nul();      /* [2] 内嵌 NUL */

    printf("C99 7.24.4.2.4 wmemmove: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「[1] 原型要求 s1 为 wchar_t *（可写）」：
 * 传入 const wchar_t * 作为第一个实参，丢弃 const 限定，
 * gcc -std=c99 应报错（discards qualifiers / passing argument 1 ...）。
 * 注意：这里用强制转换绕过会掩盖错误，故直接传 const 指针。 */
void neg_const_first_arg(void)
{
    const wchar_t src[4] = { L'a', L'b', L'c', L'd' };
    wchar_t dst[4];
    wmemmove(src, dst, 4);   /* 错误：s1 为 const wchar_t *，不能作为 wchar_t * */
}

/* 违反约束「[1] 原型要求 s2 为 const wchar_t *」：
 * 传入非 wchar_t 指针（如 int *），类型不兼容，
 * gcc -std=c99 应报错（incompatible pointer type）。 */
void neg_wrong_pointer_type(void)
{
    int a[4] = { 1, 2, 3, 4 };
    int b[4] = { 0 };
    wmemmove(a, b, 4);       /* 错误：int * 与 wchar_t * 不兼容 */
}

/* 违反约束「[1] 原型要求 n 为 size_t」：
 * 传入结构体类型，无法转换为 size_t，
 * gcc -std=c99 应报错（incompatible type for argument 3）。 */
struct NotSize { int x; };
void neg_wrong_n_type(void)
{
    wchar_t a[4] = { 0 };
    wchar_t b[4] = { 0 };
    struct NotSize n;
    wmemmove(a, b, n);       /* 错误：结构体不能作为 size_t 实参 */
}

/* 违反约束「[1] 原型要求返回 wchar_t *」：
 * 把返回值赋给不兼容的指针类型（int *），
 * gcc -std=c99 应报错（incompatible pointer type / assignment）。 */
void neg_wrong_return_type(void)
{
    wchar_t a[4] = { 0 };
    wchar_t b[4] = { 0 };
    int *p = wmemmove(a, b, 4);  /* 错误：wchar_t * 赋给 int * */
    (void)p;
}

/* 违反约束「[1] 调用参数个数必须匹配原型」：
 * 少传一个实参，gcc -std=c99 应报错（too few arguments to function）。 */
void neg_too_few_args(void)
{
    wchar_t a[4] = { 0 };
    wchar_t b[4] = { 0 };
    wmemmove(a, b);          /* 错误：缺少第三个实参 n */
}

/* 违反约束「[1] 调用参数个数必须匹配原型」：
 * 多传一个实参，gcc -std=c99 应报错（too many arguments to function）。 */
void neg_too_many_args(void)
{
    wchar_t a[4] = { 0 };
    wchar_t b[4] = { 0 };
    wmemmove(a, b, 4, 0);    /* 错误：多出第四个实参 */
}

/* 违反约束「[1] 函数返回值不可作为左值」：
 * wmemmove 返回 wchar_t *（非左值），对其赋值应编译报错
 * （lvalue required as left operand of assignment）。 */
void neg_assign_to_return(void)
{
    wchar_t a[4] = { 0 };
    wchar_t b[4] = { 0 };
    wmemmove(a, b, 4) = a;   /* 错误：函数调用结果不是左值 */
}

#endif /* 负向测试结束 */