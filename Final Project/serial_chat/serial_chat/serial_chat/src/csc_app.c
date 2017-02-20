/**
 * \file
 *
 * \brief Custom Serial Chat Application declarations
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel micro controller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel
 *Support</a>
 */

/**
 * \mainpage
 * \section preface Preface
 * This is the reference manual for the Custom Serial Chat Application declarations
 */
/*- Includes -----------------------------------------------------------------------*/
#include <asf.h>
#include "platform.h"
#include "console_serial.h"
#include "at_ble_api.h"
#include "ble_manager.h"
#include "csc_app.h"
#include "cscp.h"
#include "cscs.h"
#include <math.h>
extern ble_connected_dev_info_t ble_dev_info[1];

//#define HTPT_FAHRENHEIT
#define LED_STATUS_BIT 0
#define INT_BIT        1
#define FLOAT_BIT      2

// Function  Prototypes
void LED0_ON();
void LED0_OFF();
static void configure_gpio();

/* =========================== GLOBALS ============================================================ */

#define APP_STACK_SIZE  (1024)

volatile unsigned char app_stack_patch[APP_STACK_SIZE];
static void csc_prf_report_ntf_cb(csc_report_ntf_t *report_info);
/* Received notification data structure */
csc_report_ntf_t recv_ntf_info;

/* Data length to be send over the air */
uint16_t send_length = 1;

/* Buffer data to be send over the air */
uint8_t send_data[APP_TX_BUF_SIZE];


static const ble_event_callback_t app_gap_handle[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	app_connected_event_handler,
	app_disconnected_event_handler,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static void uart_rx_callback(uint8_t input)
{
	if(input == '\r') {
		if(send_length) {
			send_plf_int_msg_ind(UART_RX_COMPLETE, UART_RX_INTERRUPT_MASK_RX_FIFO_NOT_EMPTY_MASK, send_data, send_length);
			memset(send_data, 0, APP_TX_BUF_SIZE);
			send_length = 0;
			DBG_LOG(" ");
		}
	}
	else {
		send_data[send_length++] = input;
		DBG_LOG_CONT("%c", input);
		
		if(send_length >= APP_TX_BUF_SIZE) {
			send_plf_int_msg_ind(UART_RX_COMPLETE, UART_RX_INTERRUPT_MASK_RX_FIFO_NOT_EMPTY_MASK, send_data, send_length);
			memset(send_data, 0, APP_TX_BUF_SIZE);
			send_length = 0;
		}
	}
}

/**
* @brief app_connected_state blemanager notifies the application about state
* @param[in] at_ble_connected_t
*/
static at_ble_status_t app_connected_event_handler(void *params)
{
	return AT_BLE_SUCCESS;
}

/**
 * @brief app_connected_state ble manager notifies the application about state
 * @param[in] connected
 */
static at_ble_status_t app_disconnected_event_handler(void *params)
{
		/* Started advertisement */
		notify_recv_ntf_handler(csc_prf_report_ntf_cb);
		csc_prf_dev_adv();		

		return AT_BLE_SUCCESS;
}

/* Function used for receive data */
static void csc_app_recv_buf(uint8_t *recv_data, uint8_t recv_len)
{
	uint16_t ind = 0;
	if (recv_len){
		for (ind = 0; ind < recv_len; ind++){
			if (strncmp(recv_data,"hi",recv_len) == 0 || strncmp(recv_data,"Hi",recv_len) == 0 )
			{
				LED0_OFF();
			}
			
			else if (strncmp(recv_data,"stop",recv_len) == 0 || strncmp(recv_data,"Stop",recv_len) == 0)
			{
				at_ble_disconnect(ble_dev_info[0].conn_info.handle, AT_BLE_TERMINATED_BY_USER);
				recv_data = '\0';
			}
			//DBG_LOG_CONT("%c", recv_data[ind]);
		}
		DBG_LOG("\r\n");
	}
}

/* Callback called for new data from remote device */
static void csc_prf_report_ntf_cb(csc_report_ntf_t *report_info)
{
	DBG_LOG("\r\n");
	csc_app_recv_buf(report_info->recv_buff, report_info->recv_buff_len);
	report_info->recv_buff = '\0';
	report_info->recv_buff_len = 0;
}

/* Function used for send data */
static void csc_app_send_buf(void)
{
	uint16_t plf_event_type;
	uint16_t plf_event_data_len;
	uint8_t plf_event_data[APP_TX_BUF_SIZE] = {0, };

	platform_event_get(&plf_event_type, plf_event_data, &plf_event_data_len);
	
	if(plf_event_type == ((UART_RX_INTERRUPT_MASK_RX_FIFO_NOT_EMPTY_MASK << 8) | UART_RX_COMPLETE)) {
		csc_prf_send_data(plf_event_data, plf_event_data_len);
	}

}



//! [module_inst]
struct uart_module uart_instance;
//! [module_inst]


//! [dma_resource]
struct dma_resource uart_dma_resource_tx;
struct dma_resource uart_dma_resource_rx;
//! [dma_resource]

//! [usart_buffer]
#define BUFFER_LEN    4
static int8_t string[BUFFER_LEN];
//! [usart_buffer]


//! [transfer_descriptor]
struct dma_descriptor example_descriptor_tx;
struct dma_descriptor example_descriptor_rx;
//! [transfer_descriptor]

//! [setup]
//! [transfer_done_tx]
static void transfer_done_tx(struct dma_resource* const resource )
{
	dma_start_transfer_job(&uart_dma_resource_rx);
}
//! [transfer_done_tx]

//! [transfer_done_rx]
float TempRecvFrmGecko = 0;
void LED0_ON()
{
	gpio_pin_set_output_level(LED0_PIN, LED0_ACTIVE);
}

void LED0_OFF()
{
	gpio_pin_set_output_level(LED0_PIN, LED0_INACTIVE);
}

#define precision 1
void floatToStr(float f, char str[30])
{
	
	int a,b,c,k,l=0,m,i=0,j;
	
	// check for negetive float
	if(f<0.0)
	{
		
		str[i++]='-';
		f*=-1;
	}
	
	a=f;	// extracting whole number
	f-=a;	// extracting decimal part
	k = precision;
	
	// number of digits in whole number
	while(k>-1)
	{
		l = pow(10,k);
		m = a/l;
		if(m>0)
		{
			break;
		}
	k--;
	}

	// number of digits in whole number are k+1
	
	/*
	extracting most significant digit i.e. right most digit , and concatenating to string
	obtained as quotient by dividing number by 10^k where k = (number of digit -1)
	*/
	
	for(l=k+1;l>0;l--)
	{
		b = pow(10,l-1);
		c = a/b;
		str[i++]=c+48;
		a%=b;
	}
	str[i++] = '.';
	
	/* extracting decimal digits till precision */

	for(l=0;l<precision;l++)
	{
		f*=10.0;
		b = f;
		str[i++]=b+48;
		f-=b;
	}

	str[i]='\0';
}

static void transfer_done_rx(struct dma_resource* const resource )
{
	dma_start_transfer_job(&uart_dma_resource_rx);
	if (string [LED_STATUS_BIT]  == 0x00)
	{
		LED0_OFF();
	}
	
	else
	{
		LED0_ON();
	}
	
	
	if (string[INT_BIT] > 0)
	TempRecvFrmGecko = string[INT_BIT] + string[FLOAT_BIT] / 10.0;
	else
	TempRecvFrmGecko = string[INT_BIT] - string[FLOAT_BIT] / 10.0;
	
	char strToSend[200];
	char levelSensorStr[100];
	char MoisSensorStr[100];
	//sprintf(strToSend,"%d",TempRecvFrmGecko);
	strncat(strToSend,"Level Sensor: ",strlen("Level Sensor: "));
	//conver
	itoa(string[0],levelSensorStr, 10);
	
	strncat(strToSend,levelSensorStr,strlen(levelSensorStr));
	strncat(strToSend,"\n","\n");
	strncat(strToSend,"Temperature: ",strlen("Temperature: "));
	
	char str[30];
	floatToStr(TempRecvFrmGecko,str);
	strncat(strToSend,str,4);
	strncat(strToSend,"\n","\n");
	strncat(strToSend,"Moisture: ",strlen("Moisture: "));
	itoa(string[3],MoisSensorStr, 10);
	strncat(strToSend,MoisSensorStr,strlen(MoisSensorStr));
	//strncat(strToSend,"\n","\n");
	
	int i =0;
	for(i=0;i<strlen(strToSend);i++)
	{
		uart_rx_callback(strToSend[i]);	
	}

 	uart_rx_callback('\r');
	bzero(strToSend,sizeof(strToSend));
	//uart_rx_callback(TempRecvFrmGecko);
	
}
//! [transfer_done_rx]

//! [config_dma_resource_tx]
static void configure_dma_resource_tx(struct dma_resource *resource)
{
	//! [setup_tx_1]
	struct dma_resource_config config;
	//! [setup_tx_1]

	//! [setup_tx_2]
	dma_get_config_defaults(&config);
	//! [setup_tx_2]

	//! [setup_tx_3]
	config.des.periph = UART0TX_DMA_PERIPHERAL;
	config.des.enable_inc_addr = false;
	config.src.periph = UART0TX_DMA_PERIPHERAL;
	//! [setup_tx_3]

	//! [setup_tx_4]
	dma_allocate(resource, &config);
	//! [setup_tx_4]
}
//! [config_dma_resource_tx]

//! [setup_dma_transfer_tx_descriptor]
static void setup_transfer_descriptor_tx(struct dma_descriptor *descriptor)
{

	//! [setup_tx_5]
	dma_descriptor_get_config_defaults(descriptor);
	//! [setup_tx_5]

	//! [setup_tx_6]
	descriptor->buffer_size = BUFFER_LEN;
	descriptor->read_start_addr = (uint32_t)string;
	descriptor->write_start_addr =
	(uint32_t)(&uart_instance.hw->TRANSMIT_DATA.reg);
	//! [setup_tx_6]
}
//! [setup_dma_transfer_tx_descriptor]

//! [config_dma_resource_rx]
static void configure_dma_resource_rx(struct dma_resource *resource)
{
	//! [setup_rx_1]
	struct dma_resource_config config;
	//! [setup_rx_1]

	//! [setup_rx_2]
	dma_get_config_defaults(&config);
	//! [setup_rx_2]

	//! [setup_rx_3]
	config.src.periph = UART0RX_DMA_PERIPHERAL;
	config.src.enable_inc_addr = false;
	config.src.periph_delay = 1;
	//! [setup_rx_3]

	//! [setup_rx_4]
	dma_allocate(resource, &config);
	//! [setup_rx_4]
}
//! [config_dma_resource_rx]

//! [setup_dma_transfer_rx_descriptor]
static void setup_transfer_descriptor_rx(struct dma_descriptor *descriptor)
{
	//! [setup_rx_5]
	dma_descriptor_get_config_defaults(descriptor);
	//! [setup_rx_5]

	//! [setup_tx_6]
	descriptor->buffer_size = BUFFER_LEN;
	descriptor->read_start_addr =
	(uint32_t)(&uart_instance.hw->RECEIVE_DATA.reg);
	descriptor->write_start_addr = (uint32_t)string;
	//! [setup_tx_6]
}
//! [setup_dma_transfer_rx_descriptor]

//! [setup_usart]
static void configure_usart(void)
{
	//! [setup_config]
	struct uart_config config_uart;
	//! [setup_config]

	//! [setup_config_defaults]
	uart_get_config_defaults(&config_uart);
	//! [setup_config_defaults]

	//! [setup_change_config]
	//config_uart.baud_rate = 115200;
	config_uart.baud_rate = 9600;
	config_uart.pin_number_pad[0] = EDBG_CDC_SERCOM_PIN_PAD0;
	config_uart.pin_number_pad[1] = EDBG_CDC_SERCOM_PIN_PAD1;
	config_uart.pin_number_pad[2] = EDBG_CDC_SERCOM_PIN_PAD2;
	config_uart.pin_number_pad[3] = EDBG_CDC_SERCOM_PIN_PAD3;
	config_uart.pinmux_sel_pad[0] = EDBG_CDC_SERCOM_MUX_PAD0;
	config_uart.pinmux_sel_pad[1] = EDBG_CDC_SERCOM_MUX_PAD1;
	config_uart.pinmux_sel_pad[2] = EDBG_CDC_SERCOM_MUX_PAD2;
	config_uart.pinmux_sel_pad[3] = EDBG_CDC_SERCOM_MUX_PAD3;
	//! [setup_change_config]

	//! [setup_set_config]
	while (uart_init(&uart_instance,
	EDBG_CDC_MODULE, &config_uart) != STATUS_OK) {
	}
	//! [setup_set_config]

	//! [enable_interrupt]
	uart_enable_transmit_dma(&uart_instance);
	uart_enable_receive_dma(&uart_instance);
	//! [enable_interrupt]
}
//! [setup_usart]

//! [setup_callback]
static void configure_dma_callback(void)
{
	//! [setup_callback_register]
	dma_register_callback(&uart_dma_resource_tx, transfer_done_tx, DMA_CALLBACK_TRANSFER_DONE);
	dma_register_callback(&uart_dma_resource_rx, transfer_done_rx, DMA_CALLBACK_TRANSFER_DONE);
	//! [setup_callback_register]

	//! [setup_enable_callback]
	dma_enable_callback(&uart_dma_resource_tx, DMA_CALLBACK_TRANSFER_DONE);
	dma_enable_callback(&uart_dma_resource_rx, DMA_CALLBACK_TRANSFER_DONE);
	//! [setup_enable_callback]

	//! [enable_inic]
	NVIC_EnableIRQ(PROV_DMA_CTRL0_IRQn);
	//! [enable_inic]
}
//! [setup_callback]

//! [setup]

static void configure_gpio()
{
	struct gpio_config configure_gpio_for_UART;
	gpio_get_config_defaults(&configure_gpio_for_UART);
	
	//configure_gpio_for_UART.powersave = true;  // maybe this will be of use
	configure_gpio_for_UART.direction = GPIO_PIN_DIR_OUTPUT;
	configure_gpio_for_UART.input_pull = GPIO_PIN_PULL_DOWN;
	
	gpio_pin_set_config(LED0_PIN, &configure_gpio_for_UART);
	gpio_pin_set_output_level(LED0_PIN, LED0_INACTIVE);
	//LED0_ON();
	//gpio_pin_set_output_level(LED0_PIN, LED0_ACTIVE);
}


bool app_exec = true;
int main(void )
{
	platform_driver_init();
	acquire_sleep_lock();

	/* Initialize serial console  */
	serial_console_init();
	
	
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

		//! [setup_init]
		//! [init_system]
		system_clock_config(CLOCK_RESOURCE_XO_26_MHZ, CLOCK_FREQ_26_MHZ);
		//! [init_system]

		//! [setup_usart]
		configure_usart();
		//! [setup_usart]

		//! [setup_dma_resource]
		configure_dma_resource_tx(&uart_dma_resource_tx);
		configure_dma_resource_rx(&uart_dma_resource_rx);
		//! [setup_dma_resource]

		//! [setup_transfer_descriptor]
		setup_transfer_descriptor_tx(&example_descriptor_tx);
		setup_transfer_descriptor_rx(&example_descriptor_rx);
		//! [setup_transfer_descriptor]

		//! [add_descriptor_to_resource]
		dma_add_descriptor(&uart_dma_resource_tx, &example_descriptor_tx);
		dma_add_descriptor(&uart_dma_resource_rx, &example_descriptor_rx);
		//! [add_descriptor_to_resource]

		//! [configure_callback]
		configure_dma_callback();
		//! [configure_callback]
		//! [setup_init]

		//! [main]
		//! [main_1]
		dma_start_transfer_job(&uart_dma_resource_rx);
		//! [main_1]
		configure_gpio();
	
	DBG_LOG("Initializing Custom Serial Chat Application");
	
	/* Initialize the buffer address and buffer length based on user input */
	csc_prf_buf_init(&send_data[0], APP_TX_BUF_SIZE);
	
	/* initialize the ble chip  and Set the device mac address */
	ble_device_init(NULL);
	
	/* Initializing the profile */
	csc_prf_init(NULL);
	
	/* Started advertisement */
	csc_prf_dev_adv();
	
	ble_mgr_events_callback_handler(REGISTER_CALL_BACK,
	BLE_GAP_EVENT_TYPE,
	app_gap_handle);
	
	/* Register the notification handler */
	notify_recv_ntf_handler(csc_prf_report_ntf_cb);
	
	/* Register the user event handler */
	register_ble_user_event_cb(csc_app_send_buf);
	
	register_uart_callback(uart_rx_callback);
	
	/* Capturing the events  */
	while(app_exec){
		ble_event_task(BLE_EVENT_TIMEOUT);
	}
	return 0;
}



