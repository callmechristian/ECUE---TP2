/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/
#include <stdio.h>
#include "includes.h"

/* helper funcs from lab 1 */
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include "sys/alt_timestamp.h"
static int LUT[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};
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
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      1
#define TASK2_PRIORITY      2

OS_EVENT* q;

/* different delays */
int D2 = 5;
float k[6] = {10, 5, 2, 1, 0.5, 0.01};
float D1 = 1 * 10;

/* Task that sends the value of a counter */
void task1(void* pdata)
{
  INT8U error;
  INT32U counter = 0;

  while (1)
  { 
	  printf("k is: %f\n", k[5]);
    printf("Sending from task1: %ld\n", (long int)counter);
    OSQPost(q, (void*) counter);
    counter++;
    OSTimeDlyHMSM(0, 0, D1, 0);
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
    display_seven((int)receivedData);
    OSTimeDlyHMSM(0, 0, D2, 0);
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

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
