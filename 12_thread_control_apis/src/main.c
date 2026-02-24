#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/*Dynamically thread create */
/* Demonstrate thread control APIs 
    k_thread_create() + K_THREAD_STACK_DEFINE()         : create threads at runtime
    k_thread_name_set()                                 : name threads for clarity
    k_yield()                                           : voluntarily yield the CPU
    k_msleep()/ k_sleep() / k_wakeup()                  : sleep and wakeup threads
    k_thread_suspend() / k_thread_resume()              : suspend and resume threads
    k_thread_abort()                                    : abort threads
    k_thread_priority_set() / k_thread_priority_get()   : set thread priority
*/

/* What happens at runtime
    - Creates two worker threads (A and B) at the *same priority* (to show round-robin scheduling)
    - A controller thread (slightly higher priority) performs timed actions:
    1- Yields (so you see initial fair sharing),
    2- Suspends B (A runs alone)
    3- Resumes B (sharing resumes)
    4- Boosts A's priority (A dominates ; B runs only when A sleeps),
    5- Lowers A's priority (sharing resumes)
    6- Puts B to sleep forever then wakes it with k_wakeup() (B runs alone)
    7- Aborts B (permanently end B)
    -Each action prints to UART so you can see the scheduling effects in the logs

*/

#define STACK_SIZE 1024
#define THREAD_PRIORITY 5

/*-------------------Worker Thread definitions------------------ */
/*Declare stack area for thread A and B*/
K_THREAD_STACK_DEFINE(thread_a_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_b_stack, STACK_SIZE);


/*Declare thread control block area for thread A and B*/
static struct k_thread thread_a_data;
static struct k_thread thread_b_data;

static void thread_a_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "thread_a");

    printk("\r\nThread A started , prio:%d\n", k_thread_priority_get(k_current_get()));
    while(1) {
        printk("Hello from thread A!\n");
        k_msleep(20);
    }
}

static void thread_b_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "thread_b");

    printk("\r\nThread B started , prio:%d\n", k_thread_priority_get(k_current_get()));
    while(1) {
        printk("Hello from thread B!\n");
        k_msleep(20);
    }
}

/*Controller Thread */
K_THREAD_STACK_DEFINE(stack_ctl, STACK_SIZE);
static struct k_thread thread_ctl_data;

static void print_prios()
{
    printk("\r\n[INFO] Thread A prio:%d\n", k_thread_priority_get(&thread_a_data));
    printk("[INFO] Thread B prio:%d\n", k_thread_priority_get(&thread_b_data));
}

static void thread_ctl_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "thread_ctl");

    printk("\r\nThread ctl started , prio:%d\n", k_thread_priority_get(k_current_get()));

    /*Step 0: Initial yield so A & B get turn immediately*/
    printk("[CTL] yielding once to let A & B run\n");
    k_yield();

    /*Step 1: after 1s , suspend B*/
    k_msleep(1000);
    printk("\r\n[CTL] suspending thread B\n");
    k_thread_suspend(&thread_b_data);
    k_msleep(800);
    print_prios();

    /*Step 2: resume B*/
    k_thread_resume(&thread_b_data);
    k_msleep(1000);
    print_prios();
    /*Step 3: boost A's priority above ctl*/
    printk("\r\n[CTL] boosting A's priority above ctl (ctl prio:%d)\n", k_thread_priority_get(&thread_a_data));
    k_thread_priority_set(&thread_a_data, 3);
    k_msleep(1000);
    print_prios();

    /*Step 4: lower A's priority back to normal*/
    printk("\r\n[CTL] lowering A's priority back to normal\n");
    k_thread_priority_set(&thread_a_data, THREAD_PRIORITY);
    k_msleep(1000);
    print_prios();

    /*Step 5: put B to sleep forever then wake it up*/
    printk("\r\n[CTL] putting thread B to sleep forever\n");
    k_thread_suspend(&thread_b_data);
    k_msleep(600);
    printk("\r\n[CTL] waking up thread B\n");
    k_thread_resume(&thread_b_data); /*need to resume before wakeup since the thread is suspended, if we call k_wakeup() while the thread is suspended, it will not wake up the thread and the thread will remain suspended*/
    k_msleep(800);

    /*Step 6: abort thread B*/
    printk("\r\n[CTL] aborting thread B\n");
    k_thread_abort(&thread_b_data);

    printk("[CTL] A still running , B aborted!\n");
    while(1) {
        k_sleep(K_FOREVER);
    }
}

static void start_thread(struct k_thread *tcb , k_thread_stack_t *stack , size_t stack_sz , k_thread_entry_t entry, int prio , const char *name)
{
    k_tid_t tid = k_thread_create(
        tcb,
        stack,
        stack_sz,
        entry,
        NULL,NULL,NULL,
        prio,
        0,
        K_NO_WAIT
    );

    k_thread_name_set(tid,name);
}


int main()
{
    /*First start for RTOS*/
    printk("[MAIN] Thread control API started\n");

    start_thread(&thread_a_data,thread_a_stack,STACK_SIZE,thread_a_entry,THREAD_PRIORITY,"thread_A");

    start_thread(&thread_b_data,thread_b_stack,STACK_SIZE,thread_b_entry,THREAD_PRIORITY,"thread_B");

    start_thread(&thread_ctl_data,thread_b_stack,STACK_SIZE,thread_ctl_entry,4,"CTL");
    while(1) {

        k_sleep(K_FOREVER);
    }

    return 0;
}