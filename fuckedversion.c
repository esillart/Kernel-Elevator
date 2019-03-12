#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>
//#include <linux/export.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#define MODPARENT NULL
#define MODPERMISSIONS 0644
#define ENTRY_NAME "elevator"
MODULE_AUTHOR("Group 12");
MODULE_DESCRIPTION("elevator proc file");
MODULE_LICENSE("GPL");
#define MAXWEIGHT 30

#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

#define ADULT 1
#define CHILD 2
#define RS 3
#define BELLHOP 4


struct mutex passLock; 

#define START_ELEVATOR 333
#define ISSUE_REQUEST 334
#define STOP_ELEVATOR 335

#define waitBetFloors 2
#define waitUnload 1

#define firstfloor 1
#define lastfloor 10

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
    int passenger_units;
    int state;
    struct list_head onboard_passengers;
    /*0 is idle, 1 is running, -1 is off.
     * * we could use another type, just my first idea*/
    int current_floor;
    int next_floor;
} elevator;


//Elevator states

/*  You will also need to print the following for each floor of the building:
    •The load of the waiting passengers
    •The total number of passengers that have been serviced   */
int i;

int started=0;
int next_state;
int current_passengers;
int current_weight;
int total_served;
int removeNum;

struct list_head allfloors[10];

struct list_head *pos;
struct list_head *pos2;

elevator *e1;
passengers *new_passenger;
passengers *tmp;
passengers *tmp2;
passengers *tmp3;
static int read_p;
static char *message_1;
static char *message_2;

//struct of elevator threads, this is a must
struct task_struct * elevatorthread;


static struct file_operations fops; /* points to proc file definitions */

int load_elevator(int floor);
int scheduler(void *d);
void print_who_on_it(elevator *e);
/* Activates the elevator for service. From that point onward, the elevator exists and will begin to service requests.
   This system call will return 1 if the elevator is already active, 0 for a successful elevator start, and -ERRORNUM if it could not initialize
   (e.g. -ENOMEM if it couldn’t allocate memory). */
extern int (*STUB_start_elevator)(void);
int start_elevator(void)
{
    printk(KERN_WARNING "CALLED START ELEVATOR\n");

    //changed this to GFP_KERNEL from GFP_ATOMIC after reading stuff on GFP kmalloc

    //bool to check if program has started. Will stay 1 until program is completely restarted
    if(started==1)
    {
	return 1;
    }

    else
    {//initialize the elevator
	e1 = kmalloc(sizeof (elevator), GFP_KERNEL);

	if(!e1)
	{
	    //error has occured
	    return -ENOMEM;
	}

	e1->state = IDLE;
	e1->current_floor = 1;
	e1->next_floor = 2;
	e1->current_weight = 0;
	e1->passenger_units = 0;
	INIT_LIST_HEAD(&(e1->onboard_passengers));

	//this should initalize the array of linked list for the floors.
	for(i = 0; i < 10; i++)
	{
	    INIT_LIST_HEAD(&allfloors[i]);
	}


    }
    // return -ENOMEN if couldnt allocate memory

    started = 1;

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

    //unlock the elevato:r

    kthread_stop(elevatorthread);


    if(started == 1)
    {
	printk(KERN_INFO "turning off elevator\n");
	e1->state = OFFLINE;
	//empty passengers
	started = 0;
	return 0;
    }
    else
	return 1; 
}


/* Creates a passenger of: type passenger_typeat start_floorthat wishes to go to destination_floor.
   This function returns 1 if the request is not valid (one of the variables is out of range), and 0 otherwise. */
extern int (*STUB_issue_request)(int,int,int);
int issue_request(int passenger_type, int start_floor, int destination_floor)
{
    if( start_floor > 10 || start_floor < 1)
	return -1;
    else if( destination_floor > 10 || destination_floor < 1)
	return -1;

    new_passenger = kmalloc(sizeof(passengers), GFP_KERNEL);
    new_passenger->type = passenger_type;
    new_passenger->destination_floor = destination_floor;
    new_passenger->start_floor = start_floor;


    if(passenger_type == ADULT)
    {
	new_passenger->weight_units= 2;
	new_passenger->passenger_units = 1;
    }
    else if(passenger_type == CHILD)
    {
	new_passenger->weight_units = 1;
	new_passenger->passenger_units = 1;
    }
    else if(passenger_type == RS)
    {
	new_passenger->weight_units = 4;
	new_passenger->passenger_units = 2;
    }
    else if (passenger_type == BELLHOP)
    {
	new_passenger->weight_units = 8;
	new_passenger->passenger_units = 2;
    }
    else
	return -1;

    /*	this works, so problem is not with new_passenger
	printk("testing elevator");
	e1->current_weight = e1->current_weight + new_passenger->weight_units;
	e1->passenger_units++;
	*/
    //mutex lock : mutex_lock_interruptible(//// &passengermutex ///)
    list_add_tail(&(new_passenger->list), &allfloors[start_floor-1]);
    //mutex unlock(&allfloorsMUTEX);

    message_2 = kmalloc(sizeof(char) * 150, GFP_KERNEL);
    printk("PRINTING ALL FLOORS");
    for(i = 0; i < 10; i++)
    {	//iterate throu array and access the linked list
	printk("i is = %d",i);
	list_for_each(pos, &allfloors[i])	//func to iterat throu list
	{
	    tmp2 = list_entry(pos, passengers, list);//list_entry gets you the thing the list is in. 
	    sprintf(message_2,"type: %d, start: %d, destination: %d",tmp2->type, tmp2->start_floor, tmp2->destination_floor);
	    printk(message_2);

	}
    }
    //put it in list of one less bc account for 0
    kfree(message_2);	

    printk(KERN_INFO "calling scheduler\n");
    elevatorthread = kthread_run(scheduler, NULL, "Elevator scheduler thread started");
    //scheduler();


    return 0;
}

ssize_t  proc_read(struct file *sp_file, char __user*buf, size_t size, loff_t *offset){
    int len;

    /*

    //if theres a child
    if(1 //change to odd number of children){
    message_1, "Elevator direction: %s \n Current Floor: %d \n Next Floor: %d \nCurrent Passengers: %d\nCurrent Weight: %d.5 units);// \nPassengers waiting %s \nPassengers serviced: %d \n", 
    directionString(mainDirection), current_floor, next_floor, current_weight, passenger_units); //, queueToString(),passengers_done);

    }
    else{//no child

    }

*/

    read_p=!read_p;

    if(read_p){
	return 0;
    }

    sprintf(message_1, "STATE: %d\n Current floor: %d\n Destination: \n Weight units: %d\n Passenger units: %d\n Total serviced: %d\n",
	    e1->state, e1->current_floor, e1->current_weight, e1->passenger_units, total_served);
    len = strlen(message_1);
    printk(KERN_INFO "proc called read\n");
    printk(KERN_WARNING "%s",message_1);
    copy_to_user(buf,message_1,len);
    return len;

}

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file){

    printk(KERN_INFO "proc called open!\n");	


    read_p = 1;

    message_1 = kmalloc(sizeof(char) * 2048, __GFP_RECLAIM | __GFP_IO | __GFP_FS);


    if(!message_1){
	printk(KERN_WARNING "ERROR: elevator_proc_open");
	return -ENOMEM;
    }

    return 0;
}




int elevator_proc_release(struct inode *sp_inode, struct file*sp_file){
    printk(KERN_NOTICE "proc called release\n");
    kfree(message_1);
    return 0;
}


static void elevator_exit(void){
    remove_proc_entry(ENTRY_NAME, NULL);
    printk(KERN_NOTICE "removing /proc/%s\n", ENTRY_NAME);

    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
    STUB_issue_request = NULL;


    return;
}

int scheduler(void *d){

    printk(KERN_INFO "Elevator state is currently: %d\n", e1->state);

    printk(KERN_INFO "scheduler() started\n");


    /*
    //grab destination of 1st pass on the elevator
    //compare with current_floor 
    if(&(e1->onboard_passengers) == NULL)

    {
    printk(KERN_INFO "e1->onboard_passengers is null or something\n");
    return -1;
    }	

    if(list_empty(&(e1->onboard_passengers)))
    {
    printk(KERN_INFO "list_empty(&(e1->onboard_passengers)) started\n");
    return -1;
    }

    tmp = list_first_entry(&(e1->onboard_passengers), passengers, list);

    if(tmp->destination_floor > e1->current_floor)
    {
    printk(KERN_INFO "(tmp->destination_floor > e1->current_floor) started\n");
    next_state = UP;
    }
    else if( tmp->destination_floor < e1->current_floor)
    {
    printk(KERN_INFO "( tmp->destination_floor < e1->current_floor)\n");
    next_state = DOWN;
    }
    else
    {
    printk(KERN_INFO "last else statement started\n");
    //dump, increment return
    list_del(&tmp->list);
    total_served++;
    return 0;
    }
    */

    while(!kthread_should_stop()){
	printk(KERN_INFO "while !kthread loop started\n");

	if(e1->state == OFFLINE){
	    // Nothing happens
	    printk(KERN_INFO "e1->state == OFFLINE started\n");

	}
	else if(e1->state == IDLE){
	    //if idle it can load elevator
	    e1->state = LOADING;		
	    printk(KERN_INFO "e1->state == IDLE started\n");
	}
	else if(e1->state == LOADING){
		//sleep one second
		//ssleep(1);
		//unload();
		//unload and increase passengers done counter
		printk(KERN_INFO "e1->state ==LOADING started\n");

		if(started == 1){
			//if(can_load()){
			printk(KERN_INFO "started == 1 started, about to load_elevator()\n");

			load_elevator(e1->current_floor);
			//}

			//third attempt at trying to stop the infinite thread loop by placing this kthread_stop here....
			kthread_stop(elevatorthread);
		}

		if(next_state == UP){
			printk(KERN_INFO "next_state == UP started\n");

			if(e1->current_floor == 10){
				printk(KERN_INFO "e1->current_floor == 10 started\n");

				e1->state = DOWN;
				e1->next_floor = e1->current_floor - 1;
			}
			else{
				printk(KERN_INFO "else statement to current_floor == 10 \n");

				e1->next_floor = e1->current_floor + 1;
			}
			//global_state = next_state;
		}

		else if(next_state == DOWN){
			printk(KERN_INFO "next_state == DOWN started\n");

			if(e1->current_floor == 1){
				e1->state = UP;
				e1->next_floor = e1->current_floor + 1;
			}
			else{
				e1->next_floor = e1->current_floor - 1;
			}

		}
	}
	else if(e1->state == UP){
		printk(KERN_INFO "e1->state == UP started\n");

		//function to actually move elevator
		if(e1->current_floor == 10){
			e1->state = DOWN;
			e1->next_floor = e1->current_floor - 1;
		}
		e1->state = next_state;				
	}

	else if(e1->state == DOWN){
		printk(KERN_INFO "e1->state == DOWN started\n");

		// function actually move elevator
		if(e1->current_floor == 1){
			e1->state = UP;
			e1->next_floor = e1->current_floor + 1;
		}
		else{
			e1->next_floor = e1->current_floor - 1;
		}

		//implement to check if thread stopped to turn offline
	}
	}
	return 0;
}


/*this function tries to push as many passengers, in order, onto the elevator
 * can probably copy the structure for an unload function
 *
 */

int load_elevator(int floor)
{
	//**ERROR CHECKING DELETE THIS**
	//checking to see if this function was ran
	printk(KERN_INFO "load_elevator started\n");
	removeNum = 0;
	if(e1->current_floor == floor)
	{
		printk(KERN_INFO "if(e1->current_floor == floor) started\n");
		printk(KERN_INFO "floor sent in %d \n", floor-1);
		//mutex_lock_interruptible(&passLock);
		list_for_each_safe(pos2, pos, &allfloors[floor-1])
		//list_for_each_entry(pos2, &allfloors[floor-1], list)
		{
			removeNum++;
		}

		for(i = 0; i < removeNum; i++)
		{
			printk(KERN_INFO "HERE IN MY FOR LOOP!\n");
			//tmp = list_entry(pos, passengers, list);
			//list entry gets you the content of the surrounding list. 
			//so if list is within a passenger struct, will get you the passenger information
				//list_entry returns a pointer to information, so no need to kmalloc for tmp
				//tmp is now the passenger information found in the list
			tmp = list_first_entry(&allfloors[floor-1], passengers, list);					

			new_passenger = kmalloc(sizeof(passengers), GFP_KERNEL);
    			new_passenger->type = tmp->type;
    			new_passenger->destination_floor = tmp->destination_floor;
    			new_passenger->start_floor = tmp->start_floor;
			
			printk(KERN_INFO "type: %d dest floor %d\n ",new_passenger->type, new_passenger->destination_floor);



			if (tmp == NULL)
				break;

			if((tmp->weight_units + e1->current_weight < MAXWEIGHT) && (tmp->passenger_units + e1->passenger_units < 10))
			{
				e1->current_weight = e1->current_weight + tmp->weight_units;
				e1->passenger_units = e1->passenger_units + tmp->passenger_units;

				//**ERROR CHECKING DELETE THIS LATER**
				printk(KERN_INFO "just added %d weight units and %d passenger units\n", tmp->weight_units, tmp->passenger_units);



				//this deletes from one list and adds as another tail
				//THIS LINE OF CODE IS CAUUSING US PROBLEMS WITHIN THE LOOP, NEED TO FIX
			//	list_move_tail(&(tmp->list), &(e1->onboard_passengers));
		//		print_who_on_it(e1);				

				printk(KERN_INFO "e1 test\n");

				//this should be a list_delete function instead	
				//commenting it out would essentially delete already
				kfree(tmp);
			}
			else
			{
				//another attempt at stopping infinite looping thread
				printk(KERN_INFO "else statement in load elevator started\nAlso kthread_stop\n");
			//	kthread_stop(elevatorthread);
				break;
			}
		}
		//mutex_unlock(&passLock);

	}

	//stop the thread here as the infinite thread loop seems to repeat this function a lot	
	//contd...: okay so this didn't stop the infinite thread loop
	//kthread_stop(elevatorthread);

	return 0;
}






static int elevator_init(void){

    fops.open = elevator_proc_open;
    fops.read = proc_read;
    fops.release = elevator_proc_release;

    STUB_start_elevator = &start_elevator;
    STUB_stop_elevator = &stop_elevator;
    STUB_issue_request = &issue_request;

    if(!proc_create(ENTRY_NAME, MODPERMISSIONS, NULL, &fops)){
	printk(KERN_WARNING "ERROR proc create\n");
	remove_proc_entry(ENTRY_NAME, NULL);
	return -ENOMEM;
    }

    //**MIGHT DELETE THIS LATER**
    //next_state = LOADING;

    /*
       printk(KERN_INFO "elevator thread about to start\n");
       elevatorthread = kthread_run(scheduler, NULL, "Elevator scheduler thread started");
       */

    return 0;
}

static int move_elevator(int floor){
    if(floor != e1->current_floor){
	printk("Now moving floor to %d\n", floor);
	//sleep elevator
	//ssleep(2);
	e1->current_floor = floor;
	return 1;
    }
    else{
	return -1;

    }
}
void print_who_on_it( elevator *e)
{
	message_2 = kmalloc(sizeof(char) * 150, GFP_KERNEL);
	printk(KERN_INFO "PRINTING THE ELEVATOR PASSENGERS");
	
	list_for_each(pos, &(e->onboard_passengers))
	{
		tmp3 = list_entry(pos, passengers, list);
		sprintf(message_2,"type: %d, start: %d, destination: %d",tmp3->type, tmp3->start_floor, tmp3->destination_floor);
		printk(message_2);
		kfree(tmp3);
	}
}

module_init(elevator_init);
module_exit(elevator_exit);
