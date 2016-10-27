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

#include <sel4/sel4.h>
#include <sel4utils/process.h>
#include <vspace/vspace.h>

vka_t vka;
vspace_t vspace;

/* constants */
#define EP_CPTR SEL4UTILS_FIRST_FREE // where the cap for the endpoint was placed.
#define MSG_DATA 0x2 //  arbitrary data to send
#define READ_COMMAND 0x0
#define WRITE_COMMAND 0X1
#define ACKNOWLEDGE_COMMAND 0X2


/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 10)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* dimensions of virtual memory for the allocator to use */
#define ALLOCATOR_VIRTUAL_POOL_SIZE ((1 << seL4_PageBits) * 100)

/* static memory for virtual memory bootstrapping */
UNUSED static sel4utils_alloc_data_t data;


int main(int argc, char **argv) {
        
    printf("Driver:Just entered the driver function\n");
   /* int error;
    
    allocman = bootstrap_use_current_1level(SEL4UTILS_CNODE_SLOT,
                                             CONFIG_SEL4UTILS_CSPACE_SIZE_BITS,
                                             0,
                                             (1u << CONFIG_SEL4UTILS_CSPACE_SIZE_BITS),
                                             ALLOCATOR_STATIC_POOL_SIZE,
                                             allocator_mem_pool);


    allocman_make_vka(&vka, allocman);




    vka_object_t ep_object = {0};
    error = vka_alloc_endpoint(&vka, &ep_object);
    ZF_LOGF_IFERR(error, "Failed to allocate new endpoint object.\n");

    cspacepath_t ep_cap_path;
    seL4_CPtr new_ep_cap;
    vka_cspace_make_path(&vka, ep_object.cptr, &ep_cap_path);

    //new_ep_cap = sel4utils_mint_cap_to_process(&new_process, ep_cap_path,
    //    seL4_AllRights, seL4_CapData_Badge_new(EP_BADGE));
    //ZF_LOGF_IF(new_ep_cap == 0, "Failed to mint a badged copy of the IPC endpoint into the new thread's CSpace.\n"
    //    "\tsel4utils_mint_cap_to_process takes a cspacepath_t: double check what you passed.\n");

    //printf("NEW CAP SLOT: %x.\n", ep_cap_path.capPtr);*/

    //seL4_Word sender_badge;
    seL4_MessageInfo_t tag;
    seL4_Word msg;
    int command;

    seL4_CPtr ep = atoi(argv[0]);
    printf("Driver: The argument is %p\n", ep);

    int k = 5;
    while(k<5)
    {
        /* wait for a message */
        tag = seL4_Recv(ep, NULL);

        /* make sure it is what we expected */
        assert(seL4_MessageInfo_get_length(tag) == 1);

        command = seL4_GetMR(0);
        printf("Driver: got a message which is %u \n", msg);

        if(command == 0)
        {
            printf("Driver: Reading Value from the hardware now which is %d\n",MSG_DATA);
            seL4_SetMR(0,ACKNOWLEDGE_COMMAND);
            seL4_SetMR(1,MSG_DATA);
            seL4_ReplyRecv(ep, tag, NULL);
        }
        if(command == 1)
        {
            msg = seL4_GetMR(1);
            seL4_SetMR(0,ACKNOWLEDGE_COMMAND);
            seL4_ReplyRecv(ep, tag, NULL);
            printf("Driver: This is where value %u is sent to hardware\n", msg);
        }
        k--;
    }


    printf("Driver: Print from the driver process\n");



    return 0;
}
