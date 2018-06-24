#ifndef TASK_WRITE_H
#define TASK_WRITE_H
// UMSATS - CDH Scheduler 
//
// File Description:
//   Contains the function prototypes for the mock tasks for the scheduler.
//
// History
// 2018-05-12 by Tamkin Rahman
// - Created.
// 2018-06-11 by Tamkin Rahman
// - Added "MockInput" task prototype.

void TaskWrite_low_priority(void *pvParams);
void TaskWrite_high_priority(void *pvParams);

void MockInput(void *pvParams);

#endif
