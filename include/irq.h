#ifndef IRQ_H
#define IRQ_H
#ifdef __cplusplus
extern "C" {
#endif

int irq_enable(void);
void irq_disable(void);
void irq_mask(unsigned irq);
void irq_unmask(unsigned irq);
void irq_eoi(unsigned irq);

#ifdef __cplusplus
}
#endif
#endif // IRQ_H
