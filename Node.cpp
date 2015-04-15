/*
 * Node.cpp
 *
 *  Created on: Feb 17, 2014
 *      Author: vapashos
 */

#include "Node.h"
#include <algorithm>
#include <sstream>
int Node::counter=0;
bool Node::useWlan=true;
channelWlan Node::wChannel=channelWlan(0.1);
deque<segment*> Node::globalSegList;
ofstream Node::fl("codedPackets.txt");
Node::Node() {
	counter++;
	id=counter;
	index=0;
	cout<<"Create node with id "<<id<<endl;
	initiateDecoder();/*Initialize decoder assign myDecoder pointer on decoder of finite field 2^8*/
	/*initialize all pointer to NULL*/
	bdSegment=NULL;
}

Node::Node(const Node &n)
{
	id=n.id;
	inQueue=n.inQueue;
	outQueue=n.outQueue;
	index=n.index;
	//sendChannels=n.sendChannels;
	//recvChannels=n.recvChannels;
	neighborList=n.neighborList;
	myDecoder=n.myDecoder;
}

Node::~Node() {
	//fl.close();
	// TODO Auto-generated destructor stub
}

/*functions */
void Node::addNeighbor(Node *x)
{
	if(x->id!=id && find(neighborList.begin(),neighborList.end(),x)==neighborList.end()){
		neighborList.push_back(x);
	}
}


void Node::sendPacket(int timeSlot,int receiverID){

	cout<<"Node "<<id<<" sendPacket:";
	if(outQueue.empty()){
		cout<<"There is no packet to be send"<<endl;
		return;
	}
	cout<<"outQueue size"<<outQueue.size()<<endl;
	showPacketQueue(outQueue,"out");
	Packet Temp=outQueue.front();

	if(randomNumber(10)>Node::wChannel.lossProb){
		Temp.timeSend=timeSlot;
		Temp.setReceiverID(receiverID);
		Temp.setSenderID(id);
		Node::wChannel.buffer.push_back(Temp);
	}
	else{
		cout<<"Failed to send packet "<<Temp.id<<endl;
		//outQueue.push_back(Temp);
	}
	outQueue.pop_front();
}


void Node::recvPacket(int timeSlot){

	cout<<"Node"<<id<<" recvPacket:";

	if(Node::wChannel.buffer.empty() ){
		cout<<"there are no packets to download"<<endl;
		return;
	}

	segment *s;
	channelWlan *q=&Node::wChannel;
	cout<<"node "<<id<<" inside RECVPacket "<<q->buffer.size();
	for(deque<Packet>::iterator it=q->buffer.begin();it!=q->buffer.end();it++){
		if(it->receiverID==id){
			cout<<"found packet for me";
			if(it->timeSend<timeSlot){
				Packet myPacket=*it;
				s=getSegment(myPacket.segmentID);
				/*check if the packet to receive is a coded packet that belongs to a previously decoded segment*/
				if(decodedSegments[s]==false || decodedSegments.find(s)==decodedSegments.end()){
					cout<<"it is a packet that doesn't belong to previously decoded segment"
							"and i will receive it right now"<<endl;
					if(segInProgress[s]++==1){/*Update segInProgress mapping <segment,counter of packets that belong in segment>*/
						decodedSegments[s]=false;
					}
					addCoefVector(myPacket.coefVector,s);//!=NULL){/*Add coefficient vector on local table in node if result is not null start decoding*/
					inQueue.push_back(myPacket);
					cout<<"Add packet "<<myPacket.id<<" in my inQueue"<<endl;
				}
				else{
					cout<<"i was previously decoded that segment i don't need that packet i'll drop it"<<endl;
				}
				q->buffer.erase(it);
				showPacketQueue(inQueue,"in");
				return;
			}
			cout<<"but i will receive it next timeslot"<<endl;
		}
	}

	cout<<"there was no packet destined for me"<<endl;
}


void Node::recv3GPacket(Packet &p,int timeslot){

	cout<<"Node"<<id<<" recv3GPacket:";
	queue3G.push_back(p);
//	cout<<"recv3GPacket packet "<<queue3G.back().id<<" in buffer  added with payload "<<(int)queue3G.back().payload[8]<<endl;
	if(queue3G.back().isLast()){
		cout<<"!!!!!!!!!!  user"<<id<<" finished downloading segment "<<p.getSegmentID()<<" on timeslot "<<timeslot+1<<endl;
		cout<<" now create list of coded packets for sending "<<endl;
		ofstream fz("3GDownloads.txt",fstream::app);

		deque<Packet> temp;
		for(unsigned int i=index;i<queue3G.size();i++){
			temp.push_back(queue3G[i]);
		}
		/*Create new segment and add it on corresponding queue*/
		int segmentID=queue3G.back().getSegmentID();
		fz<<"node "<<id<<" downloaded successfully segment "<<segmentID<<" on timeslot "<<timeslot<<endl;
		fz.close();
		segQueue.push_back(segment(segmentID,temp));
//		cout<<"segQueue packet in buffer "<<segQueue.back().packets.front().id<<" added with payload "<<(int)segQueue.back().packets.front().payload[8]<<endl;
		/*Update index variable*/
		index=queue3G.size();
		/*After that the user will make initial push of packets on a user
		 * but all the transmissions are pseudo broadcast so it will broadcast
		 * the segment*/
//		cout<<"i'll created coded packets of the segment"<<segQueue.back().id<<endl;
		/*Create coded packets equal to segment's size and add them to outgoing queue*/
//		cout<<"before creating coded packets size of outqueue"<<outQueue.size()<<endl;
		for(unsigned int i=0;i<segQueue.back().packets.size();i++){
			outQueue.push_back(createCodedPacket(segQueue.back(),timeslot));
			outQueue.back().showCoefVector();
		}
//		cout<<"after creating coded packets size of outqueue"<<outQueue.size()<<endl;
	}
}


bool Node::broadcastPacket(int timeSlot,Node *brNode){

	bool broadcastFlag=true;
	cout<<"Node"<<id<<" broadcastPacket:";
	if(outQueue.empty()){
		cout<<"Node "<<id<<" has nothing to broadcast"<<endl;
		return broadcastFlag;
	}

	if(brNode==NULL){
		cout<<"broadcast node is not been set yet"<<endl;
		return broadcastFlag;
	}

	if(this!=brNode){
		cout<<"i cannot transmit the channel is in use"<<endl;
		return broadcastFlag;
	}
	int segmentID;
	Packet Temp=outQueue.front();
	segmentID=Temp.getSegmentID();/*This is needed when its the time to create the advertisment*/

	//if(bdSegment!=)


	if(timeSlot>Temp.timeCreated){
		for(unsigned int i=0;i<neighborList.size();i++){
			if(randomNumber(10)>Node::wChannel.lossProb){
				Temp.timeSend=timeSlot;
				Temp.setReceiverID(neighborList[i]->id);
				Temp.setSenderID(id);
				Node::wChannel.buffer.push_back(Temp);
			}
		}
		broadcastFlag=false;
		outQueue.pop_front();
		//if(outQueue.empty() || outQueue.back().segmentID!=bdSegment->id){
		/*Αλλαγή γιατι κάποιος κόμβος μπορει να παραχωρήσει τη σειρά του σε άλλον αν εκεινη τη στιγμή που κάνει
		 * broadcast τύχει και κατεβάσει segment και δημιουργήσει καινούρια coded packets*/
		if(outQueue.empty() || outQueue.front().segmentID!=bdSegment->id){
			cout<<"Finished sending packets i'll release the flag"<<endl;
			broadcastFlag=true;
			/*After finishing initial push create advertisment one for each of the neighbors*/
			createAdvertisment(segmentID,timeSlot);
		}
	}
	else{
	//	cout<<"Transmit next timeSlot current TimeSlot="<<timeSlot<<" packet Created on "<<Temp.timeCreated<<endl;
	}
	cout<<"******************************"<<endl;
	Node::wChannel.showChannelContents();
	cout<<"******************************"<<endl;
	return broadcastFlag;
}


void Node::sendMessage(int timeSlot){

	cout<<"Node"<<id<<" sendMessage:";

	if(outMsgQueue.empty()){
		cout<<"Node"<<id<<" outMsgQueue is empty"<<endl;
		return;
	}
	message *Temp;
	if(timeSlot>outMsgQueue.front()->timeCreated){
		if(outMsgQueue.front()->isAdvertisment()){
			cout<<"This is an Advertisment"<<endl;
			for(unsigned int i=0;i<neighborList.size();i++){
				if(randomNumber(10)>Node::wChannel.lossProb){
					Temp=new advertisment(outMsgQueue.front(),timeSlot);
					Temp->timeSend=timeSlot;
					Temp->receiverID=neighborList[i]->id;
					Node::wChannel.msgBuffer.push_back(Temp);
				}
				else{
					cout<<"Failed to send message "<<Temp->id<<endl;
					//outQueue.push_back(Temp);
				}
				cout<<"MessageQueue after advertisment send "<<endl;
				showMsgQueue(Node::wChannel.msgBuffer,"wchannelMsgBuffer");
			//	cin>>c;
			}
			Node::wChannel.showMsgBuffer();
		}
		else{
			Temp=new request(outMsgQueue.front(),timeSlot);
			Temp->timeSend=timeSlot;
			Node::wChannel.msgBuffer.push_back(Temp);
		}
		outMsgQueue.pop_front();
	}
	else{
		cout<<"i will send message next timeslot"<<endl;
	}
}

void Node::recvMessage(int timeSlot){

	cout<<"Node"<<id<<" recvMessage:";

	if(Node::wChannel.msgBuffer.empty() ){
		cout<<"empty global message list"<<endl;
		return;
	}

		channelWlan *q=&Node::wChannel;
		cout<<"node "<<id<<" inside RECVMessage "<<q->msgBuffer.size()<<endl;
		q->showMsgBuffer();
		deque<message*>::iterator last;
		last=q->msgBuffer.end();
		last--;
		for(deque<message*>::iterator it=q->msgBuffer.begin();it!=q->msgBuffer.end();it++){
			if((*it)->receiverID==id){
				if((*it)->timeSend<timeSlot){
					message *myMsg=*it;
					/*Ο κόμβος απο τη στιγμή που παραλάβει ένα message δεν το τοποθετεί
					 * στην inMsgQueue παίρνει το advertisment και αμέσως δημιουργεί
					 * request προσθέτοντας το στην outMsgQueue έχοντας υπολογίσει τον
					 * αριθμό των πακέτων που χρειάζεται*/
					/*inMsgQueue.push_back(myMsg);*/
					//char c;
					q->showMsgBuffer();
					cout<<"message is "<<myMsg->getMessageType()<<endl;
					//cin>>c;
					if(myMsg->isAdvertisment()){
						/*Advertisment Case*/
						/*Υπολογισμός Πακέτων που λείπουν για να συμπληρώσει για την κωδικοποίηση του segment*/
						int packetsToRequest=myMsg->segmentPtr->getSize()-segInProgress[myMsg->segmentPtr];
						if(packetsToRequest>0){/*Create request message only if packetsToRequest is more than 0*/
							cout<<"create request after accepting the advertisment,Request is for "<<packetsToRequest<<" packets"<<endl;
							cout<<"before creating request"<<endl;
							createRequest(myMsg->senderID,myMsg->segmentID,packetsToRequest,timeSlot);
						//outMsgQueue.push_back(new request(getSegment(myMsg->segmentID),id,myMsg->senderID,packetsToRequest,timeSlot));//request(myMsg,timeSlot));
							cout<<"getType message"<<outMsgQueue.back()->getMessageType()<<endl;
							showMsgQueue(outMsgQueue,"out");
						}
					}
					else{
						/*Request case*/
						cout<<"found request for segment "<<myMsg->segmentPtr->id<<"dimensions required:"<<myMsg->getDimensions()<<endl;
						inMsgQueue.push_back(myMsg);
					}

					cout<<"Add message "<<myMsg->id<<"with receiver id"<< myMsg->receiverID<<" in my inQueue"<<endl;
					if(q->msgBuffer.size()==1){
						q->msgBuffer.clear();
						break;
					}
					else if(it==last){
						q->msgBuffer.pop_back();
						break;
					}
					else{
						q->msgBuffer.erase(it);
					}
				}
					//return;
				cout<<"timeslot:"<<timeSlot<<"wait for next timeslot to get the message"<<endl;
			}
		}
}


bool Node::isEmpty(const deque<Packet> &queue){
	return queue.empty();
}

void Node::createRequest(int receiverID,int segID,int dimensions,int timeSlot){
	cout<<"Node"<<id<<" createRequest:";
	outMsgQueue.push_back(new request(getSegment(segID),id,receiverID,dimensions,timeSlot));
	cout<<"After creating request check Type "<<outMsgQueue.back()->isRequest()<<"request receiver "<<outMsgQueue.back()->getDimensions()<<endl;

}

void Node::createAdvertisment(int segID,int timeSlot){

	cout<<"Node"<<id<<" createAdvertisment:";
	cout<<"inside create advertisment "<<segID<<endl;
	outMsgQueue.push_back(new advertisment(getSegment(segID),id,0,timeSlot));

}


void Node::addSegmentToDownloadList(segment *x){
	cout<<"Node"<<id<<" addSegmentToDownloadList:";
	cout<<"Node "<<id<<" add to toDownloadList segment with id "<<x->id<<endl;
	segToDownloadList[x]=false;
}

void Node::checkSegmentDownload(){

	map<segment*,bool>::iterator i;
	for(i=segToDownloadList.begin();i!=segToDownloadList.end();i++){

	}
}


Packet Node::createCodedPacket(const segment &s,int timeSlot) const{


	deque <unsigned char > r;
	r.push_back(randomNumber(0,256));
	Packet Temp=r.back()*s.packets.front();
	for(unsigned int i=1;i<s.packets.size();i++){
	/*Pick up a random number*/
		r.push_back(randomNumber(0,256));
		Temp=Temp+r.back()*s.packets[i];
	}
	Temp.setCoefVector(r);
	Temp.id=Packet::counter++;
	Temp.setTimeCreated(timeSlot);
	/*Write Data on file for Tracing purposes*/

	fl<<"Coded Packet of segment"<<s.id<<" coef vector =[";
	for(unsigned int i=0;i<r.size();i++){
		fl<<(int)r[i]<<" ";
	}
	fl<<"]"<<endl;
	fl<<"------------   Packet id"<<Temp.id<<" --------"<<endl;
	fl<<"Header Info segID"<<Temp.segmentID;
	fl<<" indx="<<Temp.header[0].number;
	fl<<" size="<<Temp.header[1].number;
	fl<<" RTPid="<<Temp.header[2].number;
	fl<<" lid="<<Temp.header[3].number;
	fl<<" tid="<<Temp.header[4].number;
	fl<<" qid="<<Temp.header[5].number<<endl;
	for(unsigned int i=0;i<Temp.payload.size();i++){
		fl<<i+1<<")["<<(int)Temp.payload[i]<<"]"<<(bitset<8>)Temp.payload[i]<<" ";
	}
	fl<<endl;


	return Temp;

}


void Node::createPoolOfCodedPackets(segment *s,int num,int timeSlot) {/*creates a number of new coded packets based on segment packets are added directly on node's output queue using function createCodedPacket*/

	for(int i=0;i<num;i++){
		//outQueue.push_back(createCodedPacket(*s,timeSlot));
		//I will put the packets that are created as recovery packet of packet losses in front of outQueue
		outQueue.push_front(createCodedPacket(*s,timeSlot));
	}
	this->showPacketQueue(outQueue,"out");
}


void Node::checkIncomingCodedPackets(int timeSlot){
	/*this function is called whenever an advertisment is received*/
	cout<<"Node"<<id<<" checkIncomingCodedPackets:";
	if(segInProgress.empty()){
		cout<<"There is no segment in progress to download"<<endl;
		return;
	}
	char c;
	for(map<segment*,int>::iterator it=segInProgress.begin();it!=segInProgress.end();it++){
		/*If packets of segment downloaded equals to segment size and decoding of the segment never happened(false)*/
		/*i can start decoding*/

		deque<deque<Packet>::iterator> itTable;
		if(it->second==it->first->size && !decodedSegments[it->first]){
			cout<<"Node "<<id<<" i'm going to start decoding!!!!!!!!!!!!!!!!!"<<endl;
			this->showcoefTables();
			myDecoder->setMatrix(coefTable[it->first]);
			cout<<"coefficient tables shown by Decoder"<<endl;
			myDecoder->showMatrix();
			/*Create packet list for the packets that are going to be decoded
			 * we need to pass this as argument inside decode Packets function*/
			/*for(unsigned int i=0;i<inQueue.size();i++){
				if(inQueue[i].getSegmentID()==it->first->getID()){
					decodedMapping[it->first].push_back(inQueue[i]);
					cout<<"packet with id "<<decodedMapping[it->first].back().id<<" added "<<endl;
				}
			}*/
			/*Use Iterator case*/
			bool flag;
			cout<<"before start decoding size of inQueue "<<inQueue.size()<<endl;
			showPacketQueue(inQueue,"in");
			for(deque<Packet>::iterator z=inQueue.begin();z!=inQueue.end();z++){
				if(z->getSegmentID()==it->first->getID()){
					decodedMapping[it->first].push_back(*z);
					if(inQueue.size()==1 || z->id==inQueue.back().id){
						cout<<"before erase "<<z->id<<endl;
						cout<<"packet with id "<<decodedMapping[it->first].back().id<<" added corresponting to segment "<<decodedMapping[it->first].back().getSegmentID()<<endl;
						inQueue.pop_back();
						break;
					}
					else{
						cout<<"before erase "<<z->id<<endl;
						if(z->id!=inQueue.front().id)
							flag=true;
						inQueue.erase(z);

					}
					cout<<"packet with id "<<decodedMapping[it->first].back().id<<" added corresponting to segment "<<decodedMapping[it->first].back().getSegmentID()<<endl;
					//itTable.push_back(z);
					//cout<<"remove packet"<<itTable.back()->id<<endl;
					if(flag){
						z--;
						flag=false;
					}
				}
			}


			//cout<<"TEMP PACKET LIST SIZE"<<tempPacketList.size()<<endl;
			/*After exiting for loop tempPacketList will contain only coded packets
			 * that belong to the segment that is going to be decoded*/
			stringstream ss;
			ss << id;
			string str = ss.str();
			string filename="Node"+str+"decSeg.txt";
			string fileNameBefDec="Node"+str+"BefDecoding.txt";
			/*Write Data on file before decoding operation*/
			befDec.open(fileNameBefDec.c_str(),ios::out | ios::app);
			befDec<<"packet"<<decodedMapping[it->first].back().id<<" timeslot"<<timeSlot<<endl;
			decodedMapping[it->first].back().showPayload(befDec);/*print payload of packet inside file*/
			/*################## Decode Packet ###########################*/
			myDecoder->decodePackets(decodedMapping[it->first]);
			/*################## End of Packet Decoding  #################*/
			/*Write Data on file after decoding operation*/
			string fileNameAfterDec="Node"+str+"AfterDecoding.txt";
			afterDec.open(fileNameAfterDec.c_str(),ios::out | ios::app);
			afterDec<<"packet"<<decodedMapping[it->first].back().id<<" timeslot"<<timeSlot<<endl;
			decodedMapping[it->first].back().showPayload(afterDec);/*print payload of packet inside file*/

			/*Check the segment as decoded*/
			decodedSegments[it->first]=true;

			ds.open(filename.c_str(),ios::out | ios::app);
			ds<<"segment"<<it->first->id<<" timeSlot:"<<timeSlot<<endl;

			/*Write in file data of segment that has been decoded*/
			it->first->showSegmentPackets(ds);
			/*this is new code for deleting elements from in Queue using the itTable table of iterators*/
			//cout<<"Before erasing packets used for decoding "<<inQueue.size()<<" and iterTable "<<itTable.size()<<endl;
		/*	cout<<"show list that is going to be erased"<<endl;
			for(unsigned int pp=0;pp<itTable.size();pp++){
				cout<<"p"<<itTable[pp]->id<<" ";
			}
			cout<<endl;
			showPacketQueue(inQueue,"in");*/
		/*	for(unsigned int pp=itTable.size()-1;pp>=0;pp--){

				int xid=itTable[pp]->id;
				inQueue.erase(itTable[pp]);
				cout<<"erased packet <<"<<xid;
				cout<<" after erasing "<<inQueue.size()<<endl;
				//cin>>c;
			}*/
			cout<<"size of inQueue after decoding "<<inQueue.size()<<endl;
			showPacketQueue(inQueue,"in");
			//itTable.clear();
			//cin>>c;
			ds.flush();
			befDec.flush();
			afterDec.flush();
			ds.close();
			break;
		}
	}
}



segment* Node::addCoefVector(deque<unsigned char> &c,segment *segID){

	/*Create mapping with segmentID - 2 dimensional Array of coefficients*/
	for(map<segment*,deque<deque<unsigned char> > >::iterator it=coefTable.begin();it!=coefTable.end();it++){
		if(it->first==segID){
			it->second.push_back(c);
			if(segID->getSize()==(int)it->second.size())
				return segID;
			else
				return NULL;
		}

	}
	deque<deque <unsigned char> > tempDeque;
	tempDeque.push_back(c);
	coefTable[segID]=tempDeque;
	return NULL;
}


segment * Node::getSegment(int segID){
	for(unsigned int i=0;i<globalSegList.size();i++){
		if(globalSegList[i]->id==segID){
			return globalSegList[i];
		}
	}
	return NULL;
}


void Node::initiateDecoder(){

	myDecoder=new decoder(ff);
}

int Node::checkRequests(){
/*With this function node checks its requests inside inMsgQueue and
 * returns , determines the maximum dimension to send*/
	cout<<"Node"<<id<<" checkRequests:";
	if(inMsgQueue.empty()){
		cout<<"There is no incoming Message";
		return -1;
	}
	int maxDim=0;

	if(inMsgQueue.front()->isAdvertisment()){
		cout<<"I have to handle advertisment reception "<<endl;
		return -1;
	}
	segment* currSeg=NULL;
	for(deque<message *>::iterator it=inMsgQueue.begin();it!=inMsgQueue.end();it++){
			if((*it)->isRequest()){
				currSeg=(*it)->segmentPtr;
				break;
			}
	}

	for(deque<message *>::iterator it=inMsgQueue.begin();it!=inMsgQueue.end();it++){
		if((*it)->isAdvertisment() || currSeg==NULL)
			break;
		if(currSeg!=(*it)->segmentPtr)
			break;

		if(maxDim<(*it)->getDimensions())
			maxDim=(*it)->getDimensions();

		if(inMsgQueue.size()==1){
			inMsgQueue.clear();
			break;
		}
		else
			inMsgQueue.erase(it);

	}

	/*for(unsigned int i=0;i<inMsgQueue.size();i++){

		if(inMsgQueue[i]->isRequest()){
			if (maxDim<inMsgQueue[i]->getDimensions())
				maxDim=inMsgQueue[i]->getDimensions();
		}
	}*/
	return maxDim;
}

/*Show functions */
void Node::showNeighborList(){
	cout<<"Node"<<id<<" showNeighborList:";
	cout<<"NeighborList of node"<<id<<":";
	if(neighborList.empty()){
		cout<<"EMPTY"<<endl;
		return;
	}

	for(unsigned int i=0;i<neighborList.size();i++){
		cout<<"n"<<neighborList[i]->id<<" ";
	}
	cout<<endl;
}

void Node::showPacketQueue(const deque<Packet> &queue,string type){
	cout<<"Node"<<id<<" showPacketQueue:";
	cout<<"node"<<id<<" packet "<<type<<" Queue list:";
	if(queue.empty()){
		cout<<"EMPTY"<<endl;
		return;
	}
	cout<<endl;
	deque<int> temp;
	map<int,int> tempMap;/*this has segmentid , times of reference*/
	for(unsigned int i=0;i<queue.size();i++){
		if(i%15==0)
			cout<<endl;
		cout<<"p"<<queue[i].id<<" ";
		tempMap[queue[i].getSegmentID()]++;
	}
	cout<<endl;

	for(unsigned int i=0;i<temp.size();i++){
		if(i==0)
			cout<<"segments Completed ";
		cout<<"s"<<temp[i]<<" ";
	}
	cout<<endl;
	cout<<"results :"<<endl;
	for(map<int,int>::iterator it=tempMap.begin();it!=tempMap.end();it++){
		cout<<"segmentid"<<it->first<<":"<<it->second<<endl;
	}


}

void Node::showMsgQueue(deque<message*> &queue,string type){
	cout<<"Node"<<id<<" showMsgQueue:";
	cout<<"node"<<id<<" "<<type<<" Queue Message list:";
	if(queue.empty()){
		cout<<"EMPTY"<<endl;
		return;
	}
	cout<<endl;
	for(unsigned int i=0;i<queue.size();i++){
		cout<<queue[i]->getMessageType()<<queue[i]->id<<" ";
	}
	cout<<endl;
	for(unsigned int i=0;i<queue.size();i++){
			cout<<"receiver "<<queue[i]->receiverID<<" ";
	}
	cout<<endl;
	for(unsigned int i=0;i<queue.size();i++){
		cout<<"dimRequested "<<queue[i]->getDimensions()<<" ";
	}
	cout<<endl;
}

void Node::showSegQueue(const deque<segment> &queue,string type){

	cout<<"node"<<id<<" "<<type<<" Queue list:";
	for(unsigned int i=0;i<queue.size();i++){
		cout<<"s"<<queue[i].id<<"(";
		for(unsigned int j=0;j<queue[i].packets.size();j++){
			cout<<"p"<<(queue[i].packets[j]).id<<" ";
		}
		cout<<") |";
	}
	cout<<endl;
}

void Node::showGlobalSegList(){
	cout<<"showGlobalSegList:";
	cout<<"size of global seglist "<<globalSegList.size()<<endl;
	for(unsigned int i=0;i<globalSegList.size();i++){
		cout<<"seg"<<globalSegList[i]->id<<"("<<globalSegList[i]->packets.size()<<") ";
	}
	cout<<endl;
}



void Node::showSegInProgressMapping(){

	cout<<"Node"<<id<<" showSegInProgressMapping:";
	if(segInProgress.empty())
		return;

	cout<<"Node "<<id<<" packets downdloaded until now"<<endl;

	for(map<segment*,int>::iterator it=segInProgress.begin();it!=segInProgress.end();it++){
		cout<<"seg"<< it->first->id <<" ("<<it->second<<") ";
	}
	cout<<endl;
	for(map<segment*,bool>::iterator it=decodedSegments.begin();it!=decodedSegments.end();it++){
			cout<<"    "<<it->second<<"      ";
		}
	cout<<endl;
}


void Node::showcoefTables(){
	cout<<"Node"<<id<<" showcoefTables:";
	cout<<"Coefficient tables for node "<<id;
	if(coefTable.empty()){
		cout<<" is empty"<<endl;
		return;
	}
	cout<<endl;
	/*for(map<segment*,deque<deque<unsigned char> > > ::iterator it=coefTable.begin();it!=coefTable.end();it++){
		cout<<"*** segment"<<it->first->getID()<<" ****";
	}
	cout<<endl;*/
	cout.flush();
	for(map<segment*,deque<deque<unsigned char> > > ::iterator it=coefTable.begin();it!=coefTable.end();it++){
		cout<<"------------------segment "<<it->first->getID()<<"-----------------------------"<<endl;
		for(unsigned int i=0;i<it->second.size();i++){
			cout<<i+1<<"):";
			for(unsigned int j=0;j<it->second[i].size();j++){
				cout<<(int)it->second[i][j]<<" ";
			}
			cout<<endl;
		}
		cout<<"------------------segment "<<it->first->getID()<<"-----------------------------"<<endl;
		cout.flush();
	}
}

/*Function to print segment data inside file it parses decodedSegment mapping*/
void Node::showSegDataInFiles(ofstream &f){
	f<<"Number of downloaded segments "<<decodedSegments.size()<<endl;
	map<segment*,bool>::iterator it;

	for(it=decodedSegments.begin();it!=decodedSegments.end();it++){
		if(it->second==true){
			f<<"Segment"<<(it->first)->getID()<<endl;
			it->first->showSegmentPackets(f);
		}
	}
}

