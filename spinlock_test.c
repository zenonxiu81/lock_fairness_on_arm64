/**
 * spinlock_test.c
 * Copyright (C) 2021 Zenon Xiu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#define _GNU_SOURCE
#include <stdio.h>

#ifdef SPINLOCK
 #include "spin_lock.h"
#endif 

#ifdef TICKET_SPINLOCK
 #include "ticket_spinlock.h"
#endif 


#include<stdlib.h>
 
#include<pthread.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>


typedef struct {
	
  spinlock_t lock;
  long long Num_of_cpu0;
  long long Num_of_cpu1;
  unsigned int total;
  unsigned int total_count;
  int cpu_run_T0;
  int cpu_run_T1;
  

} payload;

payload lock_test_data;

pthread_t tidp[2];


void * lock_test(void * a)
{
  int id, raw_id; 
  unsigned int  i;
  payload * p = ( payload *)a;
  cpu_set_t cpuset;
  int ret;
  pthread_t pid;
    
   
   while(tidp[0]==0); //Wait for thread 0 being created.
   CPU_ZERO(&cpuset);
   CPU_SET(p->cpu_run_T0,&cpuset);  //set which CPU to run the thread 0
   pthread_setname_np(tidp[0],"spinlock T0");
   ret=pthread_setaffinity_np(tidp[0],sizeof(cpu_set_t),&cpuset);
   if(ret<0)
    printf("set thread0 affinity failed.. %d\n",ret);
   
   while(tidp[1]==0); //Wait for thread 1 being created.
   CPU_ZERO(&cpuset);
   CPU_SET(p->cpu_run_T1,&cpuset);  //set which CPU to run the thread 1
   pthread_setname_np(tidp[1],"spinlock T1");
   ret=pthread_setaffinity_np(tidp[1],sizeof(cpu_set_t),&cpuset);
  if(ret<0)
   printf("set thread1 affinity failed.. %d\n", ret);
  
  pid=pthread_self();

  if(pid==tidp[0]){
  printf("T0 started!\n");
  for(i=0;i<p->total; i++)
    {
    //id=get_cpu_id();
    spin_lock(&(p->lock));
    p->Num_of_cpu0 ++;
    p->total_count ++; 
    if(p->total_count>=p->total)
     {
      spin_unlock(&(p->lock));
      break; 
     }
    spin_unlock(&(p->lock)); 
   }  
   printf("T0 done!\n");
  }
  else if(pid==tidp[1]){
   printf("T1 started!\n");
    for(i=0;i<p->total; i++)
     {
      //id=get_cpu_id();
      spin_lock(&(p->lock));
      p->Num_of_cpu1 ++;
      p->total_count ++;
      if(p->total_count>=p->total)
       {
        spin_unlock(&(p->lock));
        break; 
       }
      spin_unlock(&(p->lock)); 
     } 
   printf("T1 done!\n"); 
  } 	
}

int main(int agrc,char* argv[])
{

 int i;

 spin_lock_init(&lock_test_data.lock);

 lock_test_data.Num_of_cpu0=0;
 lock_test_data.Num_of_cpu1=0;
 lock_test_data.total=0;
 tidp[0]=0;
 tidp[1]=0;
 
 if(agrc<6)
  {
   printf("Wrong paramters\n");
   printf("-n to specific number of iteration, -c to specific two cpus to run,\n for example \" -n 10000 -c 1 2\" \n");
   return -1;
  }
 for(i=1; i<agrc; )
  {
   if(strcmp(argv[i],"-n")==0)
     {
       lock_test_data.total=atoi(argv[i+1]);
       i+=2;
       continue;
     }
   if(strcmp(argv[i],"-c")==0)
     {
       lock_test_data.cpu_run_T0=atoi(argv[i+1]);
       lock_test_data.cpu_run_T1=atoi(argv[i+2]);
       i+=3;
       continue;
     }
   else 
   {
     i++; 
     continue;
   } 
 }
 
 printf("Parameter used: -c %d -n %d %d\n",lock_test_data.total,lock_test_data.cpu_run_T0,lock_test_data.cpu_run_T1);


 __asm__("dmb ish ");
 if ((pthread_create(&tidp[0], NULL, lock_test, (void*)&lock_test_data)) == -1)
  {
    printf("create error!\n");
    return 1;
  }
 
  
  __asm__("dmb ish ");

  if ((pthread_create(&tidp[1], NULL, lock_test, (void*)&lock_test_data)) == -1)
  {
    printf("create error!\n");
    return 1;
  }

  if (pthread_join(tidp[0], NULL))
  {
    printf("thread is not exit...\n");
    return -2;
  }

  if (pthread_join(tidp[1], NULL))
  {
    printf("thread is not exit...\n");
    return -2;
  }
 
 printf("T0 num: %d, T1 num: %d \n",lock_test_data.Num_of_cpu0,lock_test_data.Num_of_cpu1);
  return 0;	
	
}
