/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t{
  int job_id;
  int time;
  int running_time;
  int priority;
  int busy; //Idle(0) and busy(1)

} job_t;

typedef struct core_t{
  int core_id;
  job_t* job; 
} core_t;

typedef struct sched_t {
  scheme_t scheduler_scheme; 
  core_t* cores_array;
  priqueue_t pq_id;

  // complete 
} sched_t;

sched_t schedy;

typedef int(*comparator_t)(const void*, const void*);
typedef int priority_t; //integer for priority queue 
typedef int sched_t; //integer for sscheduler 

//new_job func job_t* jobNew()
job_t* jobNew(int job_number, int time, int running_time, int priority) {
    job_t* newJob = malloc(sizeof(job_t));
    newJob->job_id = job_number;
    newJob->time = time;
    newJob->running_time = running_time;
    newJob->priority = priority;
    newJob->busy = 1; // busy job

    // error handling 
    if (newJob == NULL) {
        return NULL;
    }

    return newJob;
}


priority_t getPriorityFCFS(job_t* job){
  return job->time;
}

// getting priority_scheduler id
sched_t getPRISched(job_t* job){
  return job->priority; 
}

// getting shortest_job first
priority_t getPrioritySJF(job_t* job){
  return job->running_time;
}

//comparators for scheduling algorithms 
int comparatorFCFS(const void* a, const void* b){
  job_t* job1 = (job_t*)a;
  job_t* job2 = (job_t*)b;

  return getPriorityFCFS(job1) - getPriorityFCFS(job2);
}

int comparatorSJF(const void* a, const void* b){
  job_t* job1 = (job_t*) a;
  job_t* job2 = (job_t*) b;

  int cmp = getPrioritySJF(job1) - getPrioritySJF(job2);

  return cmp == 0 ? comparatorFCFS(a,b) : cmp;
}

//priorityPRI scheduler
int comparatorPRIsched(const void* a, const void* b){
  job_t* job1 = (job_t*)a;
  job_t* job2 = (job_t*)b;

  int cmp = getPRISched(job1) - getPRISched(job2);

  return cmp == 0 ? comparatorFCFS(a,b) : cmp; 
}

//PPRI (Preemptive Priority scheduling) comparator 
int comparatorPPRI(const void* a, const void* b){
  job_t* job1 = (job_t*)a;
  job_t* job2 = (job_t*)b;

  int cmp = getPRISched(job1) - getPRISched(job2);
  
  if (cmp != 0){
    return cmp;
  } else {
    int runningDiffTime = getPrioritySJF(job1) - getPrioritySJF(job2);

    if (runningDiffTime != 0){
      return runningDiffTime;
    } else {
      return comparatorFCFS(a, b);
    }
  }

}

// return priority for RR
int comparatorRR(const void* a, const void* b){
  //will depend on how priqueue was implemented
  return -1; 
}

comparator_t getComparator(sched_t scheme){
  comparator_t ret = comparatorFCFS;

  switch (scheme) {
    case FCFS:
      ret = comparatorFCFS;
      break;
    
    case SJF:
      ret = comparatorSJF;
      break;

    case PSJF:
      ret = comparatorSJF;
      break;

    case PRI:
      ret = comparatorPRIsched;
      break; 

    // case PPRI
    case PPRI:
      ret = comparatorPPRI;
      break;
    
    case RR:
      ret = comparatorRR;
      break; 

    default:
      break; 
  }

  return ret; 
}

bool isPreemptive(scheme_t scheme){
  return ( scheme == PSJF || scheme == PPRI);
}

/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  priqueue_init(&schedy.pq_id, getComparator(scheme));
  schedy.cores_array = malloc(sizeof(core_t) * cores);

  for(int i = 0; i < cores; i++){
    schedy.cores_array[i].core_id = i;
    schedy.cores_array[i].job = NULL;
  }
}


/**
  Called when a new job arrives.
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumption:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{

  //check idle cores
  for (int i = 0; i < schedy.cores_array; i++){
    if (schedy.cores_array[i].job == NULL){
      schedy.cores_array[i].job = jobNew(job_number, time, running_time, priority);
      return i;
    }
  }

  // if no idle cores
  switch (schedy.scheduler_scheme){
    case FCFS:
    case SJF:
    case PSJF:
      break;
    case PRI:
      for (int i = 0; i < schedy.cores_array; i++){
        if (schedy.cores_array[i].job->priority > priority){
          job_t* temp = schedy.cores_array[i].job;
          schedy.cores_array[i].job = jobNew(job_number, time, running_time, priority);
          free(temp);
          return i;
        }
      }
      break;
    case PPRI:
        int lowestPRI = 0;
        for (int i = 1; i < schedy.cores_array; i++){
          if (schedy.cores_array[i].job->priority < schedy.cores_array[lowestPRI].job->priority){
            lowestPRI = i;
          }
        }

        if (schedy.cores_array[lowestPRI].job->priority > priority){
          job_t* temp = schedy.cores_array[lowestPRI].job;
          schedy.cores_array[lowestPRI].job = jobNew(job_number, time, running_time, priority);
          free(temp);
          return lowestPRI;
        }
    case RR:
    // may need further implementation
  }
  return -1;

}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
  // find core in priority queue that needs to be finished. Highest priority
  // returns job_id to schedule on given core
  core_t *finished_core = &schedy.cores_array[core_id];

  //free memory
  free(finished_core->job);
  finished_core ->job = NULL;

  //Don't schedule non-preemptive jobs immediately
  if (!isPreemptive(schedy.scheduler_scheme)){
    return -1;
  }

  //For preemptive schedulers, find highest priority job and
  // schedule to the core
  job_t *highest_priority_job = NULL;
  int highest_priority = INT_MAX;

  // Traverse the priority queue and find highest priority job
  for (size_t i = 0; i < priqueue_size(&schedy.pq_id); ++i){
    job_t *current_job = priqueue_at(&schedy.pq_id, i);
    if (current_job->priority < highest_priority){
      highest_priority = current_job->priority;
      highest_priority_job = current_job;
    }
  }

  // return job id if a job is found 
  if (highest_priority_job != NULL){
    priqueue_remove_at(&schedy.pq_id, priqueue_at(&schedy.pq_id, highest_priority_job));
    finished_core->job = highest_priority_job;
    return highest_priority_job->job_id;
  }

	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
  // RR priority queue, see's what jobs needs to be queued on the core
  // check if core is idle 
  core_t *expired_core = &schedy.cores_array[core_id];

  //If core is idle, no need to schedule a new job
  if (expired_core->job == NULL){
    return -1;
  }

  //remove current job
  job_t *current_job = expired_core->job;
  expired_core->job = NULL;

  //enqueue the current job at the end of priority queue
  priqueue_offer(&schedy.pq_id, current_job);

  //find the next job
  job_t *next_job = priqueue_poll(&schedy.pq_id);
  
  // if no next job
  if(next_job == NULL){
    return -1;
  }
	
  //assign next job
  expired_core->job = next_job;

  //return job id
  return next_job->job_id;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
  int total_waiting_time = 0;
  int total_jobs = 0;
  int time = 0;

  //Iterate through all cores to calculate waiting time
  for (int i = 0; i < schedy.cores_array; i++){
    job_t *current_job = schedy.cores_array[i].job;
    if (current_job != NULL){
      int waiting_time = time - current_job->time; 
      total_waiting_time += waiting_time;
      total_jobs++;
    }
  }

  float average_waiting_time = (float)total_waiting_time / total_jobs;

	return average_waiting_time;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	int total_turnaround_time = 0;
  int total_jobs = 0;
  int time = 0;

  //Iterate through all cores to calculate waiting time
  for (int i = 0; i < schedy.cores_array; i++){
    job_t *current_job = schedy.cores_array[i].job;
    if (current_job != NULL){
      int turnaround_time = time - current_job->time; 
      total_turnaround_time += turnaround_time;
      total_jobs++;
    }
  }

  float average_turnaround_time = (float)total_turnaround_time / total_jobs;

	return average_turnaround_time;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
  int total_response_time = 0;
  int total_jobs = 0;
  int time = 0;

  //Iterate through all cores to calculate waiting time
  for (int i = 0; i < schedy.cores_array; i++){
    job_t *current_job = schedy.cores_array[i].job;
    if (current_job != NULL){
      int response_time = current_job->time - current_job->running_time; 
      total_response_time += response_time;
      total_jobs++;
    }
  }

  float average_response_time = (float)total_response_time / total_jobs;

	return average_response_time;
}


/**
  Free any memory associated with your scheduler.
 
  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  if (schedy.cores_array != NULL){
    free(schedy.cores_array);
    schedy.cores_array = NULL;
  }

  while (!priqueue_is_empty(&schedy.pq_id)){
    job_t *job = priqueue_poll(&schedy.pq_id);
    free(job); 
  }

  priqueue_destroy(&schedy.pq_id);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
