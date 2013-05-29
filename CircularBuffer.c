#include <linux/module.h>
#include <linux/slab.h>
#include "CircularBuffer.h"

MODULE_LICENSE("GPL");
 
MODULE_DESCRIPTION("circular-buffer");
 
MODULE_AUTHOR("4Mosfet");
 
/* Opaque buffer element type.  This would be defined by the application. */


/* Circular buffer object */

 
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
    int i=0;
    int j=0;
    int trovato=0;
    int indiceIndirizzo = 0;
    int indicePortaSuccessiva = 0;
    int PortaSuccessiva = 0;
    int riconosciuta = 0;
    printk(KERN_INFO "struct IN packet info: src ip: %u, dest port: %u\n", elem->src_ip, elem->dest_port);
    printk(KERN_INFO "end : %d" ,cb->end);
    if(cb->end == 0) {
      printk(KERN_INFO "primo porta : %u , seq : %u\n", elem->dest_port,PortSequence[0]);
      if (elem->dest_port == PortSequence[0])	{		//Se è la prima allora sovrascrivo altrimenti cancello
	printk(KERN_INFO "primo inserito");
	cb->elems[cb->end] = *elem;				//Aggiungo un elemento in coda al vettore
	cb->end = (cb->end + 1) % cb->size;
	if (cb->end == cb->start)
	  cb->start = (cb->start + 1) % cb->size;
      }	 
    }
    else {
      
      //cerco nel buffer se esiste indirizzo ip
      indiceIndirizzo = -1; //se rimane così vuol dire che non è stato trovato
      for (i = 0; i <= cb->end; i++)
      {
	if(elem->src_ip == cb->elems[i].src_ip)
	{
	  printk(KERN_INFO "gia' ricevuto da ip uguale");
	  indiceIndirizzo = i;
	}
      }
      
      if(indiceIndirizzo != -1) //indirizzo trovato
      {
	for(j=0;j < 8;j++) {
	  if(cb->elems[indiceIndirizzo].dest_port == PortSequence[j]){ 
	    PortaSuccessiva = PortSequence[j+1];
	    indicePortaSuccessiva = j+1;
	    break;
	   }
	 }
	 //guardo se porta successiva corrisponde a quella ricevuta
	 if(PortaSuccessiva == elem->dest_port){
	   printk(KERN_INFO "vecchia destport %u" , cb->elems[indiceIndirizzo].dest_port);
	   printk(KERN_INFO "avanziamo di porta!!");
	   cb->elems[indiceIndirizzo] = *elem;
	   printk(KERN_INFO "indicePortaSuccessiva %u" , indicePortaSuccessiva);
	   if(indicePortaSuccessiva == 7){
	     printk(KERN_INFO "RICONOSCIUTA");
	     delete(indiceIndirizzo,cb);
	   }
	      
	 }
	 else {
	   if(elem->dest_port == PortSequence[0]){
	     cb->elems[indiceIndirizzo] = *elem;
	     printk(KERN_INFO "ricomincia");
	   }
	   else {
	     printk(KERN_INFO "eliminato");
	     delete(indiceIndirizzo,cb);  
	   }
         }
       }
       else {
	 printk(KERN_INFO "aggiungo ip");
	 cb->elems[cb->end] = *elem;				//Aggiungo un elemento in coda al vettore
	 cb->end = (cb->end + 1) % cb->size;
	 if (cb->end == cb->start)
	   cb->start = (cb->start + 1) % cb->size; }/* full, overwrite */
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