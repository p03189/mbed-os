#include "mbed.h"
#include "greentea-client/test_env.h"
#include "rtos.h"

#if defined(MBED_RTOS_SINGLE_THREAD)
  #error [NOT_SUPPORTED] test not supported
#endif

#define QUEUE_SIZE              5
#define THREAD_DELAY            250
#define QUEUE_PUT_ISR_VALUE     128
#define QUEUE_PUT_THREAD_VALUE  127

/*
 * The stack size is defined in cmsis_os.h mainly dependent on the underlying toolchain and
 * the C standard library. For GCC, ARM_STD and IAR it is defined with a size of 2048 bytes
 * and for ARM_MICRO 512. Because of reduce RAM size some targets need a reduced stacksize.
 */
#if (defined(TARGET_EFM32HG_STK3400)) && !defined(TOOLCHAIN_ARM_MICRO)
    #define STACK_SIZE 512
#elif (defined(TARGET_EFM32LG_STK3600) || defined(TARGET_EFM32WG_STK3800) || defined(TARGET_EFM32PG_STK3401)) && !defined(TOOLCHAIN_ARM_MICRO)
    #define STACK_SIZE 768
#elif (defined(TARGET_EFM32GG_STK3700)) && !defined(TOOLCHAIN_ARM_MICRO)
    #define STACK_SIZE 1536
#elif defined(TARGET_MCU_NRF51822) || defined(TARGET_MCU_NRF52832)
    #define STACK_SIZE 768
#elif defined(TARGET_XDOT_L151CC)
    #define STACK_SIZE 1024
#else
    #define STACK_SIZE DEFAULT_STACK_SIZE
#endif

Queue<uint32_t, QUEUE_SIZE> queue;

DigitalOut myled(LED1);

void queue_isr() {

    queue.put((uint32_t*)QUEUE_PUT_ISR_VALUE);
    myled = !myled;
}

void queue_thread(void const *argument) {
    while (true) {
        queue.put((uint32_t*)QUEUE_PUT_THREAD_VALUE);
        Thread::wait(THREAD_DELAY);
    }
}

int main (void) {
    GREENTEA_SETUP(20, "default_auto");

    Thread thread(queue_thread, NULL, osPriorityNormal, STACK_SIZE);
    Ticker ticker;
    ticker.attach(queue_isr, 1.0);
    int isr_puts_counter = 0;
    bool result = true;

    while (true) {
        uint32_t *msg = queue.get();
        if (msg == NULL) {
            printf("QUEUE_GET: FAIL\r\n");
            result = false;
            break;
        } else {
            printf("QUEUE_GET: Value(%u) ... [OK]\r\n", (uint32_t)msg);
            if ((uint32_t)msg == QUEUE_PUT_ISR_VALUE) {
                isr_puts_counter++;
            }
            if (isr_puts_counter >= QUEUE_SIZE) {
                break;
            }
        }
    }

    GREENTEA_TESTSUITE_RESULT(result);
    return 0;
}
