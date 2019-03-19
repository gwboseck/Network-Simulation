#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
typedef struct node {
	void * data;
	struct node * next;
}node;

struct queue {
	/* TODO Phase 1 */
	int size;
	node * head;
	node * tail;
};

queue_t queue_create(void)
{
	/* TODO Phase 1 */
	queue_t newQ;
	newQ = (struct queue *) malloc(sizeof(struct queue));
	newQ->size = 0;
	return newQ;
}

int queue_destroy(queue_t queue)
{
	/* TODO Phase 1 */
	//Queue must be empty
	if(queue == NULL)
	{
		// printf("Destroy Error: Queue is NULL \n");
		return -1;
	}
	if(queue->size != 0)
	{
		// printf("Destroy Error: Queue not empty \n");
		return -1;
	}
	else
	{
		free(queue);
		queue = NULL;
		return 0;
	}
}

int queue_enqueue(queue_t queue, void *data)
{
	//create new node pointer
	if(queue != NULL)
	{
		// printf("queue isn't null \n");
	}
	if(queue == NULL || data == NULL)
	{
		// printf("Enqueue Error: Queue or Data is NULL \n");
		return -1;
	}
	node * temp;
	temp = (node *) malloc(sizeof(node));
	temp->data = data;
	if(queue->size == 0)
	{
		queue->head = temp;
		queue->tail = temp;
		// printf("Inserted %i as the first item \n", *(int*)queue->head->data);
	}// If the queue is empty assign both head and tail to new node
	else
	{
		queue->tail->next = temp;
		// Old tail gets new tail as next
		queue->tail = temp;
		// New tail becomes new tail
	}// If queue is not empty then attach new node to previous tail and change tail
	queue->size++;
	return 0;
	/* TODO Phase 1 */
}

int queue_dequeue(queue_t queue, void **data)
{
	/* TODO Phase 1 */
	if(data == NULL || queue == NULL)
	{
		// printf("Dequeue Error: Queue or Data is NULL \n");
		return -1;
	}
	if(queue->head == queue->tail)
	{
		node * temp = queue->head;
		*data = temp->data;
	}
	node * temp = queue->head;
	*data = temp->data; //Data is equal to prev head data
	//printf("Given Address: %p", *data);
	queue->head = temp->next; //Head of queue becomes next in line
	free(temp);

	// printf("New Address: %p \n", *data);
	// printf("Queue->head->data is %i \n", *(int*)*data);
	queue->size -= 1;
	return 0;
} 

int queue_delete(queue_t queue, void *data)
{
	/* TODO Phase 1 */
	if(queue == NULL || data == NULL || queue->size == 0)
	{
		// printf("Delete Error: Queue is NULL, Data is NULL, or QSize is 0 \n");
		return -1; //Return: -1 if @queue or @data are NULL
	}
	node * temp;
	int counter = 0;
	temp = queue->head;
	if(temp->data == data)
	{
		queue->head = temp->next;
		free(temp);
		temp = NULL;
		return 0;
	} //Data was at the head remove the head link head to the next in line;
	node * deleteN;
	while(temp->next->data != data)
	{
		if(temp->next->data == NULL)
		{
			// printf("Delete Error: No Match \n");
			return -1; //Return: -1 if @data was not found in queue
		} //There is no data in node end of queue
		counter++;
		temp = temp->next;
	} //While the data in the next node isn't equal to the data we move to next node
	if(queue->tail == temp->next)
	{
		deleteN = queue->tail;
		queue->tail = temp;
		free(deleteN);
		deleteN = NULL;
	} //Data found at the end of the queue change the tail
	else
	{
		deleteN = temp->next;
		temp->next->next = temp->next;
		free(deleteN);
		deleteN = NULL;
	} //Otherwise remove the link in the queue for this item
	return 0;
	


}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	/* TODO Phase 1 */
	int index = 0;
	node * temp = queue->head;
	

	while(index != queue->size)
	{
		func(queue,arg,temp->data);
		temp = temp->next;
		index++;
	}
	
	return 0; 
	// while index doesn't equal length
}

int queue_length(queue_t queue)
{
	/* TODO Phase 1 */
	if(queue == NULL)
	{
		// printf("Length Error: Queue is NULL \n");
		return -1;
	}
	return queue->size;
}