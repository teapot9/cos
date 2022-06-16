#ifndef DEV_TTY_SERIAL_SERIAL_H
#define DEV_TTY_SERIAL_SERIAL_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <cpp.h>
#include <device.h>

#define STANDARD_BASE_COM1 0x3F8
#define STANDARD_BASE_COM2 0x2F8
#define STANDARD_BASE_COM3 0x3E8
#define STANDARD_BASE_COM4 0x2E8

enum serial_register {
	SERIAL_BUFFER,
	SERIAL_DIV_LOW,
	SERIAL_INT_ENABLE,
	SERIAL_DIV_HIGH,
	SERIAL_INT_IDENTIFICATION,
	SERIAL_FIFO_CONTROL,
	SERIAL_LINE_CONTROL,
	SERIAL_MODEM_CONTROL,
	SERIAL_LINE_STATUS,
	SERIAL_MODEM_STATUS,
	SERIAL_SCRATCH,
};

enum uart_id {
	UART_ID_16750,
	UART_ID_16550A,
	UART_ID_16550,
	UART_ID_16450,
	UART_ID_8250,
};

struct _packed_ interrupt_enable_register {
	bool received_data_available : 1;
	bool transmitter_holding_register_empty : 1;
	bool receiver_line_status : 1;
	bool modem_status : 1;
	bool sleep_mode : 1;
	bool low_power_mode : 1;
	unsigned _reserved0 : 2;
};
static_assert(sizeof(struct interrupt_enable_register) == 1,
	      "interrupt_enable_register must be 1B");

struct _packed_ interrupt_identification_register {
	bool interrupt_pending : 1;
	unsigned mode : 3;
	unsigned _reserved0 : 1;
	bool fifo_64b : 1;
	unsigned fifo : 2;
};
static_assert(sizeof(struct interrupt_identification_register) == 1,
	      "interrupt_identification_register must be 1B");

struct _packed_ fifo_control_register {
	bool enable : 1;
	bool clear : 1;
	bool transmit : 1;
	unsigned dma_mode : 1;
	unsigned _reserved0 : 1;
	bool fifo_64b : 1;
	unsigned int_trigger_level : 2;
};
static_assert(sizeof(struct fifo_control_register) == 1,
	      "fifo_control_register must be 1B");

struct _packed_ line_control_register {
	unsigned word_length : 2;
	unsigned stop_bit : 1;
	unsigned parity : 3;
	bool break_enable : 1;
	bool divisor_latch : 1;
};
static_assert(sizeof(struct line_control_register) == 1,
	      "line_control_register must be 1B");

struct _packed_ modem_control_register {
	bool data_terminal_ready : 1;
	bool request_to_send : 1;
	bool aux1 : 1;
	bool aux2 : 1;
	bool loopback : 1;
	bool autoflow_control : 1;
	unsigned _reserved0 : 2;
};
static_assert(sizeof(struct modem_control_register) == 1,
	      "modem_control_register must be 1B");

struct _packed_ line_status_register {
	bool data_ready : 1;
	bool overrun_error : 1;
	bool parity_error : 1;
	bool framing_error : 1;
	bool break_interrupt : 1;
	bool empty_transmitter_reg : 1;
	bool empty_data_reg : 1;
	bool error_in_fifo : 1;
};
static_assert(sizeof(struct line_status_register) == 1,
	      "line_status_register must be 1B");

struct _packed_ modem_status_register {
	bool delta_clear_to_send : 1;
	bool delta_data_set_ready : 1;
	bool trailing_edge_ring_indicator : 1;
	bool delta_data_carrier_detect : 1;
	bool clear_to_send : 1;
	bool data_set_ready : 1;
	bool ring_indicator : 1;
	bool carrier_detect : 1;
};
static_assert(sizeof(struct modem_status_register) == 1,
	      "modem_status_register must be 1B");

struct serial {
	uint16_t base;
	enum uart_id type;
};

void serial_out(uint16_t base, enum serial_register reg, uint8_t data);
uint8_t serial_in(uint16_t base, enum serial_register reg);
enum uart_id serial_id(uint16_t base);
const char * uart_id_str(enum uart_id id);
int serial_init_port(uint16_t base);
int serial_reset(uint16_t base);
int serial_reg(uint16_t base);
int serial_write(uint16_t base, const char * s);
int serial_console_reg(const struct device * dev);

extern struct module serial_mod;

#endif // DEV_TTY_SERIAL_SERIAL_H
