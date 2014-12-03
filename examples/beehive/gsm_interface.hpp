#pragma once
#define ENABLE_DEBUG (0)

extern "C"
{
#include "simcom.h"
#include "debug.h"
#include "ipv6.h"
#include "udp.h"
}

class GsmInterface
{
	public:
		ipv6_hdr_t* receive_ipv6();
		static void transmit_ipv6(ipv6_hdr_t* packet)
		{
			//tcp_init();
			//char buffer[128];
			//char* payload;
			//payload = (char*) (packet + IPV6_HDR_LEN);

			//memcpy(buffer, payload, NTOHS(udp_header->length) - UDP_HDR_LEN);
			printf("packet length: %d\n", NTOHS(packet->length) +IPV6_HDR_LEN);

			char* byte_packet = (char*) packet;
			uint16_t* length = (uint16_t*) (byte_packet + IPV6_HDR_LEN + 4);
			printf("length: %d\n",NTOHS(*length) - UDP_HDR_LEN);
			char* payload = (char*)(byte_packet + IPV6_HDR_LEN + UDP_HDR_LEN);

			// for(int i = 0; i < NTOHS(packet->length) - UDP_HDR_LEN; i++)
			// {
			// 	printf("%c",packet[i]);
			// }
			//printf("\n" );
			http_post_payload(payload, NTOHS(*length) - UDP_HDR_LEN);
			//tcp_send((char*)(byte_packet + IPV6_HDR_LEN + UDP_HDR_LEN), NTOHS(*length) - UDP_HDR_LEN);
			return;
		};
};