#include "simcom.h"
#define ENABLE_DEBUG    (1)
#include "debug.h"
#include "string.h"
#include "periph/uart.h"
#include "ringbuffer.h"
#include "vtimer.h"
#include <stdio.h>
#include "chardev_thread.h"
#include "thread.h"
#include "posix_io.h"
#include "periph/gpio.h"
static char gsm_rx_buf_mem[128];
static ringbuffer_t gsm_rx_buf;
kernel_pid_t gsm_handler_pid = KERNEL_PID_UNDEF;
char gsm_thread_stack[512];
static char ctrl_z[] = {0x1a, '\0'};
void rx_callback(void *arg, char data)
{
    if (gsm_handler_pid) {
	    ringbuffer_add_one(&gsm_rx_buf, data);
	    if(data == 0)
	    {
	    	DEBUG("0\n");
	    }
        gsm_notify_thread();
    }
}
void simcom_init(void)
{
	ringbuffer_init(&gsm_rx_buf, gsm_rx_buf_mem, sizeof(gsm_rx_buf_mem));
	uart_init(UART_1, 115200, rx_callback, 0, 0);
	gpio_init_out(SIMCOM_POWER_SELECT, GPIO_PULLUP);
	gpio_clear(SIMCOM_POWER_SELECT);
	vtimer_usleep(1000000);

	gpio_set(SIMCOM_POWER_SELECT);
    kernel_pid_t pid = thread_create(
                  gsm_thread_stack,
                  sizeof(gsm_thread_stack),
                  PRIORITY_MAIN - 1,
                  CREATE_STACKTEST | CREATE_SLEEPING,
                  gsm_thread_entry,
                  &gsm_rx_buf,
                  "gsm"
              );
    gsm_handler_pid = pid;
    thread_wakeup(pid);
    posix_open(gsm_handler_pid, 0);
    sendATAndWait("ATE0\r\n", NULL, 3);
	http_init();
    puts("gsm_init() [OK]");

    posix_close(gsm_handler_pid);

}
void *gsm_thread_entry(void *rb_)
{
	chardev_thread_entry(rb_);
}
void gsm_notify_thread(void)
{
    msg_t m;
    m.type = 0;
    msg_send_int(&m, gsm_handler_pid);
}
int gsm_readc(void)
{
    char c = 0;
    int i = posix_read(gsm_handler_pid, &c, 1);
    return c;
}
static int readline(char *buf, size_t size)
{

    char *line_buf_ptr = buf;
	posix_open(gsm_handler_pid, 0);

    while (1) {
        if ((line_buf_ptr - buf) >= ((int) size) - 1) {
			posix_close(gsm_handler_pid);
            return -1;
        }
        int c = gsm_readc();

        if (c < 0) {
			posix_close(gsm_handler_pid);
            return 1;
        }
        if (c == 0)
        {
        	DEBUG("AUCH\n");
        	vtimer_usleep(1000000);
        	continue;
        }

        if (c == '\r') {
        	line_buf_ptr++;
        } 
        else if( c == '\n')
        {
			if (line_buf_ptr == buf) {
				// The line is empty. 
				DEBUG("Empty line \n");
				continue;
			}

			*line_buf_ptr = '\0';
			posix_close(gsm_handler_pid);
			return 0;
        }
        else {
            *line_buf_ptr++ = c;
        }
    }

}


void send_to_uart1(int argc, char **argv)
{
	printf("%s\n", strlen(argv[1]));
	send(argv[1], strlen(argv[1]));
	send("\r\n", 2);
	char buffer[32];
	readline(buffer, 32);
	DEBUG("%s\n", buffer);
}
void tcp_init(void)
{
	uint8_t err;

	sendATAndWait("AT+CIPMUX=0\r\n", OK, 3);

	while(sendATAndWait("AT+CIPSTATUS\r\n", "INITIAL", 3));
	sendATAndWait(CSTT, OK, 3);
	while(sendATAndWait("AT+CIPSTATUS\r\n", "START", 3));

	sendATAndWait(CIICR, OK, 3);
	while(sendATAndWait("AT+CIPSTATUS\r\n", "GPRSACT", 3));

	sendATAndWait(CIFSR, ".", 3);
	while(sendATAndWait("AT+CIPSTATUS\r\n", "IP STATUS", 3));

}
void tcp_send(char* string, int length)
{
	uint8_t err;
	err = sendATAndWait(CIPSTART, "CONNECT OK", 3);
	if(err) return;
	DEBUG("Send CIPSEND\n");
	send_string("AT+CIPSEND\r\n");
	DEBUG("wait\n");
	wait_for_string(">", 3);
	vtimer_usleep(3);
	send(string, length);
	send(ctrl_z,2);
	err = wait_for_string("SEND OK", 3);
	//send_string("AT+CIPCLOSE\r\n");
	send_string("AT+CIPSHUT\r\n");
}
void http_init(void)
{
	sendATAndWait(DEACTIVATE, OK, 10);
  	sendATAndWait(SETCONTYPE, OK, 10);
    sendATAndWait(SETAPN,     OK, 10);
    sendATAndWait(SETUSER,    OK, 10);
    sendATAndWait(SETPWD,     OK, 10);
	sendATAndWait(ACTIVATE,   OK, 10);
}
void http_post(char* url)
{
	sendATAndWait(HTTPINIT, OK, 100);
	sendATAndWait(HTTP_PARA_CID, OK, 10);
	send_string("AT+HTTPPARA=\"URL\",\"");
	send_string(url);
	send_string("\"");
	sendATAndWait("\r\n", OK, 10);
	sendATAndWait(HTTP_ACTION_GET, "+HTTPACTION:0,200,4" ,10);
	sendATAndWait(HTTP_TERMINATE, OK, 10);

}
void http_post_payload(char* payload, int length)
{
	char url[128];
	strcpy(url, REST_WEBSITE);
	DEBUG("%s\n", url);
	switch(payload[0])
	{
	case 'p':
		http_concat_datacommand(url, payload+2);
		break;
	case 'c':
		http_concat_command(url, payload+2);
		break;
	case 'n':
		break;

	}
	DEBUG("%s\n", url);
	http_post(url);
}
void http_concat_command(char* url, char* payload)
{
	char* border_id = "81180295";
	char* node = strtok(payload, "&");
	char* sensor = strtok(NULL, "&");
	char* value = strtok(NULL, "&");

	strcat(url, PUTDATA);
	strcat(url, REST_BORDER);
	strcat(url, border_id);
	strcat(url, "&");
	strcat(url, node);
	strcat(url, "&");
	strcat(url, sensor);
	strcat(url, "&");
	strcat(url, value);

	strcat(url, REST_TIMESTAMP);
	strcat(url, "now");
}

void http_concat_datacommand(char* url, char* payload)
{
	char* border_id = "81180295";
	char* node = strtok(payload, "&");
	char* sensor = strtok(NULL, "&");
	char* value = strtok(NULL, "&");

	strcat(url, PUTDATA);
	strcat(url, REST_BORDER);
	strcat(url, border_id);
	strcat(url, "&");
	strcat(url, node);
	strcat(url, "&");
	strcat(url, sensor);
	strcat(url, "&");
	strcat(url, value);

	strcat(url, REST_TIMESTAMP);
	strcat(url, "now");

}

void send_string(char* string)
{
	send(string, strlen(string));
}
void send(char* string, unsigned int count)
{
    for (int i = 0; i < count; i++) {
        uart_write_blocking(UART_1, string[i]);
    }
}

int sendATAndWait(char* command, char* waitString, int timeout)
{
	DEBUG("%s\n", command);
	send(command, strlen(command));
	return wait_for_string(waitString, timeout);
}



int wait_for_string(char* string, int timeout)
{
	char buffer[128];
	int timeout_counter = 0;
	if(string == NULL) return 0;
	while((timeout_counter < timeout)){
		timeout_counter++;
		readline(buffer, sizeof(buffer)); //A response is usually <CR><LF>response<CR><LF>
		//DEBUG("readline: \"%s\", counter: %d, timeout: %d \n", buffer, timeout_counter, timeout);
		if(strlen(buffer) == 0) continue;
		if(strstr(buffer, string) != NULL) return 0;
		if(strstr(buffer, "CME ERROR") != NULL) return -1;
		//vtimer_usleep(10000);
	}
	DEBUG("NOT SUCCESS :( \n");
	return -1;
}

#ifdef __cplusplus
}
#endif