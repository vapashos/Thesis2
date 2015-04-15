/*
 * Packet.cpp
 *
 *  Created on: Feb 17, 2014
 *      Author: vapashos
 */

#include "Packet.h"

int Packet::counter=1;
Packet::Packet() {

	id=counter;
	last=false;
}

Packet::Packet(char *pload,int size,int *subHeader,int segID,bool lastFlag,int id){

	this->id=id;
	last=lastFlag;
	//header[0]=ffNumber(0,ff);
	header[0]=ffNumber(0,ff);
	header[1]=ffNumber(size,ff);
	header[2]=ffNumber(subHeader[0],ff);/*RTP packetID*/
	header[3]=ffNumber(subHeader[1],ff);/*lid*/
	header[4]=ffNumber(subHeader[2],ff);/*tid*/
	header[5]=ffNumber(subHeader[3],ff);/*qid*/
	timeCreated=0;
	for(int i=0;i<MTUSIZE;i++){
		if(i<size)
			payload.push_back(pload[i]);
		else
			payload.push_back(0);
	}
	segmentID=segID;
	segLength=0;
//	cout<<"packet ID"<<id<<" segmentID"<<header[0]<<" size"<<header[2]<<" RTP id="<<header[3]<<" lid="<<header[4]<<" tid="<<header[5]<<" qid="<<header[6]<<endl;

}

Packet::Packet(char *pload,int from,int to,int *subHeader,int segID,bool lastFlag,int id){

	this->id=id;
//	cout<<"payload size to copy="<<to-from+1;
//	header[0]=ffNumber(0,ff);/*segment id*/
	header[0]=ffNumber(0,ff);/*index inside segment*/
	header[1]=ffNumber(MTUSIZE,ff);	/*size*/
	header[2]=ffNumber(subHeader[0],ff);/*RTP packetID*/
	header[3]=ffNumber(subHeader[1],ff);/*lid*/
	header[4]=ffNumber(subHeader[2],ff);/*tid*/
	header[5]=ffNumber(subHeader[3],ff);/*qid*/
	timeCreated=0;
	cout<<"packet id "<<id<<"payload from "<<from<<" to "<<to<<endl;
	for(int i=from;i<=to;i++){
		payload.push_back(pload[i]);
	}

	for(int i=to-from+1;i<MTUSIZE;i++){
		payload.push_back(0);
	}
	segmentID=segID;
	segLength=0;
//	cout<<"packet ID"<<id<<" segmentID"<<header[0]<<" size"<<header[2]<<" RTP id="<<header[3]<<" lid="<<header[4]<<" tid="<<header[5]<<" qid="<<header[6]<<endl;
}


Packet::Packet(unsigned char *pload,int size,ffNumber *hder,int length,int segID,int id){
	/*This constructor used inside overloaded operators creating coding packets*/
	this->id=id;
	for(int i=0;i<6;i++)
			header[i]=hder[i];

	for(int i=0;i<MTUSIZE;i++){
		if(i<size)
			payload.push_back(pload[i]);
		else
			payload.push_back(0);
	}
	char c;
	segmentID=segID;
	segLength=length;
	timeCreated=0;
//	cout<<"packet ID"<<id<<" segmentID"<<header[0]<<" size"<<header[2]<<" RTP id="<<header[3]<<" lid="<<header[4]<<" tid="<<header[5]<<" qid="<<header[6]<<endl;
}


Packet::Packet(const Packet &p){

	id=p.id;
	payload=p.payload;
	coefVector=p.coefVector;
	segmentID=p.segmentID;
	for(int i=0;i<6;i++)
		header[i]=p.header[i];
	timeCreated=p.timeCreated;
	timeSend=p.timeSend;
	senderID=p.senderID;
	receiverID=p.receiverID;
	last=p.last;
	segLength=p.segLength;

}

Packet::~Packet() {
	payload.clear();
	coefVector.clear();
}

/*Set functions */
void Packet::setID(int id){
	this->id=id;
}

void Packet::setSegmentID(int segID,int packIndex){

	segmentID=segID;
	header[0]=ffNumber(packIndex,ff);
	cout<<"packet"<<id<<" i set segmentid:"<<segmentID<<endl;
}

void Packet::setPayload(deque<unsigned char> &pLoad){
		payload=pLoad;
}

void Packet::setSenderID(int sID){
	senderID=sID;
}

void Packet::setReceiverID(int rID){
	receiverID=rID;
}

void Packet::setLast(bool l){
	last=l;
}

void Packet::setCoefVector(const deque<unsigned char> &coef){
	coefVector=coef;
}
void Packet::setTimeCreated(int timeSlot){
	timeCreated=timeSlot;
}

void Packet::setSegmentLength(int length){
	segLength=length;
}


/*Get functions*/

int Packet::getSegmentID() const{
	return segmentID;
}
int Packet::getIndexInsideSegment() const{
	return header[0].number;
}

/*Show function*/

void Packet::showPayload(){
	cout<<"packet "<<id<<" and size "<<payload.size()<<endl;
	cout<<"metadata:segmentID"<<header[0]<<" index inside segment"<<header[1]<<" size in bytes"<<header[2]<<endl;
	for(unsigned int i=0;i<payload.size();i++){
		if(i%10==0)
			cout<<endl;
		cout<<i+1<<")"<<bitset<8>(payload[i])<<" ";
	}
	cout<<"*---------------------------------------*"<<endl;
}

void Packet::showPayload(ofstream &f){
	f<<"packet "<<id<<" and size "<<payload.size()<<endl;
	f<<"metadata:segmentID"<<header[0]<<" index inside segment"<<header[1]<<" size in bytes"<<header[2]<<endl;
	for(unsigned int i=0;i<payload.size();i++){
		if(i%15==0)
			f<<endl;
		f<<i+1<<")"<<bitset<8>(payload[i])<<" ";
		f.flush();
	}
	f<<endl<<"*---------------------------------------*"<<endl;
}


void Packet::showCoefVector(){
	cout<<"packet"<<id<<" coeff Vector:";
	for(unsigned int i=0;i<coefVector.size();i++){
		cout<<(int)coefVector[i]<<" ";
	}
	cout<<endl;
}

bool Packet::comparePackets(const Packet &p1){

	int size=(this->payload.size()<p1.payload.size()) ? this->payload.size():p1.payload.size();
	for(int i=0;i<size;i++){
		if(this->payload[i]!=p1.payload[i])
			return false;
	}
	return true;
}

bool Packet::isLast() const{
	return last;
}

/*Overloaded Operators*/

Packet operator * (const Packet& a,int x){

	/*Update header information */
	ffNumber tempHeader[6];
	/*tempHeader[0] corresponts to segmentID its information that doesn't need to be coded cause we only create
	 * coded packets of packets that belong on the same segment */
	int tempSegID=a.segmentID;
	for(int i=0;i<6;i++){
		//cout<<"temp Table i="<<i+1<<endl;
		tempHeader[i]=a.header[i]*x;
	}

	unsigned char tempPayload[MTUSIZE];


	for(int i=0;i<MTUSIZE;i++){
		tempPayload[i]=ff->mul[a.payload[i]][x];
	}

	return Packet(tempPayload,MTUSIZE,tempHeader,0,a.segmentID,0);
}

Packet operator * (int x,const Packet &a){
	return a*x;
}


Packet operator + (const Packet& a,const Packet& b){
	ffNumber tempHeader[6];
	int tempSegID=a.segmentID;
	for(int i=0;i<6;i++){
		tempHeader[i]=a.header[i]+b.header[i];
	}

	unsigned char tempPayload[MTUSIZE];

	for(int i=0;i<MTUSIZE;i++){
		tempPayload[i]=ff->sum[a.payload[i]][b.payload[i]];
	}

	return Packet(tempPayload,MTUSIZE,tempHeader,0,a.segmentID,0);


}



