#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

int main() {
	uv_loop_t *loop;
	uv_udp_t sock;
	struct sockaddr_in addr;
	
	loop = uv_default_loop();
	
	uv_udp_init(loop, &sock);
	uv_ip4_addr("0.0.0.0", 0, &addr);
	uv_udp_bind(&sock, (const struct sockaddr *)&addr, 0);

	// Bridge discovery packet from Hue Documentation
	char *msearch = "M-SEARCH * HTTP/1.1\r\n"
                	"HOST:239.255.255.250:1900\r\n"
                	"MAN:\"ssdp:discover\"\r\n"
                	"ST:ssdp:all\r\n"
                	"MX:1"; 

	uv_udp_send_t send_req;
	uv_buf_t discover_packet;
	discover_packet = uv_buf_init(msearch, strlen(msearch));

	struct sockaddr_in send_addr;
	uv_ip4_addr("239.255.255.250", 1900, &send_addr);
	uv_udp_send(&send_req, &sock, &discover_packet, 1,
		(const struct sockaddr *)&send_addr, NULL);

	
	return uv_run(loop, UV_RUN_DEFAULT);
}
