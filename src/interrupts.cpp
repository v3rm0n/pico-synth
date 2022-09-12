#include "interrupts.hpp"
#include "hardware/gpio.h"
#include "iostream"

using namespace std;

Interrupts *Interrupts::interrupts = nullptr;

Interrupts::Interrupts() {
    Interrupts::interrupts = this;
}

void Interrupts::register_switch(InterruptHandler *handler) {
    handlers[handler->get_pin()] = handler;
    gpio_set_irq_enabled_with_callback(handler->get_pin(), GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true,
                                       &Interrupts::switch_interrupt_handler);
}

void Interrupts::handle(uint gpio, uint32_t events) {
    cout << "Handling state change on pin " << gpio << endl;
    auto handler = handlers[gpio];
    if (handler != nullptr) {
        if ((events & GPIO_IRQ_EDGE_FALL) != 0) {
            cout << "Button was released" << endl;
            handler->handle_release();
        } else {
            cout << "Button was pressed" << endl;
            handler->handle_press();
        }
    }
}
