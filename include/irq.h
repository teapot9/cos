/**
 * @file irq.h
 * @brief Interrupt management
 */

#ifndef __IRQ_H
#define __IRQ_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enable IRQs
 * @return errno
 */
int irq_enable(void);

/**
 * @brief Disable IRQs
 */
void irq_disable(void);

/**
 * @brief Mask an IRQ
 * @param irq IRQ number
 */
void irq_mask(unsigned irq);

/**
 * @brief Unmask an IRQ
 * @param irq IRQ number
 */
void irq_unmask(unsigned irq);

/**
 * @brief Send an end-of-interrupt
 * @param irq IRQ number
 */
void irq_eoi(unsigned irq);

#ifdef __cplusplus
}
#endif
#endif // __IRQ_H
