/*********************************FILE__HEADER*********************************\
* File:	pqueue.c						 		  								
* Author:	Daniel Brodsky					  								
* Date:	16/04/2021							   								
* Version:	1.0	(Pre-Review)						
* Reviewer:	Olga						   								
* Description:	Implementation of a task scheduler.		 
\******************************************************************************/

/********************************* Inclusions *********************************/
#include <assert.h>				/*	assert			*/
#include <stddef.h>				/*	size_t, NULL	*/
#include <stdlib.h>				/*	malloc, free	*/

#include "UID.h"				/*	UIDCreate, UIDIsEqual, UIDGetBadUID		*/
#include "utils.h"				/*	status_ty, boolean_ty					*/
#include "operation_func.h"		/*	operation_func_ty definition			*/

/***************************** Macros Definitions *****************************/
#define VOID_PTR_TO_TASK_PTR(void_ptr) ((task_ty *)void_ptr)
#define TIME_TO_INT(time) (int)(long)(time)
#define CURRENT_TIME time(0)

/**************************** Forward Declarations ****************************/
int SortTasks(const void *task1, const void *task2);
bolean_ty MatchUIDs(const void *task, const void *uid2);

/***************************** Struct__Definition *****************************/
struct scheduler
{
	p_queue_ty *tasks;				/*	a queue which stores the
										scheduler's tasks and is sorted 
										by their expected time to run.	*/
											
	boolean_ty to_stop;				/*	a flag which determines if the 
										scheduler should be stopped.	*/
};
/******************************************************************************/
/************************* Functions__Implementations *************************/
/******************************************************************************/
scheduler_ty *SchedulerCreate(void)
{
	scheduler_ty new_scheduler = (scheduler_ty *)malloc(sizeof(scheduler_ty));
	if (NULL == new_scheduler)
	{
		return (NULL);
	}
	
	new_scheduler->tasks = PqueueCreate(SortTasks);
	if (NULL == scheduler->tasks)
	{
		free(new_scheduler)
		new_scheduler = NULL;
		
		return (NULL);
	}
	
	new_scheduler->to_stop = FALSE;
	
	return (new_scheduler);
}
/******************************************************************************/
void SchedulerDestroy(scheduler_ty *scheduler)
{
	if (NULL != scheduler)
	{
		if (!SchedulerIsEmpty(scheduler))
		{
			SchedulerClear(scheduler);
		}
			
		PqueueDestroy(scheduler->pqueue);
		scheduler->pqueue = NULL;
				
		free(scheduler);
		scheduler = NULL;
	}
}
/******************************************************************************/
ilrd_uid_ty SchedulerAdd(scheduler_ty *scheduler,
			operation_func_ty operation_func,  size_t interval, void *param)
{
	task_ty new_task = NULL;
	
	assert(scheduler);
	assert(operation_func);
	
	new_task = TaskCreate(operation_func, interval, CURRENT_TIME, param);

	/*	if the task was successfully created and added to the scheduler		*/
	if (new_task && (SUCCESS == PqueueEnqueue(scheduler->tasks, new_task)))
	{
		return (TaskGetUid(new_task))
	}
	else
	{
		return (UIDGetBadUID()); /*	failed to create or to add the task	*/
	}
}
/******************************************************************************/
status_ty SchedulerRemove(scheduler_ty *scheduler, ilrd_uid_ty uid)
{
	ilrd_uid_ty uid_to_remove = uid;
	
	assert(scheduler);
	assert(!SchedulerIsEmpty);
	
	if (!UIDIsEqual(UIDGetBadUID, uid))	/* is the received UID valid?	*/ 
	{
		void *task_to_remove = PqueueErase(scheduler->tasks,
													MatchUIDs, &uid_to_remove);

		if(NULL != task_to_remove) /* task has been found in the scheduler	*/
		{		
			TaskDestroy(task_to_remove);
			return (SUCCESS);	/* the task was successfully removed	*/
		}
	}
	
	return (FAILURE);
}
/******************************************************************************/
run_status_ty SchedulerRun(scheduler_ty *scheduler)
{
	task_ty *task_to_run;
	oper_ret_ty ret_status = 0;

	assert(scheduler);

	while(!SchedulerIsEmpty(scheduler) && FALSE == scheduler->to_stop)
	{
		task_to_run = (task_ty *)PqueueDequeue(scheduler->tasks);

		/*	check if the expected time to run of the task is yet to come
			and pause the process until the time is right	*/
		while (TaskGetTimeToRun(task_to_run) > CURRENT_TIME)
		{
			sleep(TaskGetTimeToRun(task_to_run) - CURRENT_TIME);
		}
		
		ret_status = TaskRun(task_to_run);

		switch(ret_status)
		{
			case NOT_DONE:
				TaskSetTimeToRun(task_to_run, CURRENT_TIME);
				if (SUCCESS == PqueueEnqueue(scheduler->tasks, task_to_run))
				{
					break;
				}
				else
				{
					return (SCH_FAILURE);
				}

			case DONE:
				TaskDestroy(task_to_run);
				break;

			case FAILURE:
				TaskDestroy(task_to_run);
				return (FUNC_FAILURE);
		}	
	}
	
	if (SchedulerIsEmpty(scheduler))
		{
			return	(FINISHED);
		}
		else
		{
			return (STOPPED);
		}
}
/******************************************************************************/
void SchedulerStop(scheduler_ty *scheduler)
{
	assert(scheduler);
	
	scheduler->to_stop = TRUE;
}
/******************************************************************************/ 
size_t SchedulerSize(const scheduler_ty *scheduler)
{
	assert(scheduler);
	
	return (PqueueSize(scheduler->tasks));
}
/******************************************************************************/
boolean_ty SchedulerIsEmpty(const scheduler_ty *scheduler)
{
	return (PqueueIsEmpty(scheduler->tasks));
}
/******************************************************************************/
void SchedulerClear(scheduler_ty *scheduler)
{
	assert(scheduler);
	
	while (!SchedulerIsEmpty)
	{
		TaskDestroy(PqueuePeek(scheduler->tasks));
		PqueueDequeue(scheduler->tasks);
	}
}
/******************************************************************************/
/*	Returns a positive value if task2 has an
	earlier expected time to run than task1	*/
int SortTasks(const void *task1, const void *task2)
{
	time_t expected_time_task1 = TaskGetTimeToRun(VOID_PTR_TO_TASK_PTR(task1));
	time_t expected_time_task2 = TaskGetTimeToRun(VOID_PTR_TO_TASK_PTR(task2));
	
	return	(TIME_TO_INT(expected_time_task1 - expected_time_task2));
}
/******************************************************************************/
/* first paramater is each element's data in the scheduler which means a task.
	second paramater is an address of a valid uid	*/
bolean_ty MatchUIDs(const void *task, const void *uid)
{
	ilrd_uid_ty uid_of_task = TaskGetUid(task);
	ilrd_uid_ty uid_to_find = *(ilrd_uid_ty *)uid;
	
	return (UIDIsEqual(uid_of_task, uid_to_find));
}
/******************************************************************************/
