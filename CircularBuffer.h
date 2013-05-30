#include <linux/module.h>
 
#include <linux/kernel.h>
 
#include <linux/proc_fs.h>
 
#include <linux/list.h>
 
#include <asm/uaccess.h>
 
#include <linux/udp.h>
 
#include <linux/tcp.h>
 
#include <linux/skbuff.h>
 
#include <linux/ip.h>

#include <net/ip.h>
 
#include <linux/netfilter.h>
 
#include <linux/netfilter_ipv4.h>

#include <linux/netfilter_ipv4/ip_tables.h>

typedef struct {unsigned int src_ip;
		unsigned int src_port;
		unsigned int dest_ip;
		unsigned int dest_port;
		} ElemType;

typedef struct {
  int size;
  int n;
  ElemType *elems;
} ArrayList;
		
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    ElemType   *elems;  /* vector of elements                   */
} CircularBuffer;


void delete(int posizione,CircularBuffer *cb);
void addRule(CircularBuffer *cb,int indiceIndirizzo, ArrayList *allowed );
void cbInit(CircularBuffer *cb, int size);
void cbFree(CircularBuffer *cb);
int cbIsFull(CircularBuffer *cb);
int cbIsEmpty(CircularBuffer *cb);
void cbWrite(CircularBuffer *cb, ElemType *elem, ArrayList *allowed);
void cbRead(CircularBuffer *cb, ElemType *elem);