/*
 * Simulator.cpp
 *
 *  Created on: Feb 17, 2014
 *      Author: sportbilly
 */

#include "Simulator.h"



Simulator::Simulator(int nodesNum,string bitStreamfileName,string traceFileName){

	int genSize=10;
	cout<<"Generation Size:"<<genSize<<endl;
	//cin>>genSize;
	/*The number of 3G channels are equal to the number of users*/
	s=new server(genSize,bitStreamfileName,traceFileName);
	for(int i=0;i<nodesNum;i++){
		Node temp=Node();
		/*Init node list*/
		nodeList.push_back(temp);
		/*Init 3G channel List*/
		channel3GList.push_back(channel3G(s,&nodeList.back(),randomNumber(7)));/*We assume that 3G channels loss probabilities range from 0.1 to 0.7*/
	}
	cout<<"Simulation with "<<nodeList.size()<<" nodes created"<<endl;
	allocateSegmentsOnUsers(s);
	broadcastNode=NULL;
}

Simulator::~Simulator() {
	// TODO Auto-generated destructor stub
}

void Simulator::createMeshTopology(){
	for(unsigned int i=0;i<nodeList.size();i++){
		for(unsigned int j=0;j<nodeList.size();j++)
			nodeList[i].addNeighbor(&nodeList[j]);
	//	nodeList[i].initChannelWLAN();
	}
}


bool Simulator::setBroadcastNode(){
	//Decide between nodes with no empty outgoing queues

		deque<Node *> tempList;
		for(unsigned int i=0;i<nodeList.size();i++){
			if(!nodeList[i].isEmpty(nodeList[i].outQueue)){
				tempList.push_back(&nodeList[i]);
			}
		}

		if(tempList.empty()){
			cout<<"There is no node that has data to transmit"<<endl;
			broadcastNode=NULL;
			return false;
		}
		if(tempList.size()==1){
			broadcastNode=tempList.front();
			broadcastNode->bdSegment=Node::getSegment(broadcastNode->outQueue.back().segmentID);
			return true;
		}
		//broadcastNode=tempList.front();
		broadcastNode=tempList[randomNumber(0,tempList.size())];
		broadcastNode->bdSegment=Node::getSegment(broadcastNode->outQueue.front().segmentID);

		cout<<"Broadcast node selected "<<broadcastNode->id<<" with segment to Broadcast"<<broadcastNode->bdSegment->id<<endl;
		return true;
}

void Simulator::allocateSegmentsOnUsers(server *s){
/*stream to capture segments data while allocating them on users*/
	ofstream f("SegmentAllocationOnUsers.txt");
	for(unsigned int i=0;i<s->segments.size();i++){
		f<<"node"<<i%nodeList.size()+1<<" seg"<<s->segments[i].getID()<<endl;
		nodeList[i%nodeList.size()].addSegmentToDownloadList(&s->segments[i]);
		for(unsigned int j=0;j<channel3GList.size();j++){
			if(channel3GList[j].receiver->id==nodeList[i%nodeList.size()].id){
				f<<"packetData:"<<endl;
				for(unsigned int k=0;k<s->segments[i].packets.size();k++){
					channel3GList[j].addPacket(s->segments[i].packets[k]);
					s->segments[i].packets[k].showPayload(f);
				}
			}
		}
	}
	f.close();
	/*Show packets on channels*/
	showChannelBuffer();
}

void Simulator::simulate3GTransmissions(int timeSlot){

	channel3G::status=0;
	Node::useWlan=true;/*Each time slot this flag is set to true so that any user can use the wlan*/
	for(unsigned int i=0;i<channel3GList.size();i++){
		channel3GList[i].sendPacket(timeSlot);
	}
}

void Simulator::simulateWIFIBroadcasts(int timeSlot){
	cout<<"timeslot "<<timeSlot+1<<"inside simulateWIFIBroadcasts"<<endl;
	if(timeSlot==0){
		broadcastNode=NULL;
	}
	bool flag=true;
	int packetsToRetransmit;
	segment *s ;
	for(unsigned int i=0;i<nodeList.size();i++){
		packetsToRetransmit=0;
		cout<<"----------------  Node "<<nodeList[i].id<<" timeslot="<<timeSlot<<"---------------------------"<<endl;
			flag=flag&nodeList[i].broadcastPacket(timeSlot,broadcastNode);
			nodeList[i].recvPacket(timeSlot);
			cout<<" ************   Checking for Messages  ****************"<<endl;
			nodeList[i].sendMessage(timeSlot);
			nodeList[i].recvMessage(timeSlot);
			nodeList[i].showPacketQueue(nodeList[i].inQueue,"in Packet Queue");
			cout.flush();
			nodeList[i].showPacketQueue(nodeList[i].outQueue,"out Packet Queue");
			cout<<"-------- Simulator checking message list -------"<<endl;
			nodeList[i].showMsgQueue(nodeList[i].inMsgQueue,"in Message Queue");
			nodeList[i].showMsgQueue(nodeList[i].outMsgQueue,"out Message Queue");
			cout.flush();

			s=(nodeList[i].inMsgQueue.empty() ? NULL:nodeList[i].inMsgQueue.front()->segmentPtr);
			packetsToRetransmit=nodeList[i].checkRequests();

			if(packetsToRetransmit>0 && s!=NULL){	/*Code for adding  coded packets inside outPacketQueue*/
				cout<<"on timeSlot:"<<timeSlot<<"node "<<nodeList[i].id<<" has to send additional "<<packetsToRetransmit<<endl;
				nodeList[i].createPoolOfCodedPackets(s,packetsToRetransmit,timeSlot);
			}

			cout<<"-------- Simulator end checking message list -------"<<endl;
			cout.flush();
			nodeList[i].showSegInProgressMapping();
			nodeList[i].showcoefTables();
			nodeList[i].checkIncomingCodedPackets(timeSlot);
			cout.flush();
			cout<<" **********  End Checking for Messages  ****************"<<endl;
		cout<<"-----------------  End Node "<<nodeList[i].id<<" timeslot="<<timeSlot<<"---------------------------"<<endl;
		cout.flush();
	}
	if(flag){
		cout<<"on timeslot "<<timeSlot+1<<" i will set new brodacast Node if any"<<endl;
		setBroadcastNode();
	}
}


bool Simulator::checkNodeQueues(int timeSlot){/*returns true if there is no message to be transmitted or no packet to be transmitted over the network*/
	bool flag=true;
	for(unsigned int i=0;i<nodeList.size();i++){
		if(!nodeList[i].outQueue.empty() || !nodeList[i].outMsgQueue.empty() || !nodeList[i].inMsgQueue.empty())
			return false;
	}
	return true;
}


void Simulator::showNodesBuffer(){
	for(unsigned int i=0;i<nodeList.size();i++){
		cout<<"node "<<nodeList[i].id;
		nodeList[i].showPacketQueue(nodeList[i].queue3G,"queue3G");
	}
}

void Simulator::showChannelBuffer(){

	for(unsigned int i=0;i<channel3GList.size();i++){
		channel3GList[i].showBuffer();
	}
}

void Simulator::showNodeSegmentList(){
	for(unsigned int i=0;i<nodeList.size();i++){
		nodeList[i].showSegQueue(nodeList[i].segQueue,"segments");
	}
}

