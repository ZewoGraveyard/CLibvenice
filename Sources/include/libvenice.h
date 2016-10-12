/*

  Copyright (c) 2015 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef LIBMILL_H_INCLUDED
#define LIBMILL_H_INCLUDED

#include <errno.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

/******************************************************************************/
/*  ABI versioning support                                                    */
/******************************************************************************/

/*  Don't change this unless you know exactly what you're doing and have      */
/*  read and understand the following documents:                              */
/*  www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html     */
/*  www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html  */

/*  The current interface version. */
#define MILL_VERSION_CURRENT 18

/*  The latest revision of the current interface. */
#define MILL_VERSION_REVISION 2

/*  How many past interface versions are still supported. */
#define MILL_VERSION_AGE 0

/******************************************************************************/
/*  Symbol visibility                                                         */
/******************************************************************************/

#if !defined __GNUC__ && !defined __clang__
#error "Unsupported compiler!"
#endif

#if defined MILL_NO_EXPORTS
#   define MILL_EXPORT
#else
#   if defined _WIN32
#      if defined MILL_EXPORTS
#          define MILL_EXPORT __declspec(dllexport)
#      else
#          define MILL_EXPORT __declspec(dllimport)
#      endif
#   else
#      if defined __SUNPRO_C
#          define MILL_EXPORT __global
#      elif (defined __GNUC__ && __GNUC__ >= 4) || \
             defined __INTEL_COMPILER || defined __clang__
#          define MILL_EXPORT __attribute__ ((visibility("default")))
#      else
#          define MILL_EXPORT
#      endif
#   endif
#endif

/******************************************************************************/
/*  Helpers                                                                   */
/******************************************************************************/

#define mill_string2_(x) #x
#define mill_string1_(x) mill_string2_(x)
#define MILL_HERE_ (__FILE__ ":" mill_string1_(__LINE__))

#define mill_concat_(x,y) x##y

MILL_EXPORT int64_t mill_now_(
    void);
MILL_EXPORT pid_t mill_mfork_(
    void);

#if defined MILL_USE_PREFIX
#define mill_now mill_now_
#define mill_mfork mill_mfork_
#else
#define now mill_now_
#define mfork mill_mfork_
#endif

/******************************************************************************/
/*  Coroutines                                                                */
/******************************************************************************/

#define MILL_FDW_IN_ 1
#define MILL_FDW_OUT_ 2
#define MILL_FDW_ERR_ 4

MILL_EXPORT extern volatile int mill_unoptimisable1_;
MILL_EXPORT extern volatile void *mill_unoptimisable2_;

#if defined __x86_64__
typedef uint64_t *mill_ctx;
#else
typedef sigjmp_buf *mill_ctx;
#endif

MILL_EXPORT mill_ctx mill_getctx_(
    void);
MILL_EXPORT __attribute__((noinline)) void *mill_prologue_(
    const char *created);
MILL_EXPORT __attribute__((noinline)) void mill_epilogue_(
    void);

MILL_EXPORT void mill_goprepare_(
    int count,
    size_t stack_size);
MILL_EXPORT void mill_yield_(
    const char *current);
MILL_EXPORT void mill_msleep_(
    int64_t deadline,
    const char *current);
MILL_EXPORT int mill_fdwait_(
    int fd,
    int events,
    int64_t deadline,
    const char *current);
MILL_EXPORT void mill_fdclean_(
    int fd);
MILL_EXPORT void *mill_cls_(
    void);
MILL_EXPORT void mill_setcls_(
    void *val);


#if defined(__x86_64__)
#if defined(__AVX__)
#define MILL_CLOBBER \
        , "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",\
        "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"
#else
#define MILL_CLOBBER
#endif
#define mill_setjmp_(ctx) ({\
    int ret;\
    asm("lea     LJMPRET%=(%%rip), %%rcx\n\t"\
        "xor     %%rax, %%rax\n\t"\
        "mov     %%rbx, (%%rdx)\n\t"\
        "mov     %%rbp, 8(%%rdx)\n\t"\
        "mov     %%r12, 16(%%rdx)\n\t"\
        "mov     %%rsp, 24(%%rdx)\n\t"\
        "mov     %%r13, 32(%%rdx)\n\t"\
        "mov     %%r14, 40(%%rdx)\n\t"\
        "mov     %%r15, 48(%%rdx)\n\t"\
        "mov     %%rcx, 56(%%rdx)\n\t"\
        "mov     %%rdi, 64(%%rdx)\n\t"\
        "mov     %%rsi, 72(%%rdx)\n\t"\
        "LJMPRET%=:\n\t"\
        : "=a" (ret)\
        : "d" (ctx)\
        : "memory", "rcx", "r8", "r9", "r10", "r11",\
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",\
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"\
          MILL_CLOBBER\
          );\
    ret;\
})
#define mill_longjmp_(ctx) \
    asm("movq   (%%rax), %%rbx\n\t"\
	    "movq   8(%%rax), %%rbp\n\t"\
	    "movq   16(%%rax), %%r12\n\t"\
	    "movq   24(%%rax), %%rdx\n\t"\
	    "movq   32(%%rax), %%r13\n\t"\
	    "movq   40(%%rax), %%r14\n\t"\
	    "mov    %%rdx, %%rsp\n\t"\
	    "movq   48(%%rax), %%r15\n\t"\
	    "movq   56(%%rax), %%rdx\n\t"\
	    "movq   64(%%rax), %%rdi\n\t"\
	    "movq   72(%%rax), %%rsi\n\t"\
	    "jmp    *%%rdx\n\t"\
        : : "a" (ctx) : "rdx" \
    )
#else
#define mill_setjmp_(ctx) \
    sigsetjmp(*ctx, 0)
#define mill_longjmp_(ctx) \
    siglongjmp(*ctx, 1)
#endif

#define mill_go_(fn) \
    do {\
        void *mill_sp;\
        mill_ctx ctx = mill_getctx_();\
        if(!mill_setjmp_(ctx)) {\
            mill_sp = mill_prologue_(MILL_HERE_);\
            int mill_anchor[mill_unoptimisable1_];\
            mill_unoptimisable2_ = &mill_anchor;\
            char mill_filler[(char*)&mill_anchor - (char*)(mill_sp)];\
            mill_unoptimisable2_ = &mill_filler;\
            fn;\
            mill_epilogue_();\
        }\
    } while(0)

#if defined MILL_USE_PREFIX
#define MILL_FDW_IN MILL_FDW_IN_
#define MILL_FDW_OUT MILL_FDW_OUT_
#define MILL_FDW_ERR MILL_FDW_ERR_
#define mill_coroutine __attribute__((noinline))
#define mill_go(fn) mill_go_(fn)
#define mill_goprepare mill_goprepare_
#define mill_yield() mill_yield_(MILL_HERE_)
#define mill_msleep(dd) mill_msleep_((dd), MILL_HERE_)
#define mill_fdwait(fd, ev, dd) mill_fdwait_((fd), (ev), (dd), MILL_HERE_)
#define mill_fdclean mill_fdclean_
#define mill_cls mill_cls_
#define mill_setcls mill_setcls_
#else
#define FDW_IN MILL_FDW_IN_
#define FDW_OUT MILL_FDW_OUT_
#define FDW_ERR MILL_FDW_ERR_
#define coroutine __attribute__((noinline))
#define go(fn) mill_go_(fn)
#define goprepare mill_goprepare_
#define yield() mill_yield_(MILL_HERE_)
#define msleep(deadline) mill_msleep_((deadline), MILL_HERE_)
#define fdwait(fd, ev, dd) mill_fdwait_((fd), (ev), (dd), MILL_HERE_)
#define fdclean mill_fdclean_
#define cls mill_cls_
#define setcls mill_setcls_
#endif

MILL_EXPORT void co(void *ctx, void (*routine)(void *), const char *created);
MILL_EXPORT size_t mill_clauselen();
MILL_EXPORT int mill_number_of_cores(void);

/******************************************************************************/
/*  Channels                                                                  */
/******************************************************************************/

struct mill_chan;

typedef struct{void *f1; void *f2; void *f3; void *f4;
    void *f5; int f6; int f7; int f8;} mill_clause_;
#define MILL_CLAUSELEN_ (sizeof(mill_clause_))

MILL_EXPORT struct mill_chan *mill_chmake_(
    size_t bufsz,
    const char *created);
MILL_EXPORT void mill_chclose_(
    struct mill_chan *ch,
    const char *current);
MILL_EXPORT void mill_chs_(
    struct mill_chan *ch,
    const char *current);
MILL_EXPORT void mill_chr_(
    struct mill_chan *ch,
    const char *current);
MILL_EXPORT void mill_chdone_(
    struct mill_chan *ch,
    const char *current);
MILL_EXPORT void mill_choose_init_(
    const char *current);
MILL_EXPORT void mill_choose_in_(
    void *clause,
    struct mill_chan *ch,
    int idx);
MILL_EXPORT void mill_choose_out_(
    void *clause,
    struct mill_chan *ch,
    int idx);
MILL_EXPORT void mill_choose_deadline_(
    int64_t deadline);
MILL_EXPORT void mill_choose_otherwise_(
    void);
MILL_EXPORT int mill_choose_wait_(
    void);
MILL_EXPORT void *mill_choose_val_(
    size_t sz);

#if defined MILL_USE_PREFIX
typedef struct mill_chan *mill_chan;
#define mill_chmake(tp, sz) mill_chmake_(sizeof(tp), sz, MILL_HERE_)
#define mill_chdup(ch) mill_chdup_((ch), MILL_HERE_)
#define mill_chclose(ch) mill_chclose_((ch), MILL_HERE_)
#define mill_chs(ch, tp, val) mill_chs__((ch), tp, (val))
#define mill_chr(ch, tp) mill_chr__((ch), tp)
#define mill_chdone(ch, tp, val) mill_chdone__((ch), tp, (val))
#define mill_choose mill_choose_init__
#define mill_in(ch, tp, nm) mill_choose_in__((ch), tp, nm, __COUNTER__)
#define mill_out(ch, tp, val) mill_choose_out__((ch), tp, (val), __COUNTER__)
#define mill_deadline(dd) mill_choose_deadline__(dd, __COUNTER__)
#define mill_otherwise mill_choose_otherwise__(__COUNTER__)
#define mill_end mill_choose_end__
#else
typedef struct mill_chan *chan;
#define chmake(tp, sz) mill_chmake_(sizeof(tp), sz, MILL_HERE_)
#define chdup(ch) mill_chdup_((ch), MILL_HERE_)
#define chclose(ch) mill_chclose_((ch), MILL_HERE_)
#define chs(ch, tp, val) mill_chs__((ch), tp, (val))
#define chr(ch, tp) mill_chr__((ch), tp)
#define chdone(ch, tp, val) mill_chdone__((ch), tp, (val))
#define choose mill_choose_init__
#define in(ch, tp, nm) mill_choose_in__((ch), tp, nm, __COUNTER__)
#define out(ch, tp, val) mill_choose_out__((ch), tp, (val), __COUNTER__)
#define deadline(dd) mill_choose_deadline__(dd, __COUNTER__)
#define otherwise mill_choose_otherwise__(__COUNTER__)
#define end mill_choose_end__
#endif

/******************************************************************************/
/*  IP address library                                                        */
/******************************************************************************/

#define IPADDR_IPV4 1
#define IPADDR_IPV6 2
#define IPADDR_PREF_IPV4 3
#define IPADDR_PREF_IPV6 4
#define IPADDR_MAXSTRLEN 46

typedef struct {char data[32];} ipaddr;

MILL_EXPORT ipaddr iplocal(const char *name, int port, int mode);
MILL_EXPORT ipaddr ipremote(const char *name, int port, int mode, int64_t deadline);
MILL_EXPORT const char *ipaddrstr(ipaddr addr, char *ipstr);

/******************************************************************************/
/*  TCP library                                                               */
/******************************************************************************/

typedef struct mill_tcpsock *tcpsock;

MILL_EXPORT tcpsock tcplisten(ipaddr addr, int backlog, int reuseport);
MILL_EXPORT int tcpport(tcpsock s);
MILL_EXPORT tcpsock tcpaccept(tcpsock s, int64_t deadline);
MILL_EXPORT ipaddr tcpaddr(tcpsock s);
MILL_EXPORT tcpsock tcpconnect(ipaddr addr, int64_t deadline);
MILL_EXPORT size_t tcpsend(tcpsock s, const void *buf, size_t len, int64_t deadline);
MILL_EXPORT void tcpsendfile(tcpsock s, const char *filepath, int64_t deadline);
MILL_EXPORT void tcpflush(tcpsock s, int64_t deadline);
MILL_EXPORT size_t tcprecv(tcpsock s, void *buf, size_t len, int64_t deadline);
MILL_EXPORT size_t tcprecvlh(tcpsock s, void *buf, size_t lowwater, size_t highwater, int64_t deadline);
MILL_EXPORT size_t tcprecvuntil(tcpsock s, void *buf, size_t len, const char *delims, size_t delimcount, int64_t deadline);
MILL_EXPORT void tcpclose(tcpsock s);
MILL_EXPORT tcpsock tcpattach(int fd, int listening);
MILL_EXPORT int tcpdetach(tcpsock s);

/******************************************************************************/
/*  UDP library                                                               */
/******************************************************************************/

typedef struct mill_udpsock *udpsock;

MILL_EXPORT udpsock udplisten(ipaddr addr);
MILL_EXPORT int udpport(udpsock s);
MILL_EXPORT void udpsend(udpsock s, ipaddr addr, const void *buf, size_t len);
MILL_EXPORT size_t udprecv(udpsock s, ipaddr *addr,
                           void *buf, size_t len, int64_t deadline);
MILL_EXPORT void udpclose(udpsock s);
MILL_EXPORT udpsock udpattach(int fd);
MILL_EXPORT int udpdetach(udpsock s);

/******************************************************************************/
/*  UNIX library                                                              */
/******************************************************************************/

typedef struct mill_unixsock *unixsock;

MILL_EXPORT unixsock unixlisten(const char *addr, int backlog);
MILL_EXPORT unixsock unixaccept(unixsock s, int64_t deadline);
MILL_EXPORT unixsock unixconnect(const char *addr);
MILL_EXPORT void unixpair(unixsock *a, unixsock *b);
MILL_EXPORT size_t unixsend(unixsock s, const void *buf, size_t len,
                            int64_t deadline);
MILL_EXPORT void unixflush(unixsock s, int64_t deadline);
MILL_EXPORT size_t unixrecv(unixsock s, void *buf, size_t len,
                            int64_t deadline);
MILL_EXPORT size_t unixrecvuntil(unixsock s, void *buf, size_t len,
                                 const char *delims, size_t delimcount, int64_t deadline);
MILL_EXPORT void unixclose(unixsock s);
MILL_EXPORT unixsock unixattach(int fd, int listening);
MILL_EXPORT int unixdetach(unixsock s);

/******************************************************************************/
/*  File library                                                              */
/******************************************************************************/

typedef struct mill_file *mfile;
MILL_EXPORT mfile fileopen(const char *pathname, int flags, mode_t mode);
MILL_EXPORT size_t filewrite(mfile f, const void *buf, size_t len, int64_t deadline);
MILL_EXPORT void fileflush(mfile f, int64_t deadline);
MILL_EXPORT size_t fileread(mfile f, void *buf, size_t len, int64_t deadline);
MILL_EXPORT size_t filereadlh(mfile f, void *buf, size_t lowwater, size_t highwater, int64_t deadline);
MILL_EXPORT void fileclose(mfile f);
MILL_EXPORT mfile fileattach(int fd);
MILL_EXPORT int filedetach(mfile f);
MILL_EXPORT off_t filetell(mfile f);
MILL_EXPORT off_t fileseek(mfile f, off_t offset);
MILL_EXPORT off_t filesize(mfile f);
MILL_EXPORT int fileeof(mfile f);
MILL_EXPORT int fileremove(const char *path);

/******************************************************************************/
/*  Debugging                                                                 */
/******************************************************************************/

/* These symbols are not wrapped in macros so that they can be used
   directly from the debugger. */
MILL_EXPORT void goredump(
    void);
MILL_EXPORT void gotrace(
    int level);

#if defined MILL_USE_PREFIX
#define mill_goredump goredump
#define mill_gotrace gotrace
#endif

#if defined(__cplusplus)
}
#endif

#endif
