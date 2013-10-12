/* Lab 3 Example Program Solution */
// main.cpp
// This program spawns a user defined set of  producers and consumers which
// communicate via a circular buffer that contains the producer ID and the
// consumer ID. The size of the buffer is given by the user at run time.
using namespace std;
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "shfree.cpp" //wrapper to allow semaphore to automatically be removed

#define MX_CHILDREN 100
#define prod_index sh_mem[0]
#define con_index sh_mem[1]
#define consumed_items sh_mem[2]

#define SHM_KEY 0x5562 // define shared memory key
#define SHM_SIZE ((N*2+3)*sizeof(int))
#define SHM_PERMS 0777
#define SHM_GET_FLG IPC_CREAT|SHM_PERMS

#define SEM_KEY 0x5656
#define N_SEMS 3
#define SEM_PERMS 0777
#define SEM_GET_FLG IPC_CREAT|SEM_PERMS
#define SHM_AT_FLG SHM_RND

// Semaphore Number Declarations
#define MUTEX 0
#define EMPTY 1
#define FULL  2

#define DOWN -1 /* Semaphore P operation */
#define UP   1  /* Semaphore V operation */

// routine to ask the user at run time for the number of producer and
// consumer processes, number of data items to be produced/consumed,
// and the size of the communication buffer
void init_data(int * producers, int * consumers, int * items, int *N) {
   string hld;
   do {
      cout << "Enter Number of Producers:";
      cin >> *producers;
   } while (*producers < 1);

   do {
      cout << "Enter Number of Consumers:";
      cin >> *consumers;
   } while (*consumers < 1);

   do {
      cout << "Enter the Total Number of Data Items Produced/Consumed:";
      cin >> *items;
   } while (*items < 1);

   do {
      cout << "Enter the Size of the Circular Buffer:";
      cin >> *N;
   } while (*N < 1);

   /* reset diary file */
   {
      ofstream filestr;       // declare filestream object
      filestr.open("diary");  // open text file named diary for writing
      filestr.close();        // then close it
   }
}

// routine to convert integers into old style C strings
void int_to_C_string_conv(char * C_str_num, int num) {
   std::string s;
   std::stringstream out;
   out << num;
   s = out.str();
   strcpy(C_str_num,s.c_str());
}

main() {

   /* major runtime parameters to be obtained from user */
   int num_producers; // number of producer processes
   int num_consumers; // number of consumer processes
   int num_items;     // number of data items
   int N;             // size of circular buffer

   char num_prod_items[10],num_con_items[10],buf_size[10];
   char process_logical_id[10],shared_mem_id[10];
   char semaphore_id[10];

   int *sh_mem; //shared memory pointer used to initialize buffer elements

   int shm_id,sem_id,sem_stat,exec_ret;
   int base_num,extra_cutoff,status;
   pid_t pid_child;
   int i;

   // prompt user for number of producers, consumers,items, and buffer size
   init_data(&num_producers, &num_consumers, &num_items,&N);

   // convert buffer size to ANSI C string representation
   int_to_C_string_conv(buf_size,  N);

   // create shared memory region for 2*N+3 integers that represent the
   // producer logical process ID and data item produced for a buffer size
   // of N elements
   shm_id = shmget(SHM_KEY, SHM_SIZE, SHM_GET_FLG);
   if (shm_id <0) {
      perror("error on shmget for obtaining buffer in shared memory\n");
      exit(1);
   }

   /* attach shared memory region */
   sh_mem = (int *) shmat(shm_id, (int *) NULL, SHM_AT_FLG);
   if(sh_mem <0) {
      perror("error attaching shared memory:");
      exit(1);
   }

   // clear next producer and consumer slot indices
   con_index=0;
   prod_index=0;

   // Create semaphore set, MUTEX, FULL, and EMPTY
   sem_id = semget(SEM_KEY, N_SEMS, SEM_GET_FLG);
   if(sem_id<0) {
      perror("error in creating semaphores");
      exit(1);
   }

   // Initializing Semaphores
   // the prototype is semctl(semid, semnum, cmd, arg)
   sem_stat = semctl(sem_id, MUTEX, SETVAL, 1);
   if (sem_stat <0) {
      perror("error setting MUTEX to 1");
      exit(1);
   }
   sem_stat = semctl(sem_id, FULL, SETVAL, 0);
   if (sem_stat <0) {
      perror("error setting MUTEX to 1");
      exit(1);
   }
   sem_stat = semctl(sem_id, EMPTY, SETVAL, N);
   if (sem_stat <0) {
      perror("error setting MUTEX to N");
      exit(1);
   }

   // check to see if maximum number of child process limit is exceeded
   if (num_producers+num_consumers>MX_CHILDREN) {
      cout << "Error: Too many producer/consumer processes requested" << endl;
      exit(1);
   }

   // keep up with number of consumed items to allow for a join to occur
   consumed_items = 0;

   // create producer processes
   // used to divide the set of data items as evenly as possible
   // among the producer processes
   base_num = num_items/num_producers;
   extra_cutoff=num_items%num_producers;
   for (i=0;i<num_producers;i++) {
      pid_child = fork();
      if (pid_child==0) {
         // determine and create a string representation of the
         // number of data items to produce for each producer
         int_to_C_string_conv(num_prod_items,base_num+(extra_cutoff>i));

         // create string representation of producer processes logical id
         // starting at 0 and continuing in sequence
         int_to_C_string_conv(process_logical_id,i);

         // create strign representation of common shared memory id
         int_to_C_string_conv(shared_mem_id,shm_id);

         // create strign representation of common shared memory id
         int_to_C_string_conv(semaphore_id,sem_id);

         exec_ret = execl("prod","prod",process_logical_id,buf_size,
                           num_prod_items,shared_mem_id,semaphore_id,NULL);
         perror("Error in producer process\n");
         fflush(NULL);
         exit(0);
      }
      else {
         if (pid_child < 0) {
            cout << "Error in producer process " << i << " creation" << endl;
            exit(1);
         }
      }
   }
   // create consumer processes
   // used to divide the set of data items as evenly as possible
   // among the consumers processes
   base_num = num_items/num_consumers;
   extra_cutoff=num_items%num_consumers;
   for (i=0;i<num_consumers;i++) {
      pid_child = fork();
      if (pid_child==0) {
         // determine and create a string representation of the
         // number of data items to consume at each consumer
         int_to_C_string_conv(num_con_items,base_num+(extra_cutoff>i));

         // create string representation of consumer process' logical id
         // starting at 0 and continuing in sequence
         int_to_C_string_conv(process_logical_id,i);

         // create strign representation of common shared memory id
         int_to_C_string_conv(shared_mem_id,shm_id);

         // create strign representation of common shared memory id
         int_to_C_string_conv(semaphore_id,sem_id);

         exec_ret = execl("con","con",process_logical_id,buf_size,
                           num_con_items,shared_mem_id,semaphore_id,NULL);
         perror("Error in Consumer process\n");
         fflush(NULL);
         exit(0);
      }
      else {
         if (pid_child < 0) {
            cout << "Error in consumer process %d creation" << endl;
            exit(1);
         }
      }
   }
   // Simulate a Join Operation by monitoring a global memory location
   while(num_items>consumed_items);

   // Remove semaphore from memory
   sem_stat = semctl(sem_id, 0, IPC_RMID, 0);
   if (sem_stat <0) {
      perror("error removing semaphore set");
      exit(1);
   }

   // Remove shared memory region
   shfree(shm_id);
}
