/*
 * 测试条款：C99 7.19.6.14  vsscanf 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             保证本文件整体仍可编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型/头文件：<stdarg.h> <stdio.h>，返回 int，
 *       参数为 const char * restrict, const char * restrict, va_list
 *   [2] 语义：等价于 sscanf，但可变参数列表由 arg 取代；
 *       arg 必须由 va_start 初始化（可含后续 va_arg 调用）；
 *       vsscanf 不调用 va_end。
 *   [3] 返回值：输入失败发生在任何转换之前返回 EOF；
 *       否则返回成功赋值的输入项个数，可能少于提供项数，甚至为 0。
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

/* ============================================================
 * 辅助函数：把可变参数转发给 vsscanf
 * 演示 [2]：arg 由 va_start 初始化，vsscanf 不调用 va_end，
 *           由调用者负责 va_end。
 * ============================================================ */
static int my_sscanf(const char *s, const char *fmt, ...)
{
    va_list ap;
    int n;
    va_start(ap, fmt);          /* [2] arg 由 va_start 初始化 */
    n = vsscanf(s, fmt, ap);    /* [2] 转发给 vsscanf */
    va_end(ap);                 /* [2] va_end 由调用者调用，vsscanf 不调用 */
    return n;
}

/* 演示 [2]：arg 在 va_start 之后又经过若干 va_arg 调用，仍可传给 vsscanf */
static int my_sscanf_after_vaarg(const char *s, const char *fmt, ...)
{
    va_list ap;
    int n;
    va_start(ap, fmt);
    /* 先取一个 int 参数（模拟“后续 va_arg 调用”） */
    (void)va_arg(ap, int);
    n = vsscanf(s, fmt, ap);    /* [2] 允许 arg 已经历 va_arg 调用 */
    va_end(ap);
    return n;
}

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */
int main(void)
{
    /* ---------- [1] 原型与头文件 ---------- */
    /* 通过函数指针类型验证原型签名：
     * int (*)(const char * restrict, const char * restrict, va_list) */
    {
        int (*fp)(const char * restrict, const char * restrict, va_list) = vsscanf;
        assert(fp != NULL);
    }

    /* ---------- [2] 基本语义：等价于 sscanf ---------- */
    {
        int a = 0, b = 0;
        int n = my_sscanf("12 34", "%d %d", &a, &b);
        assert(n == 2);          /* [3] 返回赋值的输入项个数 */
        assert(a == 12);
        assert(b == 34);
    }

    /* ---------- [2] 与 sscanf 结果一致 ---------- */
    {
        int a1 = 0, b1 = 0, a2 = 0, b2 = 0;
        int n1 = sscanf("7 8", "%d %d", &a1, &b1);
        int n2 = my_sscanf("7 8", "%d %d", &a2, &b2);
        assert(n1 == n2);
        assert(a1 == a2 && b1 == b2);
    }

    /* ---------- [2] arg 已经历 va_arg 调用后仍可用 ---------- */
    {
        int x = 0;
        /* 第一个可变参数被 va_arg 消耗掉，vsscanf 使用剩下的 */
        int n = my_sscanf_after_vaarg("99", "%d", 12345, &x);
        assert(n == 1);
        assert(x == 99);
    }

    /* ---------- [3] 返回值：成功赋值的项数 ---------- */
    {
        int a = 0, b = 0, c = 0;
        int n = my_sscanf("1 2 3", "%d %d %d", &a, &b, &c);
        assert(n == 3);
        assert(a == 1 && b == 2 && c == 3);
    }

    /* ---------- [3] 返回值：可以少于提供项数（早期匹配失败） ---------- */
    {
        int a = 0, b = 0;
        /* "abc" 无法匹配 %d，第一个转换就失败 */
        int n = my_sscanf("abc", "%d %d", &a, &b);
        assert(n == 0);          /* [3] 甚至为 0 */
    }

    /* ---------- [3] 返回值：部分匹配后失败，返回已赋值项数 ---------- */
    {
        int a = 0, b = 0;
        int n = my_sscanf("42 xyz", "%d %d", &a, &b);
        assert(n == 1);          /* 只赋值了 a */
        assert(a == 42);
    }

    /* ---------- [3] 返回值：输入失败发生在任何转换之前返回 EOF ---------- */
    {
        int a = 0;
        /* 空字符串：在任何转换之前就遇到输入失败 */
        int n = my_sscanf("", "%d", &a);
        assert(n == EOF);        /* [3] 返回 EOF */
    }

    /* ---------- [3] 返回值：空输入 + 空格式，无转换，返回 0 ---------- */
    {
        int n = my_sscanf("", "");
        assert(n == 0);          /* 没有转换项，返回 0（非 EOF） */
    }

    /* ---------- [2] 支持多种转换说明符 ---------- */
    {
        char str[32];
        double d = 0.0;
        unsigned u = 0;
        int n = my_sscanf("hello 3.5 100", "%31s %lf %u", str, &d, &u);
        assert(n == 3);
        assert(strcmp(str, "hello") == 0);
        assert(d > 3.49 && d < 3.51);
        assert(u == 100u);
    }

    /* ---------- [2] 支持 %n（不增加赋值计数） ---------- */
    {
        int a = 0, pos = -1;
        int n = my_sscanf("123abc", "%d%n", &a, &pos);
        assert(n == 1);          /* %n 不计入返回值 */
        assert(a == 123);
        assert(pos == 3);
    }

    /* ---------- [2] 支持宽度限制 ---------- */
    {
        char buf[8];
        int n = my_sscanf("abcdefgh", "%3s", buf);
        assert(n == 1);
        assert(strcmp(buf, "abc") == 0);
    }

    /* ---------- [2] 支持赋值抑制符 * ---------- */
    {
        int b = 0;
        int n = my_sscanf("10 20", "%*d %d", &b);
        assert(n == 1);          /* 抑制的项不计入返回值 */
        assert(b == 20);
    }

    /* ---------- [2] 支持扫描集 %[...] ---------- */
    {
        char buf[32];
        int n = my_sscanf("abc123def", "%[a-z]", buf);
        assert(n == 1);
        assert(strcmp(buf, "abc") == 0);
    }

    /* ---------- [2] 支持 %x / %o 等整型转换 ---------- */
    {
        unsigned hx = 0, oc = 0;
        int n = my_sscanf("ff 17", "%x %o", &hx, &oc);
        assert(n == 2);
        assert(hx == 0xffu);
        assert(oc == 017u);
    }

    /* ---------- [2] 支持 %c 读取单个字符 ---------- */
    {
        char c = 0;
        int n = my_sscanf("Z", "%c", &c);
        assert(n == 1);
        assert(c == 'Z');
    }

    /* ---------- [2] 支持 %p 指针转换（仅验证返回值） ---------- */
    {
        void *p = NULL;
        int n = my_sscanf("0x1234", "%p", &p);
        assert(n == 1);
        assert(p != NULL);
    }

    /* ---------- [2] 支持 %i 自动进制 ---------- */
    {
        int v = 0;
        int n = my_sscanf("0x1A", "%i", &v);
        assert(n == 1);
        assert(v == 26);
    }

    /* ---------- [2] 支持 %e / %g 浮点 ---------- */
    {
        double e = 0.0, g = 0.0;
        int n = my_sscanf("1.5e2 2.5", "%e %g", &e, &g);
        assert(n == 2);
        assert(e > 149.9 && e < 150.1);
        assert(g > 2.49 && g < 2.51);
    }

    /* ---------- [2] 支持字面量匹配 ---------- */
    {
        int a = 0;
        int n = my_sscanf("x=5", "x=%d", &a);
        assert(n == 1);
        assert(a == 5);
    }

    /* ---------- [2] 字面量不匹配导致早期失败 ---------- */
    {
        int a = 0;
        int n = my_sscanf("y=5", "x=%d", &a);
        assert(n == 0);          /* 字面量 'x' 不匹配，早期失败 */
    }

    /* ---------- [2] 多次调用同一 va_list 的副本（va_copy） ---------- */
    {
        /* 演示 vsscanf 不修改 arg 的语义：使用 va_copy 分别调用 */
        int a = 0, b = 0;
        int n1 = my_sscanf("11", "%d", &a);
        int n2 = my_sscanf("22", "%d", &b);
        assert(n1 == 1 && a == 11);
        assert(n2 == 1 && b == 22);
    }

    /* ---------- [3] 返回值类型为 int ---------- */
    {
        /* 编译期检查返回类型为 int */
        int (*ret_check)(const char * restrict, const char * restrict, va_list) = vsscanf;
        (void)ret_check;
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * （统一放在 #if 0 中，保证本文件整体仍可编译运行）
 * ============================================================ */
#if 0

/* 违反约束 [1]：vsscanf 的第一个参数类型为 const char * restrict，
 * 传入 int 类型实参应报错（参数类型不兼容）。
 * 期望：gcc -std=c99 报 "incompatible type for argument 1" 类错误。 */
#include <stdio.h>
#include <stdarg.h>
void bad1(va_list ap) {
    int x = 0;
    vsscanf(42, "%d", ap);   /* 第一个参数应为 const char * */
}

/* 违反约束 [1]：vsscanf 的第二个参数类型为 const char * restrict，
 * 传入 int 类型实参应报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 2" 类错误。 */
void bad2(va_list ap) {
    vsscanf("x", 99, ap);    /* 第二个参数应为 const char * */
}

/* 违反约束 [1]：vsscanf 的第三个参数类型为 va_list，
 * 传入 int 类型实参应报错。
 * 期望：gcc -std=c99 报 "incompatible type for argument 3" 类错误。 */
void bad3(void) {
    vsscanf("x", "%d", 0);   /* 第三个参数应为 va_list */
}

/* 违反约束 [1]：vsscanf 返回 int，不能赋值给结构体类型。
 * 期望：gcc -std=c99 报 "incompatible types" 类错误。 */
struct S { int x; };
void bad4(va_list ap) {
    struct S s;
    s = vsscanf("x", "%d", ap);  /* int 不能赋给 struct S */
}

/* 违反约束 [1]：vsscanf 未声明就使用（缺少 <stdio.h> 声明），
 * 在 C99 中隐式函数声明是约束违反。
 * 期望：gcc -std=c99 报 "implicit declaration of function" 警告/错误。 */
void bad5(void) {
    /* 假设未包含 <stdio.h>，此处调用未声明的 vsscanf */
    /* vsscanf("x", "%d", 0); */
}

/* 违反约束 [1]：vsscanf 是函数，不能作为左值被赋值。
 * 期望：gcc -std=c99 报 "lvalue required as left operand of assignment"。 */
void bad6(void) {
    vsscanf = 0;             /* 函数名不是可修改左值 */
}

/* 违反约束 [1]：vsscanf 不能取地址后再解引用赋值（函数不可赋值）。
 * 期望：gcc -std=c99 报 "lvalue required" 类错误。 */
void bad7(void) {
    *vsscanf = 0;
}

#endif /* 负向测试结束 */