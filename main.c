#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "queue.h"
#define MAXBUFFER 300
#define ARRIVAL 0
#define DEPARTURE 1
#define TOKENBUSY 2
long totalByteTrans;
double timer;
double utilization;
double areaSum;
double transDelay;
double queueDelay;
double propDelay = 0.00001; //time it takes to move token to next host
double transRate = 100000000; // 100 million bytes a second;
double transToken = 0.00000003; // tokens are special 3 byte frames, so time to transToken is 3 bytes by 100 Mbps
int totalFrames;
int hostSize;
int tokenBusy;
int length;
int arrivals;
int departures;
int counter;
int packetDropNum;
int curtok;
double arrivRate;
double interLength;
// event stores the time it completes the type of event and the host we are acting on as well as prev next nodes
typedef struct event {
	double time; // time since start of proc
	int type; // 0 for arrival 1 for token free and 2 for token busy (frame)
	int host; // Host causing arrial or departure
	struct event * next;
	struct event * prev;
}event;

//packet stores a time and id and time, only really use the size right now but data analysis may need the other
typedef struct packet {
	int size;
	int id;
	double time;
}packet;

//hosts store a token bool and a queue
struct host {
	queue_t pQueue;
};
struct host ** hosts; // list of hosts that are initiated at the beginning
event * listHead = NULL;
event * listTail = NULL;
//Given function by prof for finding intertime
double interTime(double rate) 
{ 
	double u;
	u = drand48();
	return ((-1/rate)*log(1-u));
}

//Find a random size between 64 and 1518 bytes 
int randomSize()
{
	int r = rand() % 1454;// distributed from 0 - 1454
	r += 64; //increment size by base 64 bytes
	return r;
}

//Find a random host between the host size
int randomHost()
{
	int r = rand() % hostSize;
	return r;
}

//Make a packet with a random size
packet * makePacket(double time)
{
	packet * temp = (packet *) malloc(sizeof(packet));
	temp->size = randomSize();
	temp->id = counter;
	temp->time = time;
	return temp;
}

//Make an event with completion time, type, and host
event * makeEvent(double time, int type, int host)
{
	event * temp = (event *) malloc(sizeof(event));
	temp->time = time;
	temp->type = type;
	temp->host = host;
	temp->next = NULL;
	temp->prev = NULL;
	return temp;
} 
void printList()
{
	event * temp = listHead;
	while(temp != NULL)
	{
		printf("temp time %lf is and event is %d, and host is %d. \n", temp->time, temp->type, temp->host);
		temp = temp->prev;
	}
	temp = NULL;
	free(temp);
}
event * getHead()
{
	
	event * temp = listHead;
	if(listHead == listTail)
	{
		listHead = NULL;
		listTail = listHead;
		return temp;
	}
	listHead = listHead->prev;
	listHead->next = NULL;
	return temp;
}
// divide the size of all packets by 100 Mbps
double findFT(double size)
{
	return size/transRate;
}
void insertEvent(event * given)
{
	
	if(given->type == ARRIVAL)
		arrivals++;
	//printf("Given event time is: %lf \n", given->time);
	// base case nothing in the list
	if(given == NULL)
	{
		//printf("Given is null error \n");
		return;
	}
	if(listHead == NULL && listTail == NULL)
	{
		//printf("Inserting first event\n");
		listHead = (event *) malloc(sizeof(event));
		listHead = given;
		listTail = given;
		return;
	}
	// second case, one thing in the list insert before or after
	if(listHead == listTail)
	{
		//printf("Inserting second event\n");
		if(given->time >= listHead->time)
		{
			listTail = NULL;
			listTail = (event *) malloc(sizeof(event));
			listTail = given;
			listHead->prev = given;
			given->next = listHead;
			return;
		} // insert after
		else
		{
			listHead = NULL;
			listHead = (event *) malloc(sizeof(event));
			listHead = given;
			listTail->next = given;
			given->prev = listTail;
			return;
		} // insert before
	}
	// general case
	//printf("Inserting general event\n");
	event * temp = listTail;
	while(temp != NULL)
	{
		
		// given event is later than current temp, closer to tail
		if(given->time >= temp->time)
		{
			//temp is listTail
			if(temp == listTail)
			{
				listTail = given;
				temp->prev = given;
				given->next = temp; 
				return;
			}
			// have to insert given in between to doubly linked nodes
			// temp->prev->next was temp now must be given
			given->prev = temp->prev;
			given->prev->next = given;
			// temp->prev is now given
			temp->prev = given;
			// given next points to
			given->next = temp;
			return;
		} //Given event belongs after temp
		if(temp == listHead)
		{
			temp->next = given;
			given->prev = temp;
			listHead = given;
			//printf("Given event was earlier than head \n");
			return;
		} // reached the beginning of the list, given is head

		temp = temp->next;	//Move from tail to head
		//printf("Reached the end of insert\n");
	} 

}
void init()
{
	srand(time(NULL));
	counter = 0;
	timer = 0;
	length = 0;
	utilization = 0;
	printf("What is the number of hosts? \n -->");
	scanf("%d",&hostSize);
	if(hostSize < 2)
	{
		printf("Cannot create a token ring with 1 host \n");
		return;
	}
	printf("What is the arrival rate? \n -->");
	scanf("%lf",&arrivRate);
	printf("Arrival Rate is: %lf \n", arrivRate);

	/* Allocate for the list of hosts */
	hosts = (struct host **) malloc(sizeof(struct host*)*hostSize);

	/* Initialize all the hosts and assign the token to the first host */
	for(int i = 0; i<hostSize; i++)
	{
		hosts[i] = (struct host *) malloc(sizeof(struct host));
		hosts[i]->pQueue = queue_create();
		event * start = makeEvent(interTime(arrivRate), ARRIVAL, randomHost());
		insertEvent(start);
	}
	// Create the first arrival and first token movement
	printList();

	return;
}

// Simply set the current time insert the packet into the specified queue and create a new arrival for GEL 
void proccessArrival(event * node)
{
	// set the time to the current time of event
	timer = node->time;
	event * newArrival = makeEvent(interTime(arrivRate)+timer, ARRIVAL, randomHost());
	packet * newPacket = makePacket(node->time);
	//printf("Packet address %p \n", (void*)newPacket);
	//printf("New packet time is: %lf size is: %d\n", newPacket->time, newPacket->size);
	counter++;
	//printf("Inserting new arrival event \n");
	queue_enqueue(hosts[node->host]->pQueue,newPacket); 
	insertEvent(newArrival);
	return;

}


int main(void)
{
	totalFrames = 0;
	arrivals = 0;
	departures = 0;
	totalByteTrans = 0;
	transDelay = 0;
	queueDelay = 0;
	tokenBusy = 0;
	init();

	for (int i = 0; i < 100000; i++)
	{
		
		event * current;
		current = getHead();
		if(current->type == 1)
			printf("Current->type is %d \n", current->type);
		if(current->type == ARRIVAL)
		{
			// Process a new arrival
			proccessArrival(current);
			// If currently processing a frame, free arrival event and continue
			if(tokenBusy == 1)
			{
				free(current);
				continue;
			}
			// Token must be free and there is a packet in one of these queues from processArrival
			// Loop through hosts and find a non empty queue
			for(int j = 0; j < hostSize; j++)
			{
				if(queue_length(hosts[j]->pQueue) != 0)
				{
					//printf("Token busy has been set\n");
					tokenBusy = 1;
					totalFrames++;
					double frameTime;
					double frameSize;
					// empty queue and determine time needed to transfer data by finding total size
					departures += queue_length(hosts[current->host]->pQueue);
					while(queue_length(hosts[current->host]->pQueue) != 0)
					{
						void * temp;
						queue_dequeue(hosts[current->host]->pQueue, &temp);
						packet * data = (packet *)temp;
						double delay = timer - data->time;
						//printf("Timer is: %lf, packet arrived: %lf, size is: %d \n", timer, data->time, data->size);
						//For queue delay
						queueDelay += delay;
						frameSize += data->size;
						free(temp);
					}
					
					frameTime = findFT(frameSize);
					frameTime += propDelay;
					frameTime = frameTime * hostSize;
					//For transmission delay
					transDelay += frameTime;
					// For throughput
					totalByteTrans += frameSize;
					if(current->host + 1 == hostSize)
					{
						event * newFrame = makeEvent(timer + frameTime, DEPARTURE, 0);
						insertEvent(newFrame);
						break;
					}
					//This isn't last host index so keep incrementing
					else
					{
						event * newFrame = makeEvent(timer + frameTime, DEPARTURE, current->host + 1);
						insertEvent(newFrame);
						break;
					}
				}
				timer += propDelay + transToken;
			}
		}
		if(current->type == DEPARTURE)
		{
			timer = current->time;
			tokenBusy = 0;
			//printf("Token busy has be reset \n");
			// Token must be free and there is a packet in one of these queues from processArrival
			// Loop through hosts and find a non empty queue
			for(int j = 0; j < hostSize; j++)
			{
				if(queue_length(hosts[j]->pQueue) != 0)
				{
					tokenBusy = 1;
					totalFrames++;
					//printf("Token busy has been set\n");
					double frameTime;
					double frameSize;
					// empty queue and determine time needed to transfer data by finding total size
					departures += queue_length(hosts[current->host]->pQueue);
					while(queue_length(hosts[current->host]->pQueue) != 0)
					{
						void * temp;
						queue_dequeue(hosts[current->host]->pQueue, &temp);
						packet * data = (packet *)temp;
						double delay = timer - data->time;
						//printf("Timer is: %lf, packet arrived: %lf, size is: %d \n", timer, data->time, data->size);
						queueDelay += delay;
						frameSize += data->size;
						free(temp);
					}
					frameTime = findFT(frameSize);
					frameTime += propDelay;
					frameTime = frameTime * hostSize;
					// For throughput
					transDelay += frameTime;
					totalByteTrans += frameSize;
					if(current->host + 1 == hostSize)
					{
						event * newFrame = makeEvent(timer + frameTime, DEPARTURE, 0);
						insertEvent(newFrame);
						break;
					}
					//This isn't last host index so keep incrementing
					else
					{
						event * newFrame = makeEvent(timer + frameTime, DEPARTURE, current->host + 1);
						insertEvent(newFrame);
						break;
					}
				}
				timer += propDelay + transToken;
			}
		}
		free(current);

	}
	// have to free the GEL
	printf("Time is %lf", timer);
	printf("A total of %d arrivals and %d departures the total # Frames processed is %d \n", arrivals, departures, totalFrames);
	printf("Total throughput is %lf, and average packet delay is %lf \n", totalByteTrans/timer, (queueDelay+transDelay)/timer);
	for(int i = 0; i < hostSize; i++)
	{
		queue_destroy(hosts[i]->pQueue);
	}
	//Output Statistics
	return 1;
}