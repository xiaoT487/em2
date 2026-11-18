
#ifndef MIAO1_TASK_STATUS_H
#define MIAO1_TASK_STATUS_H

typedef enum
{
    TASK_OK ,
    TASK_ERROR ,
} task_result;

extern volatile int task_busy;
extern volatile int task_link;
extern volatile int task_link_status;

void task_start(void);
void task_end(task_result task_result);

#endif //MIAO1_TASK_STATUS_H
