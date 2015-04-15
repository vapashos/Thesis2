/*
 * generalFunc.h
 *
 *  Created on: Feb 18, 2014
 *      Author: vapashos
 */

#ifndef GENERALFUNC_H_
#define GENERALFUNC_H_
#include <iostream>
#include <fstream>
#include <bitset>
#include "ffNumber.h"
using namespace std;
int randomNumber(int min,int max);
float randomNumber(int limit);
int getLinesNumber(ifstream &f);
void moveInsideFile(ifstream &f,int moveStep);
void showTableInBytes(char *p,int size);
extern finiteField *ff;
extern int MTUSIZE;
#endif /* GENERALFUNC_H_ */
