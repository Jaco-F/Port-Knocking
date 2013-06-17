
#include "CircularBuffer.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("linux-simple-firewall");
MODULE_AUTHOR("4Mosfet");

static circular_buffer cb;
static circular_buffer *bufferPointer;
static elem_type e;
static array_list allowed;
static int port_dest = 10090; //set default
static int port_init[10];
static int count; 

static int error = 0;

//the structure used to register the function
static struct nf_hook_ops nfho;
 
//the hook function itself: registered for filtering incoming packets
 unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, 
 
        const struct net_device *in, const struct net_device *out,
 
        int (*okfn)(struct sk_buff *)) {
   int i = 0;
 
   //get src address, src netmask, src port, dest ip, dest netmask, dest port, protocol
 
   struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
   struct tcphdr *tcp_header;

   unsigned int src_port = 0; 
   unsigned int dest_port = 0;
 
   //get src and dest ip addresses
   unsigned int src_ip= ip_header->saddr;
   unsigned int dest_ip= ip_header->daddr;
   
   /*char src_ip[16];
   snprintf(src_ip, 16, "%pI4", &ip_header->saddr);

   char dest_ip[16];
   snprintf(dest_ip, 16, "%pI4", &ip_header->daddr);*/
 
   // control if the packet is TCP
   if (ip_header->protocol == 6) {
      // save tcp header
      tcp_header = (struct tcphdr *)(skb_network_header(skb) + ip_hdrlen(skb));
         
	 // if is the first packet hooked
         if (bufferPointer == NULL) { 
	    // init the circular buffer
	    printk(KERN_INFO "inizializzo buffer");
	    cb_init(&cb,10);
	    bufferPointer = &cb;
	    
	    // init the list allowed 
	    printk(KERN_INFO "inizializzo allowed");
	    allowed.n = 0;
	    allowed.size = 10;
	    allowed.elems = kmalloc(allowed.size*sizeof(elem_type), GFP_KERNEL);
	    
	    //init port_sequence
	    if((count > 1)&&(count<10)){
	       for(i = 0;i < count ;i++){
                  if(port_init[i] < 0)
                     error = 1;
	       }
	    }
	    else{
	       error = 1;
	    }
	    
	    if(port_dest <=0){
	       port_dest = 10090;
	       error = 2;
	    }
	    //control error
            if(error == 1){
               printk(KERN_INFO "Param_error (port sequence)-> set Default");   
	    }
	    else{
	       init_port_sequence(port_init, count);
	    }
	    
	    if(error == 2){
	       printk(KERN_INFO "Param_error (dest_port)-> set Default");      
	    }
	    
         }   
          
      // get src and dest port number 
      src_port = (unsigned int)ntohs(tcp_header->source);
      dest_port = (unsigned int)ntohs(tcp_header->dest);
      
      // save the information in the structure
      e.src_ip = src_ip;
      e.src_port = src_port;
      e.dest_ip = dest_ip;
      e.dest_port = dest_port; 
      //printk(KERN_INFO "dest_port = %u" , dest_port);
      //printk(KERN_INFO "src_port = %u" , src_port);
      
      // control if dest_port is the target port
      if(dest_port == port_dest){
         int i;
	 printk(KERN_INFO "entrato nell if!");
	 
	 // control if src_ip is in allowed
	 for(i = 0; i < allowed.n;i++){
	    printk(KERN_INFO "entrato nel for!");
	    printk(KERN_INFO "allowed.elems[i].src_ip %u , src_ip %u, allowed.n %u", allowed.elems[i].src_ip, src_ip, allowed.n);
	    if(allowed.elems[i].src_ip == src_ip){
	       // src_ip is in allowed -> accept the packet
	       printk(KERN_INFO "VITTORIA!");
	       return NF_ACCEPT;
	    }
	 }
	 // src_ip isn't in allowed -> drop the packet
	 printk(KERN_INFO "drop!");
	 return NF_DROP;
      }
      
      // if syn flag is set
      if(tcp_header->syn == 1) { 
         printk(KERN_INFO "SYN!");
         cb_write(bufferPointer,&e, &allowed);
      }
 
   }
		
   
   return NF_ACCEPT;

 }

module_param_array(port_init, int, &count, 0);
module_param(port_dest, int, 0);

 
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