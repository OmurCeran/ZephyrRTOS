#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/*Define stack size in bytes*/
#define THREAD_1_STACK_SIZE 1024

/*Define priority lower number = higher priority*/
#define THREAD_1_PRIORITY 5


static void thread_1_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    while(1) {
        printk("Hello from thread 1!\n");
        k_msleep(2000);
    }
}
/*Define thread*/
K_THREAD_DEFINE(
                        thread_1_tid, /*symbolic thread identifier*/
                        THREAD_1_STACK_SIZE, /*stack size in bytes*/
                        thread_1_entry, /*thread entry function*/
                        NULL, /*parameters to thread entry function*/
                        NULL, /*parameters to thread entry function*/
                        NULL, /*parameters to thread entry function*/
                        THREAD_1_PRIORITY, /*thread priority*/
                        0, /*thread options*/
                        0 /*delay before thread starts*/);

int main()
{
    /*First start for RTOS*/
    printk("RTOS Thread demo started\n");

    while(1) {

        k_sleep(K_FOREVER);
    }

    return 0;
}