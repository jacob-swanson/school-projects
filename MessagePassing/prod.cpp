// Producer routine -- prod.cpp
using namespace std;
#include <fstream>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>

// macro's that let me use pointers into the shared memory region in a somewhat
// umbiquious manner making them appear as simple variables and one dimentional
// arrays in the program code instead of dereferenced pointers pointed to
// by the int * sh_mem pointer that is to be declared in function main and
// attached using the shmat function.
#define prod_index (sh_mem[0])
#define prod_id_arry(x) (sh_mem[x+3])
#define data(x)    (sh_mem[x+N+3])

#define SHM_PERMS 0777
#define SHM_GET_FLG IPC_CREAT|SHM_PERMS
#define SHM_AT_FLG SHM_RND

// Semaphore Number Declarations
#define MUTEX 0
#define EMPTY 1
#define FULL  2

// Semaphore UP Down Declarations
#define UP 1
#define DOWN -1

/* from the man page for semctl */
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
   /* union semun is defined by including <sys/sem.h> */
#else
   /* according to X/OPEN we have to define it ourselves */
   union semun {
   int val;                    /* value for SETVAL */
   struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
   unsigned short int *array;  /* array for GETALL, SETALL */
   struct seminfo *__buf;      /* buffer for IPC_INFO */ };
#endif

// routine the "produces" the data items
// takes a pseudorandom amount of CPU time to do this
#define MX 100
int prod_next_data(int seed) {
   static int st_flg = 1;
   static long int count = 0;
   int i,cnt;
   if (st_flg) {
      srand(seed);
      st_flg = 0;
   }
   cnt = rand()/10;
   for (i=0;i<cnt;i++) {
      seed += 1;
      seed -= 1;
   }
   if (count>=MX)return -1;
   else return count++;
}

int main(int argc, char * argv[]) {

   int * sh_mem;  // pointer to shared memory region
   int log_id;    // the process's logical id
   int N;         // buffer size
   int num_items; // number of items to produce before terminating
   int shm_id;    // shared memory id
   int sem_id;    // semaphore set id
   struct sembuf semops; // semaphore buffer structure declaration
   int i,new_data;

   char prodfile[15];

   // read command line parameters of process logical id, buffer size
   // number of items to produce, shared memory id, and semaphore id.
   // Then convert them to integers
   log_id = atoi(argv[1]);
   N = atoi(argv[2]);
   num_items = atoi(argv[3]);
   shm_id = atoi(argv[4]);
   sem_id = atoi(argv[5]);

   /* attach shared memory region */
   sh_mem = (int *) shmat(shm_id, (int *) NULL, SHM_AT_FLG);
   if(sh_mem <0) {
      perror("error attaching shared memory:");
      exit(1);
   }

   ofstream diary;       // declare filestream object
   diary.open("diary",ios::app);  // open text file named diary for writing

   strcpy(prodfile,"prod_");
   strcat(prodfile,argv[1]);     // append confile number to file name
   ofstream prod_file(prodfile); // open prodfile

   // main producer loop -- loop until specified number of items
   //                       are produced
   for (i=0;i<num_items;i++) {

      // produce next data item
      new_data = prod_next_data(log_id);

      // Are there any slots in the buffer? -- DOWN on EMPTY
      semops.sem_num = EMPTY;
      semops.sem_op = DOWN; /* P operation */
      semops.sem_flg = 0;
      semop(sem_id, &semops, 1);

      // Can I enter Critical Region? -- DOWN on MUTEX
      semops.sem_num = MUTEX;
      semops.sem_op = DOWN; /* P operation */
      semops.sem_flg = 0;
      semop(sem_id, &semops, 1);

      // Critical Region of Code where produced item is placed in
      // next slot of buffer and global index is incremented by one
      // in a modulo manner

      // place new data and pid in buffer
      prod_id_arry(prod_index) = log_id;
      data(prod_index) = new_data;

      // log activity in common diary file
      diary << "Producer" << log_id << " placed data [" << data(prod_index) <<
               "] in buffer slot(" << prod_index << ")" << endl;
      diary.flush(); // make sure everything is sent to file now and not
                     // stored in some OS buffer

      // also log activity in local prod file
      prod_file << "Producer" << log_id << " placed data [" << data(prod_index)
                << "] in buffer slot(" << prod_index << ")" << endl;

      /* increment prod_index to point to next slot */
      prod_index = (prod_index+1)%N;

      // Leaving Critical Region -- UP on MUTEX
      semops.sem_num = MUTEX;
      semops.sem_op = UP; /* V operation */
      semops.sem_flg = 0;
      semop(sem_id, &semops, 1);

      // Informing consumers there is another element in buffer-- UP on FULL
      semops.sem_num = FULL;
      semops.sem_op = UP; /* V operation */
      semops.sem_flg = 0;
      semop(sem_id, &semops, 1);
   }
   prod_file.close();
}
