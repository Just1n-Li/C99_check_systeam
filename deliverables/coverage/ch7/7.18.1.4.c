/*
 * 测试 C99 7.18.1.4 —— intptr_t / uintptr_t
 *
 * 条款要点：
 *   [1] intptr_t  : 有符号整型，任何 void* 转成它再转回 void*，结果与原指针相等。
 *       uintptr_t : 无符号整型，任何 void* 转成它再转回 void*，结果与原指针相等。
 *       这两个类型是可选的（optional）。
 *
 * 预期行为：
 *   正向测试：若 <stdint.h> 定义了 intptr_t/uintptr_t，则往返转换必须保持指针相等，
 *             且它们必须是整型（可做算术、可比较大小），程序应编译并运行通过。
 *   负向测试：违反约束的片段（如把 intptr_t 当指针解引用、把指针赋给整型而不转换等）
 *             应导致编译报错，统一放在 #if 0 中。
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

int main(void)
{
    /* [1] 这两个类型是可选的：用宏判断是否提供。
     *     若提供，则必须满足往返转换语义。 */
#if defined(INT8_MAX) /* 仅用于确认 stdint.h 已被包含 */
#endif

    int obj = 42;
    int arr[4] = { 1, 2, 3, 4 };
    void *p_obj = &obj;
    void *p_arr = arr;
    void *p_null = NULL;

    /* [1] intptr_t 往返转换：void* -> intptr_t -> void* 必须相等 */
#ifdef INTPTR_MAX
    {
        intptr_t i1 = (intptr_t)p_obj;
        void *back1 = (void *)i1;
        assert(back1 == p_obj);

        intptr_t i2 = (intptr_t)p_arr;
        void *back2 = (void *)i2;
        assert(back2 == p_arr);

        intptr_t i3 = (intptr_t)p_null;
        void *back3 = (void *)i3;
        assert(back3 == p_null);

        /* intptr_t 是有符号整型：可以做算术、比较 */
        assert(i1 == i1);
        assert(i1 - i1 == 0);
        printf("intptr_t OK: sizeof=%zu\n", sizeof(intptr_t));
    }
#else
    printf("intptr_t not provided (optional type)\n");
#endif

    /* [1] uintptr_t 往返转换：void* -> uintptr_t -> void* 必须相等 */
#ifdef UINTPTR_MAX
    {
        uintptr_t u1 = (uintptr_t)p_obj;
        void *back1 = (void *)u1;
        assert(back1 == p_obj);

        uintptr_t u2 = (uintptr_t)p_arr;
        void *back2 = (void *)u2;
        assert(back2 == p_arr);

        uintptr_t u3 = (uintptr_t)p_null;
        void *back3 = (void *)u3;
        assert(back3 == p_null);

        /* uintptr_t 是无符号整型：可以做算术、比较 */
        assert(u1 == u1);
        assert(u1 - u1 == 0);
        printf("uintptr_t OK: sizeof=%zu\n", sizeof(uintptr_t));
    }
#else
    printf("uintptr_t not provided (optional type)\n");
#endif

    /* [1] 若两者都提供，则它们应能容纳同一指针的数值表示，
     *     且 (intptr_t) 与 (uintptr_t) 的位模式在往返后都还原指针。 */
#if defined(INTPTR_MAX) && defined(UINTPTR_MAX)
    {
        intptr_t si = (intptr_t)p_obj;
        uintptr_t ui = (uintptr_t)p_obj;
        /* 两者转回 void* 都必须等于原指针 */
        assert((void *)si == p_obj);
        assert((void *)ui == p_obj);
        /* 同一指针的两种表示，其数值在无符号视角下应一致 */
        assert((uintptr_t)si == ui);
    }
#endif

    /* [1] 用 memcpy 验证往返转换对任意对象地址都成立 */
#ifdef UINTPTR_MAX
    {
        char buf[16];
        memset(buf, 0xAB, sizeof buf);
        void *pv = buf;
        uintptr_t uv = (uintptr_t)pv;
        void *pv2 = (void *)uv;
        assert(pv2 == pv);
        assert(((unsigned char *)pv2)[0] == 0xAB);
    }
#endif

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「intptr_t 是整型，不是指针类型」：
 * 对 intptr_t 做解引用（*）要求操作数为指针，gcc -std=c99 应报错。 */
void neg1(void)
{
    intptr_t i = 0;
    *i;                 /* error: invalid type argument of unary '*' */
}

/* 违反约束「uintptr_t 是整型，不是指针类型」：
 * 对 uintptr_t 使用 -> 成员访问要求操作数为指向结构体的指针，应报错。 */
struct S { int x; };
void neg2(void)
{
    uintptr_t u = 0;
    u->x;               /* error: invalid type argument of '->' */
}

/* 违反约束「指针不能直接赋给整型（无显式转换）」：
 * 把 void* 直接赋给 intptr_t 需要显式转换，隐式赋值应报错。 */
void neg3(void)
{
    int obj = 0;
    void *p = &obj;
    intptr_t i;
    i = p;              /* error: assignment makes integer from pointer without a cast */
}

/* 违反约束「整型不能直接赋给指针（无显式转换）」：
 * 把 uintptr_t 直接赋给 void* 需要显式转换，隐式赋值应报错。 */
void neg4(void)
{
    uintptr_t u = 0;
    void *p;
    p = u;              /* error: assignment makes pointer from integer without a cast */
}

/* 违反约束「intptr_t 与 uintptr_t 是可选类型，若未定义则不能使用」：
 * 在未包含 <stdint.h> 或实现未提供时使用它们应报错。
 * （此处演示：若实现未定义，则标识符未声明。） */
void neg5(void)
{
    /* 假设实现未提供该类型时： */
    /* intptr_t x; */  /* error: unknown type name 'intptr_t' */
}

#endif