#pragma once
#include "Dipendenze.h"

class TCP_Client {
private:
	string cammino;
	SOCKADDR_IN ip;
	int is_foto;
	string MAC;
	string* path_foto;
	mutex* mt_path_foto;
	HANDLE pipeHandle;
public:
	TCP_Client(string path, SOCKADDR_IN ip, int is_foto, string MAC, mutex* mt_path_foto, string* percorso_foto, HANDLE pipe);
	void operator()();
	deque<struct info> navigazione_fs(string root, int& flag_is_file); //esplorare file system quando bisogna inviare un albero di direttori
	int calcoloTotalSize(deque<struct info> paths);
};