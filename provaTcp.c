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

#include "CircularBuffer.h"


MODULE_LICENSE("GPL");
 
MODULE_DESCRIPTION("linux-simple-firewall");
 
MODULE_AUTHOR("4Mosfet");

//the structure used to register the function
 
static struct nf_hook_ops nfho;
 
//the hook function itself: registered for filtering incoming packets
 
unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, 
 
        const struct net_device *in, const struct net_device *out,
 
        int (*okfn)(struct sk_buff *)) {
 
   /*get src address, src netmask, src port, dest ip, dest netmask, dest port, protocol*/
 
   struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
 
   struct tcphdr *tcp_header;

   

 
   /**get src and dest ip addresses**/
 
   
   unsigned int src_ip= ip_header->saddr;
   unsigned int dest_ip= ip_header->daddr;
   /*char src_ip[16];
   snprintf(src_ip, 16, "%pI4", &ip_header->saddr);

   char dest_ip[16];
   snprintf(dest_ip, 16, "%pI4", &ip_header->daddr);*/
 
 
   unsigned int src_port = 0;
 
   unsigned int dest_port = 0;
 
   /***get src and dest port number***/
 

 
    //	TCP
   if (ip_header->protocol == 6) {
       
       
 
       tcp_header = (struct tcphdr *)(skb_network_header(skb) + ip_hdrlen(skb));
       
       //if(tcp_header->syn == 1) {
	  if (bufferPointer == NULL) { 
	    printk(KERN_INFO "inizializzo");
	    cbInit(&cb,10);
	    bufferPointer = &cb;
          }
 
       src_port = (unsigned int)ntohs(tcp_header->source);
 
       dest_port = (unsigned int)ntohs(tcp_header->dest);
       
       e.src_ip = src_ip;
       e.src_port = src_port;
       e.dest_ip = dest_ip;
       e.dest_port = dest_port; 
       
       cbWrite(bufferPointer,&e);
	 
      printk(KERN_INFO "end in provatcp nell'if: %u", cb.end);
      //}
 
   }
				
   
   printk(KERN_INFO "end in provatcp: %u", cb.end);
   
   return NF_ACCEPT;

 }
 
 
/* Initialization routine */
 
int init_module() {
    printk(KERN_INFO "initialize kernel module\n");
 
    /* Fill in the hook structure for incoming packet hook*/
 
    nfho.hook = hook_func_in;
 
    nfho.hooknum = NF_INET_LOCAL_IN;
 
    nfho.pf = PF_INET;
 
    nfho.priority = NF_IP_PRI_FIRST;
 
    nf_register_hook(&nfho);         // Register the hook
  
    //cbInit(&cb,10);
 
    return 0;
 
}

void cleanup_module() {

    nf_unregister_hook(&nfho);
 
    printk(KERN_INFO "kernel module unloaded.\n");
 
}