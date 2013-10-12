// Consumer routine -- con.cpp
#include <fstream>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <iostream>

// macro's that lets me use pointers into the shared memory region in a somewhat
// umbiquious manner making them appear as simple variables and one dimentional
// arrays in the program code instead of dereferenced pointers pointed to
// by the int * sh_mem pointer that is to be declared in function main and
// attached using the shmat function.
#define con_index (sh_mem[1])
#define consumed_items (sh_mem[2])
#define prod_id_arry(x) (sh_mem[x+3])
#define data(x) (sh_mem[x+N+3])

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

#define MSG_KEY         12345
#define MSG_PERMS       0777
#define MAX_BUF         1024

using namespace std;

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

// routine that "consumes" the item that is fetched
// takes a pseudorandom amount of CPU time to complete
void con_delay(int seed) {
    static int st_flg = 1;
    int i,cnt;
    if (st_flg) {
        srand(seed/341);
        st_flg = 0;
    }
    cnt = rand()/10;
    for (i=0;i<cnt;i++) {
        seed += 1;
        seed -= 1;
    }
}

int main(int argc, char * argv[]) {

    int * sh_mem;  // pointer to shared memory region
    int log_id;    // the process's logical id
    int N;         // buffer size
    int num_items; // number of items to produce before terminating
    int shm_id;    // shared memory id
    int sem_id;    // semaphore set id
    struct sembuf semops; // semaphore buffer structure declaration
    int i,prod_id;//,prod_data;

    int msg_id;
    int msg_ctl_val;
    struct msqid_ds msg_ds_buf;
    int msg_flag;
    int msg_size;
    struct msgbuf{
        long mtype;
        char msgtxt[MAX_BUF];
    } msg_buf;
    int msg_rcv_val;
    int msgtyp;


    char confile[15];

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

    /* create a shared message queue and test for errors*/
    msg_flag = IPC_CREAT|MSG_PERMS;
    msg_id = msgget(MSG_KEY, msg_flag);
    if (msg_id <0) {
        perror("error on msgget:");
        exit(1);
    }

    /* find out what the largest message can be */
    msg_ctl_val = msgctl(msg_id, IPC_STAT, &msg_ds_buf);
    if (msg_ctl_val <0) {
        perror("error on msgctl");
    }


    ofstream diary;       // declare filestream object
    diary.open("diary",ios::app);  // open text file named diary for writing
    // in append mode
    strcpy(confile,"con_");
    strcat(confile,argv[1]);    // append confile number to file name
    ofstream con_file(confile); // open confile

    // main consumer loop -- loop until specified number of items
    //                       are consumed
    for (i=0;i<num_items;i++) {
        // Are there any Data in the buffer? -- DOWN on FULL
        semops.sem_num = FULL;
        semops.sem_op = DOWN; /* P operation */
        semops.sem_flg = 0;
        semop(sem_id, &semops, 1);

        // Can I enter Critical Region? -- DOWN on MUTEX
        semops.sem_num = MUTEX;
        semops.sem_op = DOWN; /* P operation */
        semops.sem_flg = 0;
        semop(sem_id, &semops, 1);

        // Critical Region of Code where produced item is consumed from
        // next slot of buffer and global index is incremented by one
        // in a modulo manner
        //prod_id=prod_id_arry(con_index);
        //prod_data=data(con_index);

        // Critical Region of Code where produced item is consumed from
        // next slot of buffer and global index is incremented by one
        // in a modulo manner
        prod_id=prod_id_arry(con_index);
        //prod_data=data(con_index);
        msg_flag = 0; // line 53
        /* now message size is the larest we will accept */
        msg_size = 1024;
        /* we need to specify which type to read first if any */
        /* in this case 0 means we will take first come first served */
        msgtyp = 0;
        msg_rcv_val = msgrcv(msg_id, &msg_buf, msg_size, msgtyp,msg_flag);
        if (msg_rcv_val <0) {
            perror(" error on msg rcv:");
            exit(1);
        }

        // log activity in common diary file
        diary << "Consumer" << log_id << " Received Producer" << prod_id
              << "'s data item [" << msg_buf.msgtxt << "] in queue" << endl;
              //<< con_index << ")" << endl;

        diary.flush(); // make sure everything is sent to file now and not
        // stored in some OS buffer

        // also log activity in local con file
        con_file << "Consumer" << log_id << " Received Producer" << prod_id
                 << "'s data item [" << msg_buf.msgtxt << "] in queue" << endl;
//                 << "'s data item [" << msg_buf.msgtxt << "] via the buffer slot ("
//                 << con_index << ")" << endl;

        // increment con_index to point to next slot
        //con_index = (con_index+1)%N;

        /* increment global count of the number of consumed items */
        consumed_items++;

        // Leaving Critical Region -- UP on MUTEX
        semops.sem_num = MUTEX;
        semops.sem_op = UP; /* V operation */
        semops.sem_flg = 0;
        semop(sem_id, &semops, 1);

        // Informing producers there is another element open in Buffer
        // UP on EMPTY
        semops.sem_num = EMPTY;
        semops.sem_op = UP; /* V operation */
        semops.sem_flg = 0;
        semop(sem_id, &semops, 1);

        // somehow use the consumed data in a meaningful way
        con_delay(log_id);
    }

    con_file.close();
}
