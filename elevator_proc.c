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
#define PERMS 0644
#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000


MODULE_AUTHOR("Group 12");
MODULE_DESCRIPTION("elevator proc file");
MODULE_LICENSE("GPL");

#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4
#define EQUAL 5

#define ADULT 1
#define CHILD 2
#define RS 3
#define BELLHOP 4

#define MAXWEIGHT 30
#define MAXUNITS 10
/*multiply all weight and maxweight by two
  to account for fractions*/


#define START_ELEVATOR 333
#define ISSUE_REQUEST 334
#define STOP_ELEVATOR 335

#define firstfloor 1
#define lastfloor 10


typedef struct passenger{
	int weight_units;
	int space_units;
	int start_floor;
	int destination_floor;
	int type;
	int state;

	struct list_head list;
} Passenger;

struct{
	int current_weight;
	int current_units;

	int previous_state;
	int state;
	int next_state;

	int current_floor;
	int next_floor;

	struct list_head onboard;
} elevator;


int i;
int started = 0;
int odd_children = 0;
//should this be inside the elevator struct?
int next_state;

int total_served;

//similar to remove_number int in other file
int removed_from_floor;

//struct of floors that contains list_head for Passenger types
struct list_head floors[10];
int served[10] = {0,0,0,0,0,0,0,0,0,0};

static struct file_operations fops;

//for struct iteration
struct list_head *pos;

static char *message;
static int read_p;

struct thread_parameter {
	int id;
	struct task_struct *kthread;
	struct mutex mutex;
};

struct thread_parameter thread1;

//prototypes
int add_passenger(int type, int start_floor, int destination_floor);
int print_passengers(void);
int delete_passenger(void);
int load_elevator(int floor);
int unload_elevator(int floor);
int move_elevator(int floor);
int min_onboard(void);
int min_floor(int floor);
int print_who_on_it(void);
int scheduler(void *data);
/***********************************************************************************/


extern int (*STUB_start_elevator)(void);
int start_elevator(void){

	printk(KERN_INFO "CALLED START ELEVATOR\n");

	if(started == 1)
		return 1;
	else{
		elevator.state = IDLE;
		elevator.current_floor = 1;
		started = 1;
	}

	return 0;
}

extern int (*STUB_stop_elevator)(void);
int stop_elevator(void){

	printk(KERN_INFO "CALLED STOP ELEVATOR");

	if(started == 1){
		if(elevator.state == UP || elevator.state == DOWN){
			elevator.previous_state = elevator.state;
		}
		elevator.state = OFFLINE;
		started = 0;

		return 0;
	}
	else
		return 1;
}

extern int (*STUB_issue_request)(int,int,int);
int issue_request(int passenger_type, int start_floor, int destination_floor){

	printk(KERN_INFO "CALLED ISSUE REQUEST\n");

	/*error checking*/
	if(passenger_type > 4 || passenger_type < 1){
		return 1;
	}	
	else if(start_floor > 10 || start_floor < 1){
		return 1;
	}
	else if(destination_floor > 10 || destination_floor < 1){
		return 1;
	}
	else{
		add_passenger(passenger_type, start_floor, destination_floor);

		return 0;
	}
}

/******************************************************************************/

void thread_init_parameter(struct thread_parameter *parm) {
	static int id = 1;
	parm->id = id++;
	mutex_init(&parm->mutex);
	parm->kthread = kthread_run(scheduler, parm, "Scheduler thread %d", parm->id);
}

/*******************************************************************************/



int add_passenger(int type, int start_floor, int destination_floor){
	Passenger *p;

	p = kmalloc(sizeof(Passenger) * 1, __GFP_RECLAIM);

	if (p == NULL)
		return -ENOMEM;

	p->type = type;
	p->start_floor = start_floor;
	p->destination_floor = destination_floor;


	if(type == ADULT)
	{
		p->weight_units= 2;
		p->space_units = 1;
	}
	else if(type == CHILD)
	{	
		p->weight_units = 1;
		p->space_units = 1;
	}
	else if(type == RS)
	{
		p->weight_units = 4;
		p->space_units = 2;
	}
	else if(type == BELLHOP)
	{
		p->weight_units = 8;
		p->space_units = 2;
	}
	else{
		return -1;
	}

	if(start_floor - destination_floor > 0)
		p->state = DOWN;

	else if(start_floor - destination_floor < 0)
		p->state = UP;

	else
		p->state = EQUAL;

	//may have to surround this with locks later
	list_add_tail(&p->list, &floors[start_floor - 1]);

	print_passengers();

	return 0;
}

int print_passengers(void){
	Passenger *p;
	struct list_head *temp;

	printk("PRINTING ALL FLOORS\n");
	for(i = 0; i < 10; i++){   

		//iterate through nodes
		printk("floor # is = %d\n",i + 1);

		list_for_each(temp, &floors[i]){       //iterate through nodes of list

			p = list_entry(temp, Passenger, list);//list_entry gets you the struct the list is in. 
			printk("type: %d, start: %d, destination: %d, state: %d\n", p->type, p->start_floor, p->destination_floor, p->state);
		}
	}
	return 0;
}

int print_who_on_it(void){

	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;



	printk(KERN_INFO "printing whose on it\n");	

	list_for_each_safe(temp, dummy, &elevator.onboard){
		p = list_entry(temp, Passenger, list);

		printk("type: %d, start: %d, destination: %d\n", p->type, p->start_floor, p->destination_floor);
	}
	return 0;
}

int delete_passenger(void){
	//clear all floors of passengers
	struct list_head move_list;
	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;
	int i;
	int count = 0;

	INIT_LIST_HEAD(&move_list);

	printk(KERN_INFO "CALLED DELETE_PASSENGER");
	for(i = 0; i < 10; i++){
		list_for_each_safe(temp, dummy, &floors[i]){
			list_move_tail(temp, &move_list);
			count++;
		}

	}
	list_for_each_safe(temp, dummy, &move_list){
		p = list_entry(temp , Passenger, list);
		list_del(temp);
		kfree(p);
	}
	printk(KERN_INFO "deleted %d people\n",count);
	return 0;
}

int load_elevator(int floor){
	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;

	if(floor != elevator.current_floor){
		printk(KERN_WARNING "floor and current_floor do not match\n");
		return -1;
	}
	else{
		printk(KERN_INFO "LOAD_ELEVATOR\n");	
		if(list_empty(&floors[floor-1]))
			return -1;

		list_for_each_safe(temp,dummy,&floors[floor-1]){
			p = list_entry(temp, Passenger, list);
			printk(KERN_INFO "p.type: %d, p.start_floor: %d, p.destination_floor: %d\n",p->type, p->start_floor, p->destination_floor);

			if(p->state == EQUAL){
				served[floor-1]++;
				total_served++;
				list_del(&p->list);
			}

			else if( elevator.current_weight + p->weight_units <= MAXWEIGHT 
					&& elevator.current_units + p->space_units <= MAXUNITS){

				if(p->type == CHILD)
					odd_children++;

				elevator.current_weight += p->weight_units;
				elevator.current_units += p->space_units;

				list_move_tail(&p->list, &elevator.onboard);
				print_who_on_it();
			}
			else if(elevator.current_weight + p->weight_units > MAXWEIGHT || elevator.current_units + p->space_units > MAXUNITS ){
				printk(KERN_INFO "people do not fit correctly\n");
				return -1;
			}
			else{
				printk(KERN_INFO "no one\n");
				return -1;
			}
		}
	}
	return 0;
}

int unload_elevator(int floor){

	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;

	if(elevator.current_floor != floor){
		printk(KERN_WARNING "floor and current_floor do not match\n");
		return -1;
	}

	list_for_each_safe(temp,dummy,&elevator.onboard){
		p = list_entry(temp, Passenger, list);
		if(p->destination_floor == floor){
			printk(KERN_INFO "Unloading passenger\n");
			elevator.current_weight -= p->weight_units;
			elevator.current_units -= p->space_units;
				
			if(p->type == CHILD)
				odd_children--;
			served[floor - 1]++;
			total_served++;
			list_del(&p->list);
			print_who_on_it();
		}
	}

	return 0;
}

int scheduler(void *data){
	int i;
	Passenger *p;
	struct thread_parameter *parm = data;
	printk(KERN_INFO "HERE IN THE SCHEDULER\n");

	while (!kthread_should_stop()) {
		if(elevator.state == OFFLINE){
			//			printk(KERN_INFO "elevator.state == OFFLINE\n");

			if (mutex_lock_interruptible(&parm->mutex) == 0){
				while(!list_empty(&elevator.onboard)){
					move_elevator(min_onboard());			
					unload_elevator(elevator.current_floor);
				}
			}
			mutex_unlock(&parm->mutex);
			//drop off people in elevator
			//when empty, stop.
		}
		else if(elevator.state == IDLE){
			for(i = 0; i < 10; i++){
				if(!list_empty(&floors[i])){
					p = list_first_entry(&floors[i],Passenger,list);
					move_elevator(p->start_floor);
					elevator.state = LOADING;
				}
			}		
		}

		else{


			if (mutex_lock_interruptible(&parm->mutex) == 0){
				elevator.state = LOADING;
				unload_elevator(elevator.current_floor);
				ssleep(1);
				load_elevator(elevator.current_floor);	
				elevator.state = UP;
			}
			mutex_unlock(&parm->mutex);

			if(elevator.state == UP){
				if (mutex_lock_interruptible(&parm->mutex) == 0){
					if(elevator.current_floor == 10)
						move_elevator(1);
					else
						move_elevator(elevator.current_floor+1);
				}
				mutex_unlock(&parm->mutex);
			}

			if (elevator.state == DOWN){
				if (mutex_lock_interruptible(&parm->mutex) == 0){
					if(elevator.current_floor ==  1)
						elevator.state = UP;
					else
						move_elevator(elevator.current_floor-1);
					mutex_unlock(&parm->mutex);
				}
			}	

			if(list_empty(&elevator.onboard)){
				elevator.state = IDLE;
			}

		}
	}
		return 0;
}

int move_elevator(int floor){
	if(floor != elevator.current_floor){
		int x;
		printk("Now moving floor to %d\n", floor);

		x = elevator.current_floor- floor;
		if(x<0){
			x = x* -1;
		}

		elevator.next_floor = floor;	
		//sleep elevator
		ssleep(2*x);
		elevator.current_floor = floor;
		return 1;
	}
	else{
		return -1;
	}
}

int min_floor(int floor){

	int minDiff = 10;
	int minFloor = 0;
	int tempNum;
	int i;
	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;


	printk(KERN_INFO "in min_floor function...\n");

	for(i = 0; i < 10; i++){
		list_for_each_safe(temp, dummy, &floors[i]){
			p = list_first_entry(temp, Passenger, list);
			if((elevator.previous_state == p->state)
					&& (elevator.previous_state == UP)
					&& (elevator.current_weight + p->weight_units <= MAXWEIGHT)
					&& (elevator.current_units + p->space_units <= MAXUNITS)){

				tempNum = p->start_floor - elevator.current_floor;
				if(tempNum < minDiff){
					minDiff = tempNum;
					minFloor = p->start_floor;
				}
			}

			else if((elevator.previous_state == p->state)
					&& (elevator.previous_state == DOWN)
					&& (elevator.current_weight + p->weight_units <= MAXWEIGHT)
					&& (elevator.current_units + p->space_units <= MAXUNITS)){

				tempNum =  elevator.current_floor - p->start_floor;
				if(tempNum < minDiff){
					minDiff = tempNum;
					minFloor = p->start_floor;
				}
			}
			else if((elevator.previous_state != UP) && (elevator.previous_state != DOWN)
					&& (elevator.current_weight + p->weight_units <= MAXWEIGHT)
					&& (elevator.current_units + p->space_units <= MAXUNITS)){
				
				printk(KERN_INFO "previous elevator state wasn't UP or DOWN\n");

				//this passenger is below the elevator
				if(p->start_floor < elevator.current_floor){
					printk(KERN_INFO "passenger is below the elevator\n");
					printk(KERN_INFO "p->start floor is: %d and ele.current_floor is: %d\n",
						       p->start_floor, elevator.current_floor);
					tempNum = elevator.current_floor - p->start_floor;
					elevator.state = DOWN;
				}//this one is above
				else{
					printk(KERN_INFO "passenger is above the elevator\n");
					printk(KERN_INFO "p->start floor is: %d and ele.current_floor is: %d\n",
						       p->start_floor, elevator.current_floor);
					tempNum = p->start_floor - elevator.current_floor;
					elevator.state = UP;
				}
				if(tempNum < minDiff){
					minDiff = tempNum;
					minFloor = p->start_floor;
				}
			}
		}
	}
	printk(KERN_INFO "returning minFloor value (in min_floor) is: %d\n", minFloor);
	return minFloor;
}



int min_onboard(void){
	int minDiff = 10;
	int minFloor = 0;
	int tempNum;
	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;

	printk(KERN_INFO "CALLING min_onboard\n");
	if(elevator.previous_state == UP){
		list_for_each_safe(temp, dummy, &elevator.onboard){
			p = list_entry(temp, Passenger, list);
			tempNum = p->destination_floor - elevator.current_floor;

			//found a closer floor in the up direction
			if(tempNum < minDiff){
				minDiff = tempNum;
				minFloor = p->destination_floor;
			}
		}
	}

	else if(elevator.previous_state == DOWN){
		list_for_each_safe(temp, dummy, &elevator.onboard){
			p = list_entry(temp, Passenger, list);
			tempNum = elevator.current_floor - p->destination_floor;

			//found a closer floor in the down direction
			if(tempNum < minDiff){
				minDiff = tempNum;
				minFloor = p->destination_floor;
			}
		}
	}

	else{
		return -1;
	}

	printk(KERN_INFO "returning minFloor value (in min_onboard) is: %d\n", minFloor);
	return minFloor;
}


/******************************************************************************/


int elevator_proc_open(struct inode *sp_inode, struct file *sp_file){
	printk(KERN_INFO "proc called open\n");

	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);

	if (message == NULL) {
		printk(KERN_WARNING "elevator_proc_open");
		return -ENOMEM;
	}

	/* in random_animals, this was here, we may or may not need 
	   add_passenger();
	   */

	//kfree(message);

	return 0;
}

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){
	int len;
	int i;
	int decimal;
	char buffer[1000];

	int weightSum = 0;
	int spaceSum = 0;

	struct list_head *temp;
	struct list_head *dummy;
	Passenger *p;

	read_p = !read_p;
	if(read_p)
		return 0;

	if(odd_children % 2 == 0 && odd_children >= 0)
		decimal = 0;
	else
		decimal = 5;

	sprintf(message, "\nSTATE: %d\nCurrent floor: %d\nNext floor: %d\nWeight units: %d.%d\nPassenger units: %d\nTotal serviced: %d\n",
			elevator.state, elevator.current_floor, elevator.next_floor, elevator.current_weight/2, decimal, elevator.current_units, total_served);

	strcat(message, "\n");
	for(i = 0; i < 10; i++){
		sprintf(buffer, "Served on floor %d:%d\n", i+1, served[i]);
		strcat(message,buffer);
	}	

	strcat(message, "\n");
	strcat(message, "People Waiting\n");
	for(i = 0; i < 10; i++){
		list_for_each_safe(temp, dummy, &floors[i]){
			p = list_entry(temp, Passenger, list);
			weightSum += p->weight_units;
			spaceSum += p->space_units;
		}
		sprintf(buffer, "floor: %d, weight: %d, space: %d\n",i+1,weightSum/2,spaceSum);
		strcat(message, buffer);
		spaceSum =0;
		weightSum = 0;
	}
	strcat(message, "\n");

	printk(KERN_INFO "proc called read\n");	

	len = strlen(message);
	copy_to_user(buf, message, len);
	memset(buffer,0,1000);
	return len;
}


int elevator_proc_release(struct inode *sp_inode, struct file *sp_file){
	printk(KERN_NOTICE "proc called release\n");
	kfree(message);
	return 0;
}


/**************************************************************************************************/


static int elevator_init(void){
	printk(KERN_NOTICE "/proc/%s create\n",ENTRY_NAME);

	fops.open = elevator_proc_open;
	fops.read = elevator_proc_read;
	fops.release = elevator_proc_release;

	STUB_start_elevator = &start_elevator;
	STUB_stop_elevator = &stop_elevator;
	STUB_issue_request = &issue_request;

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk(KERN_WARNING "elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	thread_init_parameter(&thread1);
	if (IS_ERR(thread1.kthread)) {
		printk(KERN_WARNING "error spawning thread");
		remove_proc_entry(ENTRY_NAME, NULL);
		return PTR_ERR(thread1.kthread);
	}	

	total_served = 0;
	elevator.current_weight = 0;
	elevator.current_units = 0;
	elevator.current_floor = 0;
	elevator.next_floor =  0;
	elevator.state = OFFLINE;
	INIT_LIST_HEAD(&elevator.onboard);

	for(i = 0; i < 10; i++){
		INIT_LIST_HEAD(&floors[i]);
	}

	return 0;
}
module_init(elevator_init);

static void elevator_exit(void){
	//not sure about these functions
	//delete_passenger();	
	kthread_stop(thread1.kthread);
	delete_passenger();
	mutex_destroy(&thread1.mutex);
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);

	STUB_start_elevator = NULL;
	STUB_stop_elevator = NULL;
	STUB_issue_request = NULL;

}
module_exit(elevator_exit);
