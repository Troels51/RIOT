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
kernel_pid_t udp_handler_pid;
char udp_server_stack_buffer[1024+512];


}
#define UDP_BUFFER_SIZE     (64)
#define SERVER_PORT     (0xFF01)
#define TRANSCEIVER TRANSCEIVER_DEFAULT
#define RADIO_CHANNEL (11)
template <typename RouterInterface>
class BorderRouter : private RouterInterface
{
	using RouterInterface::transmit_ipv6;

public:
	static int sixlowpan_lowpan_border_init(int if_id)
	{	
	    transceiver_command_t tcmd;
	    msg_t m;
	    int32_t chan = RADIO_CHANNEL;
    	int id = net_if_get_hardware_address(0);

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

	    ipv6_init_as_router();
	    /* set channel to 11 */
	    tcmd.transceivers = TRANSCEIVER;
	    tcmd.data = &chan;
	    m.type = SET_CHANNEL;
	    m.content.ptr = (char*)(&tcmd);

	    msg_send_receive(&m, &m, transceiver_pid);
	    printf("Channel set to %u\n", RADIO_CHANNEL);

	    udp_server();

	    puts("Transport layer initialized");
	};

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
	        http_post_payload(buffer_main, recsize);
	        
	    }

	    socket_base_close(sock);

	    return NULL;
	};
};



#endif /* _SIXLOWPAN_BORDER_ROUTER_GSM_H*/
