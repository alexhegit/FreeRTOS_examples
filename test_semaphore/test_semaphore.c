
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"

#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define DELAY_2_SECOND		2000UL
#define TIMER_CHECK_THRESHOLD	9

static TaskHandle_t xTask_1 = NULL;
static TimerHandle_t xTimer = NULL;

long taskCntr = 0;
long semaphoreCntr = 0;



static SemaphoreHandle_t xSemaphore = NULL;


/*-----------------------------------------------------------*/
static void vTask_1(void *pvParameters)
{
	for ( ;; )
	{
        /* Block waiting for the semaphore to become available. */
        if( xSemaphoreTake( xSemaphore, (TickType_t) 10 ) == pdTRUE )
        {
        	semaphoreCntr++;
        	xil_printf( "Freertos vTask_1, SemaphoreCntr=%d\r\n", semaphoreCntr);
        }
    	taskCntr++;
	}
}


/*-----------------------------------------------------------*/
static void vTimerCallback( TimerHandle_t pxTimer )
{
	long lTimerId;
	configASSERT( pxTimer );

	static unsigned char ucLocalTickCount = 0;

	static BaseType_t xHigherPriorityTaskWoken;

	lTimerId = ( long ) pvTimerGetTimerID( pxTimer );

	if (lTimerId != TIMER_ID) {
		xil_printf("FreeRTOS Semaphore Example FAILED\r\n");
	}

	/* Is it time for vATask() to run? */
	xHigherPriorityTaskWoken = pdFALSE;
	ucLocalTickCount++;

	xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );

	/* If the RxtaskCntr is updated every time the Rx task is called. The
	 Rx task is called every time the Tx task sends a message. The Tx task
	 sends a message every 1 second.
	 The timer expires after 10 seconds. We expect the RxtaskCntr to at least
	 have a value of 9 (TIMER_CHECK_THRESHOLD) when the timer expires. */
/*
	if (semaphoreCntr >= TIMER_CHECK_THRESHOLD) {
		xil_printf("FreeRTOS Semaphore Example PASSED, taskCntr=%d\r\n", taskCntr);
	} else {
		xil_printf("FreeRTOS Semaphore Example FAILED, taskCntr=%d\r\n, taskCntr");
	}
*/
    /* If xHigherPriorityTaskWoken was set to true you
    we should yield.  The actual macro used here is
    port specific. */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    if (ucLocalTickCount > 10)
    {
    	xTimerStopFromISR(pxTimer, &xHigherPriorityTaskWoken);
    	xil_printf("Stop timer after 10s\r\n");
    }

}

int main(void)
{
	const TickType_t xseconds = pdMS_TO_TICKS( DELAY_1_SECOND );

	xil_printf( "\r\n *** Freertos semaphpore example main *** \r\n" );

	BaseType_t xReturned;

    /* We are using the semaphore for synchronisation so we create a binary
    semaphore rather than a mutex.  We must make sure that the interrupt
    does not attempt to use the semaphore before it is created! */
    xSemaphore = xSemaphoreCreateBinary();

	/* Create the task, storing the handle. */
	xReturned = xTaskCreate(
			vTask_1,       /* Function that implements the task. */
			( const char * ) "vTask_1",          /* Text name for the task. */
			configMINIMAL_STACK_SIZE,      /* Stack size in words, not bytes. */
			NULL,    /* Parameter passed into the task. */
			tskIDLE_PRIORITY,/* Priority at which the task is created. */
			&xTask_1 );      /* Used to pass out the created task's handle. */

	xTimer = xTimerCreate(
			(const char *) "Timer",
			xseconds,
			pdTRUE, //pdTRUE for auto reload
			(void *) TIMER_ID,
			vTimerCallback);

	/* Check the timer was created. */
	configASSERT( xTimer );

	/* start the timer with a block time of 0 ticks. This means as soon
	   as the schedule starts the timer will start running and will expire after
	   10 seconds */
	xTimerStart( xTimer, 0 );

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}
