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
 * seL4 tutorial part 4: application to be run in a process
 */

#include <stdio.h>
#include <assert.h>

#include <simple/simple.h>
#include <simple-default/simple-default.h>
#include <vka/object.h>

#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>

#include <vspace/vspace.h>
#include <sel4/sel4.h>
#include <sel4utils/process.h>

#include <sel4platsupport/timer.h>
#include <platsupport/plat/timer.h>



/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 10)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 100)

/* constants */
#define EP_CPTR SEL4UTILS_FIRST_FREE // where the cap for the endpoint was placed.
#define MSG_DATA 0x2 //  arbitrary data to send
#define READ_COMMAND 0x0
#define WRITE_COMMAND 0X1
#define ACKNOWLEDGE_COMMAND 0X2

seL4_BootInfo *info;
vka_t vka;
simple_t simple;
allocman_t *allocman;
vspace_t vspace;
seL4_timer_t *timer;

#define ALLOCMAN_UT_KERNEL 1

static void add_single_untyped(allocman_t *allocator, vka_t *vka, size_t size, uintptr_t *paddr, seL4_CPtr cap, int type)
{
    cspacepath_t path;
    vka_cspace_make_path(vka,cap,&path);
    int error = allocman_utspace_add_uts(allocator,1,&path,&size,paddr);
}

int main(int argc, char **argv) {
    int error;
    printf("Just entered the user function\n");

    // initialise allocator 
    allocman_t *allocator = bootstrap_use_current_1level(SEL4UTILS_CNODE_SLOT,
                                                         CONFIG_SEL4UTILS_CSPACE_SIZE_BITS, 4,
                                                         (1u << CONFIG_SEL4UTILS_CSPACE_SIZE_BITS), 
                                                            ALLOCATOR_STATIC_POOL_SIZE,
                                                         allocator_mem_pool);

    allocman_make_vka(&vka,allocator);

    size_t size = seL4_WordBits-1;

    add_single_untyped(allocator,&vka,size,NULL,UNTYPED_SLOT,ALLOCMAN_UT_KERNEL);



    vka_object_t aep_object = {0};
    error = vka_alloc_notification(&vka, &aep_object);
    assert(error == 0);

       // get boot info 
    info = seL4_GetBootInfo();

    // init simple 
    simple_default_init_bootinfo(&simple, info);

    seL4_Word sender;
    seL4_Word msg;

    timer = sel4platsupport_get_default_timer(&vka, &vspace, &simple, aep_object.cptr);
    assert(timer != NULL);

    int count = 0;
    while(1) 
    {

        timer_oneshot_relative(timer->timer, 1000 * 1000);
        seL4_Wait(aep_object.cptr, &sender);
        sel4_timer_handle_single_irq(timer);
        count++;
        if (count == 1000 ) break;
    }

    // get the current time 
    msg = timer_get_time(timer->timer);
    printf("%p",msg);



    seL4_MessageInfo_t tag;
    int command;


    seL4_CPtr ep = atoi(argv[0]);

    //printf("User: The argument i got is %p\n", ep);

    tag = seL4_MessageInfo_new(0, 0, 0, 1);

    printf("User: Sending msg %u\n", MSG_DATA);
    int k = 5;

    while(k < 0)
    {
        seL4_SetMR(0, READ_COMMAND);    
        tag = seL4_Call(ep, tag);
        assert(seL4_MessageInfo_get_length(tag) == 1);
        command = seL4_GetMR(0);
        msg = seL4_GetMR(1);
        /////////////////////////////
        // Perform Modification 
        printf("User: The command is %u and value I got from driver is %u\n", command,msg);
        msg = msg + 10;

        //////////////// Send for writing
        seL4_SetMR(0,WRITE_COMMAND);
        seL4_SetMR(1,msg);
        tag = seL4_Call(ep, tag);
        assert(seL4_MessageInfo_get_length(tag) == 1);
        command = seL4_GetMR(0);
        printf("User: After the second call ,The command is %u and value I got from driver is %u\n", command,msg);
        int i =0;

        //Delay Loop
        while(i < 1000000)
        {
            i++;
        }
        k--;

    }
    


    printf("User:Print from the user process\n");



    return 0;
}
