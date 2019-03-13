#include <linux/string.h>    /* String */
#include <linux/uaccess.h>    /* for put_user, & mem copy */
#include <linux/module.h>    /* Needed by all modules */
#include <linux/slab.h>        /* Needed for mem alloc, kmalloc and kfree */
#include <linux/kernel.h>    /* Needed for KERN_INFO */
#include <linux/init.h>        /* Needed for the macros */
#include <linux/proc_fs.h>    /* File system & calls */

MODULE_AUTHOR("Group 12");
MODULE_DESCRIPTION("Time since epoch proc reader");
MODULE_LICENSE("GPL");

#define MODPARENT NULL
#define MODPERMISSIONS 0644
#define ENTRY_NAME "my_xtime" /* put our proc name in here */

static struct file_operations fops; /* points to proc file definitions */
static int rp1 = 0;
static int len;
static int old_nsec;
static int old_sec;
static char *message_1;
static char *message_2;

int my_xtime_open_proc(struct inode *sp_inode, struct file* sp_file){
    printk(KERN_INFO "proc called open/n");
    rp1 = 1;

    /* set up proc data here */
    message_1 = kmalloc(sizeof(char)*100,__GFP_RECLAIM | __GFP_WRITE | __GFP_IO | __GFP_FS);


    if(message_1 == NULL){
        printk(KERN_WARNING "proc_open");
        return -ENOMEM;
    }

    /* set up proc data here */


    strcpy(message_1,"yooooo!/n");

    return 0;
}


ssize_t my_xtime_read_proc(struct file *sp_file,char __user * buf, size_t size, loff_t*offset){
    struct timespec time;
    time = current_kernel_time();

    /* there is an elapsed time */
    if(old_nsec){
        int elapsec;
        int elapnsec;
        if(old_nsec > time.tv_nsec){
            elapnsec = time.tv_nsec - old_nsec + 1000000000;
            elapsec = time.tv_sec - old_sec -1;
        }
        else{
            elapnsec = time.tv_nsec - old_nsec;
            elapsec = time.tv_sec - old_sec;
        }

        sprintf(message_1, "current time: %ld.%ld\nelapsed time: %d.%d\n", time.tv_sec, time.tv_nsec, elapsec,elapnsec);
    }
    else{ /*there hasnt been an elapsed time */
        sprintf(message_1, "current time %ld.%ld\n",time.tv_sec,time.tv_nsec);
    }

    /* prepare for next call */
    old_sec= time.tv_sec;
    old_nsec= time.tv_nsec;


    //strcpy(message_1,"");
    //sprintf(message_1, "current time: %d.%d\n",time.tv_sec,time.tv_nsec);

    rp1 = !rp1;

    /* loops until you return 0 */
    if(rp1){
        return 0;
    }

    len = strlen(message_1);

    /* memory copy to proc file */
    printk(KERN_NOTICE "proc called read/n");
    copy_to_user(buf,message_1,len);
    return len;
}


int my_xtime_release_proc(struct inode *sp_inode, struct file *sp_file){
    printk(KERN_INFO "proc called release\n");
    kfree(message_1);
    kfree(message_2);
    return 0;
}

static int my_xtime_init(void){
    printk(KERN_NOTICE "/proc/%s created\n", ENTRY_NAME);
    fops.open = my_xtime_open_proc;
    fops.read = my_xtime_read_proc;
    fops.release =  my_xtime_release_proc;

/*    old_sec = 0;
    old_nsec = 0;
*/
    if(!proc_create(ENTRY_NAME, MODPERMISSIONS, NULL, &fops)){
        printk(KERN_WARNING "ERROR! proc_create\n");
        remove_proc_entry(ENTRY_NAME, NULL);
        return -ENOMEM;
    }
    return 0;
}



module_init(my_xtime_init);

static void my_xtime_exitmod(void){
    printk(KERN_NOTICE "removing /proc/%s \n", ENTRY_NAME);
    remove_proc_entry(ENTRY_NAME, NULL);
}

module_exit(my_xtime_exitmod);
