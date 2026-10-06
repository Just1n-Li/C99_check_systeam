/* 验证 C99 6.5.3.2 Address and indirection operators
 * 正向测试：应能编译并运行通过（assert 全部成立）
 * 负向测试：违反 Constraints [1][2]，应编译报错
 */
#include <stdio.h>
#include <assert.h>

/* 用于 [1] 函数指示符测试 */
int func(void) { return 42; }

/* 含位域成员，用于 [1] 负向测试 */
struct S { int a; int b : 3; };
struct S s = {1, 0};

int main(void)
{
    /* ========== 正向测试：以下代码应能编译并运行通过 ========== */

    int x = 10;
    int arr[5] = {0, 10, 20, 30, 40};
    int *p;

    /* [1] & 的操作数为函数指示符 */
    int (*fp)(void) = &func;
    assert(fp != NULL);
    assert(fp() == 42);

    /* [1] & 的操作数为 [] 的结果 */
    p = &arr[2];
    assert(p == &arr[2]);
    assert(*p == 20);

    /* [1] & 的操作数为一元 * 的结果 */
    p = &arr[1];
    int *q = &*p;
    assert(q == p);

    /* [1] & 的操作数为左值（非位域、非 register） */
    int *px = &x;
    assert(px == &x);
    assert(*px == 10);

    /* [2] * 的操作数具有指针类型 */
    p = &x;
    assert(*p == 10);

    /* [3] & 产生指向其操作数的地址；类型为 pointer to type */
    assert(sizeof(&x) == sizeof(int *));

    /* [3] 若操作数是一元 * 的结果，& 和 * 都不求值，
     *      结果如同两者都被省略，但约束仍适用且结果不是左值 */
    p = &arr[3];
    int *via_amp_star = &*p;
    assert(via_amp_star == p);

    /* [3] 若操作数是 [] 的结果，& 和隐含的 * 都不求值，
     *      结果如同移除 & 并将 [] 改为 + */
    int *via_bracket = &arr[2];
    assert(via_bracket == arr + 2);

    /* [footnote 87] &*E 等价于 E（即使 E 是空指针，* 不求值） */
    {
        int *null_p = NULL;
        int *result = &*null_p;   /* 标准保证 * 不被求值，安全 */
        assert(result == NULL);
    }

    /* [footnote 87] &(E1[E2]) 等价于 ((E1)+(E2)) */
    assert(&arr[3] == arr + 3);

    /* [footnote 87] *&E 是函数指示符或等于 E 的左值 */
    assert(*&x == x);
    assert(*&arr[2] == arr[2]);
    int (*fp2)(void) = *&func;
    assert(fp2 == &func);

    /* [footnote 87] *(T)P 是左值，类型与 T 指向的类型兼容 */
    {
        long val = 12345L;
        void *vp = &val;
        assert(sizeof(*(long *)vp) == sizeof(long));
        *(long *)vp = 99999L;     /* 作为左值被赋值 */
        assert(val == 99999L);
    }

    /* [4] * 表示间接访问；指向函数 → 函数指示符 */
    int (*fp3)(void) = func;
    assert((*fp3)() == 42);

    /* [4] 指向对象 → 左值，指代该对象 */
    int y = 5;
    int *py = &y;
    *py = 99;
    assert(y == 99);

    /* [4] 操作数类型为 pointer to type，结果类型为 type */
    double d = 3.14;
    double *pd = &d;
    assert(sizeof(*pd) == sizeof(double));

    /* [4] 通过指针修改对象 */
    *px = 77;
    assert(x == 77);

    printf("All positive tests passed.\n");

    /* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

    /* [1] 违反约束：& 的操作数为位域 —— 位域不可取地址 */
    int *bf = &s.b;            /* gcc -std=c99 应报错：bit-field 取地址 */

    /* [1] 违反约束：& 的操作数声明为 register */
    {
        register int ri = 5;
        int *pri = &ri;        /* 应报错：register 变量取地址 */
    }

    /* [1] 违反约束：& 的操作数不是左值（算术表达式结果） */
    {
        int a = 1, b = 2;
        int *p = &(a + b);     /* 应报错：a+b 非左值 */
    }

    /* [1] 违反约束：& 的操作数不是左值（整型常量） */
    {
        int *p = &5;           /* 应报错：字面量非左值 */
    }

    /* [1] 违反约束：& 的操作数不是左值（函数返回值） */
    {
        int *p = &func();      /* 应报错：func() 返回 int，非左值 */
    }

    /* [2] 违反约束：* 的操作数不具有指针类型（int） */
    {
        int n = 10;
        int v = *n;            /* 应报错：int 不是指针 */
    }

    /* [2] 违反约束：* 的操作数不具有指针类型（double） */
    {
        double dd = 1.5;
        double vv = *dd;       /* 应报错：double 不是指针 */
    }

    /* [3] &* 的结果不是左值：对其赋值应报错 */
    {
        int xx = 1;
        int *pp = &xx;
        &*pp = pp;             /* 应报错：&*pp 非左值，不可赋值 */
    }

#endif

    return 0;
}