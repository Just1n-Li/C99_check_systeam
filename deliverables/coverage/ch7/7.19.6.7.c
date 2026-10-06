/*
 * 测试条款：C99 7.19.6.7  sscanf 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型：int sscanf(const char * restrict s, const char * restrict format, ...);
 *   [2] 语义：等价于 fscanf，但输入来自字符串；到达字符串末尾等价于 fscanf 的 EOF；
 *            重叠对象间复制为 UB（UB 不作为负向测试）。
 *   [3] 返回值：任何转换前发生输入失败返回 EOF；否则返回成功赋值的输入项个数，
 *            可能少于提供的数量，甚至为 0（早期匹配失败）。
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：sscanf 的返回类型为 int，参数为 const char * restrict, const char * restrict, ... */
static int (*sscanf_ptr)(const char * restrict, const char * restrict, ...) = sscanf;

/* 辅助函数：用 va_list 转发调用 sscanf，验证可变参数（...）语义 */
static int call_sscanf(const char *s, const char *fmt, ...)
{
    va_list ap;
    int r;
    va_start(ap, fmt);
    r = vsscanf(s, fmt, ap);   /* vsscanf 与 sscanf 共享可变参数语义 */
    va_end(ap);
    return r;
}

int main(void)
{
    /* ---------- [1] 原型与基本调用 ---------- */
    {
        int n = 0;
        int r = sscanf("42", "%d", &n);
        assert(r == 1);
        assert(n == 42);
        assert(sscanf_ptr != NULL);
    }

    /* ---------- [2] 语义：等价于 fscanf，输入来自字符串 ---------- */
    {
        int a = 0, b = 0;
        /* 从字符串读取多个项 */
        int r = sscanf("10 20", "%d %d", &a, &b);
        assert(r == 2);
        assert(a == 10 && b == 20);
    }

    /* ---------- [2] 到达字符串末尾等价于 fscanf 的 EOF ---------- */
    {
        int a = 0, b = 0;
        /* 字符串只有一项，第二个转换在字符串末尾遇到“EOF” */
        int r = sscanf("7", "%d %d", &a, &b);
        assert(r == 1);          /* 只赋值了 1 项 */
        assert(a == 7);
        assert(b == 0);          /* b 未被赋值 */
    }

    /* ---------- [2] 空字符串：立即到达末尾 ---------- */
    {
        int a = 123;
        int r = sscanf("", "%d", &a);
        assert(r == EOF);        /* 任何转换前输入失败 -> EOF */
        assert(a == 123);        /* a 未被修改 */
    }

    /* ---------- [2] 格式串中的普通字符与空白匹配 ---------- */
    {
        int a = 0, b = 0;
        int r = sscanf("x=3, y=4", "x=%d, y=%d", &a, &b);
        assert(r == 2);
        assert(a == 3 && b == 4);
    }

    /* ---------- [2] 字符串输入与 %s / %c 等转换 ---------- */
    {
        char word[32] = {0};
        char ch = 0;
        int r = sscanf("hello Z", "%31s %c", word, &ch);
        assert(r == 2);
        assert(strcmp(word, "hello") == 0);
        assert(ch == 'Z');
    }

    /* ---------- [2] 可变参数（...）语义：通过 va_list 转发 ---------- */
    {
        int n = 0;
        int r = call_sscanf("99", "%d", &n);
        assert(r == 1);
        assert(n == 99);
    }

    /* ---------- [3] 返回值：成功赋值的项数 ---------- */
    {
        int a = 0, b = 0, c = 0;
        int r = sscanf("1 2 3", "%d %d %d", &a, &b, &c);
        assert(r == 3);
        assert(a == 1 && b == 2 && c == 3);
    }

    /* ---------- [3] 返回值：早期匹配失败返回 0 ---------- */
    {
        int a = 55;
        /* 第一个转换就失败：字符串以字母开头，%d 无法匹配 */
        int r = sscanf("abc", "%d", &a);
        assert(r == 0);          /* 早期匹配失败 -> 0 */
        assert(a == 55);         /* a 未被修改 */
    }

    /* ---------- [3] 返回值：部分匹配，少于提供的数量 ---------- */
    {
        int a = 0, b = 0, c = 0;
        /* 只有前两项能匹配 */
        int r = sscanf("5 6 xyz", "%d %d %d", &a, &b, &c);
        assert(r == 2);
        assert(a == 5 && b == 6);
        assert(c == 0);          /* c 未被赋值 */
    }

    /* ---------- [3] 返回值：任何转换前输入失败返回 EOF ---------- */
    {
        int a = 0;
        /* 空字符串，第一个转换前即到达末尾 */
        int r = sscanf("", "%d", &a);
        assert(r == EOF);
    }

    /* ---------- [3] 返回值：%n 不计入赋值项数 ---------- */
    {
        int a = 0, pos = -1;
        int r = sscanf("123", "%d%n", &a, &pos);
        assert(r == 1);          /* %n 不增加返回值 */
        assert(a == 123);
        assert(pos == 3);
    }

    /* ---------- [3] 返回值：抑制赋值（*）不计入项数 ---------- */
    {
        int b = 0;
        int r = sscanf("11 22", "%*d %d", &b);
        assert(r == 1);          /* 被抑制的项不计入 */
        assert(b == 22);
    }

    /* ---------- [2] 与 fscanf 等价性：用同一格式串比较 ---------- */
    {
        int a1 = 0, a2 = 0;
        int r1 = sscanf("77", "%d", &a1);
        FILE *fp = tmpfile();
        assert(fp != NULL);
        fputs("77", fp);
        rewind(fp);
        int r2 = fscanf(fp, "%d", &a2);
        fclose(fp);
        assert(r1 == r2);
        assert(a1 == a2);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「sscanf 的第一个参数类型为 const char *」：
 * 传入 int 作为第一个实参，gcc -std=c99 应报错
 * （int 不能隐式转换为 const char *）。 */
{
    int n = 0;
    sscanf(123, "%d", &n);   /* error: 第一个实参类型不匹配 */
}

/* 违反约束「sscanf 的第二个参数类型为 const char *」：
 * 传入 int 作为格式串，gcc -std=c99 应报错。 */
{
    int n = 0;
    sscanf("42", 456, &n);   /* error: 第二个实参类型不匹配 */
}

/* 违反约束「sscanf 的返回类型为 int，不能作为左值赋值」：
 * 对函数调用结果赋值，gcc -std=c99 应报错。 */
{
    sscanf("1", "%d", (int *)0) = 5;   /* error: 非左值 */
}

/* 违反约束「sscanf 需要至少两个固定参数」：
 * 只传一个实参，gcc -std=c99 应报错（参数太少）。 */
{
    sscanf("42");   /* error: too few arguments to function 'sscanf' */
}

/* 违反约束「sscanf 的 restrict 限定：两个 restrict 指针指向同一对象
 * 属于约束违反（restrict 语义要求）——此处用同一指针传入，
 * 严格实现可诊断。 */
{
    char buf[16] = "42";
    int n = 0;
    sscanf(buf, buf, &n);   /* 同一对象同时被两个 restrict 指针指向 */
}

#endif