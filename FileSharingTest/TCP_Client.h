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
	string pipeID;
public:
	TCP_Client(string path, SOCKADDR_IN ip, int is_foto, string MAC, mutex* mt_path_foto, string* percorso_foto, string pipe);
	void operator()();
	deque<struct info> navigazione_fs(string root, int& flag_is_file); //esplorare file system quando bisogna inviare un albero di direttori
};