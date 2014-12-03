#ifndef SIMCOM_H_
#define SIMCOM_H_

#ifdef __cplusplus
extern "C" {
#endif
#define CIPSHUT "AT+CIPSHUT\r\n"
#define CGATT   "AT+CGATT=1\r\n"
#define CGDCONT "AT+CGDCONT=1,\"IP\",\"internet\"\r\n"
#define CGACT   "AT+CGACT=1,1\r\n"
#define CSTT "AT+CSTT=\"internet\",\"\",\"\"\r\n"
#define CIICR "AT+CIICR\r\n"
#define CIFSR "AT+CIFSR\r\n"
#define CIPSTART "AT+CIPSTART=\"TCP\",\"128.199.36.229\",3333\r\n" //128.199.36.229

#define URL       "http://beehivemonitor.azurewebsites.net/RestApi/PutData?br=1&n=1&s=troels&v=1&t=now"
#define REST_WEBSITE   "http://beehivemonitor.azurewebsites.net/RestApi/"
#define PUTDATA        "PutData?"
#define REST_CMD		"CMD?cmd="
#define REST_BORDER    "br="
#define REST_NODE      "&n="
#define REST_SENSOR    "&s="
#define REST_VALUE     "&v="
#define REST_TIMESTAMP "&t="

void rx_cb(void *arg, char data);
void simcom_init(void);
void send(char* command, unsigned int count);
void send_string(char* command);
void gsm_notify_thread(void);
int gsm_readc(void);
void *gsm_thread_entry(void *rb_);

void print_buffer(void);
void send_to_uart1(int argc, char **argv);

void tcp_init(void);
void tcp_send(char* string, int length);
int wait_for_string(char* string, int timeout);
int sendATAndWait(char* command, char* waitString, int timeout);

#define OK              "OK"
/*      SAPBR           */
#define SETCONTYPE      "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n"
#define SETAPN          "AT+SAPBR=3,1,\"APN\",\"internet\"\r\n"
#define SETUSER         "AT+SAPBR=3,1,\"USER\",\"\"\r\n"
#define SETPWD          "AT+SAPBR=3,1,\"PWD\",\"\"\r\n"
#define ACTIVATE        "AT+SAPBR=1,1\r\n"
#define DEACTIVATE      "AT+SAPBR=0,1\r\n"
/*      Http            */
#define CREG            "AT+CREG?\r\n"
#define SAPBR           "AT+SAPBR=1,1\r\n"
#define WAIT            "WAIT=6\r\n"
#define HTTPINIT        "AT+HTTPINIT\r\n"
#define HTTP_PARA_CID   "AT+HTTPPARA=\"CID\",1\r\n"
#define HTTP_ACTION_GET "AT+HTTPACTION=0\r\n"
#define HTTP_READ       "AT+HTTPREAD\r\n"
#define HTTP_TERMINATE  "AT+HTTPTERM\r\n"

void http_init(void);
void http_post(char* url);
void http_post_payload(char* payload, int length);
void http_concat_datacommand(char* url, char* payload);



#ifdef __cplusplus
}
#endif
#endif