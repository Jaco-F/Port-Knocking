#include <linux/module.h>
#include <linux/slab.h>
#include "CircularBuffer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("circular-buffer");
MODULE_AUTHOR("4Mosfet");

// port sequence
static int port_sequence[8];

static struct timer_list my_timer;
static int index_timer;
static ArrayList *allowed_locale;
 
void cb_init(CircularBuffer *cb, int size) {
   index_timer = 0;
   // init the buffer
   cb->size  = size + 1;
   cb->start = 0;
   cb->end   = 0;
   cb->elems = kmalloc(size*sizeof(ElemType), GFP_KERNEL);
   
   // init the port sequence
   port_sequence[0]=80;
   port_sequence[1]=81;
   port_sequence[2]=82;
   port_sequence[3]=83;
   port_sequence[4]=84;
   port_sequence[5]=85;
   port_sequence[6]=86;
   port_sequence[7]=87;
}

void cbFree(CircularBuffer *cb) {
   kfree(cb->elems);  
}
 
int cbIsFull(CircularBuffer *cb) {
   return (cb->end + 1) % cb->size == cb->start;
}
 
int cbIsEmpty(CircularBuffer *cb) {
   return cb->end == cb->start; 
  
}
 
// Write an element, overwriting oldest element if buffer is full.
void cbWrite(CircularBuffer *cb, ElemType *elem, ArrayList *allowed) {
   allowed_locale = allowed;
   int i=0;
   int j=0;
   int address_index = 0;
   int next_port_index = 0;
   int next_port = 0;
   
   printk(KERN_INFO "struct IN packet info: src ip: %u, dest port: %u\n", elem->src_ip, elem->dest_port);
   printk(KERN_INFO "end : %d" ,cb->end);
   
   // search src_ip in the circular buffer
   address_index = -1;
   for (i = 0; i < cb->end; i++){
      if(elem->src_ip == cb->elems[i].src_ip){
         printk(KERN_INFO "gia' ricevuto da ip uguale");
         address_index = i;
      }
   }
    
   // if src_ip is in the circular buffer 
   if(address_index != -1){
      for(j=0;j < 8;j++) {
         if(cb->elems[address_index].dest_port == port_sequence[j]){ 
            next_port = port_sequence[j+1];
            next_port_index = j+1;
            break;
         }
      }
      // control if the port_sequence is right
      if(next_port == elem->dest_port){
         printk(KERN_INFO "vecchia destport %u" , cb->elems[address_index].dest_port);
         printk(KERN_INFO "avanziamo di porta!!");
         cb->elems[address_index] = *elem;
         printk(KERN_INFO "next_port_index %u" , next_port_index);
         if(next_port_index == 7){
            printk(KERN_INFO "RICONOSCIUTA");
            addRule(cb,address_index,allowed); 
            delete(address_index,cb);
	    /* setup your timer to call my_timer_callback */
            setup_timer(&my_timer, my_timer_callback, elem->src_ip);
            /* setup timer interval to 200 msecs */
            mod_timer(&my_timer, jiffies + msecs_to_jiffies(20000));
         }      
      }
      else {
         if(elem->dest_port == port_sequence[0]){
            cb->elems[address_index] = *elem;
            printk(KERN_INFO "ricomincia");
         }
         else {
            printk(KERN_INFO "eliminato");
            delete(address_index,cb);    
         }
       }
   }
   else {
      if(elem->dest_port == port_sequence[0]){
         printk(KERN_INFO "primo porta : %u , seq : %u\n", elem->dest_port,port_sequence[0]);
         printk(KERN_INFO "aggiungo ip");
         //add elments
         cb->elems[cb->end] = *elem;
         cb->end = (cb->end + 1) % cb->size;
         if (cb->end == cb->start)
            cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
      }
   }    
}

void delete(int posizione,CircularBuffer *cb) {
   int i;
   for (i = posizione; i<=cb->end; i++) {
      cb->elems[i]=cb->elems[i+1];
   }
   cb->end = (cb->end-1) % cb->size;
}

void addRule(CircularBuffer *cb,int address_index, ArrayList *allowed){
   printk(KERN_INFO "aggiungo regola prima fase n=%u , size = %u", allowed->n, allowed->size); 
   if(allowed->n!=allowed->size){
      printk(KERN_INFO "aggiungo regola");
      //inserisco regola in allowed.elems[n]
      allowed->elems[allowed->n].src_ip = cb->elems[address_index].src_ip;
      allowed->elems[allowed->n].dest_ip = cb->elems[address_index].dest_ip;
      allowed->elems[allowed->n].src_port = cb->elems[address_index].src_port;
      allowed->elems[allowed->n].dest_port = cb->elems[address_index].dest_port;
      allowed->n = allowed->n + 1;
   }
   else{
      printk(KERN_INFO "lista allowed piena");   
   }
  
  
}

void delete_rule(ArrayList *allowed){
   printk(KERN_INFO "Rimuovo regola!");
   int i;
   for (i = 0; i<allowed->n; i++) {
      allowed->elems[i]=allowed->elems[i+1];
   }
   allowed->n = allowed->n - 1;
}

void my_timer_callback( unsigned long data )
{
   printk(KERN_INFO "timer scaduto");
   delete_rule(allowed_locale);
}