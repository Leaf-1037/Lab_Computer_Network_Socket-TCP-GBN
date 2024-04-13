#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"


GBNRdtSender::GBNRdtSender():base(0),expectSequenceNumberSend(0),waitingState(false)
{
	for (int i = 0; i < seq_length; ++i) {
		ACKFlags[i] = 0;
	}
}


GBNRdtSender::~GBNRdtSender()
{
}



bool GBNRdtSender::getWaitingState() {
	return waitingState;
}




bool GBNRdtSender::send(const Message &message) {
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	if (expectSequenceNumberSend < base + N) {
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].acknum = -1; //���Ը��ֶ�
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].checksum = 0;
		memcpy(this->packetWaitingAck[expectSequenceNumberSend % seq_length].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[expectSequenceNumberSend % seq_length].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		ACKFlags[expectSequenceNumberSend % seq_length] = false;


		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		
		cout <<"SequenceNum:#"<<expectSequenceNumberSend << "���ͷ�������ʱ��." << endl;
		pns->startTimer(SENDER, Configuration::TIME_OUT, expectSequenceNumberSend);
		
		//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		expectSequenceNumberSend++;
		cout << "�����ѷ��ͣ�����seq_numberΪ��" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;
		}
	}																				//����ȴ�״̬
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {
	//if (this->waitingState == true) {//������ͷ����ڵȴ�ack��״̬�������´�������ʲô������
		//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum) {
		//int former_base = base;
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		//base = ackPkt.acknum + 1;
		for (int i = base+N; i < base + seq_length; ++i) {
			packetWaitingAck[i % seq_length].seqnum = -1;
		}
		cout << "���ͷ������������ݣ�[" << ' ';
		for (int i = base; i < base + N; ++i) {
			if (packetWaitingAck[i % seq_length].seqnum == -1)
				cout << '*' << ' ';
			else cout << packetWaitingAck[i % seq_length].seqnum << ' ';
		}
		cout << ']' << endl;

		if (base == ackPkt.acknum) {
			cout << "��ȷ��ACK���Ϊ��#" << ackPkt.acknum << endl;
			pns->stopTimer(SENDER, ackPkt.acknum);//�رռ�ʱ��
			ACKFlags[base % seq_length] = true;
			while (ACKFlags[base % seq_length]) {
				ACKFlags[base++ % seq_length] = false;
			}
			waitingState = false;
		}
		else if (ackPkt.acknum > base && !ACKFlags[ackPkt.acknum % seq_length]) {
			cout << "��ȷ��ACK���Ϊ:#" << ackPkt.acknum << endl;
			pns->stopTimer(SENDER, ackPkt.acknum);
			ACKFlags[ackPkt.acknum % seq_length] = true;
		}
		else
		{
			cout << "�յ�����Ҫ�����е�ACK,�����ȴ�" << endl;
		}
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);		//�رն�ʱ��
	}
	else {
		cout << "���ͷ�δ�յ���ȷ��ţ������ȴ�" << endl;
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	cout << "��ʱ�ش�" << endl;
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT,seqNum);			//�����������ͷ���ʱ��
	cout << "�ط�" << seqNum << "�ű���" << endl;
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", this->packetWaitingAck[seqNum % seq_length]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % seq_length]);			//���·������ݰ�
}
