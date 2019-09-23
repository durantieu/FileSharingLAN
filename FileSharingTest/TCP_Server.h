#pragma once

#include "Dipendenze.h"
#include "Porta.h"
#define DEFAULT_BUFLEN 65536

std::wstring s2ws(const std::string& s);
void lanciaBatch(wstring index, wstring params);

class HomePrinter {
private:
	mutex mut;
public:
	string print() {
		lock_guard<mutex> lg(mut);
		//system("echo \"\%USERPROFILE\%\" > homedir.txt");
		wstring com = L"4";
		lanciaBatch(com, L"");
		ifstream fp("homedir.txt");
		string pth;
		getline(fp, pth, '\"');
		getline(fp, pth, '\"');
		fp.close();
		//system("del homedir.txt");
		com = L"5";
		lanciaBatch(com, L"");
		return pth;
	}
};

class TCP_Server {
private:
	Porta *port;
	int tipo_file;
	int numPipe;
	mutex* mutPorts;
	HomePrinter *hp;
	string *sharedPath;
	mutex* mutSharedPath;
	string nome_sender;
	HANDLE pipe;
	bool is_directory;
	string root;
public:
	TCP_Server(Porta* port, int tipo, mutex* mutPorts, HomePrinter* hp, 
		string* sharedpath, mutex* mutSharedPath, string nome_sender, int nPipe);
	void operator()();
	bool creaPath(string cammino, string& camm);
};