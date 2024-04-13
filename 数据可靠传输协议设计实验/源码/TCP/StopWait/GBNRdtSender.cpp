#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"

#define COLOR_PURPLE "\033[0;35m"
#define COLOR_WHITE  "\033[0;37m"


GBNRdtSender::GBNRdtSender():base(0),expectSequenceNumberSend(0),waitingState(false)
{
	/*for (int i = 0; i < seq_length; ++i) {
		ACKFlags[i] = 0;
	}*/
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return waitingState;
}




bool GBNRdtSender::send(const Message &message) {
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}
	if (init_flag == 1) {
		for (int i = 0; i < seq_length; i++) {
			this->packetWaitingAck[i].seqnum = -1;
		}
		init_flag = 0;
	}

	if (expectSequenceNumberSend < base + N) {
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].acknum = -1; //忽略该字段
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].checksum = 0;
		memcpy(this->packetWaitingAck[expectSequenceNumberSend % seq_length].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		//ACKFlags[expectSequenceNumberSend % seq_length] = false;


		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		
		if (base == expectSequenceNumberSend) {
			cout << "发送方启动计时器" << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//启动发送基序列方定时器
		}
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % seq_length]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		expectSequenceNumberSend++;
		cout << "此报文发送后，expectSequenceNumberSend为" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;//进入等待状态
		}
	}																				//进入等待状态
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {
	//if (this->waitingState == true) {//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
		//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum &&  ackPkt.acknum >= base) {
		int former_base = base;
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		base = ackPkt.acknum + 1;
		for (int i = base+N; i < base + seq_length; ++i) {
			packetWaitingAck[i % seq_length].seqnum = -1;
		}
		cout << "发送方滑动窗口内容：[" << ' ';
		for (int i = base; i < base + N; ++i) {
			if (packetWaitingAck[i % seq_length].seqnum == -1)
				cout << '*' << ' ';
			else cout << packetWaitingAck[i % seq_length].seqnum << ' ';
		}
		cout << ']' << endl;

		if (base == expectSequenceNumberSend) {
			cout << "已发送分组已全部接送，关闭计时器" << endl;
			this->waitingState = false;
			pns->stopTimer(SENDER, former_base);//关闭计时器
			/*ACKFlags[base % seq_length] = true;
			while (ACKFlags[base % seq_length]) {
				ACKFlags[base++ % seq_length] = false;
			}
			waitingState = false;*/
		}
		else
		{
			pns->stopTimer(SENDER, former_base);//还没接收完，继续等待
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			this->waitingState = false;
		}
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);		//关闭定时器
	}
	else {
		if (ackPkt.acknum == last_ACK) {
			ACK_cnt++;
			if (ACK_cnt == 4) {
				cout << COLOR_PURPLE "收到了三个冗余的ACK，快速重传序号" COLOR_WHITE << ackPkt.acknum + 1 << endl;
				pns->stopTimer(SENDER, ackPkt.acknum + 1);
				pns->startTimer(SENDER, Configuration::TIME_OUT, ackPkt.acknum + 1);
				pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[base % seq_length]);
			}
		}
		else {
			last_ACK = ackPkt.acknum;
			ACK_cnt = 1;
		}
		if (checkSum != ackPkt.checksum) {
			cout << "发送方收到的ACK损坏" << endl;
		}
		else {
			cout << "发送方没有收到正确的序号，继续等待" << endl;
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	cout << "超时重传" << endl;
	pns->stopTimer(SENDER,seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//重新启动发送方定时器
	cout << "重发" << seqNum << "号报文" << endl;
	pUtils->printPacket("发送方定时器时间到，重发报文", this->packetWaitingAck[seqNum % seq_length]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % seq_length]);			//重新发送数据包
}
