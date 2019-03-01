#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>

int (*STUB_stop_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_stop_elevator);
asmlinkage int sys_stop_elevator(void) {
	if (STUB_stop_elevator)
		return STUB_stop_elevator();
	else
		return -ENOSYS;
}
