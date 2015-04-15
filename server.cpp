/*
 * server.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: sportbilly
 */

#include "server.h"
#include <cstdlib>
#include <cstring>

server::server(int genSize,string bitStreamFile,string traceFile) {
	generationSize=genSize;
	loadTraceFile(trace,traceFile);
	streamRTPPacketization(bitstream,bitStreamFile);
	cout<<"comparing packets 5 and 6"<<packets[4].comparePackets(packets[5])<<endl;
	segmentation();
	cout<<"after segmentation";
	writeSegmentationDataOnFile("segmentationData.txt");
	Node::showGlobalSegList();
//	showSegmentLastPacketIDs();
	cout<<"packets Number counted "<<RTPpackets<<" and size of "<<packets.size()<<endl;

}

server::~server() {
	// TODO Auto-generated destructor stub
}

void server::loadTraceFile(ifstream &f,string fileName){/*This function opens the trace file and fills the packetMetaData table*/


	f.open(fileName.c_str(),ios::binary);
	RTPpackets=getLinesNumber(trace)-2;
	if(!f.good()){
		cout<<"failure opening trace file "<<endl;
		return;
	}
	int currPos;/*Holds the current possition inside the file while parsing the file*/
	string line,temp;
	moveInsideFile(trace,2);
    while(getline(trace,line)){
        currPos=line.find_first_not_of(" ",10);
        deque<int> tempDeque;
        for(int i=0;i<4;i++){
        	tempDeque.push_back(atoi((temp.assign(strtok(&line[currPos]," "))).c_str()));
            currPos=line.find_first_not_of(" ",currPos+temp.length()+1);
        }
        packetMetaData.push_back(tempDeque);
        tempDeque.clear();
    }
    f.close();
}


void server::streamRTPPacketization(ifstream &bitStream,string fileName){

	if(packetMetaData.empty()){
		cout<<"There are no metadata of packets"<<endl;
		return;
	}

	bitStream.open(fileName.c_str(),ios::binary);
	if(!bitStream.good()){
		cout<<"error while opening bitstream file"<<endl;
		return;
	}

	char *p;
	int ratio,counter=0;
	cout<<"starting the packetization"<<endl;
	for(unsigned int i=0;i<packetMetaData.size();i++){
		p=new char[packetMetaData[i][0]];
		bitStream.read(p,packetMetaData[i][0]);
		/*RTP packet size exceeds MTU then break it into more packets packets*/
		ratio=(packetMetaData[i][0]<=MTUSIZE) ? 1:(packetMetaData[i][0]/MTUSIZE)+1;
		int start=0,end=0;
		int temp[4];
		//	cout<<"RTP size packet "<<packetMetaData[i][0]<<" break to "<<ratio<<" packets"<<endl;
		for(int j=0;j<ratio;j++){
			temp[0]=i+1;
			temp[1]=packetMetaData[i][1];
			temp[2]=packetMetaData[i][2];
			temp[3]=packetMetaData[i][3];
			if(ratio==1){
				packets.push_back(Packet(p,packetMetaData[i][0],temp,4));
				counter++;
			}
			else{
				end=(packetMetaData[i][0]-end+1>MTUSIZE)? ((j+1)*MTUSIZE)-1:(packetMetaData[i][0])-1;
			//	cout<<"RTP packet id"<<i<<" end="<<end<<" start="<<start<<endl;
				packets.push_back(Packet(p,start,end,temp,4));
				start=end+1;
				counter++;
			}
		}
		delete[] p;
	}
	bitStream.close();

}


void server::segmentation(){

	if(packets.empty()){
		cout<<"packetization process have to run first"<<endl;
		return;
	}
	int counter=0;
	for(unsigned int i=0;i<packets.size();i++){

		if(i%generationSize==0)
		{
			counter=0;
			if(!segments.empty())
				segments.back().showSegmentPackets();
			if(i>0)
				Node::globalSegList.push_back(&segments.back());

			segments.push_back(segment());
		}
		packets[i].setSegmentID(segments.back().getID(),counter);
		segments.back().addPacket(packets[i]);
		counter++;
	}
	Node::globalSegList.push_back(&segments.back());
	segments.back().showSegmentPackets();
	cout<<endl<<"segmentation finished num of segments="<<segments.size()<<endl;
}

void server::showSegmentLastPacketIDs(){

	for(unsigned int i=0;i<segments.size();i++){
		cout<<"segment ID:"<<segments[i].id<<" last packetID"<<segments[i].getSegmentLastPacketID()<<endl;;
	}
}

/*Help functions*/

void server::checkPayloadSize(){
	char c;
	for(unsigned int i=0;i<packets.size();i++){
		if(packets[i].payload.size()!=MTUSIZE){
			cout<<"Error in file size packet with id "<<packets[i].id<<" h"<<packets[i].header[1]<<endl;
			cin>>c;
			break;
		}
	}
}


void server::createCodedPacket(const segment &s){
	int r;
	r=randomNumber(0,256);
	Packet Temp=r*s.packets.front();
	for(unsigned int i=1;i<s.packets.size();i++){
			/*Pick up a random number*/
		r=randomNumber(0,256);
		Temp=Temp+r*s.packets[i];
	}
	Temp.id=Packet::counter++;
}


/*Write on File Functions*/

void server::writeSegmentationDataOnFile(string filename){
	ofstream f(filename.c_str());
	if(segments.empty()){
		f<<"file empty"<<endl;
		f.close();
	}
	for(unsigned int i=0;i<segments.size();i++){
		f<<"-------------------------------  segment"<<segments[i].getID()<<" ---------------------------------------"<<endl<<endl;
		for(unsigned int j=0;j<segments[i].packets.size();j++){
			f<<"** packet"<<segments[i].packets[j].id<<" segmentID="<<segments[i].packets[j].segmentID
					<<" indexInSeg="<<segments[i].packets[j].header[0].number
					<<" sizeBytes="<<segments[i].packets[j].header[1].number
					<<" RTPPackID="<<segments[i].packets[j].header[2].number
					<<" lid="<<segments[i].packets[j].header[3].number
					<<" tid="<<segments[i].packets[j].header[4].number
					<<" gid="<<segments[i].packets[j].header[5].number<<endl;
			for(unsigned int z=0;z<segments[i].packets[j].payload.size();z++){
				f<<(z+1)<<") ["<<(int)segments[i].packets[j].payload[z]<<"]"<<(bitset<8>) segments[i].packets[j].payload[z]<<" ";
				if(z%13==0 && z!=0)
					f<<endl;
			}
			f<<endl;
			f<<"****************END OF packet"<<segments[i].packets[j].id<<" ***************************"<<endl;
		}
		f<<"--------------------- END OF segment"<<segments[i].getID()<<" ---------------------------------------"<<endl<<endl;
	}
	f.close();

}



