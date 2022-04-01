#ifndef IRQ_H
#define IRQ_H

int irq_enable(void);
void irq_disable(void);
void irq_mask(unsigned irq);
void irq_unmask(unsigned irq);
void irq_eoi(unsigned irq);

#endif // IRQ_H
