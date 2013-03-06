/*mfs.c for csc 360 as 2 by Ben Church v00732962*/

#include <semaphore.h>
#include <stdio.h> // Input and Output
#include <stdlib.h> // Dynamic memory allocation, process management,...
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "mfs.h"

/*****************************
*******interval FUNCTION******
*****************************
determine the time that has 
passed since start of program*/
float interval()
{
	struct timeval curTime;
	gettimeofday(&curTime, NULL);
	float interval = (curTime.tv_sec - start.tv_sec)+((curTime.tv_usec - start.tv_usec)/1000000.0);
	return interval;
}

/*****************************
***next in queue FUNCTION****
****************************
nextQueue() removes the first item in the queue,
reorganizes the queue
and outputs the id of the flow removed*/
int nextQueue()
{
	int next = queue[0].id;
	int i = 0;
	while(i<size_queue)
	{
		queue[i]=queue[i+1];
		i++;
	}
	return next;

}


/*****************************
***add to queue FUNCTION****
****************************
addtoQueue adds a flow to the back of the queue array
and the bubble sorts the queue based on
1.priority
2. arrival time
3. transmission time
4. file order
*/
void addtoQueue (struct flow curFlow)
{
	queue[size_queue]=curFlow;
	int i = size_queue;
	struct flow tmp;
	while(i>0)
	{
		//start priority check
		if(queue[i-1].priority > queue[i].priority)
		{
			tmp = queue[i-1];
			queue[i-1] = queue[i];
			queue[i] = tmp;
		}

		else if (queue[i-1].priority == queue[i].priority)
		{
			//start arrival time check
			if(queue[i-1].arrival_time > queue[i].arrival_time)
			{
				tmp = queue[i-1];
				queue[i-1] = queue[i];
				queue[i] = tmp;
			}

			else if (queue[i-1].arrival_time == queue[i].arrival_time)
			{
				//start trans time check
				if(queue[i-1].trans_time > queue[i].trans_time)
				{
					tmp = queue[i-1];
					queue[i-1] = queue[i];
					queue[i] = tmp;
				}

				else if(queue[i-1].trans_time == queue[i].trans_time)
				{
					//start id check
					if(queue[i-1].id > queue[i].id)
					{
						tmp = queue[i-1];
						queue[i-1] = queue[i];
						queue[i] = tmp;
					}
				}
			}

		}

		i--;
	}

	size_queue++;

	return;
}

/*****************************
***STRING TO FLOW FUNCTION****
****************************
stringtoflow takes text and converts it into flow structure then returns a flow
*/
struct flow stringtoflow(char* text)
{
	struct flow hold;
	int flow_array[4];
	char buffer[128];
	int i = 0;
	strcpy(buffer, text);

	//flow id
	char* token = strtok(text, ":");
	
	//tokenize the rest
	while (token)
        {
		flow_array[i++] = atoi(token);
            	token = strtok(NULL, ",");
        }

	//create flow from array
	hold.id = flow_array[0];
	hold.arrival_time = flow_array[1];
	hold.trans_time = flow_array[2];
	hold.priority = flow_array[3];


	
	return hold;	
}


/*****************************
*********FLOW THREAD**********
*****************************/
void *yourFlow(void* arg)
{
	//variables
	//struct timeval curTime;
	struct flow curFlow = *(struct flow *)arg;
	double arv = (double)curFlow.arrival_time/10;
	double trans = (double)curFlow.trans_time/10;


	//sleep until arrival time
	usleep(curFlow.arrival_time * 1E5F);
	
	
	printf("Flow %2d arrives: arrival time (%.2f), transmission time (%.1f), priority (%2d). \n",curFlow.id, interval(), trans, curFlow.priority);
	

	

	//if no thread transmitting then transmit
	if(!trans_thread)
	{
		//set transmitting id variable to current flows id (mutex protected)
		pthread_mutex_lock(&transmit);
		trans_thread = curFlow.id;
		pthread_mutex_unlock(&transmit);

		//sleep for transmission time
		printf("Flow %2d starts its transmission at time %.2f. \n", curFlow.id, interval());
		usleep(curFlow.trans_time* 1E5F);
		printf("Flow %2d finishes its transmission at time %.2f. \n", curFlow.id, interval());
	}

	//add to queue
	else
	{
		//add to queue (mutex protected)
		pthread_mutex_lock(&queue_mutex);
		addtoQueue(curFlow);
		pthread_mutex_unlock(&queue_mutex);


		//wait for this flow to be flagged to transmit, while waiting print the id of the flow were waiting on
		printf("Flow %2d waits for the finish of flow %2d. \n", curFlow.id, trans_thread);
		int tmp = trans_thread;
		while(trans_thread != curFlow.id)
		{
			if(tmp != trans_thread && trans_thread != curFlow.id)
			{
				tmp = trans_thread;
				printf("Flow %2d waits for the finish of flow %2d. \n", curFlow.id, trans_thread);
			}
		}

		//once flagged transmit
		pthread_mutex_lock(&transmit);
		trans_thread = curFlow.id;
		pthread_mutex_unlock(&transmit);

		printf("Flow %2d starts its transmission at time %.2f. \n", curFlow.id, interval());
		usleep(curFlow.trans_time* 1E5F);
		printf("Flow %2d finishes its transmission at time %.2f. \n", curFlow.id, interval());

	}

	
	//crit sec passed removing from queue (mutex protected)
	pthread_mutex_lock(&transmit);

	//if no items are queue'd set transmiting id to 0
	if(!size_queue)
	{
		trans_thread = 0;
	
	}

	//otherwise set transmit id variable to next in queue and dequeue that flow
	else
	{
		pthread_mutex_lock(&queue_mutex);
		trans_thread = nextQueue();
		pthread_mutex_unlock(&queue_mutex);
	}

	pthread_mutex_unlock(&transmit);
	pthread_exit(NULL);
}



/*****************************
*********MAIN THREAD**********
*****************************/
int main(int argc, char* argv[]){

	char buffer[256];
	FILE * myfile;
	int i = 0;
	


	if (argc < 2) {
		fprintf(stderr, "\nSorry the input you entered is something I can't accept!\nThe format I need to run is './MFS flow.txt'\n\n");
		return 0;
	} else {
	 	
		gettimeofday(&start, NULL);//get my start time
		myfile = fopen(argv[1],"r"); //open file

		if (myfile==NULL)
		{
			printf("Could not open file\n");
			return 0;
		}

		//get the number of flows and assign to size_flow variable
		fgets(buffer,256,myfile);
		int size_flow = atoi(buffer); 

		//create my queue and thread id's based off of # of flows
		struct flow holder[size_flow];
		queue = malloc(size_flow * sizeof(struct flow));
		pthread_t tid[size_flow];

		//use function stringtoflow() to create flows from text
   		 while (i<size_flow)
   		 {
        	fgets(buffer,256,myfile);
        	holder[i]=stringtoflow(buffer);
 			i++;
    	}

    	//close file
    	fclose(myfile);
    	
		//create a thread for each flow
		for(i = 0; i<size_flow; i++)
		{
			pthread_create(&(tid[i]), NULL, yourFlow, &holder[i]);
		
		}
		
		//wait for all threads to finish
		for (i = 0; i < size_flow; i++) {
		pthread_join(tid[i], NULL);
		}
		

    		
	}
	printf("All done! Thank you for using mfs.c authored by Ben Church (v00732962)\n");
	return 0;

}
