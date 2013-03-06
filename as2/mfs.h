/*mfs.h for csc 360 as 2 by Ben Church v00732962*/


//global variables
int trans_thread = 0;
int size_queue=0;
struct timeval start;


//mutex's for que and transmit
pthread_mutex_t transmit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t trans_condvar = PTHREAD_COND_INITIALIZER;


//structure of my flow
struct flow {
   int id;
   int arrival_time;
   int trans_time;
   int priority;
};

//my queue is an array of flows
struct flow* queue;

float interval();
int nextQueue();
void addtoQueue (struct flow curFlow);
struct flow stringtoflow(char* text);
void *yourFlow(void* arg);

