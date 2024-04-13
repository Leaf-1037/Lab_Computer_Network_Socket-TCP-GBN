// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "GBNRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include <iostream>
#include <fstream>


int main(int argc, char* argv[])
{
	ofstream fout("output_log.txt");
	streambuf* oldcout;
	streambuf* oldcin;
	//if (FLAG_OUTPUT) 
		oldcout = cout.rdbuf(fout.rdbuf());

	RdtSender *ps = new GBNRdtSender();
	RdtReceiver * pr = new StopWaitRdtReceiver();
//	pns->setRunMode(0);  //VERBOSģʽ
	pns->setRunMode(1);  //����ģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
		cout.rdbuf(oldcout);
		fout.close();

	return 0;
}

