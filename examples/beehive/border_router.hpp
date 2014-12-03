/* 6LoWPAN Border Router header file */

#ifndef _SIXLOWPAN_BORDER_ROUTER_GSMH
#define _SIXLOWPAN_BORDER_ROUTER_GSM_H
#define ENABLE_DEBUG (1)
extern "C" {

#include <stdint.h>

#include "mutex.h"
#include "transceiver.h"

#include "sixlowpan/ip.h"
#include "semaphore.h"
#include "debug.h"
#include "rpl.h"
#include "net_if.h"
#include "simcom.h"
extern ipv6_addr_t *abr_addr;
extern kernel_pid_t gsm_handler_pid;

}
#define RADIO_CHANNEL (11)
template <typename RouterInterface>
class BorderRouter : private RouterInterface
{
	using RouterInterface::transmit_ipv6;

public:
	static kernel_pid_t get_serial_reader(void)
	{
		return 0; //TODO:
	}
	static int sixlowpan_lowpan_border_init(int if_id)
	{	
	    transceiver_command_t tcmd;
    	msg_t m;
	    ipv6_net_if_addr_t *addr = NULL;
	    uint8_t abr_addr_initialized = 0;
    	int32_t chan = RADIO_CHANNEL;

		net_if_set_src_address_mode(0, NET_IF_TRANS_ADDR_M_SHORT);
		uint8_t id = net_if_get_hardware_address(0);

	    // serial_reader_pid = thread_create(
	    //                         serial_reader_stack, READER_STACK_SIZE,
	    //                         PRIORITY_MAIN - 1, CREATE_STACKTEST,
	    //                         serial_reader_f, NULL,"serial_reader");
	    ip_process_pid = thread_create(ip_process_buf, IP_PROCESS_STACKSIZE,
	                                   PRIORITY_MAIN - 1, CREATE_STACKTEST,
	                                   border_process_lowpan, NULL,
	                                   "border_process_lowpan");

	    if (ip_process_pid == KERNEL_PID_UNDEF) {
	        return 0;
	    }
        net_if_set_hardware_address(0, id);

	    uint8_t state = rpl_init(0);

        rpl_init_root();
        ipv6_iface_set_routing_provider(rpl_get_next_hop);

	    /* add global address */
	    ipv6_addr_t tmp;
	    /* initialize prefix */
	    ipv6_addr_init(&tmp, 0xabcd, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, id);
	    /* set host suffix */
	    ipv6_addr_set_by_eui64(&tmp, 0, &tmp);
	    ipv6_net_if_add_addr(0, &tmp, NDP_ADDR_STATE_PREFERRED, 0, 0, 0);

	    if (!sixlowpan_lowpan_init_interface(if_id)) {
	        return 0;
	    }

	    while (net_if_iter_addresses(if_id, (net_if_addr_t **) &addr)) {
	        if (!ipv6_addr_is_multicast(addr->addr_data) &&
	            !ipv6_addr_is_link_local(addr->addr_data) &&
	            !ipv6_addr_is_loopback(addr->addr_data) &&
	            !ipv6_addr_is_unique_local_unicast(addr->addr_data)) {
	            abr_addr_initialized = 1;
	            abr_addr = addr->addr_data;
	            break;
	        }
	    }

	    if (!abr_addr_initialized) {
	        DEBUG("sixlowpan_lowpan_border_init(): A prefix must be initialized to"
	              "interface %d first", if_id);
	        return 0;
	    }

        ipv6_init_as_router();

	    /* set channel to 11 */
	    tcmd.transceivers = TRANSCEIVER_DEFAULT;
	    tcmd.data = &chan;
	    m.type = SET_CHANNEL;
	    m.content.ptr = (char*) &tcmd;

	    msg_send_receive(&m, &m, transceiver_pid);
	    printf("Channel set to %u\n", RADIO_CHANNEL);

	    puts("Transport layer initialized");


	    return 1;
	};
	static void *border_process_lowpan(void* arg)
	{
		msg_t m;

		while (1) {
		    msg_receive(&m);
		    ipv6_hdr_t *ipv6_buf = (ipv6_hdr_t *)m.content.ptr;

		    if (ipv6_buf->nextheader == IPV6_PROTO_NUM_ICMPV6) {
		        icmpv6_hdr_t *icmp_buf = (icmpv6_hdr_t *)(((uint8_t *)ipv6_buf) + IPV6_HDR_LEN);

		        if (icmp_buf->type == ICMPV6_TYPE_REDIRECT) {
		            continue;
		        }

		        if (icmpv6_demultiplex(icmp_buf) == 0) {
		            continue;
		        }

		        /* Here, other ICMPv6 message types for ND may follow. */
		    }

		    /* TODO: Bei ICMPv6-Paketen entsprechende LoWPAN-Optionen verarbeiten und entfernen */    		
		    DEBUG("CANNOT CRASH \n");
		    transmit_ipv6(ipv6_buf);
		}
	};
	/* UDP server thread */
	static void udp_server(void)
	{
	    kernel_pid_t udp_server_thread_pid = thread_create(udp_server_stack_buffer,
	                                                       sizeof(udp_server_stack_buffer),
	                                                       PRIORITY_MAIN, CREATE_STACKTEST,
	                                                       init_udp_server,
	                                                       NULL,
	                                                       "init_udp_server");
	    printf("UDP SERVER ON PORT %d (THREAD PID: %" PRIkernel_pid ")\n", HTONS(SERVER_PORT), udp_server_thread_pid);
	};

	static void *init_udp_server(void *arg)
	{
	    (void) arg;

	    sockaddr6_t sa;
	    char buffer_main[UDP_BUFFER_SIZE];
	    uint32_t fromlen;
	    int sock = socket_base_socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	    memset(&sa, 0, sizeof(sa));

	    sa.sin6_family = AF_INET;
	    sa.sin6_port = HTONS(SERVER_PORT);

	    fromlen = sizeof(sa);

	    if (-1 == socket_base_bind(sock, &sa, sizeof(sa))) {
	        printf("Error bind failed!\n");
	        socket_base_close(sock);
	        return NULL;
	    }

	    while (1) {
	        int32_t recsize = socket_base_recvfrom(sock, (void *)buffer_main, UDP_BUFFER_SIZE, 0, &sa, &fromlen);

	        if (recsize < 0) {
	            printf("ERROR: recsize < 0!\n");
	        }

	        printf("UDP packet received, payload: %s\n", buffer_main);
	    }

	    socket_base_close(sock);

	    return NULL;
	};
};



#endif /* _SIXLOWPAN_BORDER_ROUTER_GSM_H*/
