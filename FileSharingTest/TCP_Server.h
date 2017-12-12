#pragma once

#include "Dipendenze.h"
#include "Porta.h"
#define DEFAULT_BUFLEN 512

class HomePrinter {
private:
	mutex mut;
public:
	string print() {
		lock_guard<mutex> lg(mut);
		system("echo \"\%USERPROFILE\%\" > tmp.txt");
		ifstream fp("tmp.txt");
		string pth;
		getline(fp, pth, '\"');
		getline(fp, pth, '\"');
		fp.close();
		system("del tmp.txt");
		return pth;
	}
};

class TCP_Server {
private:
	Porta *port;
	int tipo_file;
	mutex* mutPorts;
	HomePrinter *hp;
	string *sharedPath;
	mutex* mutSharedPath;
	string nome_sender;
public:
	TCP_Server(Porta* port, int tipo, mutex* mutPorts, HomePrinter* hp, string* sharedpath, mutex* mutSharedPath, string nome_sender);
	void operator()();
	bool creaPath(string cammino, string& camm);
};