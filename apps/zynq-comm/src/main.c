/*
 * Copyright 2015, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

/*
 * seL4 tutorial part 4: create a new process and IPC with it
 */


/* Include Kconfig variables. */
#include <autoconf.h>

#include <stdio.h>
#include <assert.h>

#include <sel4/sel4.h>

#include <simple/simple.h>
#include <simple-default/simple-default.h>

#include <vka/object.h>

#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>

#include <vspace/vspace.h>

#include <sel4utils/vspace.h>
#include <sel4utils/mapping.h>
#include <sel4utils/process.h>

#include <sel4platsupport/timer.h>
#include <platsupport/plat/timer.h>

/* constants */
#define EP_BADGE 0x61 // arbitrary (but unique) number for a badge
#define MSG_DATA 0x6161 // arbitrary data to send

#define USER_PRIORITY seL4_MaxPrio
#define DRIVER_PRIORITY seL4_MaxPrio
#define USER_IMAGE_NAME "zynq-comm-user"
#define DRIVER_IMAGE_NAME "zynq-comm-driver"
#define NUM_ARGS 1

/* global environment variables */
seL4_BootInfo *info;
simple_t simple;
vka_t vka;
allocman_t *allocman;
vspace_t vspace;
seL4_timer_t *timer;

/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 10)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 100)

/* static memory for virtual memory bootstrapping */
UNUSED static sel4utils_alloc_data_t data;

/* stack for the new thread */
#define THREAD_2_STACK_SIZE 4096
UNUSED static int thread_2_stack[THREAD_2_STACK_SIZE];

/* convenience function */
extern void name_thread(seL4_CPtr tcb, char *name);

int main(void)
{
    UNUSED int error;

    /* give us a name: useful for debugging if the thread faults */
    name_thread(seL4_CapInitThreadTCB, "zynq-comm");

    /* get boot info */
    info = seL4_GetBootInfo();

    /* init simple */
    simple_default_init_bootinfo(&simple, info);

    /* print out bootinfo and other info about simple */
    simple_print(&simple);

    /* create an allocator */
    allocman = bootstrap_use_current_simple(&simple, ALLOCATOR_STATIC_POOL_SIZE,
        allocator_mem_pool);
    assert(allocman);

    /* create a vka (interface for interacting with the underlying allocator) */
    allocman_make_vka(&vka, allocman);

   /* create a vspace object to manage our vspace */
    error = sel4utils_bootstrap_vspace_with_bootinfo_leaky(&vspace,
        &data, simple_get_pd(&simple), &vka, info);

    /* fill the allocator with virtual memory */
    void *vaddr;
    UNUSED reservation_t virtual_reservation;
    virtual_reservation = vspace_reserve_range(&vspace,
        ALLOCATOR_VIRTUAL_POOL_SIZE, seL4_AllRights, 1, &vaddr);
    assert(virtual_reservation.res);
    bootstrap_configure_virtual_pool(allocman, vaddr,
        ALLOCATOR_VIRTUAL_POOL_SIZE, simple_get_pd(&simple));


     printf("Going to work on configuring processes \n");
   /* use sel4utils to make a new process */
   sel4utils_process_t user_process;
   error = sel4utils_configure_process(&user_process, &vka, &vspace, USER_PRIORITY, USER_IMAGE_NAME);
   assert(error == 0);

    // give the new process's thread a name 
    name_thread(user_process.thread.tcb.cptr, "user-app: Production Controller");

    
    sel4utils_process_t driver_process;
    error = sel4utils_configure_process(&driver_process, &vka , &vspace, DRIVER_PRIORITY , DRIVER_IMAGE_NAME);
    
    // give the new process's thread a name 
    name_thread(driver_process.thread.tcb.cptr, "driver: Device Driver");
    assert(error == 0);
    printf("Spawning Processes now \n");

    //copy the capabilities
     /* create an endpoint */
    vka_object_t ep_object = {0};
    error = vka_alloc_endpoint(&vka, &ep_object);
    assert(error == 0);

    /*
     * make a badged endpoint in the new process's cspace.  This copy 
     * will be used to send an IPC to the original cap
     */

    /* make a cspacepath for the new endpoint cap */
    cspacepath_t ep_cap_path;
    seL4_CPtr user_ep_cap,driver_ep_cap;
    vka_cspace_make_path(&vka, ep_object.cptr, &ep_cap_path);

    /* copy the endpont cap and add a badge to the new cap */
    user_ep_cap = sel4utils_copy_cap_to_process(&user_process, ep_cap_path);
    assert(user_ep_cap != 0);

    driver_ep_cap = sel4utils_copy_cap_to_process(&driver_process, ep_cap_path);
    assert(driver_ep_cap != 0);

    char *user_argv[NUM_ARGS];
    char *driver_argv[NUM_ARGS];
    char user_argv_strings[NUM_ARGS][WORD_STRING_SIZE];
    char driver_argv_strings[NUM_ARGS][WORD_STRING_SIZE];




    sel4utils_create_word_args(user_argv_strings, user_argv, NUM_ARGS, user_ep_cap);
    sel4utils_create_word_args(driver_argv_strings, driver_argv, NUM_ARGS, driver_ep_cap);



    // spawn the processes 
    error = sel4utils_spawn_process_v(&user_process, &vka, &vspace, NUM_ARGS, user_argv, 1);
    assert(error == 0);
    error = sel4utils_spawn_process_v(&driver_process, &vka, &vspace, NUM_ARGS, driver_argv, 1);
    assert(error == 0);




    printf ("Root Task : Completing Now \n");

    return 0;
}
