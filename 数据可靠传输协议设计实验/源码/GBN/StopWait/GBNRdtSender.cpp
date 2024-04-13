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


	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	if (init_flag == 1) {
		for (int i = 0; i < seq_length; ++i) {
			this->packetWaitingAck[i].seqnum = -1;
		}
		init_flag = 0;
	}

	if (expectSequenceNumberSend < base + N) {
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].acknum = -1; //���Ը��ֶ�
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].checksum = 0;
		memcpy(this->packetWaitingAck[expectSequenceNumberSend % seq_length].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[expectSequenceNumberSend % seq_length]);

		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		if (base == expectSequenceNumberSend) {
			cout << "���ͷ�������ʱ��." << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck[expectSequenceNumberSend % seq_length].seqnum);
		}
		//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % N]);
		expectSequenceNumberSend++;
		cout << "�����ѷ��ͣ�����seq_numberΪ��" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;
		}
	}																				//����ȴ�״̬
	////if (FLAG_OUTPUT) {
	//	cout.rdbuf(oldcout);
	//	fout.close();
	////}
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {
	//if (this->waitingState == true) {//������ͷ����ڵȴ�ack��״̬�������´�������ʲô������
		//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//ofstream fout("output_log.txt");
	//streambuf* oldcout;
	//streambuf* oldcin;
	////if (FLAG_OUTPUT) 
	//	oldcout = cout.rdbuf(fout.rdbuf());
	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= base) {
		int former_base = base;
		//this->expectSequenceNumberSend = 1 - this->expectSequenceNumberSend;			//��һ�����������0-1֮���л�
		//this->waitingState = false;
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		base = ackPkt.acknum + 1;
		for (int i = base + N; i < base + 8; ++i) {
			packetWaitingAck[i % seq_length].seqnum = -1;
		}
		cout << "���ͷ������������ݣ�[" << ' ';
		for (int i = base; i < base + N; ++i) {
			if (packetWaitingAck[i % seq_length].seqnum == -1)
				cout << '*' << ' ';
			else cout << packetWaitingAck[i % seq_length].seqnum << ' ';
		}
		cout << ']' << endl;
		if (base == expectSequenceNumberSend) {
			cout << "�ѷ��ͷ���ɹ��������رռ�ʱ��" << endl;
			this->waitingState = false;
			pns->stopTimer(SENDER, former_base);//�رռ�ʱ��
		}
		else {
			// ��δ������ɣ�������
			pns->stopTimer(SENDER, former_base);
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			this->waitingState = false;
		}
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);		//�رն�ʱ��

	}
	else {
		//pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ�ϣ��ط��ϴη��͵ı���", this->packetWaitingAck);
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);									//���ȹرն�ʱ��
		//pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck.seqnum);			//�����������ͷ���ʱ��
		//pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//���·������ݰ�
		if (checkSum != ackPkt.checksum) {
			cout << "���ͷ��յ�ACK����" << endl;
		}
		else {
			cout << "���ͷ�δ�յ���ȷ��ţ������ȴ�" << endl;
		}
	}
	////if (FLAG_OUTPUT) {
	//	cout.rdbuf(oldcout);
	//	fout.close();
	////}
	//}	
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	// �ض������ļ�
	//ofstream fout("output_log.txt");
	//streambuf* oldcout;
	//streambuf* oldcin;
	////if (FLAG_OUTPUT) 
	//	oldcout = cout.rdbuf(fout.rdbuf());

	cout << "��ʱ�ش�������N��" << endl;
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//�����������ͷ���ʱ��
	int i = base;
	do {
		cout << "�����ط�" << i << "�ű���" << endl;
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->packetWaitingAck[i % seq_length]);
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[i % seq_length]);			
		//���·������ݰ�
		++i;
	} while (i != expectSequenceNumberSend);
	////if (FLAG_OUTPUT) {
	//	cout.rdbuf(oldcout);
	//	fout.close();
	////}
}
