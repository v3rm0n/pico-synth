#include <cstdio>

class InterruptHandler {
public:
    virtual uint8_t get_pin() = 0;

    virtual void handle_press() = 0;

    virtual void handle_release() = 0;
};

class Interrupts {
public:
    Interrupts();

    void register_switch(InterruptHandler *handler);

private:
    InterruptHandler *handlers[32];
    static Interrupts *interrupts;

    void handle(uint gpio, uint32_t events);

    static void switch_interrupt_handler(uint gpio, uint32_t events) {
        interrupts->handle(gpio, events);
    }
};