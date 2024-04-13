#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"
#include <iostream>
#include <fstream>

#define FLAG_OUTPUT 1


GBNRdtSender::GBNRdtSender():base(0),expectSequenceNumberSend(0),waitingState(false)
{
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return waitingState;
}




bool GBNRdtSender::send(const Message &message) {

	//ofstream fout("output_log.txt");
	//streambuf* oldcout;
	//streambuf* oldcin;
	////if (FLAG_OUTPUT) 
	//	oldcout = cout.rdbuf(fout.rdbuf());


	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}

	if (init_flag == 1) {
		for (int i = 0; i < seq_length; ++i) {
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

		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		if (base == expectSequenceNumberSend) {
			cout << "发送方启动计时器." << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[expectSequenceNumberSend % seq_length].seqnum);
		}
		//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % N]);
		expectSequenceNumberSend++;
		cout << "报文已发送，期望seq_number为：" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;
		}
	}																				//进入等待状态
	////if (FLAG_OUTPUT) {
	//	cout.rdbuf(oldcout);
	//	fout.close();
	////}
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {
	//if (this->waitingState == true) {//如果发送方处于等待ack的状态，作如下处理；否则什么都不做
		//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//ofstream fout("output_log.txt");
	//streambuf* oldcout;
	//streambuf* oldcin;
	////if (FLAG_OUTPUT) 
	//	oldcout = cout.rdbuf(fout.rdbuf());
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= base) {
		int former_base = base;
		//this->expectSequenceNumberSend = 1 - this->expectSequenceNumberSend;			//下一个发送序号在0-1之间切换
		//this->waitingState = false;
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		base = ackPkt.acknum + 1;
		for (int i = base + N; i < base + 8; ++i) {
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
			cout << "已发送分组成功，即将关闭计时器" << endl;
			this->waitingState = false;
			pns->stopTimer(SENDER, former_base);//关闭计时器
		}
		else {
			// 还未接受完成，继续等
			pns->stopTimer(SENDER, former_base);
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			this->waitingState = false;
		}
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);		//关闭定时器

	}
	else {
		//pUtils->printPacket("发送方没有正确收到确认，重发上次发送的报文", this->packetWaitingAck);
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);									//首先关闭定时器
		//pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck.seqnum);			//重新启动发送方定时器
		//pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//重新发送数据包
		if (checkSum != ackPkt.checksum) {
			cout << "发送方收到ACK包损坏" << endl;
		}
		else {
			cout << "发送方未收到正确序号，继续等待" << endl;
		}
	}
	////if (FLAG_OUTPUT) {
	//	cout.rdbuf(oldcout);
	//	fout.close();
	////}
	//}	
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	// 重定向至文件
	//ofstream fout("output_log.txt");
	//streambuf* oldcout;
	//streambuf* oldcin;
	////if (FLAG_OUTPUT) 
	//	oldcout = cout.rdbuf(fout.rdbuf());

	cout << "超时重传，回退N步" << endl;
	pns->stopTimer(SENDER,seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//重新启动发送方定时器
	int i = base;
	do {
		cout << "正在重发" << i << "号报文" << endl;
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packetWaitingAck[i % seq_length]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[i % seq_length]);			
		//重新发送数据包
		++i;
	} while (i != expectSequenceNumberSend);
	////if (FLAG_OUTPUT) {
	//	cout.rdbuf(oldcout);
	//	fout.close();
	////}
}
