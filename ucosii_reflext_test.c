// THE CODE FROM LAB 1
/*
srand(time(NULL));   // Initialization, should only be called once.

void ex_c4() {
	init_button_pio();
	double avg_stats_sum = 0;
	double avg_stats = 0;
	volatile int game_started = 0;
	int	counter = 1;
	while(1) {
		float my_time;

		if(edge_capture == 1) {
			// Returns a pseudo-random integer between 0 and RAND_MAX. // multiply by 1e6 to get the desired ms
			int r = 1 + (rand()%read_nr_switches());
			game_started = 1;

			printf("r is %d\n", r);
			// intialize the timer
			alt_timestamp_start();


			//start the game
//			game_started = true;
			while((alt_timestamp()/alt_timestamp_freq()) < r) {
				printf("%d\n", (alt_timestamp()/alt_timestamp_freq()));
				IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0b01010101010);
				usleep(1000000/2);
				IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0b10101010101);
				usleep(1000000/2);
				printf("%d\n", (alt_timestamp()/alt_timestamp_freq()));
			}

//			alt_timestamp(); // read the timer
//			alt_timestamp_freq() // read the frequence of the timer

			IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0xff);

			// at this point, timer ended and all LEDs are ON, waiting for user to press button 4
			// start a timer
			alt_timestamp_start();

			 edge_capture = 9; // ???
		}

		// if user presses 4, stop the timer and display
		if((log2(edge_capture) == 3 )&& (game_started == 1)) {
			printf("We pressed button 4");
			// save current time into a var
			printf("My time is: %f\n", (double)alt_timestamp()/(double)alt_timestamp_freq());
			// if we don't have stats, don't compute the mean

			if(counter > 1) {
				avg_stats_sum += (double)alt_timestamp()/(double)alt_timestamp_freq();
				avg_stats = avg_stats_sum / counter;
			} else {
				avg_stats_sum = (double)alt_timestamp()/(double)alt_timestamp_freq();
				avg_stats = (double)alt_timestamp()/(double)alt_timestamp_freq();
			}


			// display the current time
			IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0x00);
			display_seven_time((double)alt_timestamp()/(double)alt_timestamp_freq());
			game_started = 0;
			counter += 1;
		}

		// display the stats if user presses button 2 outside of the game
		if(log2(edge_capture) == 1) {
			display_seven_time(avg_stats);
		}
	}
}
*/
// ------------------------------------------------------------

#include <stdio.h>
#include "includes.h"

INT16U taskStartTimestamp;
INT16U OSTmrCtr;

/* stats */
void OSTaskSwHook(void)
{
INT16U taskStopTimestamp, time;
TASK_USER_DATA *puser;
taskStopTimestamp = OSTimeGet();
time =(taskStopTimestamp - taskStartTimestamp) / (OS_TICKS_PER_SEC / 1000); // in ms
puser = OSTCBCur->OSTCBExtPtr;
if (puser != (TASK_USER_DATA *)0) {
puser->TaskCtr++;
puser->TaskExecTime = time;
puser->TaskTotExecTime += time;
}
taskStartTimestamp = OSTimeGet();
}
void OSInitHookBegin(void)
{
 OSTmrCtr = 0;
 taskStartTimestamp = OSTimeGet();
}
void OSTimeTickHook (void)
{
 OSTmrCtr++;
 if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
 OSTmrCtr = 0;
 OSTmrSignal();
 }
}

void OSTmrSignal(void)
{
}

void OSTaskStatHook (void)
{
}

void OSTaskCreateHook (OS_TCB *p_tcb) /* OS_CPU_C.C */
{
#if OS_CFG_APP_HOOKS_EN > 0u
 if (OS_AppTaskCreateHookPtr != (OS_APP_HOOK_TCB)0) { /* Call application hook */
 (*OS_AppTaskCreateHookPtr)(p_tcb);
 }
#endif
}

void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}

void OSInitHookEnd(void)
{
}

void OSTaskIdleHook(void)
{
}

void OSTCBInitHook(OS_TCB *ptcb)
{
}
/* helper funcs from LAB 1 */
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include "sys/alt_timestamp.h"
static int LUT[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};
#define INT_SIZE sizeof(int) * 8 /* Total number of bits in integer */
volatile int edge_capture;
// display number on the seven segment display
void display_seven(int number) {
	int i = 0, numlen = (int)log10(number)+1;
	if(number == 0) {
		numlen = 1;
	}
	for (i = 0; i < 6; i++){
		if(numlen <= i) {
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, 0xff);
		} else {
//			printf("%d\n", number);
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, LUT[number%10]);
		}
		number = number/10;
	}
}
// display number with milliseconds
void display_seven_time(double number) {
	int i = 0;
	int seconds = floor(number);
	int remaining_ms = (number - seconds)*1000;

	int numlen1 = log10(seconds) + 1;
	int numlen2 = 3;

	if(seconds == 0) {
		numlen1 = 1;
	}

	for(i=0; i<6; i++) {
		IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, 0xff);
	}

	// write ms to display
	for (i = 0; i < numlen2; i++){
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, LUT[remaining_ms%10]);
			remaining_ms = remaining_ms/10;
	}

	// write s to display
	for (i = 4; i < 6 - (numlen1 == 1); i++){
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, LUT[seconds%10]);
			seconds = seconds/10;
	}
}
// program interrupts
static void handle_button_interrupts(void* context, alt_u32 id) {
	/* Cast context to edge_capture's type. It is important that this
	be declared volatile to avoid unwanted compiler optimization. */
	volatile int* edge_capture_ptr = (volatile int*) context;
	/* Read the edge capture register on the button PIO. Store value. */
	*edge_capture_ptr =
	IORD_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE);
	/* Write to the edge capture register to reset it. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE, 0);
	/* Read the PIO to delay ISR exit. This is done to prevent a
	spurious interrupt in systems with high processor -> pio
	latency and fast interrupts. */
	IORD_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE);
}
// registering the interrupt handler
static void init_button_pio() {
	/* Recast the edge_capture pointer to match the alt_irq_register() function
	prototype. */
	void* edge_capture_ptr = (void*) &edge_capture;
	/* Enable all 4 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK (KEY_BASE, 0xf);
	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE, 0x0);
	/* Register the ISR. */
	alt_irq_register(KEY_IRQ, edge_capture_ptr,handle_button_interrupts );
}

// count number of 1s in the binary structure of the number
int count_ones(int nr) {
	int zeros, ones, i;

	zeros = 0;
	ones = 0;

	for(i=0; i<INT_SIZE; i++)
	{
		/* If LSB is set then increment ones otherwise zeros */
		if(nr & 1)
			ones++;
		else
			zeros++;

		/* Right shift bits of nr to one position */
		nr >>= 1;
	}

	return ones;
}

// read number of HIGH switches
int read_nr_switches() {
	int data;
	data = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
	printf("Our number %d\n", data);
	printf("Nr of ones %d\n", count_ones(data));
	return count_ones(data);
}
TASK_USER_DATA TaskUserData1[4] ;
/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE]; // button events
OS_STK    task2_stk[TASK_STACKSIZE]; // display value on 7 segments display
OS_STK    task3_stk[TASK_STACKSIZE]; // state handler
OS_STK    task4_stk[TASK_STACKSIZE]; // usage stats handler

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2
#define TASK3_PRIORITY      3
#define TASK4_PRIORITY      4

OS_EVENT* q;

volatile int game_running = 0;
volatile int display_stats = 0;
volatile int wait_for_button = 0;
volatile int button_was_pressed = 0;
INT32U avg_stats = 0;
int avg_stats_sum = 0;
int counter = 1;

/* Task that handles buttons and states */
void task1(void* pdata)
{
//  INT8U error;
//  INT32U counter = 0;

  while (1)
  {
    if(edge_capture == 1) {
    	if(game_running != 1) {
    		game_running = 1;
    		// start the game
    	}
    }
    if(log2(edge_capture) == 1) {
    	if(game_running != 1) {
    		display_stats = 1;
    		// display stats
    	}
    }
    if(log2(edge_capture) == 3){
//    	printf("We pressed button 4");
    	if(game_running == 1 && wait_for_button == 1) {
    		printf("We pressed button 4");
    		game_running = 0;
    		wait_for_button = 0;
    		button_was_pressed = 1;
    		// user pressed stop button
    	}
    }

//    OSQPost(q, (void*) counter);
//    counter++;
    edge_capture = 9;
    OSTimeDlyHMSM(0, 0, 0, 1);
  }
}

/* Task that displays the received value on the 7-segments */
void task2(void* pdata)
{
  INT8U error;
  INT32U receivedData;

  while (1)
  {
    receivedData = (INT32U) OSQPend(q, 0, &error);
//    printf("\nWe received: %d", (int)receivedData);
    display_seven_time((double)receivedData/1000); // print to 7 segm
//    printf("Received at task2 from task1: %ld\n", receivedData);
    OSTimeDlyHMSM(0, 0, 0, 1);
  }
}

void task3(void* pdata)
{
//  INT8U error;
//  INT32U receivedData;

  while (1)
  {
//    receivedData = (INT32U) OSQPend(q, 0, &error);
//    displaySeven((int)receivedData); // print to 7 segm
//    printf("Received at task2 from task1: %ld\n", receivedData);
	if(game_running == 1 && wait_for_button != 1) {
		int r = 1 + (rand()%read_nr_switches())*1000;
		OSTimeSet(0);
		while(OSTimeGet() < r) {
//			printf("%d\n", OSTimeGet());
			IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0b01010101010);
			usleep(1000000/2);
			IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0b10101010101);
			usleep(1000000/2);
//			printf("%d\n", OSTimeGet);
		}
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0xff);
		wait_for_button = 1;
		OSTimeSet(0);
	}


//	OSQPost(q, (void*) OSTimeGet());

	if(button_was_pressed == 1) {
		INT32U my_time = (INT32U)OSTimeGet();
		avg_stats_sum = avg_stats_sum + OSTimeGet();
		avg_stats = (INT32U)avg_stats_sum/counter;
		counter = counter + 1;

		printf("My time was: %d", (int)my_time);
		OSQPost(q, (void*) my_time);
		button_was_pressed = 0;
	}

	if(display_stats == 1) {
		OSQPost(q, (void*) avg_stats);
		display_stats = 0;
	}
    OSTimeDlyHMSM(0, 0, 0, 1);
  }
}

void task4(void* pdata)
{

  while (1)
  {
	  OS_STK_DATA task_data;

	 if(OSTaskStkChk(TASK1_PRIORITY, &task_data) == OS_NO_ERR) {
		 printf("T1 OSFree: %d free bytes ", (int)task_data.OSFree);
		 printf("OSUsed %d used bytes \n", (int)task_data.OSUsed);
	 } else {
		 printf("Taks stack check failed.\n");
	 }
	 if(OSTaskStkChk(TASK2_PRIORITY, &task_data) == OS_NO_ERR) {
		 printf("T2 OSFree: %d free bytes ", (int)task_data.OSFree);
		 printf("OSUsed %d used bytes \n", (int)task_data.OSUsed);
	 } else {
		 printf("Taks stack check failed.\n");
	 }
	 if(OSTaskStkChk(TASK3_PRIORITY, &task_data) == OS_NO_ERR) {
		 printf("T3 OSFree: %d free bytes ", (int)task_data.OSFree);
		 printf("OSUsed %d used bytes \n", (int)task_data.OSUsed);
	 } else {
		 printf("T4 Taks stack check failed.\n");
	 }
	 if(OSTaskStkChk(TASK4_PRIORITY, &task_data) == OS_NO_ERR) {
	 		 printf("T4 OSFree: %d free bytes ", (int)task_data.OSFree);
	 		 printf("OSUsed %d used bytes \n", (int)task_data.OSUsed);
	 	 } else {
	 		 printf("T4 Taks stack check failed.\n");
	 	 }

	 for(int i = 0; i < 4; i++) {
		 printf("TASK%d - ", i+1);
		 printf("Execs: %d:", TaskUserData1[i].TaskCtr);
		 printf("Exec time: %d ", TaskUserData1[i].TaskExecTime);
		 printf("Total exec time: %d\n", TaskUserData1[i].TaskTotExecTime);
	 }
    OSTimeDlyHMSM(0, 0, 1, 0);
    printf("\n CPU utilization: %d%%\n", (int)OSCPUUsage);
    OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

/* The main function creates the MessageQueue, creates two tasks and starts multi-tasking */
int main(void)
{
   q = OSQCreate(10, sizeof(INT32U));

   init_button_pio();

  OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  &TaskUserData1[0],
                  OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
              
               
  OSTaskCreateExt(task2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  &TaskUserData1[1],
                  OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

  OSTaskCreateExt(task3,
                    NULL,
                    (void *)&task3_stk[TASK_STACKSIZE-1],
                    TASK3_PRIORITY,
                    TASK3_PRIORITY,
                    task3_stk,
                    TASK_STACKSIZE,
                    &TaskUserData1[2],
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

  OSTaskCreateExt(task4,
                      NULL,
                      (void *)&task4_stk[TASK_STACKSIZE-1],
                      TASK4_PRIORITY,
                      TASK4_PRIORITY,
                      task4_stk,
                      TASK_STACKSIZE,
                      &TaskUserData1[3],
                      OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);

  OSStart();
  return 0;
}
