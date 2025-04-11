#include "lztHooks.h"
#include "lztOS.h"

#if LZTOS_ENABLE_HOOKS == 1

void lztHooksCpuIdle (void) {

}

void lztHooksSysTick (void) {

}

void lztHooksTaskSwitch (lztTask *from, lztTask *to) {

}

void lztHooksTaskInit (lztTask *task) {

}

#endif
