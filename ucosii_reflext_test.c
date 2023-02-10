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

/* helper funcs from LAB 1 */
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

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE]; // button events
OS_STK    task2_stk[TASK_STACKSIZE]; // display value on 7 segments display
OS_STK    task3_stk[TASK_STACKSIZE]; // 
OS_STK    task4_stk[TASK_STACKSIZE]; //

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2
#define TASK3_PRIORITY      3

OS_EVENT* q;

/* Task that sends the value of a counter */
void task1(void* pdata)
{
  INT8U error;
  INT32U counter = 0;

  while (1)
  { 
    printf("Sending from task1: %ld\n", counter);
    OSQPost(q, (void*) counter);
    counter++;
    OSTimeDlyHMSM(0, 0, 1, 0);
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
    printf("Received at task2 from task1: %ld\n", receivedData);

    OSTimeDlyHMSM(0, 0, 2, 0);
  }
}

/* The main function creates the MessageQueue, creates two tasks and starts multi-tasking */
int main(void)
{
   q = OSQCreate(10, sizeof(INT32U));

  OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
               
  OSTaskCreateExt(task2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
  OSStart();
  return 0;
}