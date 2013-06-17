#include "CircularBuffer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("circular_buffer");
MODULE_AUTHOR("4Mosfet-tieri");

// port sequence
static int port_sequence[10];
static int num_ports;

static struct timer_list my_timer;
static int index_timer;
static array_list *local_allowed;
static int local_timer;
 
void cb_init(circular_buffer *cb, int size) {
   int i = 0; 
   index_timer = 0;
   // init the buffer
   cb->size  = size + 1;
   cb->start = 0;
   cb->end   = 0;
   cb->elems = kmalloc(size*sizeof(elem_type), GFP_KERNEL);
   
   // init default port sequence (80,81,82,83,84,85,86,87)
   for (i = 0; i < 8; i++) {
      port_sequence[i]= 80 + i;     
   }
   num_ports = i;
}

void cb_free(circular_buffer *cb) {
   kfree(cb->elems);  
}
 
// Write an element, overwriting oldest element if buffer is full.
void cb_write(circular_buffer *cb, elem_type *elem, array_list *allowed) { 
   int i = 0;
   int j = 0;
   int address_index = 0;
   int next_port_index = 0;
   int next_port = 0;
   
   local_allowed = allowed;
   
   // search for src_ip in circular buffer
   address_index = -1;
   for (i = 0; i < cb->end; i++){
      if(elem->src_ip == cb->elems[i].src_ip){
         address_index = i;
      }
   }
    
   // if src_ip is in circular buffer 
   if(address_index != -1){
      for(j = 0; j < 8; j++) {
         if(cb->elems[address_index].dest_port == port_sequence[j]){ 
            next_port = port_sequence[j+1];
            next_port_index = j+1;
            break;
         }
      }
      // control if port sequence is right
      if(next_port == elem->dest_port){
         cb->elems[address_index] = *elem;
         if(next_port_index == num_ports-1){
            add_rule(cb,address_index,allowed); 
            delete(address_index,cb);
	    // setup timer to call my_timer_callback once expired
            setup_timer(&my_timer, my_timer_callback, elem->src_ip);
            // setup timer interval to 20 secs
            mod_timer(&my_timer, jiffies + msecs_to_jiffies(local_timer));
         }      
      }
      else {
	 // check if dest_port is the first port of the sequence
         if(elem->dest_port == port_sequence[0]){
            cb->elems[address_index] = *elem;
         }
         else {
            delete(address_index,cb);    
         }
       }
   }
   //if src_ip isn't in the circular buffer
   else {
      if(elem->dest_port == port_sequence[0]){
         // add new element
         cb->elems[cb->end] = *elem;
         cb->end = (cb->end + 1) % cb->size;
         if (cb->end == cb->start)
            cb->start = (cb->start + 1) % cb->size; //full, overwrite
      }
   }    
}

void delete(int posizione,circular_buffer *cb) {
   int i = 0;
   for (i = posizione; i<=cb->end; i++) {
      cb->elems[i]=cb->elems[i+1];
   }
   cb->end = (cb->end - 1) % cb->size;
}

void add_rule(circular_buffer *cb,int address_index, array_list *allowed){
   if(allowed->n!=allowed->size){
      // insert a rule in allowed.elem[n]
      allowed->elems[allowed->n].src_ip = cb->elems[address_index].src_ip;
      allowed->elems[allowed->n].dest_ip = cb->elems[address_index].dest_ip;
      allowed->elems[allowed->n].src_port = cb->elems[address_index].src_port;
      allowed->elems[allowed->n].dest_port = cb->elems[address_index].dest_port;
      allowed->n = allowed->n + 1;
      printk(KERN_DEBUG "rule added");
   }
   else{
      printk(KERN_DEBUG "allowed list full");   
   }
  
  
}

//delete a rule from allowed list
void delete_rule(array_list *allowed){
   int i = 0;
   for (i = 0; i<allowed->n; i++) {
      allowed->elems[i]=allowed->elems[i+1];
   }
   allowed->n = allowed->n - 1;
}

// function called when the timer expires
void my_timer_callback( unsigned long data )
{
   printk(KERN_DEBUG "timer expired");
   delete_rule(local_allowed);
}

//initialize port sequence from user input
void init_port_sequence(int *ports, int count){
  int i = 0;
  num_ports = count;
  for(i = 0; i < count; i++){
    port_sequence[i] = ports[i];
  }   
}

//initialize time for the timer
void init_rule_timer(int time){
   local_timer = time;  
}