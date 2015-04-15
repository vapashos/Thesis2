/*
 * generalFunc.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: vapashos
 */

#include "generalFunc.h"
#include <cstdlib>
#include <sys/time.h>
using namespace std;


finiteField *ff=new finiteField(2,8);
int MTUSIZE=255;

int randomNumber(int min,int max){
    //Generates the random
    struct timeval t;
    gettimeofday(&t,NULL);
    srand(t.tv_usec);
    return min+rand()%(max-min);
}

float randomNumber(int limit){

    //Generates floating random number less than the limit
    struct timeval t;
    gettimeofday(&t,NULL);
    srand(t.tv_usec);
    return float(rand()%(limit))/10.0;
}


int getLinesNumber(ifstream &f){
    int i=0;
    string line;
    while(getline(f,line)) i++;

    f.clear();//Clear the value of the flag eof
    f.seekg(0,ios::beg);//move back to the beginning of the file
    return i;
}

void moveInsideFile(ifstream &f,int moveStep)
{
    //Use this function to move inside the file
    //per line number
    int i=0;
    string line;
    f.clear();
    f.seekg(0,ios::beg);
    while(i<moveStep && f.eof()!=true)
    {
        getline(f,line);
        i++;
    }
}


void showTableInBytes(char *p,int size){
	for(int i=0;i<size;i++){
		if(i%15==0)
			cout<<endl;
		cout<<i+1<<")"<<bitset<8>(p[i])<<" ";
	}
	cout<<endl;
}
