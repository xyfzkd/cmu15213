/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"

#define MAXSTRING 1024
/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if (q==NULL)
        return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->length = 0;
    return q;
}

/* Free all storage used by queue */
//void q_free(queue_t *q)
//{
//    /* How about freeing the list elements and the strings? */
//    /* Free queue structure */
//    char *sp=malloc(sizeof(char)*257);
//    do{
//        q_remove_head(q,sp,256);
//    }while (q->head!=NULL);
//    free(sp);
//    free(q);
//    length = 0;
//}
void q_free(queue_t *q) {
    /* How about freeing the list elements and the strings? */
    if (q==NULL)
        return;
    list_ele_t *p = q->head, *next = NULL;
    for(;p!=NULL;){
        next = p->next;
        free(p->value);
        free(p);
        p = next;
    }
    q->length = 0;
    /* Free queue structure */
    free(q);


}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh;
    char *news;
    /* What should you do if the q is NULL? */
    if (q==NULL)
        return false;
    newh = malloc(sizeof(list_ele_t));
    if(newh==NULL) return false;
    news = malloc(MAXSTRING*sizeof(char));
    if(news==NULL) {
        free(newh);
        return false;
    }
    strcpy(news,s);
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    newh->value = news;
    newh->next = NULL;
    if(q->head==NULL)
        q->head = q->tail = newh;
    else{
        newh->next = q->head;
        q->head = newh;
    }
    q->length++;

    return true;
}

bool q_restart(queue_t *q, char *s){
    list_ele_t *p = q->head;
    char *str = malloc(128);
    if(*p->value==*s) {
        q_remove_head(q,str,128);
        q_insert_tail(q,str);
        return true;
    }

    for (; p->next != NULL; p = p->next) {
        if(*p->next->value==*s){
            list_ele_t *d = p->next;
            str = p->next->value;
            p->next = p->next->next;
            free(d->value);
            free(d);
            q->length--;
            if (p->next == NULL) q->tail=p;
            q_insert_tail(q,s);
            return true;
        }
    }
    return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    list_ele_t *newh;
    char *news;
    if (q==NULL)
        return false;
    newh = malloc(sizeof(list_ele_t));
    if(newh==NULL) return false;
    news = malloc(MAXSTRING*sizeof(char));
    if(news==NULL) {
        free(newh);
        return false;
    }
    strcpy(news,s);
    newh->value = news;
    newh->next = NULL;
    if(q->tail==NULL)
        q->tail = q->head = newh;
    else{
        q->tail->next = newh;
        q->tail = newh;
    }
    q->length++;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* You need to fix up this code. */
    if (q==NULL || q->head==NULL)
        return false;
    list_ele_t *p;
    p = q->head;
    q->head = q->head->next;
    if(q->head==NULL) q->tail=NULL;
    if (sp) {
        strncpy(sp, p->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    free(p->value);
    free(p);
    q->length--;
    return true;
}
//bool q_remove_head(queue_t *q, char *sp, size_t bufsize) {
//    /* You need to fix up this code. */
//
//    // If q is NULL or q is empty(q->head is NULL)
//    if (!q || !q->head) {
//        return false;
//    }
//
//    list_ele_t *oldhead = q->head, *newhead = q->head->next;
//
//    // If sp is not NULL, copy as much as possible into sp
//    if (sp) {
//        int i;
//        for (i = 0; i < bufsize - 1 && *(oldhead->value + i) != '\0'; ++i) {
//            *(sp + i) = *(oldhead->value + i);
//        }
//        *(sp + i) = '\0';
//    }
//
//    // free space used by oldhead
//    free(oldhead->value);
//    free(oldhead);
//
//    // update q->head
//    q->head = newhead;
//
//    // update q->size
//    length--;
//
//    return true;
//}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if (q==NULL || q->head==NULL)
        return 0;
    return q->length;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if(q->length<2) return;
    list_ele_t *r,*s,*tmp;
    list_ele_t *head = q->head;

    for (r=q->head,s=q->head->next;s!=NULL;){
        tmp = r;
        r = s;
        s = s->next;
        r->next = tmp;

    }
    q->head = q->tail;
    q->tail = head;
    q->tail->next = NULL;

    /* You need to write the code for this function */
}


