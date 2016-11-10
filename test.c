/*
 * This test application is to read/write data directly from/to the device
 * from userspace.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#define IN 0
#define OUT 1

#define XTTCPS_CLK_CNTRL_OFFSET     0x00000000  /**< Clock Control Register */
#define XTTCPS_CNT_CNTRL_OFFSET     0x0000000C/4  /**< Counter Control Register*/
#define XTTCPS_COUNT_VALUE_OFFSET   0x00000018/4  /**< Current Counter Value */
#define XTTCPS_INTERVAL_VAL_OFFSET  0x00000024/4  /**< Interval Count Value */
#define XTTCPS_MATCH_0_OFFSET       0x00000030/4  /**< Match 1 value */
#define XTTCPS_MATCH_1_OFFSET       0x0000003C/4  /**< Match 2 value */
#define XTTCPS_MATCH_2_OFFSET       0x00000048/4  /**< Match 3 value */
#define XTTCPS_ISR_OFFSET       0x00000054/4  /**< Interrupt Status Register */
#define XTTCPS_IER_OFFSET       0x00000060/4  /**< Interrupt Enable Register */


#define XTTCPS_CNT_CNTRL_DIS_MASK   0x00000001 /**< Disable the counter */
#define XTTCPS_CNT_CNTRL_INT_MASK   0x00000002 /**< Interval mode */
#define XTTCPS_CNT_CNTRL_DECR_MASK  0x00000004 /**< Decrement mode */
#define XTTCPS_CNT_CNTRL_MATCH_MASK 0x00000008 /**< Match mode */
#define XTTCPS_CNT_CNTRL_RST_MASK   0x00000010 /**< Reset counter */
#define XTTCPS_CNT_CNTRL_EN_WAVE_MASK   0x00000020 /**< Enable waveform */
#define XTTCPS_CNT_CNTRL_POL_WAVE_MASK  0x00000040 /**< Waveform polarity */
#define XTTCPS_CNT_CNTRL_RESET_VALUE    0x00000021 /**< Reset value */

/** @name Interrupt Registers
 * Following register bit mask is for all interrupt registers.
 *
 * @{
 */
#define XTTCPS_IXR_INTERVAL_MASK    0x00000001  /**< Interval Interrupt */
#define XTTCPS_IXR_MATCH_0_MASK     0x00000002  /**< Match 1 Interrupt */
#define XTTCPS_IXR_MATCH_1_MASK     0x00000004  /**< Match 2 Interrupt */
#define XTTCPS_IXR_MATCH_2_MASK     0x00000008  /**< Match 3 Interrupt */
#define XTTCPS_IXR_CNT_OVR_MASK     0x00000010  /**< Counter Overflow */
#define XTTCPS_IXR_ALL_MASK     0x0000001F  /**< All valid Interrupts */

#define XTTCPS_CLK_CNTRL_PS_EN_MASK 0x00000001  /**< Prescale enable */
#define XTTCPS_CLK_CNTRL_PS_VAL_MASK    0x0000001E  /**< Prescale value */
#define XTTCPS_CLK_CNTRL_PS_VAL_SHIFT       1   /**< Prescale shift */
#define XTTCPS_CLK_CNTRL_PS_DISABLE     16  /**< Prescale disable */
#define XTTCPS_CLK_CNTRL_SRC_MASK   0x00000020  /**< Clock source */
#define XTTCPS_CLK_CNTRL_EXT_EDGE_MASK  0x00000040  /**< External Clock edge */

#define BIT(x)              (1 << (x))
#define CNT_WIDTH 16
#define CNT_MAX (BIT(CNT_WIDTH) - 1)


void init_timer(unsigned int * ttc_ptr)
{

	  //Stop Timer
	    *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) |=  XTTCPS_CNT_CNTRL_DIS_MASK;

	    printf("The value of cnt cntrl offset is %u \n", *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET));

	    //Cfg Initialize function

	    *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) = XTTCPS_CNT_CNTRL_RESET_VALUE;
	    *(ttc_ptr + XTTCPS_CLK_CNTRL_OFFSET) = 0;
	    *(ttc_ptr + XTTCPS_INTERVAL_VAL_OFFSET) = 0;
	    *(ttc_ptr + XTTCPS_MATCH_0_OFFSET) = 0;
	    *(ttc_ptr + XTTCPS_MATCH_1_OFFSET) = 0;
	    *(ttc_ptr + XTTCPS_MATCH_2_OFFSET) = 0;
	    *(ttc_ptr + XTTCPS_MATCH_2_OFFSET) = 0;
	    *(ttc_ptr + XTTCPS_IER_OFFSET)     = 0;
	    *(ttc_ptr + XTTCPS_ISR_OFFSET)     = XTTCPS_IXR_ALL_MASK;

	    printf("The value of cnt cntrl offset is %u \n", *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET));

	    *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) |= XTTCPS_CNT_CNTRL_RST_MASK;

	    printf("The value of cnt cntrl offset is %u \n", *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET));




	    //Need to setup options and prescaler
		*(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) = 0x22;

		printf("The value of cnt cntrl offset is %u \n", *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET));

	    int temp = *(ttc_ptr + XTTCPS_CLK_CNTRL_OFFSET);


	    int PrescalerValue = 4;

	    temp &= ~(XTTCPS_CLK_CNTRL_PS_VAL_MASK | XTTCPS_CLK_CNTRL_PS_EN_MASK);

	    if (PrescalerValue < XTTCPS_CLK_CNTRL_PS_DISABLE)
	    {

	        //Set the prescaler value and enable prescaler
	        //


	        temp |= (PrescalerValue << XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) & XTTCPS_CLK_CNTRL_PS_VAL_MASK;
	        temp |= XTTCPS_CLK_CNTRL_PS_EN_MASK;
	    }


	     //* Write the register with the new values.


	    *(ttc_ptr + XTTCPS_CLK_CNTRL_OFFSET) = temp;

	    *(ttc_ptr + XTTCPS_INTERVAL_VAL_OFFSET) = 65000;

}

void start_timer(unsigned int * ttc_ptr)
{
	*(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) =  *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) & ~XTTCPS_CNT_CNTRL_DIS_MASK;
}

int main(int argc, char *argv[])
{
	int fd;
	unsigned int gpio_addr = 0x41200000;
	unsigned int ttc_addr = 0xF8001000;

	unsigned int *gpio_ptr, *ttc_ptr;
	unsigned page_size=sysconf(_SC_PAGESIZE);
	int value,current_counter,temp;


	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR);
	if (fd < 1) {
		perror(argv[0]);
		return -1;
	}

	/* mmap the device into memory */
	gpio_ptr = (unsigned *)mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, gpio_addr);
	ttc_ptr = (unsigned *)mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, ttc_addr);

	init_timer(ttc_ptr);

	  //Stop Timer
	    *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET) |=  XTTCPS_CNT_CNTRL_DIS_MASK;

	    printf("The value of cnt cntrl offset is %u \n", *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET));

	start_timer(ttc_ptr);

	printf("The value of cnt cntrl offset is %u \n", *(ttc_ptr + XTTCPS_CNT_CNTRL_OFFSET));

	while(1)
	{
		current_counter = *(ttc_ptr + XTTCPS_COUNT_VALUE_OFFSET);

		printf("Before read %d \n", current_counter);

		//*(gpio_ptr + 0xC) = 1;  //Set Direction to Input
		//value = *(gpio_ptr+ 0x8);

		*(gpio_ptr + 0x3) = 1;  //Set Direction to Input
		temp = *(gpio_ptr+ 0x2);

		current_counter = *(ttc_ptr + XTTCPS_COUNT_VALUE_OFFSET);
		printf("After read %u \n", current_counter);

		*(gpio_ptr) = temp;

		current_counter = *(ttc_ptr + XTTCPS_COUNT_VALUE_OFFSET);
		printf("After Write %u \n", current_counter);

	}



	munmap(gpio_ptr, page_size);
	munmap(ttc_ptr, page_size);

	return 0;
}


/*
 * 	//for(count=0;  count <5; count ++)
//	{
		//volatile unsigned current_counter = *((unsigned *)(ttc_ptr + XTTCPS_COUNT_VALUE_OFFSET));
//		time_t timer1, timer2;
//		time(&timer1);

		//printf("Before read %lu s %lu ns\n", tp->tv_sec, tp->tv_nsec);

		*((unsigned *)(gpio_ptr + 0xC)) = 1;  //Set Direction to Input
		value = *((unsigned *)(gpio_ptr+ 0x8));

		//current_counter = *((unsigned *)(ttc_ptr + XTTCPS_COUNT_VALUE_OFFSET));

		//printf("After Read %u\n", current_counter);
		//time(&timer2);

		//	printf("After read %lu s \n", difftime(timer1, timer2));


		*((unsigned *)(gpio_ptr)) = value;

		//current_counter = *((unsigned *)(ttc_ptr + XTTCPS_COUNT_VALUE_OFFSET));

		//printf("After Write %u \n", current_counter);

		//ret = clock_gettime(CLOCK_MONOTONIC, tp);

			//printf("After Write %lu s %lu ns\n", tp->tv_sec, tp->tv_nsec);
	//}
 */

