/*
 * Node.h
 *
 *  Created on: Feb 17, 2014
 *      Author: vapashos
 */

#ifndef NODE_H_
#define NODE_H_
#include "channelWlan.h"
#include "request.h"
#include "advertisment.h"
#include "segment.h"
#include "decoder.h"
#include <set>

class Node {
public:

	/*Static variables*/
	static int counter;
	static channelWlan wChannel;
	static deque<segment*> globalSegList;
	static deque<message> messageList;
	static bool useWlan;
	static ofstream fl;
	ofstream ds;/*ds stream to write decoded segments*/
	ofstream befDec;/*stream to write packet payload before decoding*/
	ofstream afterDec;/*stream to write packet payload after decoding*/
	int id,index;	/*This is an index of the first packet of a segment inside queue3G
	 	 	 	 	 * it is updated any time user finishes downloading a segment inside recv3GPacket*/
					/*Packet Queues*/
	deque<Packet> queue3G;
	deque<Packet> inQueue;
	deque<Packet> outQueue;
	deque <Packet> tempQueue;
	/*Message Queues*/
	deque<message*> inMsgQueue;
	deque<message*> outMsgQueue;
	/*Segment Queue*/
	deque<segment> segQueue;
	map<segment*,int> segInProgress;
	map<segment*,bool> decodedSegments;/*list of downloaded segments the decoded segments are marked as true*/
	/*Coefficient matrix*/
	map<segment*,deque<deque<unsigned char> > > coefTable;/*2dimensional Table for each segment*/
	/*Packets Successfully Decoded By Node*/
	map<segment*,deque<Packet> > decodedMapping;
	/*Neighbor list*/
	deque<Node*> neighborList;
	/*Segments to Download List*/
	map<segment*,bool> segToDownloadList;/*if boolean value is true segment has been downloaded*/
	/*Pointer on a decoder*/
	decoder* myDecoder;
	segment* bdSegment;		/* broadcasting segment */
	/*Constructors Destructors*/
	Node();
	Node(const Node &n);
	virtual ~Node();

	/*functions */
	void addNeighbor(Node *x);
	void initChannelWLAN();
	void sendPacket(int timeSlot,int receiverID);
	void recvPacket(int timeSlot);
	void recv3GPacket(Packet &p,int timeSlot);
	bool broadcastPacket(int timeSlot,Node *brNode);
	Packet* findLatestPacket();
	Packet createCodedPacket(const segment &s,int timeSlot) const;/*create a new coded packet based on segment*/
	void createPoolOfCodedPackets(segment *s,int num,int timeSlot) ;/*creates a number of new coded packets based on segment packets are added directly on node's output queue using function createCodedPacket*/
	void checkIncomingCodedPackets(int timeSlot);
	segment* addCoefVector(deque<unsigned char> &c,segment *segID);/*Add vector on coefTable returns pointer on segment that can be decoded else NULL*/
	bool decodeSegment();
	/*Message functions*/
	void sendMessage(int timeSlot);
	void recvMessage(int timeSlot);
	bool isEmpty(const deque<Packet> &queue);

	void createRequest(int receiverID,int segID,int dimensions,int timeSlot);
	void createAdvertisment(int segID,int timeSlot);
	void addSegmentToDownloadList(segment *x);
	void checkSegmentDownload();/*At each time slot the node checks if he successfully downloaded a segment*/
	void initiateDecoder();
	int checkRequests();/*With this function node checks its requests inside inMsgQueue and determines the maximum dimension to send*/


	/*Get functions*/
	static segment * getSegment(int segID);

	/*Show functions*/
	void showNeighborList();
	void showNodeChannels();
	void showPacketQueue(const deque<Packet> &queue,string type);
	void showMsgQueue(deque<message*> &queue,string type);
	void showSegQueue(const deque<segment> &queue,string type);
	void showSegInProgressMapping();
	void showcoefTables();
	static void showGlobalSegList();
	void showSegDataInFiles(ofstream &f);

};

#endif /* NODE_H_ */
