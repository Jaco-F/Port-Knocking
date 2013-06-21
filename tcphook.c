#include "CircularBuffer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("port_knocking");
MODULE_AUTHOR("4Mosfet-tieri");

static circular_buffer cb;
static circular_buffer *bufferPointer;
static elem_type e;
static array_list allowed;
static int port_dest = 10090; //set default
static int port_seq[10];
static int rule_timer = 20000; //set default
static int count; 

static int error = 0;

//the structure used to register the function
static struct nf_hook_ops nfho;
 
//the hook function itself: registered for filtering incoming packets
unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *)) {
   int i = 0;
  
   struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
   struct tcphdr *tcp_header;

   unsigned int src_port = 0; 
   unsigned int dest_port = 0;
 
   //get src and dest ip addresses
   unsigned int src_ip= ip_header->saddr;
   unsigned int dest_ip= ip_header->daddr;
 
   // control if the packet is TCP
   if (ip_header->protocol == 6) {
      // save tcp header
      tcp_header = (struct tcphdr *)(skb_network_header(skb) + ip_hdrlen(skb));
         
	 // if first TCP packet
         if (bufferPointer == NULL) { 
	    // init circular buffer
	    cb_init(&cb,10);
	    bufferPointer = &cb;
	    // init allowed list 
	    allowed.n = 0;
	    allowed.size = 10;
	    allowed.start = 0;
	    allowed.elems = kmalloc(allowed.size*sizeof(elem_type), GFP_KERNEL);
	    
	    printk(KERN_DEBUG "packet buffer and allowed list initialized");
	    
	    //init port_sequence
	    if((count > 1)&&(count < 10)){
	       for(i = 0;i < count ;i++){
                  if(port_seq[i] < 0)
                     error = 1;
	       }
	    }
	    else{
	       error = 1;
	    }
	    
	    if(port_dest <= 0){
	       port_dest = 10090;
	       error = 2;
	    }
	    
	    if(rule_timer <= 0){
	      rule_timer = 20000;
	      error = 3;
	    }
	    //control error in the input sequence
            if(error == 1){
               printk(KERN_DEBUG "Param_error (port sequence)-> set Default");   
	    }
	    else{
	       init_port_sequence(port_seq, count);
	    }
	    
	    if(error == 2){
	       printk(KERN_DEBUG "Param_error (dest_port)-> set Default");      
	    }
	    
	    if(error == 3){
	       printk(KERN_DEBUG "Param_error (rule_timer)-> set Default");      
	    }
	
	    init_rule_timer(rule_timer);  
	  
	    
         }   
          
      // get src and dest port number 
      src_port = (unsigned int)ntohs(tcp_header->source);
      dest_port = (unsigned int)ntohs(tcp_header->dest);
      
      // save the information in the structure
      e.src_ip = src_ip;
      e.src_port = src_port;
      e.dest_ip = dest_ip;
      e.dest_port = dest_port; 
      
      // control if dest_port is the target port
      if(dest_port == port_dest){
         int i;
	 
	 // control if src_ip is in allowed
	 i = allowed.start;
	 while(i != allowed.n){
	    if(allowed.elems[i].src_ip == src_ip){
	       // src_ip is in allowed -> accept the packet
	       return NF_ACCEPT;
	    }
	    i = (i+1)%allowed.size;
	 }
	 // src_ip isn't in allowed -> drop the packet
	 return NF_DROP;
      }
      
      // if syn flag is set
      if(tcp_header->syn == 1) { 
         cb_write(bufferPointer,&e, &allowed);
      }
 
   }
		
   
   return NF_ACCEPT;

 }

module_param_array(port_seq, int, &count, 0);
module_param(port_dest, int, 0);
module_param(rule_timer, int, 0);

 
int init_module() {
   printk(KERN_INFO "initialize kernel module\n");
 
   //Fill in the hook structure for incoming packet hook
   nfho.hook = hook_func_in;
   nfho.hooknum = NF_INET_LOCAL_IN;
   nfho.pf = PF_INET;
   nfho.priority = NF_IP_PRI_FIRST;
   // Register the hook
   nf_register_hook(&nfho);         
 
   return 0;
 
}

void cleanup_module() {
   nf_unregister_hook(&nfho);
   if(bufferPointer != NULL)
      cb_free(&cb);
 
   printk(KERN_INFO "kernel module unloaded.\n");
 
}