#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

//
// This file is for grading scripts.  You shouldn't have to look at code in this file.
//

#ifdef LAB_LOCK

#define BUFSZ 4096
static struct {
  struct spinlock lock;
  char buf[BUFSZ];
  int sz;
  int off;
} stats;

int statscopyin(char *, int);
int statslock(char *, int);

int
statswrite(int user_src, uint64 src, int n)
{
  return -1;
}

int
statsread(int user_dst, uint64 dst, int n)
{
  int m;

  acquire(&stats.lock);

  if (stats.sz == 0) {
#ifdef LAB_PGTBL
    stats.sz = statscopyin(stats.buf, BUFSZ);
#endif
#ifdef LAB_LOCK
    stats.sz = statslock(stats.buf, BUFSZ);
#endif
  }
  m = stats.sz - stats.off;

  if (m > 0) {
    if (m > n)
      m = n;
    if (either_copyout(user_dst, dst, stats.buf + stats.off, m) != -1) {
      stats.off += m;
    }
  } else {
    m = -1;
    stats.sz = 0;
    stats.off = 0;
  }
  release(&stats.lock);
  return m;
}

void
statsinit(void)
{
  initlock(&stats.lock, "stats");

  devsw[STATS].read = statsread;
  devsw[STATS].write = statswrite;
}

#endif

#ifdef LAB_PGTBL

extern pagetable_t kernel_pagetable;

pte_t *
pgpte(pagetable_t pagetable, uint64 va)
{
  return walk(pagetable, va, 0);
}

int
sys_pgpte(void)
{
  uint64 va;
  struct proc *p;

  p = myproc();
  argaddr(0, &va);
  pte_t *pte = pgpte(p->pagetable, va);
  if (pte != 0) {
    return (uint64)*pte;
  }
  return 0;
}

void
cnt_level(pagetable_t pagetable, uint64 *nsuper, uint64 *n2, uint64 va_start,
          int level)
{
  uint64 va;
  int n = (level == 3) ? 1 : 512;

  for (int i = 0; i < n; i++) {
    pte_t pte = pagetable[i];
    if (pte & PTE_V) {
      va = (((uint64)i) << PXSHIFT(level)) + va_start;
      uint64 child = PTE2PA(pte);
      if (level > 0) {
        if (!PTE_LEAF(pte)) {
          cnt_level((pagetable_t)child, nsuper, n2, va, level - 1);
        } else {
          *nsuper += 1;
        }
      } else {
        *n2 += 1;
      }
    }
  }
}

uint64
sys_ksupernpte()
{
  struct proc *p;
  uint64 as, an;
  uint64 s = 0, n = 0;

  cnt_level(kernel_pagetable, &s, &n, 0, 2);

  p = myproc();
  argaddr(0, &as);
  argaddr(1, &an);

  if (copyout(p->pagetable, p->sz, as, (char *)&s, sizeof(uint64)) < 0) {
    return -1;
  }
  if (copyout(p->pagetable, p->sz, an, (char *)&n, sizeof(uint64)) < 0) {
    return -1;
  }

  return 0;
}
#endif
