#include <iostream>

using namespace std;

int TopDownMergeSort(int [], int [], int);
void TopDownSplitMerge(int [], int, int, int []);
int TopDownMerge(int [], int, int, int, int []);
void CopyArray(int B[], int iBegin, int iEnd, int A[]);

int main()
{
	int A[10] = {4, 3, 6, 5, 9, 7, 8, 2, 10, 1};
	int B[10];

	TopDownMergeSort(A, B, 10);

	for(int i = 0; i<10; i++)
	{
		cout << A[i]<< " ";
	}
	cout << "MAIN" << endl;

	cout << endl;

	return 0;
}

int TopDownMergeSort(int A[],int B[], int n)
{
    TopDownSplitMerge(A, 0, n, B);
}
 
void TopDownSplitMerge(int A[], int iBegin, int iEnd, int B[])
{
    if(iEnd - iBegin < 2)                       // if run size == 1
        return;                                 //   consider it sorted
    // recursively split runs into two halves until run size == 1,
    // then merge them and return back up the call chain
    int iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
    TopDownSplitMerge(A, iBegin,  iMiddle, B);  // split / merge left  half
    TopDownSplitMerge(A, iMiddle, iEnd,    B);  // split / merge right half
    TopDownMerge(A, iBegin, iMiddle, iEnd, B);  // merge the two half runs
    CopyArray(B, iBegin, iEnd, A);              // copy the merged runs back to A
}
 
int TopDownMerge(int A[], int iBegin, int iMiddle, int iEnd, int B[])
{
    int i0 = iBegin, i1 = iMiddle;
 
    // While there are elements in the left or right runs
    for (int j = iBegin; j < iEnd; j++) {
        // If left run head exists and is <= existing right run head.
        if (i0 < iMiddle && (i1 >= iEnd || A[i0] <= A[i1]))
            B[j] = A[i0++];  // Increment i0 after using it as an index.
        else
            B[j] = A[i1++];  // Increment i1 after using it as an index.
    }

}

void CopyArray(int B[], int iBegin, int iEnd, int A[])
{
	for(int i = iBegin; i < iEnd; i++)
	{
		A[i] = B[i];
	}
}
