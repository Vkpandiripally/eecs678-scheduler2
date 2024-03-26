/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->top = NULL;
  q->tail = NULL;
  q->size = 0;
  q->comparer = comparer;
}


/**
  Insert the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  struct Node *new = malloc(sizeof(Node));
  new->data = ptr;
  new->next = NULL;
  new->prev = NULL;
  int index = 0;

  if(q->size == 0){
    //inserting when queue is empty
    q->top = new;
    q->tail = new;
    q->size++;
    return index;
  } else {
    Node *temp = q->top;

    while (temp != NULL){
      if(q->comparer(new->data, temp->data) < 0){
        if(temp -> prev == NULL){
          //inserting at the front of queue
          temp->prev = new;
          new->next = temp;
          q->top = new;
          q->size++;
          return index;
        } else {
          //inserting in the queue
          new->prev = temp->prev;
          temp->prev->next = new;
          temp->prev = new;
          new->next = temp;
          q->size++;
          return index;
        }
      }
      //moving to next node
      index++;
      temp = temp->next;
    }

    //if node is lowest priority
    new->prev = q->tail;
    q->tail->next = new;
    q->tail = new;
    q->size++;
    return index;
  }
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  Node *temp = q->top;
  if (q->size == 0){
    //checks if queue is empty
    return NULL;
  } else {
    //returns the data if queue has a top
    return temp->data;
  }
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  if (q->size == 0){
    //returns NULL if queue is empty
    return NULL;
  } else {
    //will set next element in queue as top and remove the previous top
    Node *temp = q->top;
    q->top = temp->next;
    q->top->prev = NULL;
    q->size--;
    return temp->data;
  }
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
  Node *temp = q->top;
  int currentIndex = 0;
  while (temp != 0){
    if(currentIndex == index){
      return temp->data;
    }
    currentIndex++;
    temp = temp->next;
  }
	return NULL;
}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  Node *temp = q->top;
  int remove = 0;
  while (temp != 0){
    if(temp->data == ptr){
      if (temp->prev != NULL){
        temp->prev->next = temp->next;
      } else {
        q->top = temp->next;
      }

      if(temp->next != NULL){
        temp->next->prev = temp->prev;
      } else {
        q->tail = temp->prev;
      }

      temp = temp->next;
      remove++;
      q->size--;
    } else {
      temp = temp->next;
    }
  }
	return remove;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
  if (q->size == 0 || q->size < index+1){
    return NULL;
  } else {
    int currentindex = 0;
    Node *temp = q->top;
    while(temp != NULL){
      if (currentindex == index){
        if (temp->prev != NULL){
          temp->prev->next = temp->next;
        } else {
          q->top = temp->next;
        }

        if(temp->next != NULL){
          temp->next->prev = temp->prev;
        } else {
          q->tail = temp->prev;
        }

        q->size--;
        return temp->data;
      }
      currentindex++;
      temp = temp->next;
    }
  }
	return NULL;
}


/**
  Return the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  while(q->size != 0){
    priqueue_remove_at(q,0);
  }
}
