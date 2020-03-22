/* queue.c */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* structure to hold list of addresses */

struct qnode {
    unsigned int address;
	struct qnode *next;
};

struct qnode *addq(struct qnode *, unsigned int);
struct qnode *delq(struct qnode *);
void qprint(struct qnode *);

/* allocates enough memory to store 1 q node */

struct qnode *qalloc(void) {
        return (struct qnode *) malloc(sizeof(struct qnode));
}

/* returns pointer to duplicate of s */

struct qnode *qdup(struct qnode *s)
{
        struct qnode *temp, *p;
        p = NULL;
        temp = s;
        while (temp != NULL) {
                p = addq(p,temp->address);
                temp = temp->next;
        }
        return p;
}

/* adds q node to tail of p - use form 'p = addq(p,w,t)' */

struct qnode *addq(struct qnode *p, unsigned int address)
{
     
        if (p == NULL) {
                p = qalloc();
		if (p==NULL)
                {
                    fprintf (stderr, "out of memory for enqueue operation");
                }
                p->address = address;
                p->next = NULL;
        }
        else
          p->next = addq(p->next, address);
        return p;
        
}

/* remove node from head of p - use form 'p = delq(p)' */
/* frees memory previously used by node deleted            */

struct qnode *delq(struct qnode *p)
{
        struct qnode *t;
        
        t = p;
        
        if (p != NULL) {
          t = p->next;
          free(p);
        }
        
        return t;
} 

/* deletes all nodes from queue pointed to by p */

void clearq(struct qnode *p)
{
        while (p != NULL)
                p = delq(p);
}
        
/* prints all bursts in a q - used for debugging */

void qprint(struct qnode *p)
{
        if (p != NULL) {
                fprintf(stderr,"Address: %0.4X\n",p->address);
                qprint(p->next);
        }
}

/*
main()
{
    struct qnode *addresses;

    addresses = NULL;

    addresses = addq(addresses,0xfeac);
    addresses = addq(addresses,0xfbba);
    qprint(addresses);
    printf("--\n");
    printf("%0.4X\n",addresses->address);
    addresses = delq(addresses);
    printf("%0.4x\n",addresses->address);
}
*/
