#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 20

typedef struct
{
    int pid;
    int arrival_time;
    int burst_time;
    int time_completed;
    int end_time;
} Task;

typedef struct
{
    Task *items[MAX_SIZE];
    int front;
    int back;
} Queue;

/**
 * @brief Initialize the queue
 * @param q Pointer to the queue to initialize
 */
void initializeQueue(Queue *q)
{
    q->front = 0;
    q->back = 0;
}

/**
 * @brief Check if the queue is empty
 * @param q Pointer to the queue to check
 * @return true if the queue is empty, otherwise false
 */
bool isEmpty(Queue *q)
{
    return (q->front == q->back);
}

/**
 * @brief Check if the queue is full
 * @param q Pointer to the queue to check
 * @return true if the queue is full, otherwise false
 */
bool isFull(Queue *q)
{
    return (q->back == MAX_SIZE);
}

/**
 * @brief Add an element to the queue
 * @param q Pointer to the queue that the element will be added to
 * @param value The task to be added to the queue
 */

void push(Queue *q, Task* value)
{
    if (isFull(q))
    {
        printf("Queue is at capacity\n");
        return;
    }
    q->items[q->back] = value;
    q->back++;
}

Task* front(Queue *q)
{
    if(isEmpty(q))
    {
        return NULL;
    }
    return q->items[q->front];
}

/**
 * @brief Remove an element from the queue
 * @param q Pointer to the queue that the element will be removed from
 */
void pop(Queue *q)
{
    if (isEmpty(q))
    {
        printf("Queue is empty\n");
        return;
    }
    q->front++;
}

/**
 * @brief Store the contents of the file into a task array
 * @param file_name The name of the input file containing task information
 * @param num_processes Pointer to an int that stores the number of processes read from the file
 * @return A task array containing the tasks read from the file
 */
Task* storeFileContents(char *file_name, int *num_processes)
{
    FILE *fp;
    // Open a file in read mode
    fp = fopen(file_name, "r");

    Task* ta = malloc(MAX_SIZE * sizeof(Task));
    int index = 0;

    //Check the file opened
    if(fp == NULL) {
        printf("Not able to open the file.");
        return ta;
    }

    // Store the content of the file into the task array
    int p, a, b; 
    if(fscanf(fp, "%d", num_processes) != 1)
    {
        printf("Invalid file format");
        return ta;
    }

    printf("%d\n", *num_processes); 
    while (fscanf(fp, "%d %d %d", &p, &a, &b) == 3) {
        Task t;
        t.pid = p;
        t.arrival_time = a;
        t.burst_time = b;
        t.time_completed = 0;
        t.end_time = 0;
        ta[index] = t;
        index++;
        //printf("%d %d %d\n", p, a, b);
    }

    //check that the array contains the expected number of tasks
    if(index != *num_processes)
    {
        printf("Number of tasks from first line of input does not match the actual number of tasks read\n");
    }

    fclose(fp);
    return ta;
}

/**
 * @brief Compares tasks by arrival time
 * @param a First task
 * @param b Second task
 * @return Negative if a arrives before b, positive if a arrives after b
 */
int comp(const void* a, const void* b) {
    Task *taskA = (Task*)a;
    Task *taskB = (Task*)b;

    // Compare based on arrival time
    return (taskA->arrival_time - taskB->arrival_time);
}

/**
 * @brief First Come First Serve scheduling algorithm
 * @param task_array Array of tasks to be scheduled
 * @param num_processes Number of tasks in the array
 */
void fcfs(Task* task_array, int num_processes)
{
    printf("Starting FCFS scheduling\n");

    Queue ready_queue;
    initializeQueue(&ready_queue);
    //Initialize the time
    int time = 0;
    //Track # of completed processes
    int completed_processes = 0;
    int index = 0;
    Task *current_task = NULL;
    
    //Stop when it is equal to the number of processes
    while(completed_processes < num_processes)
    {
        //if multiple tasks arrive at the same time, add all of them to the ready queue
        //if only one task arrives at this time, it will still be added to the ready queue
        while(index < num_processes && task_array[index].arrival_time == time)
        {
            push(&ready_queue, &task_array[index]);
            index++;
        }
        //If there is no current task, select the next task in the queue
        if(current_task == NULL && !isEmpty(&ready_queue))
        {
            current_task = front(&ready_queue);
            pop(&ready_queue);
        }
        //If there is not a current task, and nothing in the queue -> idle
        else if(current_task == NULL && isEmpty(&ready_queue))
        {
            //no task is running
            printf("Time %d: ,idle\n", time);
            time++;
            continue;
        }
        printf("Time %d: Running task %d\n", time, current_task->pid);
        current_task->time_completed++;
        time++;
        //check if the current task has completed
        if(current_task->time_completed == current_task->burst_time)
        {
            completed_processes++;
            current_task->end_time = time;
            printf("Task %d ended, Time: %d\n", current_task->pid, time);
            current_task = NULL;
        }

    }
}

/**
 * @brief remove and return the task with the shortest burst time from the queue
 * @param q queue
 * @return task with the shortest burst time, NULL if the queue is empty
 */
Task* popShort(Queue* q) {
    if(isEmpty(q)) {
        return NULL;
    }
    int min_index = q->front;

    //locate the shortest burst time
    for(int i = q->front + 1; i < q->back; i++) 
    {
        if(q->items[i]->burst_time < q->items[min_index]->burst_time) 
        {
            min_index = i;
        }
    }
    //remove shortest task from the queue
    Task* shortest_task = q->items[min_index];
    for(int i = min_index; i < q->back - 1; i++) 
    {
        q->items[i] = q->items[i + 1];
    }
    q->back--;

    //return the shortest task from the queue
    return shortest_task;
}

/**
 * @brief Shortest Job First scheduling algorithm
 * @param task_array Array of tasks to be scheduled
 * @param num_processes Number of tasks in the array
 * @return Array of task IDs in the order they finished
 */
int* sjf(Task* task_array, int num_processes)
{
    int* finish_order = (int*)malloc(num_processes * sizeof(int));
    
    printf("Starting SJF scheduling\n");

    Queue ready_queue;
    initializeQueue(&ready_queue);
    //Initialize the time
    int time = 0;
    //Track # of completed processes
    int completed_processes = 0;
    int index = 0;
    Task *current_task = NULL;
    
    //Stop when it is equal to the number of processes
    while(completed_processes < num_processes)
    {
        //if multiple tasks arrive at the same time, add all of them to the ready queue
        //if only one task arrives at this time, it will still be added to the ready queue
        while(index < num_processes && task_array[index].arrival_time == time)
        {
            push(&ready_queue, &task_array[index]);
            index++;
        }
        //If there is no current task, select the next task in the queue
        if(current_task == NULL && !isEmpty(&ready_queue))
        {
            current_task = popShort(&ready_queue);
        }
        //If there is not a current task, and nothing in the queue -> idle
        else if(current_task == NULL && isEmpty(&ready_queue))
        {
            //no task is running
            printf("Time %d: ,idle\n", time);
            time++;
            continue;
        }
        printf("Time %d: Running task %d\n", time, current_task->pid);
        current_task->time_completed++;
        time++;
        //check if the current task has completed
        if(current_task->time_completed == current_task->burst_time)
        {
            completed_processes++;
            current_task->end_time = time;
            printf("Task %d ended, Time: %d\n", current_task->pid, time);
            finish_order[completed_processes - 1] = current_task->pid;
            current_task = NULL;
        }

    }

    return finish_order;

}

/**
 * @brief Round Robin scheduling algorithm
 * @param task_array Array of tasks to be scheduled
 * @param num_processes Number of tasks in the array
 * @param time_quantum Time quantum for the round robin scheduling
 */
void rr(Task* task_array, int num_processes, int time_quantum)
{
    printf("Starting RR scheduling\n");

    printf("PID   Start Time   End Time   Running Time   \n");

    Queue ready_queue;
    initializeQueue(&ready_queue);
    //Initialize the time
    int time = 0;
    //Track # of completed processes
    int completed_processes = 0;
    int index = 0;
    Task *current_task = NULL;
    int time_since_reset = 0;
    
    //Stop when it is equal to the number of processes
    while(completed_processes < num_processes)
    {
        //if multiple tasks arrive at the same time, add all of them to the ready queue
        //if only one task arrives at this time, it will still be added to the ready queue
        while(index < num_processes && task_array[index].arrival_time == time)
        {
            push(&ready_queue, &task_array[index]);
            index++;
        }
        //If there is no current task, select the next task in the queue
        if(current_task == NULL && !isEmpty(&ready_queue))
        {
            current_task = front(&ready_queue);
            pop(&ready_queue);

        }
        //If there is not a current task, and nothing in the queue -> idle
        else if(current_task == NULL && isEmpty(&ready_queue))
        {
            //no task is running
            printf("Time %d: ,idle\n", time);
            time++;
            continue;
        }
       // printf("Time %d: Running task %d\n", time, current_task->pid);
        current_task->time_completed++;
        time_since_reset++;
        time++;
        //check if the current task has completed
        if(current_task->time_completed == current_task->burst_time)
        {
            completed_processes++;
            current_task->end_time = time;
            printf(" %d      %d            %d           %d\n",
                current_task->pid,
                time - time_since_reset,
                time,
                time_since_reset);
            current_task = NULL;
            time_since_reset = 0;
        }
        //if the time since the last reset is equal to the quantum
        //add current task to queue, and set current time to null
        else if(time_since_reset == time_quantum)
        {
            printf(" %d      %d            %d           %d\n",
                current_task->pid,
                time - time_since_reset,
                time,
                time_since_reset);
            push(&ready_queue, current_task);
            current_task = NULL;
            time_since_reset = 0;
        }

    }

}


int main(int argc, char *argv[]) {

    char *file_name;
    char *scheduling_algorithm;
    int time_quantum = 0;
    int num_processes = 0;

    //Command line input_file [FCFS, RR, SJF] [time_quantum]
    //Verify command line arguments
    if(argc == 3)
    {
        file_name = argv[1];
        scheduling_algorithm = argv[2];
        //if RR, must have time quantum
        if(strcmp(scheduling_algorithm, "RR") == 0)
        {
            printf("Incorrect input. RR requires a time quantum\n");
            return 1;
        }
        else if(strcmp(scheduling_algorithm, "FCFS") != 0 && strcmp(scheduling_algorithm, "SJF") != 0)
        {
            printf("Incorrect input. Expected FCFS, RR, or SJF\n");
            return 1;
        }
    }
    else if (argc == 4)
    {
        file_name = argv[1];
        scheduling_algorithm = argv[2];
        time_quantum = atoi(argv[3]);

        if(strcmp(scheduling_algorithm, "FCFS") != 0 && strcmp(scheduling_algorithm, "SJF") != 0 
            && strcmp(scheduling_algorithm, "RR") != 0)
        {
            printf("Incorrect input. Expected FCFS, RR, or SJF\n");
            return 1;
        }
    }
    else
    {
        printf("Incorrect input. Expected format: input_file [FCFS| RR| SJF] [time_quantum]\n");
        return 1;
    }

    //Store data from the file into the task array
    Task* t = storeFileContents(file_name, &num_processes);

    //Sort the task by arrival time (only needed if they are not given in order of arrival time, unknown for this assignment)
    qsort(t, num_processes, sizeof(Task), comp);

    //Print the tasks after sorting by arrival time
    for(size_t i = 0; i < num_processes; i++)
    {
        printf("Process ID: %d\n", t[i].pid);
        printf("Arrival Time: %d\n", t[i].arrival_time);
        printf("Burst Time: %d\n", t[i].burst_time);
        printf("Completion Time: %d\n", t[i].time_completed);
        printf("End Time: %d\n", t[i].end_time);
        printf("\n");
    }

    int* order = NULL;

    //Run the correct scheduling algorithm
    if(strcmp(scheduling_algorithm, "FCFS") == 0)
    {
        fcfs(t, num_processes);
    }
    else if(strcmp(scheduling_algorithm, "SJF") == 0)
    {
        order = sjf(t, num_processes);
    }
    else if(strcmp(scheduling_algorithm, "RR") == 0)
    {
        rr(t, num_processes, time_quantum);
    }

    //Print statistical information
    int total_waiting_time = 0; 

    printf("%s\n", scheduling_algorithm);

    printf("PID   Arrival Time   Start Time   End Time   Running Time   Waiting Time\n");
    for(size_t i = 0; i < num_processes; i++)
    {
        Task* current_task;
        if(order)
            current_task = &t[order[i]];
        else
            current_task = &t[i];

        printf(" %d      %d               %d           %d         %d             %d\n", 
            current_task->pid, 
            current_task->arrival_time, 
            current_task->end_time - current_task->burst_time, 
            current_task->end_time, 
            current_task->burst_time, 
            ((current_task->end_time - current_task->burst_time) - current_task->arrival_time));
        total_waiting_time += ((current_task->end_time - current_task->burst_time) - current_task->arrival_time);
    }
    //Calc and pring average wait time
    double ave_wait_time = total_waiting_time / (double)num_processes;
    printf("Average waiting time: %.2f\n", ave_wait_time);

}
//Write a report
//how to compile and run the program,list 3 test cases, screenshots
