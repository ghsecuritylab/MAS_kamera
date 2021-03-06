#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
#include <stdio.h>
#include "sleep.h"
#include "xgpio.h"
#include "bsp/camera.h"
#include "xsdps.h"
#include "ff.h"
#include "xil_cache.h"
#include "bsp/iic.h"
#include "bsp/jpeg_encoder.h"

#include <stdio.h>

#include "xparameters.h"

#include "netif/xadapter.h"

#include "server/platform.h"
#include "server/platform_config.h"
#if defined (__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#endif

#include "lwip/tcp.h"
#include "xil_cache.h"
#include "xgpio.h"
#include "ff.h"


/* defined by each RAW mode application */
void print_app_header();
int start_application();
int transfer_data();
void tcp_fasttmr(void);
void tcp_slowtmr(void);

/* missing declaration in lwIP */
void lwip_init();

u8 cam[3*640*480] __attribute__ ((aligned(32)));

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

void
print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}



int main()
{
	int status=0;

	status = init_gpio();
	if (status != XST_SUCCESS) {
		printf("GPIO initialization failed.\n");
		return 0;
	}
	usleep(10000);

	status = init_i2c();
	if (status != XST_SUCCESS) {
		printf("I2C initialization failed.\n");
		return 0;
	}
	usleep(10000);

	sleep(1);
	status = init_sd(0);
	if (status != XST_SUCCESS) {
		printf("SD card initialization failed.\n");
		return 0;
	}
	usleep(10000);

	status = init_camera();
	if (status != XST_SUCCESS) {
		printf("Camera card initialization failed.\n");
		return 0;
	}
	usleep(10000);

	/*sleep(5);
	if(take_photo() != XST_SUCCESS){
		printf("Failed to take image.\n");
	}

	if(TakeJPG() != XST_SUCCESS){
		printf("Failed to take image.\n");
	}*/
	printf("Complete\n\r");


	ip_addr_t ipaddr, netmask, gw;

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
			{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif = &server_netif;

	init_platform();

	/* initliaze IP addresses to be used */
	IP4_ADDR(&ipaddr, 192, 168, 1, 10);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 1, 1);
	print_app_header();

	//take_photo_(cam);
	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask, &gw, mac_ethernet_address,
	PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}

	netif_set_default(echo_netif);
	/* now enable interrupts */
	platform_enable_interrupts();
	//platform_stop_interrupts();
	//sleep(5);


	/* specify that the network if is up */
	netif_set_up(echo_netif);

	print_ip_settings(&ipaddr, &netmask, &gw);


	/* start the application (web server, rxtest, txtest, etc..) */
	start_application();


	/* receive and process packets */
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
		transfer_data();

	}

	/* never reached */
	cleanup_platform();











	XGpio_DiscreteSet(&led_btns, 2, 0x00);

	return 0;
}

