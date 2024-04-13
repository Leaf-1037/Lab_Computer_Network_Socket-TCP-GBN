// StopWait.cpp : 定义控制台应用程序的入口点。
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
//	pns->setRunMode(0);  //VERBOS模式
	pns->setRunMode(1);  //安静模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");

	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
		cout.rdbuf(oldcout);
		fout.close();

	return 0;
}

