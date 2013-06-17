#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/timer.h>

typedef struct {
   unsigned int src_ip;
   unsigned int src_port;
   unsigned int dest_ip;
   unsigned int dest_port;
} elem_type;

typedef struct {
   int size;         // maximum number of elements
   int n;            // index at which to write new element        
   elem_type *elems;  // vector of elements
} array_list;
		
typedef struct {
   int size;         // maximum number of elements           
   int start;        // index of oldest element              
   int end;          // index at which to write new element  
   elem_type *elems;  // vector of elements                   
} circular_buffer;

// prototype
void delete(int posizione,circular_buffer *cb);
void add_rule(circular_buffer *cb,int indiceIndirizzo, array_list *allowed );
void cb_init(circular_buffer *cb, int size);
void cb_free(circular_buffer *cb);
void cb_write(circular_buffer *cb, elem_type *elem, array_list *allowed);
void my_timer_callback( unsigned long data );
void init_port_sequence(int *ports, int count);
