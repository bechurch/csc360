READ ME FOR CSC 360 ASSIGNMENT 2 

files included:
	- mfs.c
	- mfs.h
	- flow.txt
	- readme.txt
	- makefile

MFS.C:
	1. main(int argc, char* argv) opens the file provided in the command line, extracts the number of flows included and assigns memory to queue variable as needed then calls stringtoflow() to create flows based of the information for each line of the text document. then it creates a thread running the yourflow() function for each flow and waits for the to terminate.

	2.stringtoflow(char* text) takes a string and tokenizes it and creates a flow structure from the data then returns said flow

	3. yourflow(void* arg) sleeps the flow for the duration of the arrival time, then it sleeps for the transmit time if there is no flow currently transmitting. if there is a flow transmiting it adds itself to the queue through the addtoqueue() function. Once the transmitting id is set to its own id by the queue it will sleep for it's transmit time. then it will remove itself from queue and set the transmiting id to the next in queue using nextQueue() or 0 if there are no flows in queue.

	4. addtoQueue(struct flow curFlow) takes a flow and adds it to the back of the que then bubble sorts it in the que first based on priority, then arrival time, then transmission time, then text document ranking.

	5. nextQueue() removes the first item in the queue and returns that items id.

	6. interval() returns the running time of the program.


mfs.h:
	a header file that contains the flow struct and global variables.
