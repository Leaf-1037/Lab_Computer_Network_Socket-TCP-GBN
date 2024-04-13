#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"


StopWaitRdtReceiver::StopWaitRdtReceiver():expectSequenceNumberRcvd(0),base(0)
{
	expectSequenceNumberRcvd = base + N;
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	for (int i = 0; i < seq_length; ++i) {
		waitingFlags[i] = false;
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


StopWaitRdtReceiver::~StopWaitRdtReceiver()
{
}

void StopWaitRdtReceiver::receive(const Packet &packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum) {

		cout << "���շ�������������Ϊ " << '[' << ' ';
		for (int i = 0; i < 4; i++) {
			cout << base + i << ' ';
		}
		cout << ']' << endl;

		if (base == packet.seqnum) {//Ϊ������
			cout << "���շ��յ��ı������Ϊ" << packet.seqnum << endl;

			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
			waitingFlags[packet.seqnum % seq_length] = true;
			receivedPacket[packet.seqnum % seq_length] = packet;
			receivedPacket[packet.seqnum % seq_length].acknum = 0;
			while (waitingFlags[base % seq_length])//ȡ��Message�����ϵݽ���Ӧ�ò�
			{
				Message msg;
				memcpy(msg.data, receivedPacket[base % seq_length].payload, sizeof(receivedPacket[base % seq_length].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				waitingFlags[base++ % seq_length] = false;//�ͷŻ�����
				waitingFlags[expectSequenceNumberRcvd++ % seq_length] = false;//���뻺����
				receivedPacket[packet.seqnum % seq_length].acknum = -1;
			}
		}
		else if (base < packet.seqnum && packet.seqnum < expectSequenceNumberRcvd) {//���ǵ�һ���յ�ʱ���Ҳ��ǻ�����ʱ
			cout << "���շ��յ��ı������Ϊ" << packet.seqnum << endl;
			pUtils->printPacket("���շ��ѻ��淢�ͷ��ı���", packet);

			//�ŵ�������
			receivedPacket[packet.seqnum % seq_length] = packet;
			waitingFlags[packet.seqnum % seq_length] = true;

			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

			//this->expectSequenceNumberRcvd = 1 - this->expectSequenceNumberRcvd; //���������0-1֮���л�

		}
		else if (packet.seqnum >= base - N && packet.seqnum < base)
		{
			pUtils->printPacket("���շ���ȷ�յ���ȷ�ϵĹ�ʱ����", packet);
			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			cout << "��ʱ���շ��ڴ��������" << this->base << "~" << this->expectSequenceNumberRcvd << "֮��" << endl;
		}
	}
	else pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
}