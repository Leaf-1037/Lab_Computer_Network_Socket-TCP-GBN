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
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}
	if (init_flag == 1) {
		for (int i = 0; i < seq_length; i++) {
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
		//ACKFlags[expectSequenceNumberSend % seq_length] = false;


		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[expectSequenceNumberSend % seq_length]);
		
		if (base == expectSequenceNumberSend) {
			cout << "���ͷ�������ʱ��" << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);			//�������ͻ����з���ʱ��
		}
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % seq_length]);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		expectSequenceNumberSend++;
		cout << "�˱��ķ��ͺ�expectSequenceNumberSendΪ" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;//����ȴ�״̬
		}
	}																				//����ȴ�״̬
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {
	//if (this->waitingState == true) {//������ͷ����ڵȴ�ack��״̬�������´�������ʲô������
		//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum &&  ackPkt.acknum >= base) {
		int former_base = base;
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		base = ackPkt.acknum + 1;
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

		if (base == expectSequenceNumberSend) {
			cout << "�ѷ��ͷ�����ȫ�����ͣ��رռ�ʱ��" << endl;
			this->waitingState = false;
			pns->stopTimer(SENDER, former_base);//�رռ�ʱ��
			/*ACKFlags[base % seq_length] = true;
			while (ACKFlags[base % seq_length]) {
				ACKFlags[base++ % seq_length] = false;
			}
			waitingState = false;*/
		}
		else
		{
			pns->stopTimer(SENDER, former_base);//��û�����꣬�����ȴ�
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			this->waitingState = false;
		}
		//pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);		//�رն�ʱ��
	}
	else {
		if (ackPkt.acknum == last_ACK) {
			ACK_cnt++;
			if (ACK_cnt == 4) {
				cout << COLOR_PURPLE "�յ������������ACK�������ش����" COLOR_WHITE << ackPkt.acknum + 1 << endl;
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
			cout << "���ͷ��յ���ACK��" << endl;
		}
		else {
			cout << "���ͷ�û���յ���ȷ����ţ������ȴ�" << endl;
		}
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
