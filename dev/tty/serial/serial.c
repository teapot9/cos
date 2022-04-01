#define pr_fmt(fmt) "serial: " fmt

#include "serial.h"

#include <errno.h>

#include <print.h>
#include <asm/io.h>
#include <string.h>

#define get_divisor(baud) ((115200 / (baud)) ? (115200 / (baud)) : 1)

static inline void set_divisor_latch(uint16_t base)
{
	uint8_t _lcr = serial_in(base, SERIAL_LINE_CONTROL);
	struct line_control_register * lcr = (void *) &_lcr;
	if (!lcr->divisor_latch) {
		lcr->divisor_latch = true;
		serial_out(base, SERIAL_LINE_CONTROL, _lcr);
	}
}

static inline void unset_divisor_latch(uint16_t base)
{
	uint8_t _lcr = serial_in(base, SERIAL_LINE_CONTROL);
	struct line_control_register * lcr = (void *) &_lcr;
	if (lcr->divisor_latch) {
		lcr->divisor_latch = false;
		serial_out(base, SERIAL_LINE_CONTROL, _lcr);
	}
}

static inline void set_divisor(uint16_t base, uint16_t div)
{
	serial_out(base, SERIAL_DIV_LOW, div & 0xFF);
	serial_out(base, SERIAL_DIV_HIGH, (div >> 8) & 0xFF);
}

const char * uart_id_str(enum uart_id id)
{
	switch (id) {
	case UART_ID_16750:
		return "16750";
	case UART_ID_16550A:
		return "16550A";
	case UART_ID_16550:
		return "16550";
	case UART_ID_16450:
		return "16450";
	case UART_ID_8250:
		return "8250";
	}
	return "";
}

void serial_out(uint16_t base, enum serial_register reg, uint8_t data)
{
	switch (reg) {
	case SERIAL_BUFFER:
		unset_divisor_latch(base);
		outb(base + 0, data);
		break;
	case SERIAL_DIV_LOW:
		set_divisor_latch(base);
		outb(base + 0, data);
		unset_divisor_latch(base);
		break;
	case SERIAL_INT_ENABLE:
		unset_divisor_latch(base);
		outb(base + 1, data);
		break;
	case SERIAL_DIV_HIGH:
		set_divisor_latch(base);
		outb(base + 1, data);
		unset_divisor_latch(base);
		break;
	case SERIAL_INT_IDENTIFICATION:
		pr_err("input identification register is read-only\n", 0);
		break;
	case SERIAL_FIFO_CONTROL:
		outb(base + 2, data);
		break;
	case SERIAL_LINE_CONTROL:
		outb(base + 3, data);
		break;
	case SERIAL_MODEM_CONTROL:
		outb(base + 4, data);
		break;
	case SERIAL_LINE_STATUS:
		pr_err("line status register is read-only\n", 0);
		break;
	case SERIAL_MODEM_STATUS:
		pr_err("modem status register is read-only\n", 0);
		break;
	case SERIAL_SCRATCH:
		outb(base + 7, data);
		break;
	}
}

uint8_t serial_in(uint16_t base, enum serial_register reg)
{
	uint8_t tmp;
	switch (reg) {
	case SERIAL_BUFFER:
		unset_divisor_latch(base);
		return inb(base + 0);
	case SERIAL_DIV_LOW:
		set_divisor_latch(base);
		tmp = inb(base + 0);
		unset_divisor_latch(base);
		return tmp;
	case SERIAL_INT_ENABLE:
		unset_divisor_latch(base);
		return inb(base + 1);
	case SERIAL_DIV_HIGH:
		set_divisor_latch(base);
		tmp = inb(base + 1);
		unset_divisor_latch(base);
		return tmp;
	case SERIAL_INT_IDENTIFICATION:
		return inb(base + 2);
	case SERIAL_FIFO_CONTROL:
		pr_err("FIFO control register is write-only\n", 0);
		break;
	case SERIAL_LINE_CONTROL:
		return inb(base + 3);
	case SERIAL_MODEM_CONTROL:
		return inb(base + 4);
	case SERIAL_LINE_STATUS:
		return inb(base + 5);
	case SERIAL_MODEM_STATUS:
		return inb(base + 6);
	case SERIAL_SCRATCH:
		return inb(base +7);
	}
	return 0;
}

enum uart_id serial_id(uint16_t base)
{
	struct fifo_control_register fcr;
	memset(&fcr, 0, sizeof(fcr));
	fcr.enable = true;
	fcr.clear = true;
	fcr.transmit = true;
	fcr.fifo_64b = true;
	fcr.int_trigger_level = 3;
	serial_out(base, SERIAL_FIFO_CONTROL, *(uint8_t *) &fcr);

	struct interrupt_identification_register iir;
	*(uint8_t *) &iir = serial_in(base, SERIAL_INT_IDENTIFICATION);

	if (iir.fifo & 1) {
		if (iir.fifo & 2) {
			if (iir.fifo_64b)
				return UART_ID_16750;
			else
				return UART_ID_16550A;
		} else {
			return UART_ID_16550;
		}
	} else {
		uint8_t tmp = 0x2A;
		serial_out(base, SERIAL_SCRATCH, tmp);
		if (serial_in(base, SERIAL_SCRATCH) == tmp)
			return UART_ID_16450;
		else
			return UART_ID_8250;
	}
}

int serial_init_port(uint16_t base)
{
	struct interrupt_enable_register ier = {0};
	serial_out(base, SERIAL_INT_ENABLE, *(uint8_t *) &ier);

	set_divisor(base, get_divisor(CONFIG_SERIAL_DEFAULT_RATE));

	struct line_control_register lcr = {0};
	lcr.word_length = 3;
	lcr.stop_bit = 0;
	lcr.parity = 0;
	serial_out(base, SERIAL_LINE_CONTROL, *(uint8_t *) &lcr);

	struct fifo_control_register fcr = {
		.enable = true,
		.clear = true,
		.transmit = true,
		.dma_mode = false,
		._reserved0 = 0,
		.fifo_64b = false,
		.int_trigger_level = 3,
	};
	serial_out(base, SERIAL_FIFO_CONTROL, *(uint8_t *) &fcr);

	struct modem_control_register mcr = {0};
	mcr.data_terminal_ready = true;
	mcr.request_to_send = true;
	mcr.aux1 = true;
	mcr.aux2 = true;
	mcr.loopback = true;
	serial_out(base, SERIAL_MODEM_CONTROL, *(uint8_t *) &mcr);

	uint8_t tmp = 0x2A;
	serial_out(base, SERIAL_BUFFER, tmp);
	if (serial_in(base, SERIAL_BUFFER) != tmp)
		return -ENOTSUP;

	mcr.loopback = false;
	serial_out(base, SERIAL_MODEM_CONTROL, *(uint8_t *) &mcr);
	return 0;
}

int serial_reset(uint16_t base)
{
	uint8_t tmp;

	struct interrupt_enable_register ier = {0};
	serial_out(base, SERIAL_INT_ENABLE, *(uint8_t *) &ier);

	struct fifo_control_register fcr = {0};
	serial_out(base, SERIAL_FIFO_CONTROL, *(uint8_t *) &fcr);

	struct line_control_register lcr = {0};
	serial_out(base, SERIAL_LINE_CONTROL, *(uint8_t *) &lcr);

	struct modem_control_register mcr = {0};
	serial_out(base, SERIAL_MODEM_CONTROL, *(uint8_t *) &mcr);

	struct interrupt_identification_register iir = {0};
	iir.interrupt_pending = true;
	tmp = serial_in(base, SERIAL_INT_IDENTIFICATION);
	if (tmp != *(uint8_t *) &iir) {
		// pr_err("port 0x%x: IIR is 0x%x, expected 0x%x\n",
		//        base, tmp, *(uint8_t *) &iir);
		return -ENOTSUP;
	}

	struct line_status_register lsr = {0};
	lsr.empty_transmitter_reg = true;
	lsr.empty_data_reg = true;
	tmp = serial_in(base, SERIAL_LINE_STATUS);
	if (tmp != *(uint8_t *) &lsr) {
		// pr_err("port 0x%x: LSR is 0x%x, expected 0x%x\n",
		//        base, tmp, *(uint8_t *) &lsr);
		return -ENOTSUP;
	}

	struct modem_status_register msr = {0};
	if ((tmp & 0x0F) != *(uint8_t *) &msr) {
		// pr_err("port 0x%x: MSR is 0x%x, expected 0x%x\n",
		//        base, tmp, *(uint8_t *) &msr);
		return -ENOTSUP;
	}

	return 0;
}

int serial_write(uint16_t base, const char * s)
{
	int i = 0;
	while (s[i])
		serial_out(base, SERIAL_BUFFER, s[i++]);
	return i;
}
