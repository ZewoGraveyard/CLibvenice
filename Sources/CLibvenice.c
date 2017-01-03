#include "dill/cr.h"

void CLibvenice(void) {
}

int co(void **ptr, size_t len, void *fn, const char *file, int line, void (*routine)(void *)) {
    sigjmp_buf *ctx;
    void *stk = (ptr);
    int h = dill_prologue(&ctx, &stk, len, file, line);
    
    if(h >= 0) {
        if(!dill_setjmp(*ctx)) {
            DILL_SETSP(stk);
            routine(fn);
            dill_epilogue();
        }
    }
    
    return h;
}
