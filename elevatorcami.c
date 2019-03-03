#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <syscalls.h>


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uacess.h>
#include <linux/random.h>
#include <linux/kthread.h>
#include <linux/sched.h>


#define MAXWEIGHT = 30;
/*multiply all weight and max weight b 2;
 * */

typedef struct{
        int weight_units;
        int destination_floor;
        int start_floor;
        int type;
        int passenger_units;



	struct list_head list;
        /*adult = 1, child = 2, Romm service = 3, bellhop = 4
         * i think we have to use int as it is what the issue_request takes as a parameter.
         * space_units are the amount of space the passangers take up. adualt and child = 1
         * bellhop and roomservice = 2.  max is 10 space units. max weight is 15 */
} passengers;

typedef struct{
        int current_weight;
        int current_passengers;
        /*linked list holding passangers*/
        int state;
        /*0 is idle, 1 is running, -1 is off.
         * we could use another type, just my first idea*/
        int current_floor;

} elevator;


//Elevator states

/*  You will also need to print the following for each floor of the building:
 •The load of the waiting passengers
 •The total number of passengers that have been serviced   */


#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

#define start_elevator 333
#define issue_request 334
#define stop_elevator 335


#define waitBetFloors 2
#define waitUnload 1

#define firstfloor 1
#define lastfloor 10

int current_floor;
int next_floor;
int current_passengers;
int current_weight;
int passengers_done;
struct list_head allfloors[10];




struct elevator e1;
static int read_p;
static char *message;

/* Activates the elevator for service. From that point onward, the elevator exists and will begin to service requests.
 This system call will return 1 if the elevator is already active, 0 for a successful elevator start, and -ERRORNUM if it could not initialize
 (e.g. -ENOMEM if it couldn’t allocate memory). */
extern int (*STUB_start_elevator)(void);
int start_elevator(void)
{
        /*e1 = kmalloc(sizeof (struct elevator), GFP_ATOMIC);*/
        if(e1.state > 0)
        {
                return 1;
        }
        else
                e.state = 0;


	e1.current_floor = 1;
        e1.current_weight = 0;
        e1.current_passengers = 0;

	// return -ENOMEN if couldnt allocate memory

	return 0;


}

extern int (*STUB_stop_elevator)(void);
int stop_elevator(void)
{
        /* Description: Deactivates the elevator. At this point, this elevator will process no more new requests
         * (that is, passengers waiting on floors). However, before an elevator completely stops, it must offload
         * all of its current passengers. Only after the elevator is empty may it be deactivated (state =
         * OFFLINE). This function returns 1 if the elevator is already in the process of deactivating, and 0
         * otherwise.*/


	printk(KERN_DEBUG "Stopping Elevator");
	//lock elevator
	//

	//check if elevator is stopped


	//stop the elevator

	//unlock the elevator

        if(e1.state != -1)
        {
                e1.state == -1;
		//empty passengers
                return 0;
        }
        else
                return 1; 
}


/* Creates a passenger of type passenger_typeat start_floorthat wishes to go to destination_floor.
 This function returns 1 if the request is not valid (one of the variables is out of range), and 0 otherwise. */
extern int (*STUB_issue_request)(int)(int)(int);
int issue_request(int passenger_type, int start_floor, int destination_floor)
{
        if( start_floor > 10 || start_floor < 1)
                return 1;
        else if( destination_floor > 10 || destination_floor < 1)
                return 1;

        struct passenger new_passenger = kmalloc(sizeof(struct passenger), __GFP_RECLAIM);
        new_passenger->type = passenger_type;
        new_passenger->destination_floor = destination_floor;
	new_passenger->current_floor =  start_floor;


        if(passenger_type == 1) //ADULT
	{
                new_passenger.weight = 2;
                new_passenger.passenger_units = 1;
        }
        else if(passenger_type == 2) //CHILD
        {
                new_passenger.weight = 1;
                new_passenger.passenger_units = 1;
        }
        else if(passenger_type == 3)
        {
                new_passenger.weight = 4;
                new_passenger.passenger_units = 2;
        }
        else if (passenger_type == 4)
        {
                new_passenger.weight = 8;
                new_passenger.passenger_units = 2;
        }
        else
                return 1;


	//mutex lock : mutex_lock_interruptible(//// &passengermutex ///)
	list_add_tail(&new_passenger->list, &allfoors[start-1];
	//mutex unlock(&allfloorsMUTEX);

	//put it in list of one less bc account for 0

}

ssize_t  proc_read(struct file *sp_file, char__user*buf, size_t size, loff_t*offset){
	int len;

	//if theres a child
	if(1 /*change to odd number of children*/){message_1, "Elevator direction: %s \n Current Floor: %d \n Next Floor: %d \nCurrent Passengers: %d\nCurrent Weight: %d.5 units);// \nPassengers waiting %s \nPassengers serviced: %d \n", 
		directionString(mainDirection), current_floor, next_floor, current_weight, current_passengers); //, queueToString(),passengers_done);

        }
	else{//no child


	}




	read_p=!read_p;
	if(read_p){
		return 0;
	}

	len= strlen(message_1);
	printk(KERN_INFO "proc called read\n");
	copy_to_user(buf,message_1,len);
	return len;

}

int elevator_proc_release(struct inode *sp_inode, struct file*sp_file){
	printk(KERN_NOTICE "proc called release\n");
	kfree(message_1);
}


static void elevator_exit(void){
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_NOTICE "removing /proc/%s\n", ENTRY_NAME);


	STUB_issue_request = NULL;
        STUB_stop_elevator = NULL;
        STUP_start_elevator = NULL;

}


static int elevator_init(void){
	printk(KERN_NOTICE "/proc/%s create\n", ENTRY_NAME);
	fops.open = elevator_proc_open;
	fops.read = elevator_proc_read;
	fops.release = elevator_proc_release;

	STUB_issue_request = sys_issue_request;
        STUB_stop_elevator = sys_stop_elevator;
        STUP_start_elevator = sys_start_elevator;


	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk(KERN_WARNING "proc create\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

		return 0;
}
