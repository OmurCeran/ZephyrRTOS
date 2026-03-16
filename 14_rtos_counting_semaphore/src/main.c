#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>

/*
 * -------------------------------------------------------------------------
 * Counting Semaphore - Parking Lot Demo (Zephyr, multiple threads)
 * -------------------------------------------------------------------------
 *
 * IDEA
 * A counting semaphore represents N identical resources - here, N parking
 * slots. Each "car" thread must take a slot before entering the "lot." If
 * the lot is full (count == 0), cars wait with a timeout and retry later.
 *
 * WHAT THIS SHOWS
 * - Counting semaphore count ∈ [0, limit]. The count starts at CAPACITY,
 * meaning all slots are free. Each take consumes 1; each give returns 1.
 * - At most CAPACITY threads can be inside the critical section at a time.
 * - Timed takes: threads can report "lot full (timeout)" and try again.
 *
 * STRUCTURE
 * - K_SEM_DEFINE(parking_sem, CAPACITY, CAPACITY)
 * - NUM_CARS threads created at the same priority; each loops forever:
 * try-take (with timeout) -> "PARK" (critical section) -> give -> exit
 * The "PARK" time differs per car to make the output dynamic.
 *
 * -------------------------------------------------------------------------
 */

 /*----------------Configuration------------------*/
#define CAPACITY 3 /*parking lot capacity (# of slots)*/
#define NUM_CARS 5 /*number of car threads*/
#define TAKE_TIMEOUT_MS 150 /*time a car waits to take a slot before retrying,how long a car waits before "lot full"*/

/*Each car's park time (inside critical section)*/
static const int park_time_ms[NUM_CARS] = {200, 280, 240, 300, 260};

/*Pause after leaving the lot(outside critical section) , per car*/
static const int cruise_time_ms[NUM_CARS] = {100, 80, 120, 90, 110};

/*---------------Counting Semaphore : N slots in the parking lot------------------*/
/*Initial count = CAPACITY (all slots are free) , limit = CAPACITY(max tokens)*/
K_SEM_DEFINE(parking_sem, CAPACITY, CAPACITY);

/*---------------Observable invariant: number of cars in the lot------------------*/
/*We track "in room" to verify that it never exceeds CAPACITY*/
static atomic_t in_room = ATOMIC_INIT(0);

/*---------------Thread creation variables and stacks------------------*/
#define STACK_SIZE 1024
#define PRIORITY 5

static struct k_thread car_threadsblock[NUM_CARS];
K_THREAD_STACK_ARRAY_DEFINE(car_stacks, NUM_CARS, STACK_SIZE);

/*-----------------Thread entry functions-----------------*/
static void car_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    const int car_id = (int)(intptr_t)p1; /*car ID passed as argument*/
    const int park_time = park_time_ms[car_id];
    const int cruise_time = cruise_time_ms[car_id];

    printk("Car %d: started: park=%d ms , cruise=%d ms , timeout=%d ms\n", car_id, park_time, cruise_time, TAKE_TIMEOUT_MS);

    while(1) {
        printk("Car %d: trying to take a slot with %d ms timeout\n", car_id, TAKE_TIMEOUT_MS);

        /*Try to take a slot with a timeout*/
        int ret = k_sem_take(&parking_sem, K_MSEC(TAKE_TIMEOUT_MS));

        if (ret == 0) {
            /*Enter critical section*/
            int prior = atomic_inc(&in_room);
            /*prior is old value , new value is prior + 1*/
            if (prior + 1 >= CAPACITY) {
                /*This should not happen if the semaphore is working correctly*/
                printk("ERROR! Car %d: in_room was %d\n", car_id, prior + 1);
            }
            printk("Car %d: PARKED in the lot (in_room=%d/%d)\n", car_id, prior + 1, CAPACITY);
            /*Do protected work: park for some time*/
            k_msleep(park_time);

            /*Exit critical section , give slot back to (count++) , allowing others to enter*/
            printk("Car %d: EXITING the lot\n", car_id);
            atomic_dec(&in_room);
            k_sem_give(&parking_sem);

            /*Pause outside of the lot, non critical work outside of the lot*/
            k_msleep(cruise_time);
        } else {
            /*Failed to get in after waiting; do something else instead of entering*/
            printk("Car %d: lot full (timeout:%d ms), will try again,\n", car_id, TAKE_TIMEOUT_MS);
            k_msleep(50); /*wait before retrying*/
        }
    }
}

/*--------------------Thread creation--------------------*/

int main()
{
    /*First start for RTOS*/
    printk("[MAIN] Counting semaphore RTOS demo : CAPACITY = %d , NUM_CARS = %d\n", CAPACITY, NUM_CARS);
    printk("[MAIN] One token == one free slot ; k_sem_give() returns a slot\n");
    printk("[MAIN] Up to %d cars can park concurrently; the rest wait/timeout\n", CAPACITY);

    for (int i = 0; i < NUM_CARS; i++) {
       k_tid_t tid = k_thread_create(&car_threadsblock[i], car_stacks[i], STACK_SIZE,
                        car_entry, (void *)(intptr_t)i, NULL, NULL,
                        PRIORITY, 0, K_NO_WAIT);

       char name[12];
       (void)snprintf(name, sizeof(name), "Car %d", i);
       k_thread_name_set(tid, name);
       k_msleep(10); /*stagger thread starts for clearer output*/
    }

    while(1) {

        k_sleep(K_FOREVER);
    }

    return 0;
}