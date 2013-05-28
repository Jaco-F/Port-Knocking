#include <linux/module.h>
#include <linux/slab.h>
#include "CircularBuffer.h"

MODULE_LICENSE("GPL");
 
MODULE_DESCRIPTION("circular-buffer");
 
MODULE_AUTHOR("4Mosfet");
 
/* Opaque buffer element type.  This would be defined by the application. */


/* Circular buffer object */
static int PortSequence[8];
 
void cbInit(CircularBuffer *cb, int size) {
    cb->size  = size + 1; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->elems = kmalloc(size*sizeof(ElemType), GFP_KERNEL);
    PortSequence[0]=80;
    PortSequence[1]=81;
    PortSequence[2]=82;
    PortSequence[3]=83;
    PortSequence[4]=84;
    PortSequence[5]=85;
    PortSequence[6]=86;
    PortSequence[7]=87;
}

void delete(int posizione,CircularBuffer *cb);

void cbFree(CircularBuffer *cb) {
    kfree(cb->elems); /* OK if null */ }
 
int cbIsFull(CircularBuffer *cb) {
    return (cb->end + 1) % cb->size == cb->start; }
 
int cbIsEmpty(CircularBuffer *cb) {
    return cb->end == cb->start; }
 
/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
void cbWrite(CircularBuffer *cb, ElemType *elem) {
    //printk(KERN_INFO "struct IN packet info: src ip: %u, src port: %u; dest ip: %u, dest port: %u\n", elem->src_ip, elem->src_port, elem->dest_ip, elem->dest_port);
    int i=0;
    int j=0;
    printk(KERN_INFO "struct IN packet info: src ip: %u, dest port: %u\n", elem->src_ip, elem->dest_port);
    printk(KERN_INFO "end : %d" ,cb->end);
    if(cb->end == 0) {
      printk(KERN_INFO "primo porta : %u , seq : %u\n", elem->dest_port,PortSequence[0]);
      if (elem->dest_port == PortSequence[0])	{		//Se è la prima allora sovrascrivo altrimenti cancello
	printk(KERN_INFO "primo inserito");
	cb->elems[cb->end] = *elem;				//Aggiungo un elemento in coda al vettore
	cb->end = (cb->end + 1) % cb->size;
	printk(KERN_INFO "end : %d" ,cb->end);
	if (cb->end == cb->start)
	  cb->start = (cb->start + 1) % cb->size;
      }	 
    }
    else {
      for(i=0; i <=cb->end ; i++) {          				//Scorre il buffer
	if(elem->src_ip == cb->elems[i].src_ip) {			//Controlla se ha gia ricevuto un pacchetto con lo stesso IP
	  printk(KERN_INFO "gia' ricevuto da ip uguale");
	  for(j=0;j < 8;j++) {						//Scorre il vettore di PortSequence
	    if((cb->elems[i].dest_port == PortSequence[j])&&(PortSequence[j+1] == elem->dest_port)) {//Controlla a che porta è arrivato in precedenza e se la porta corrisponde a quella che mi aspetto sostituisco
	      printk(KERN_INFO "vecchia destport %u" , cb->elems[i].dest_port);
	      cb->elems[i] = *elem;
	      printk(KERN_INFO "avanziamo di porta!!");
	      printk(KERN_INFO "end : %d" ,cb->end);
	      if(j+1 == 7) {
		printk(KERN_INFO "RICONOSCIUTA");
		delete(i,cb);
	      }
	    }
	    else { 
	      if((cb->elems[i].dest_port == PortSequence[j])&&(PortSequence[j+1] != elem->dest_port)) {						//Se la porta non corrisponde a nessuna di quelle del vettore
		if (elem->dest_port == PortSequence[0])		//Se è la prima allora sovrascrivo altrimenti cancello
		  cb->elems[i] = *elem;			
		else {
		  printk(KERN_INFO "eliminato");
		  delete(i,cb);
		}
	      }
	    }
	    printk(KERN_INFO "FUORI!!!");
	    break;
	  }
	}
	else {							//Se non ho ricevuto un pacchetto con lo stesso IP
	  cb->elems[cb->end] = *elem;				//Aggiungo un elemento in coda al vettore
	  cb->end = (cb->end + 1) % cb->size;
	  if (cb->end == cb->start)
	    cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
	}
      }
    }
}
 
/* Read oldest element. App must ensure !cbIsEmpty() first. */
void cbRead(CircularBuffer *cb, ElemType *elem) {
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
}
void delete(int posizione,CircularBuffer *cb) {
    int i;
    for (i = posizione; i<=cb->end; i++) {
      cb->elems[i]=cb->elems[i+1];
    }
    cb->end = (cb->end-1) % cb->size;
}