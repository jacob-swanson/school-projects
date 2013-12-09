// A Template file for the serial version of the Moore's Algorithm
// presented in Chapter 7 of the text
// B. Earl Wells
/**
  * Heat Distribution Problem
  * Author: Jacob Swanson
  * Date: 8 December 2013
  * Build Instructions:
  *     g++ main.cpp -o more-shortestpath -lpthread
  * Usage:
  *     more-shortestpath [number of threads (optional)]
  */

using namespace std;
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <list>

#define N 100        // maximum dimension of adj_matrix
#define MX_LN 256    // maximum line size
#define INFINITY (unsigned int) ~0  // define infinity value

// copied from mpbench
#define TIMER_CLEAR  (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0)
#define TIMER_START   gettimeofday(&tv1, (struct timezone*)0)
#define TIMER_ELAPSED ((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000))
#define TIMER_STOP    gettimeofday(&tv2, (struct timezone*)0)
struct timeval tv1,tv2;

// Queue
int num_ele = 0,get_ele=0,put_ele=0;
int vrt_queue[N];

// Graph
unsigned int w[N][N];
unsigned int dist[N];
unsigned int num_vertex,src_vertex;

class Arguments {
public:
    unsigned int i;
};

/* arg_fetch:  This routine allows individual arguments
   to be parsed out from a string that contains a number
   of arguments separated from one another by spaces.
   argout: is a string outputted by the routine containing
           the seperated argument.
   strin:  is the string containing a number of arguments.
   argnm:  is the desired argument number, starting at 0
           for the first argument.
   If argument exists function returns a 1.  Otherwise a 0
   is returned by the function                             */

int  arg_fetch(char *argout,char *strin,int argnm)
{
    int indx,curarg,argstrt,i,j;
    curarg= -1;
    indx=0;
    do {
        curarg+=1;
        // finding starting letter of next argument
        while (strin[indx]==' ')
            indx+=1;

        // if end of strin is reached argnm too large
        // exit routine returning a "false" value
        if (strin[indx]=='\0') {
            argout[0]=(char) NULL;
            return 0;
        }

        // save beginning character of argument
        argstrt=indx;

        // find ending character of argument
        while ((strin[indx]!=' ')&(strin[indx]!='\0'))
            indx+=1;

    }  while (curarg!=argnm);
    j=0;
    for (i=argstrt;i<indx;++i) {
        argout[j]=strin[i];
        j++;
    }
    argout[j]='\0';

    // success
    return 1;
}

// routine to read an adjacency matrix of a Directed Graph
// into a two dimentional array
void read_adj(unsigned int array[N][N],unsigned int *n,char *file)
{
    FILE *fp1;
    unsigned int rw,col;
    unsigned int num;
    char ch[MX_LN],ch2[MX_LN];
    *n=0;
    rw=0;

    if((fp1=fopen(file,"r"))==NULL) {
        cout << "Cannot open '" << file << "' file" << endl;
        exit(1);
    }

    // read adjacency matirx
    while (NULL!=fgets(ch,80,fp1)) {
        //     save row in memory
        col=0;
        while (arg_fetch(ch2,ch,col)) {
            num=atoi(ch2);
            if (num==0) num=INFINITY;
            array[rw][col]=num;
            col++;
        }

        // find dim of array, n
        if(rw==0 && col>0) *n=col;

        // check for inconsistent line sizes
        if(rw>0 && *n!=col) {
            cout << "ERROR:  Improper Adjacency Matrix" << endl;
            exit(1);
        }

        // start next row
        // if line not blank, increment row counter
        if (col) rw++;
    }
    if (rw!=col || rw==0) {
        cout << "ERROR:  Improper Adjacency Matrix" << endl;
        exit(1);
    }
    fclose(fp1);
}

// routine that outputs adjacency matrix to screen
void echo_matrix(unsigned int adj_matrix[N][N],int n) {
    int i,j;
    // echo adjacency matrix to screen
    cout << "Adjacency Matrix" << endl;
    for (i=0;i<n;++i) {
        for (j=0;j<n;++j) {
            if (adj_matrix[i][j]==INFINITY) {
                cout << "   0";
            }
            else {
                cout << right << setw(4) << adj_matrix[i][j];
            }
        }
        printf("\n");
    }
}


int next_vertex(int n) {
    int output;
    if (num_ele==0) output=n;
    else {
        output = vrt_queue[get_ele];
        get_ele = ((get_ele+1)%N);
        num_ele--;
    }
    return output;
}

void append_queue(int vert) {
    if (N-1<num_ele) {
        cout << "error: too many verticies in queue" << endl;
        exit(1);
    }
    vrt_queue[put_ele]=vert;
    num_ele++;
    put_ele = ((put_ele+1)%N);
}

// routine that outputs the distance matrix to the screen
void output_distance(unsigned int *distance,int n) {
    int i;
    cout << endl;
    for (i=0;i<n;i++) cout << "Vertex " << i << " => "
                           << distance[i] << endl;
    cout << endl;
}

// routine that sets up the distance vector
void set_distance(unsigned int *distance,int n,int src_vert) {
    int i;
    for (i=0;i<n;i++) distance[i]=INFINITY;
    distance[src_vert]=0; // distance to source vertex is 0
}

void* thread(void *argsPtr)
{
    Arguments *args = ((Arguments*)argsPtr);

    for(unsigned int j = 0; j < num_vertex; j++)
    {
        if (w[args->i][j] != INFINITY)
        {
            unsigned int newdist_j = dist[args->i] + w[args->i][j];
            if (newdist_j < dist[j])
            {
                dist[j] = newdist_j;
                append_queue(j);
            }
        }
    }

    pthread_exit((void*)0);
}


int main(int argc, char *argv[])
{

    if (argc!=2) {
        cout <<"usage: moore [adjacency matrix file]" << endl;
        exit(1);
    }

    // read input adjacency matrix
    read_adj(w,&num_vertex,argv[1]);

    // echo adjacency matrix to screen
    echo_matrix(w,num_vertex);

    // define source vertex
    src_vertex=0;

    // initialize the distance matrix
    // distance to source vertex is zero other distances INFINITY
    set_distance(dist,num_vertex,src_vertex);

    // enter source vertex in queue
    append_queue(src_vertex);

    // see page 220 of text for detailed description of this
    // section of the algorithm
    while (num_ele > 0)
    {
        // Create workers
        list<pthread_t*> workers;
        while (num_ele > 0)
        {
            Arguments *args = new Arguments();
            args->i = next_vertex(num_vertex);

            workers.push_back(new pthread_t());
            pthread_create(workers.back(), NULL, thread, args);
        }
        cout << "Created: " << workers.size() << " thread(s)" << endl;

        // Wait for them to finish
        // Need to wait because the queue will be empty because the
        // threads need to finish their work
        while (!workers.empty())
        {
            pthread_join(*workers.front(), NULL);
            workers.pop_front();
        }
    }

    output_distance(dist,num_vertex);
}

