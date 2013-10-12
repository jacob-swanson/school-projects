/* This function is a wrapper or jacket for the  */
/* ipcrm shm command that will allow a program   */
/* that includes this file to remove a shared    */
/* memory segment from the system if it has the  */
/* specified sh_id.                              */
/* Input Argument: shared memory input id number */

#include <unistd.h>

void shfree (int shm_id) {
   char argument[3][10];

   /* copy command name and first argument to string arrays */
   strcpy(argument[0],"ipcrm");
   strcpy(argument[1],"shm");

   /* convert shm_id to a string and store as final argument */
   /* for the execl command                                  */
   sprintf(argument[2],"%d",shm_id);

  /* execute system command ipcrm shm to remove specified  */
  /* shared memory region.                                 */
  /* to allow routine to be called anywhere in the main    */
  /* program the execl function is executed in a child     */
  /* process -- when it returns it exits the process as    */
  /* part of its normal action. Since it is in a child the */
  /* parent process in not terminated but lives on. Since  */
  /* it is a garbage collection routine no synchronization */
  /* between other processes is needed.                    */
  if (!fork())
     execl("/usr/bin/ipcrm",argument[0],argument[1],argument[2],NULL);

  /* forking error checking and execl error checking code */
  /* is omitted from this program                         */
}

