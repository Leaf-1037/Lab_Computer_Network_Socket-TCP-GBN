#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class StopWaitRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	Packet lastAckPkt;				//上次发送的确认报文
	int base;
	Packet receivedPacket[seq_length];	// 已发送并等待ack数据包
	bool waitingFlags[seq_length];
public:
	StopWaitRdtReceiver();
	virtual ~StopWaitRdtReceiver();

public:
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
};

#endif

