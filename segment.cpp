/*
 * segment.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: vapashos
 */

#include "segment.h"

int segment::counter=0;


segment::segment() {
	counter++;
	id=counter;
	cout<<"segment with ID"<<id<<" created"<<endl;
}

segment::segment(const segment &s){
	counter++;
	id=s.id;
//	cout<<"inside copy constructor "<<s.packets.empty()<<endl;
	packets=s.packets;
//	cout<<"copy constructor:segment with ID"<<id<<" created";
	//if(!s.packets.empty())
//		cout<<(int)s.packets[0].payload[8];
	//cout<<endl;
}

segment::segment(int id,deque<Packet> &x){
	this->id=id;
	packets=x;
	//cout<<"2arg constructor:segment with ID"<<id<<" created"<<(int)packets[0].payload[8]<<endl;
}

segment::~segment() {
	counter--;
	packets.clear();
	// TODO Auto-generated destructor stub
}



void segment::addPacket(Packet &x){

	if(!packets.empty())
		packets.back().setLast(false);

	x.setLast(true);
	packets.push_back(x);
	size=packets.size();
	//cout<<"segment"<<id<<" add packet p"<<x.id<<" "<<(int)x.payload[4]<<" payload size"<<x.header[2]<<"segmentid"<<x.header[0]<<" index inside seg"<<x.header[1]<<endl;

}

void segment::showSegmentPackets(){
	cout<<"segment"<<id<<":"<<endl;
	for(unsigned int i=0;i<packets.size();i++){
		cout<<"p"<<packets[i].id<<" ";
	}
	cout<<endl;
}

void segment::showSegmentPackets(ofstream &f){
	//f<<"segment"<<id<<":"<<endl;
	for(unsigned int i=0;i<packets.size();i++){
			f<<"p"<<packets[i].id<<" ";
	}
	f<<endl;
}

int segment::getSegmentLastPacketID(){
	for(unsigned int i=0;i<packets.size();i++){
		if(packets[i].isLast()){
			return packets[i].id;
		}
	}
	return -1;
}

int segment::getID(){
	return id;
}

int segment::getSize(){
	return packets.size();
}

