README 
The project team members' names and division of labor:
  Camila Rios: Worked on part 2, worked collaboratively on part 3
  Casey Kwatkosky: Set up kernel, worked on part 1, worked collaboratively on part 3
  Emilio Sillart: Set up kernel, worked on part 1, worked on part 2, worked collaboratively on part 3

•The contents of your tar archive and a brief description of each file:
  tar file contains 3 directories and this README.txt file.
    part1: Makefile and exectuable for part 1.
		made to show syscalls used in machine for code used 
    part2: Makefile and executable for part 2.
		prints time and time difference
	part3: Makefile and exectuable for elevator_proc, sys_start_elevator, sys_stop_elevator, sys_issue_request
		elevator_proc.c is what actually runs the whole project. All of the code for the elevator goes in there. The only code that 			doesn't go in this file is the code for the syscalls, which are included in their own respective files. 
			
	

•How to compile your executables using your Makefile:
- For parts 1 and 2, simply using the "make" command with the "./a.out" command will compile the executables.
- For part 3, the "make" command will compile the elevator_proc.c file along with the other syscall files. After using make:
- "sudo insmod elevator_proc.ko" will install the elevator proc module. 
- "lsmod" can be used to check if it was properly installed. We like to use "lsmod | grep elevator" to quickly identify it.
- "gcc -o c.out consumer.c" will make the executable for consumer and rename it to "c.out".
- "./c.out --start" runs the elevator, "./c.out --stop" stops the elevator from running.
- "gcc -o p.out producer.c" will make the executable for producer and rename it to "p.out".
- "./p.out" will fill the floors with either all passengers or one passenger at a time, depending on the producer.c file used.
- *NOTE: after the elevator module has been installed, and while either ./c.out and ./p.out has been used, the proc file with the 	its output can be checked with "cat /proc/elevator" (this can vary depending on where proc file is located in machine) * 
- "sudo rmmod elevator_proc" (.ko is optional) removes the elevator module. This can be double checked again 
   with "lsmod | grep elevator".
  

•Known bugs and unfinished portions of the project:
	- There are no known bugs (that we know of) in the project. The whole elevator runs fine and services all the passengers once
	enough time has passed. The elevator was implemented with a FIFO scheduler, so we know it's not the most effective elevator.
	We originally tried to implement the scheduler with a version of SCAN that would have started the elevator on the 10th floor
	and serviced passengers in an efficient manner, but we realized that there was too many issues and infinite looping that was
	involved with implementing it that way, so we settled for FIFO as we were running out of time to complete the scheduler. 
	
	- We did not run the stress test 
	- We did however run the versions of producer.c that dispatches one passenger at a time and all passengers at once to each 			floor.
	- There are no unfinished portions of the project. 


•Special considerations or anything I should know when grading your solution◦Any completed extra credit must be documented in the README to receive credit:
	-During the initial stages of part 3, we tried to install the feature of being able to ssh into our current machine but our
	machine could not properly install the feature, along with many others, due to a problem with files being locked. We got
	someone from systems group to come in and help us, and they could not figure it out, so we got assigned a different machine.
	We were originally assigned machine 11, and then got re-assigned into machine 22. This set us back a bit during the inital
	stages of development in part 3.

- UNKNOWN SPECIFICS ABOUT WHAT YOU MEANT FOR "sleep between loading" we assumed you meant sleep 2 at each individual floor.
  -We assumed you meant the total serviced based on the floor they got off not the floor they started on.

