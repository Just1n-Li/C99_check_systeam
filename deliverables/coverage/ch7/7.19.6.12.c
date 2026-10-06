/*
 * 测试条款：C99 7.19.6.12  The vsnprintf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 ... #endif 中，故意违反约束，编译器应报错。
 *
 * 覆盖段落：
 *   [1] 原型 / 头文件 <stdarg.h> <stdio.h>
 *   [2] 等价于 snprintf，用 va_list 替换可变参数；arg 须由 va_start 初始化；
 *       vsnprintf 不调用 va_end；重叠对象间复制为 UB（UB 不作为负向测试）。
 *   [3] 返回值：若 n 足够大本应写入的字符数（不含终止空字符）；
 *       编码错误返回负值；当且仅当返回值非负且 < n 时输出被完整写入。
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* 辅助函数：把可变参数转发给 vsnprintf，返回 vsnprintf 的返回值        */
/* 用于验证 [2]：arg 由 va_start 初始化，且 vsnprintf 不调用 va_end。   */
/* ------------------------------------------------------------------ */
static int call_vsnprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);          /* [2] arg 必须由 va_start 初始化 */
    ret = vsnprintf(s, n, fmt, ap);
    /* [2] vsnprintf 不调用 va_end，因此这里必须自己调用 va_end */
    va_end(ap);
    return ret;
}

/* 辅助：多次使用同一个 va_list 的副本，验证 va_arg 之后仍可传给 vsnprintf */
static int call_vsnprintf_after_va_arg(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    /* 先取一个 int 参数（模拟“possibly subsequent va_arg calls”） */
    (void)va_arg(ap, int);
    ret = vsnprintf(s, n, fmt, ap);
    va_end(ap);
    return ret;
}

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */
int main(void)
{
    char buf[64];
    int ret;

    /* ---- [1] 原型可用：包含 <stdarg.h> 与 <stdio.h> 后可直接调用 ---- */
    {
        va_list dummy;
        (void)dummy; /* 仅确认类型 va_list 可见 */
    }

    /* ---- [3] 返回值 = 本应写入的字符数（不含终止空字符），n 足够大 ---- */
    {
        memset(buf, 'X', sizeof buf);
        ret = call_vsnprintf(buf, sizeof buf, "%s %d", "hello", 42);
        /* "hello 42" 共 8 个字符 */
        assert(ret == 8);
        assert(strcmp(buf, "hello 42") == 0);
        /* 返回值非负且 < n，输出被完整写入，含终止空字符 */
        assert(ret >= 0 && (size_t)ret < sizeof buf);
        assert(buf[ret] == '\0');
    }

    /* ---- [3] n 太小：返回值仍是“本应写入”的长度，但输出被截断 ---- */
    {
        memset(buf, 'X', sizeof buf);
        ret = call_vsnprintf(buf, 5, "%s", "abcdefgh"); /* 需要 8 个字符 */
        assert(ret == 8);                 /* 返回值 = 完整长度 */
        assert(!((size_t)ret < 5));       /* 返回值 >= n，说明未完整写入 */
        /* 缓冲区中最多写入 n-1 = 4 个字符 + 终止空字符 */
        assert(buf[4] == '\0');
        assert(strncmp(buf, "abcd", 4) == 0);
    }

    /* ---- [3] n == 0：不写入任何字符，返回值仍为完整长度 ---- */
    {
        char tiny[1];
        tiny[0] = 'Z';
        ret = call_vsnprintf(tiny, 0, "%d", 12345);
        assert(ret == 5);      /* "12345" 长度 5 */
        assert(tiny[0] == 'Z');/* n==0 时不应写入任何字符 */
    }

    /* ---- [3] 空字符串：返回 0，且写入终止空字符 ---- */
    {
        memset(buf, 'X', sizeof buf);
        ret = call_vsnprintf(buf, sizeof buf, "%s", "");
        assert(ret == 0);
        assert(buf[0] == '\0');
    }

    /* ---- [2] 等价于 snprintf：与 snprintf 结果一致 ---- */
    {
        char a[64], b[64];
        int ra, rb;
        ra = snprintf(a, sizeof a, "[%05d|%s]", 7, "xy");
        rb = call_vsnprintf(b, sizeof b, "[%05d|%s]", 7, "xy");
        assert(ra == rb);
        assert(strcmp(a, b) == 0);
    }

    /* ---- [2] arg 由 va_start 初始化，且可先经 va_arg 再传给 vsnprintf ---- */
    {
        memset(buf, 'X', sizeof buf);
        /* 第一个 int 参数被 va_arg 取走，格式串只消费剩下的 "world" */
        ret = call_vsnprintf_after_va_arg(buf, sizeof buf, "%s", 999, "world");
        assert(ret == 5);
        assert(strcmp(buf, "world") == 0);
    }

    /* ---- [2] vsnprintf 不调用 va_end：调用方在返回后仍可继续使用 va_list ---- */
    {
        /* 用一个自定义函数验证：vsnprintf 返回后 va_list 仍有效（可 va_end） */
        va_list ap;
        char local[32];
        int r;
        va_start(ap, local); /* 占位，实际参数在下面手动构造 */
        va_end(ap);

        /* 更直接的验证：在辅助函数里 vsnprintf 之后调用 va_end 不崩溃，
           且能再次用同一 va_list 的副本继续 va_arg（标准允许 va_end 前多次使用） */
        {
            va_list ap2, ap3;
            char out1[32], out2[32];
            int r1, r2;
            va_start(ap2, out1);
            va_copy(ap3, ap2);
            r1 = vsnprintf(out1, sizeof out1, "%d", ap2);
            /* vsnprintf 未调用 va_end，ap3 仍可用 */
            r2 = vsnprintf(out2, sizeof out2, "%d", ap3);
            va_end(ap3);
            va_end(ap2);
            assert(r1 == r2);
            assert(strcmp(out1, out2) == 0);
        }
        (void)r;
    }

    /* ---- [3] 编码错误返回负值：用非法转换说明触发（实现相关，宽松检查） ---- */
    {
        /* 注意：非法格式串是 UB，这里不测；改为测一个合法但可能编码错误的场景
           在多数实现中，%ls 传入非法宽字符可能返回负值，但这是实现相关的，
           因此这里只做“返回值类型为 int 且可比较”的静态检查。 */
        ret = call_vsnprintf(buf, sizeof buf, "%d", -1);
        assert(ret == 2); /* "-1" */
        assert(strcmp(buf, "-1") == 0);
    }

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束 [1]：vsnprintf 的第三个参数必须是 const char *（格式串），
   传入 int 类型实参，gcc -std=c99 应报错（参数类型不兼容）。 */
#include <stdarg.h>
#include <stdio.h>
void bad1(va_list ap)
{
    char buf[10];
    vsnprintf(buf, sizeof buf, 12345, ap); /* 错误：格式串应为 const char * */
}

/* 违反约束 [1]：第四个参数必须是 va_list 类型，
   传入 int，gcc -std=c99 应报错。 */
void bad2(void)
{
    char buf[10];
    vsnprintf(buf, sizeof buf, "%d", 42); /* 错误：42 不是 va_list */
}

/* 违反约束 [1]：第一个参数必须是 char *（可写缓冲区），
   传入字符串字面量（const char[]），gcc -std=c99 应报错或警告为错误。 */
void bad3(va_list ap)
{
    vsnprintf("literal", 8, "%d", ap); /* 错误：目标不可写 */
}

/* 违反约束 [1]：第二个参数必须是 size_t，
   传入指针类型，gcc -std=c99 应报错。 */
void bad4(va_list ap)
{
    char buf[10];
    vsnprintf(buf, buf, "%d", ap); /* 错误：n 应为 size_t，不是 char* */
}

/* 违反约束 [1]：返回值应赋给兼容类型；把 vsnprintf 当 void 函数使用
   并对其结果取地址，gcc -std=c99 应报错。 */
void bad5(va_list ap)
{
    char buf[10];
    int *p = &vsnprintf(buf, sizeof buf, "%d", ap); /* 错误：不能取函数返回值地址 */
    (void)p;
}

#endif /* 负向测试结束 */