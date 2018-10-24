#pragma once

#include "Dipendenze.h"
#include "TCP_Server.h"
#include "TCP_Client.h"
#include "Sync_mappa.h"


class TCP_Listener {
	map<int, future<bool>> *activePorts;
	Sync_mappa* m1;
	bool actives[50];
	mutex *mutPorts;
	string MAC;
	string *sharedFilePath;
	string *sharedFotoPath;
	mutex *mutShared; //per proteggere il puntatore alla variabile sharedFilePath
	int pipeNumRic;
	HANDLE mainPipe;
public:
	TCP_Listener(Sync_mappa *m, string MAC, string* path, mutex* mut, string* path_picture);
	void operator()();
	string port_number_calculator();
	bool TCP_throw_thread(string port, int tipo_file, string nome);
};