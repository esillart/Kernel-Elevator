#include <linux/uaccess.h> 
#include <linux/kthread.h> 
#include <linux/module.h> 
#include <linux/init.h> 
#include <linux/proc_fs.h> 
#include <linux/slab.h> 
#include <linux/mutex.h> 
#include <syscalls.h> 

MODULE_AUTHOR("Group 12");
MODULE_DESCRIPTION("Time since epoch proc reader");
MODULE_LICENSE("GPL");

#define MODPARENT NULL
#define MODPERMISSIONS 0644


/* you'll need to add and implement: */
int start_elevator(void);
int issue_request(int, int, int);
int stop_elevator(void); 
