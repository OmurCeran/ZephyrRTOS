#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>
/*
 * -------------------------------------------------------------------------
 * Binary Semaphore
 * -------------------------------------------------------------------------
 *
 * IDEA
 * One binary semaphore models a single "key". Two threads contend for it:
 * whoever successfully k_sem_take()'s the key enters the critical section,
 * does some work, then k_sem_give()'s the key back.
 *
 * WHAT THIS SHOWS
 * - Binary semaphore count ∈ {0,1}. A second k_sem_give() while count==1
 * has no effect (it does not "stack").
 * - Only one thread can be in the critical section at a time.
 * - You can block forever (Thread A), or try with a timeout (Thread B).
 *
 * HOW IT WORKS
 * - key_sem starts at 1 (key is available at boot).
 * - Thread A: blocks forever waiting for the key, then "enters", works,
 * and "exits" (give).
 * - Thread B: attempts to take with a timeout; reports "waiting..." vs
 * "enter", then gives on exit if it got in.
 **************************************************************************/

 #define STACK_SIZE 1024
 #define PRIORITY 5

 #define A_WORK_TIME_MS 150  /*time A spends inside the room*/
 #define A_PAUSE_TIME_MS 50  /*time A spends outside of the room*/
 #define B_WORK_TIME_MS 120  /*time B spends inside the room*/
 #define B_WAIT_TIME_MS 100 /*time B spends waiting for the key*/
 #define B_PAUSE_TIME_MS 50  /*time B spends outside of the room*/


/*Create binary semaphore*/
K_SEM_DEFINE(key_sem, 1, 1);

/*-----------------Shared Variables(visible to both threads)-----------------*/
/*Track "who is inside" and detect violations if any (should never occur)*/
static atomic_t in_room = ATOMIC_INIT(0);

/*Thread creation variables and stacks*/
static struct k_thread thread_a;
static struct k_thread thread_b;
K_THREAD_STACK_DEFINE(thread_a_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_b_stack, STACK_SIZE);

/*Helper functions*/

static inline void work_ms(int ms)
{
    k_msleep(ms);
}

static void thread_a_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    while(1) {
        /*Wait until the key is available;consumes the token (count to 0)*/
        k_sem_take(&key_sem, K_FOREVER);

        /*Enter critical section*/
        int prev = atomic_inc(&in_room);
        if (prev != 0) {
            printk("Thread A: ERROR! in_room was %d\n", prev);
        }
        printk("Thread A: entered the room\n");
        /*Do protected work*/
        work_ms(A_WORK_TIME_MS);

        /*Exit critical section , give key back to (count to 1),allowing others to enter*/
        printk("Thread A: exited the room\n");
        atomic_dec(&in_room);
        k_sem_give(&key_sem);

        /*Pause outside of the room, non critical work outside of the room*/
        work_ms(A_PAUSE_TIME_MS);
    }
}
static void thread_b_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    printk("Thread B: started: will try to take key with %d ms timeout\n", B_WAIT_TIME_MS);

    while(1) {
        /*Try to take the key with a timeout*/
        int ret = k_sem_take(&key_sem, K_MSEC(B_WAIT_TIME_MS));

        if (ret == 0) {
            /*Enter critical section*/
            int prev = atomic_inc(&in_room);
            if (prev != 0) {
                printk("Thread B: ERROR! in_room was %d\n", prev);
            }
            printk("Thread B: entered the room\n");
            /*Do protected work*/
            work_ms(B_WORK_TIME_MS);

            /*Exit critical section , give key back to (count to 1),allowing others to enter*/
            printk("Thread B: exited the room\n");
            atomic_dec(&in_room);
            k_sem_give(&key_sem);

            /*Pause outside of the room, non critical work outside of the room*/
            work_ms(B_PAUSE_TIME_MS);
        } else {
            /*Failed to get in after waiting; do something else instead of entering*/
            printk("Thread B: waiting...\n");
            work_ms(20);
        }
    }
}
int main()
{
    /*First start for RTOS*/
    printk("[MAIN] Binary Semaphore RTOS Thread demo - one key , two threads started\n");

    k_thread_create(&thread_a, thread_a_stack, STACK_SIZE,
                    thread_a_entry, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(&thread_a, "Thread A");

    k_thread_create(&thread_b, thread_b_stack, STACK_SIZE,
                    thread_b_entry, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(&thread_b, "Thread B");

    while(1) {

        k_sleep(K_FOREVER);
    }

    return 0;
}