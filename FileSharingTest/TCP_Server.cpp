#include "TCP_Server.h"

ofstream ServerLog("C:\\Users\\duran\\Desktop\\logServ.txt");

/*
*Check if file exists on filesystem
*/
bool exists_test(std::string name) {
	ifstream f(name.c_str());
	return f.good();
}

//Funzione di conversione da string a wstring
std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

//Costruttore dell'oggetto TCPserver
TCP_Server::TCP_Server(Porta* port, int tipo, mutex* mutPorts, HomePrinter* hp,
	string* path, mutex* mutSharedPath, string nome_sender, int nPipe) {
	this->port = port;
	this->tipo_file = tipo;
	this->mutPorts = mutPorts;
	this->hp = hp;
	this->mutSharedPath = mutSharedPath;
	this->sharedPath = path;
	this->nome_sender = nome_sender;
	this->numPipe = nPipe;
	this->is_directory = false;
	this->root = "";

	//alloco la pipe solamente se stiamo trasferendo un file
	if (tipo_file == 1) {
		string strPipeName("\\\\.\\pipe\\pipe");
		strPipeName.append(to_string(nPipe));

		std::wstring stemp = s2ws(strPipeName);
		LPCWSTR pipeName = stemp.c_str();

		//creazione dell'oggetto pipe
		pipe = CreateNamedPipe(
			pipeName,				  // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,              // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			1024,                // output buffer size 
			1024,                // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

									  //verifica di corretta apertura della pipe
		if (pipe == INVALID_HANDLE_VALUE)
		{
			cout << "INVALID_HANDLE_VALUE" << GetLastError() << endl;
			cin.get();
			return;
		}
	}
	
}

void TCP_Server::operator()() {

	//4)creazioni direttori necessari
	//5)invio ack
	//6)altra recv sullo stesso socket per ricevere il file
	//7)cicla;


	//mostrare a chi è relativo l'indirizzo IP mostrato
	//da implementare

	WSADATA wsaData;

	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	SOCKADDR_IN serv;
	SOCKADDR_IN client;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	WORD wVersionRequested = 0x0202;

	int wsa_startup;
	int slen = sizeof(sockaddr);

	//controllo versione 
	wsa_startup = WSAStartup(wVersionRequested, &wsaData);
	if (wsa_startup != NO_ERROR) {
		cout << "Errore WSA_STARTUP()" << endl;
	}

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket < 0) {
		cout << "Impossibile aprire il socket" << endl;
	}


	serv.sin_family = AF_INET;
	serv.sin_port = htons(stoi(port->get_numero()));
	serv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	iResult = ::bind(ListenSocket, (sockaddr *)&serv, sizeof(sockaddr));
	if (iResult == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << endl;

		closesocket(ListenSocket);
		WSACleanup();
	}

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cout << "listen failed with error: %d\n" << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		WSACleanup();
	}

	//comunicazione al TCP_listener che il Server è in ascolto sulla porta stabilita
	port->setPromiseStarted();

	//-----------LOG-------------------
	ServerLog << "Promise settata" << endl;
	//-----------LOG-------------------

	char* buffer = new char[1024];
	string bufferStr;

	/*
											!!!ATTENZIONE!!!
	!!!Attenzione a questa accept, prima era dopo il if tipofile == 1, da verificare il funzionamento!!!!
											!!!ATTENZIONE!!!
	*/
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed with error: " << WSAGetLastError() << endl;
		closesocket(ClientSocket);
		closesocket(ListenSocket);
		port->setPromiseExit();
		return;
	}

	bool accettato = true;
	if (tipo_file == 1) {
		//Connessione alla pipe verso la GUI

		ConnectNamedPipe(pipe, NULL);

		//recv di Hello message
		recv(ClientSocket, buffer, 1024, 0);

		//-----------LOG-------------------
		ServerLog << "Messaggio di HELLO dal client ricevuto" << endl;
		//-----------LOG-------------------

		//scrittura in pipe di Hello message
		bufferStr = buffer;
		WriteFile(this->pipe, bufferStr.c_str(), 6, 0, NULL);


		//lettura dalla pipe di accettazione file
		ReadFile(this->pipe, buffer, 1024, 0, NULL);
		bufferStr = buffer;

		//-----------LOG-------------------
		ServerLog << "La GUI ha detto: " << bufferStr << endl;
		//-----------LOG-------------------

		//Invio al CLIENT del messaggio di accettazione da parte del server
		send(ClientSocket, bufferStr.c_str(), 3, 0);
		if (bufferStr.find("X") != string::npos) {
			//comunicazione killata, chiudere il server
			closesocket(ClientSocket);
			closesocket(ListenSocket);
			port->setPromiseExit();
			return;
		}
	}

		//cout << "Preparazione alla ricezione delle informazioni" << endl;

		int total_dati_rimasti = 0;
		int total_size;
		float percentuale = 0;
		int percentualePrec = 0;

		while (1) {
			char sockBuf[DEFAULT_BUFLEN];
			for (int i = 0; i < DEFAULT_BUFLEN; i++)
				sockBuf[i] = '\0';

			//Ricezione delle info relative ai files da ricevere
			iResult = recv(ClientSocket, sockBuf, recvbuflen, 0);
			if (iResult > 0) {
				//cout << "Bytes ricevuti (tcp_server): " << iResult << endl;

			}
			else {
				cout << "Errore in ricezione delle informazioni" << endl;
				port->setPromiseExit();
				closesocket(ClientSocket);
				closesocket(ListenSocket);
				return;
			}

			bufferStr = "OK";
			//Invio al CLIENT del messaggio ACK-INFO
			send(ClientSocket, bufferStr.c_str(), bufferStr.length(), 0);

			//-----------LOG-------------------
			ServerLog << "Ricevute info sui files ed Inviato ACK sulle info: " << sockBuf << endl;
			//-----------LOG-------------------

			//estrapolazione dei parametri del file da acquisire
			//sockBuf contiene le info del file
			stringstream str(sockBuf);
			string path, ext, size, nomefile, nomeFoto, total_sizeStr;
			getline(str, path, '|');

			//-----------LOG-------------------
			ServerLog << "1: Separate le info sul file: " << path << endl;
			//-----------LOG-------------------

			if (path != "end") {
				getline(str, nomefile, '|');
				getline(str, ext, '|');
				getline(str, size, '|');
				getline(str, nomeFoto, '|'); //nome foto è il MAC address
				getline(str, total_sizeStr, '|');

				total_size = stoi(total_sizeStr);

				if (total_dati_rimasti == 0)
					total_dati_rimasti = total_size;
			}
			//-----------LOG-------------------
			ServerLog << "2: Separate le info sul file: " << path << endl;
			//-----------LOG-------------------
			

			//-----------LOG-------------------
			ServerLog << "3: Separate le info sul file: " << path  << endl;
			//-----------LOG-------------------

			string ack("1");
			/*		
			send(ClientSocket, ack.c_str(), 1, 0);

			if (ClientSocket == INVALID_SOCKET) {
				cout << "accept failed with error: " << WSAGetLastError() << endl;
			}*/


			if (tipo_file == 3) {

				system("echo %USERPROFILE% > homedir.txt");
				ifstream fpp;
				string path_tmp;
				fpp.open("homedir.txt");
				getline(fpp, path_tmp, ' ');
				fpp.close();
				system("del homedir.txt");

				path_tmp.append("\\FileSharing\\Immagini_utenti\\");
				string comando = "del ";
				comando.append(path_tmp).append(nomeFoto);

				system(comando.c_str());

				path_tmp.append(nomeFoto).append(ext);

				FILE* fp = fopen(path_tmp.c_str(), "wb");
				if (fp == NULL) {
					cout << "File non aperto" << endl;
					return;
				}

				int dati_rimanenti = stoi(size);
				int byte_letti;
				do {
					byte_letti = recv(ClientSocket, sockBuf, recvbuflen, 0);
					fwrite(sockBuf, byte_letti, sizeof(char), fp);
					dati_rimanenti -= byte_letti;
				} while (dati_rimanenti > 0);

				send(ClientSocket, ack.c_str(), 1, 0);

				//cout << "Foto ricevuta correttamente" << endl;
				fclose(fp);
				send(ClientSocket, ack.c_str(), 1, 0);
				closesocket(ListenSocket);
				closesocket(ClientSocket);
				port->setPromiseExit();

				return;
			}

			//-----------LOG-------------------
			ServerLog << "Condizione pre-END" << endl;
			//-----------LOG-------------------

			//condizione di terminazione della ricezione dei files
			if (path == "end") {
				cout << "Files finiti" << endl;

				//-----------LOG-------------------
				ServerLog << "File inviato completamente" << endl;
				//-----------LOG-------------------

				if (tipo_file == 1) {
					string buffer("|");
					buffer.append("100").append("|");
					WriteFile(this->pipe, buffer.c_str(), 5, 0, NULL);
					//::CloseHandle(this->pipe);

					
				}

				closesocket(ListenSocket);
				closesocket(ClientSocket);
				port->setPromiseExit();
				return;
			}

			string camm;
			if (creaPath(path, camm) == true) {

				//il percorso è stato creato e verificato, procedere all'acquisizione del file
				//cout << "Preparazione all'acquisizione del file" << endl;

				//Codice per assegnare nome diverso agli oggetti con lo stesso nome
				string tmpNome;

				int index = 1;

				

				//gestione dei nomi duplicati
				while (true) {

					tmpNome.assign(camm);
					tmpNome.append("\\").append(nomefile);
					tmpNome.append(ext);

					if (exists_test(tmpNome)) {
						nomefile = nomefile.substr(0, nomefile.find_last_of("."));
						if (index > 1)
							nomefile = nomefile.substr(0, nomefile.find_last_of("("));
						nomefile.append("(").append(to_string(index)).append(")");
					}
					else {
						break;
					}
					index++;
				}

				camm.append("\\").append(nomefile).append(ext);
				/*
				//comunico al client che ho creato il direttorio e sono pronto a ricevere
				string ack;
				ack.assign("1");
				send(ClientSocket,ack.c_str(), 1, 0 );
				*/

				if (ClientSocket == INVALID_SOCKET) {
					cout << "accept failed with error: " << WSAGetLastError() << endl;
				}
				//verificare se crea il file nelle cartelle giuste
				//apertura del file e controllo di correttezza
				FILE* fp = fopen(camm.c_str(), "wb");
				if (fp == NULL) {
					cout << "Impossibile aprire il file" << endl;
					return;
				}

				int dati_rimanenti = stoi(size);
				int byte_letti;
				char* bufPipe = new char[1024];

				//-----------LOG-------------------
				ServerLog << "Ingresso nel trasferimento" << endl;
				//-----------LOG-------------------

				do {
					//ricevo i dati dal client e li scrivo sul file
					byte_letti = recv(ClientSocket, sockBuf, recvbuflen, 0);
					fwrite(sockBuf, byte_letti, sizeof(char), fp);

					dati_rimanenti -= byte_letti;
					total_dati_rimasti -= byte_letti;

					percentuale = (((float)total_size - (float)total_dati_rimasti) / (float)total_size) * 100;

					if (percentuale >= 99) {
						percentuale = 99;
					}

					//Scrivo sulla pipe la percentuale di completamento
					bufferStr = "|";
					bufferStr.append(to_string((int)percentuale)).append("|");
					WriteFile(this->pipe, bufferStr.c_str(), 3, 0, NULL);
					percentualePrec = percentuale;

					//-----------LOG-------------------
					ServerLog << "Percentuale di completamento: " << bufferStr << endl;
					//-----------LOG-------------------
					
					//leggo dalla GUI ACK di dato
					ReadFile(this->pipe, buffer, 3, 0, NULL);

					//-----------LOG-------------------
					ServerLog << "ACK di dato letta: " << buffer << endl;
					//-----------LOG-------------------

					//inoltro al server ACK di dato
					bufferStr = buffer;
					send(ClientSocket, bufferStr.c_str(), bufferStr.length(), 0);

					char* bufferAckA = new char[2];
					//ricezione di ACKA dal client
					recv(ClientSocket, bufferAckA, 2, 0);
					bufferStr = bufferAckA;

					//-----------LOG-------------------
					ServerLog << "ACK-A ricevuta: " << bufferStr << endl;
					//-----------LOG-------------------

					WriteFile(this->pipe, bufferStr.c_str(), bufferStr.length(), 0, NULL);
					if (bufferStr.find("X") != string::npos) {

						//fare rollback del trasferimento
						//gestendo separatamente la situazione in cui si elimina una cartella
						//da quella in cui si elimina un file singolo

						fclose(fp);

						if (is_directory) {
							string comando("rmdir /Q /S ");
							comando.append(root);
							system(comando.c_str());

							//-----------LOG-------------------
							ServerLog << "System di rollback: " << comando << endl;
							//-----------LOG-------------------
						}
						else {
							string comando("del ");
							comando.append(camm.c_str());
							system(comando.c_str());

							
						}

						//killare la comunicazione
						closesocket(ClientSocket);
						closesocket(ListenSocket);
						port->setPromiseExit();
						::CloseHandle(this->pipe);
						return;
					}

				
					//leggo ACKB dalla GUI
					ReadFile(this->pipe, buffer, 1024, 0, NULL);
					bufferStr = buffer;

					//-----------LOG-------------------
					ServerLog << "ACK-B ricevuta dalla GUI, la sto inviando al TCP_Client: " << buffer << endl;
					//-----------LOG-------------------

					//Invio di ACKB al client
					send(ClientSocket, buffer, 2, 0);

					if (bufferStr.find("X") != string::npos) {

						//fare rollback del trasferimento
						//gestendo separatamente la situazione in cui si elimina una cartella
						//da quella in cui si elimina un file singolo

						fclose(fp);

						if (is_directory) {
							string comando("rmdir /Q /S ");
							comando.append(root);
							system(comando.c_str());

							//-----------LOG-------------------
							ServerLog << "System di rollback: " << comando << endl;
							//-----------LOG-------------------
						}
						else {
							string comando("del ");
							comando.append(camm.c_str());
							system(comando.c_str());

							//-----------LOG-------------------
							ServerLog << "System di rollback: " << comando << endl;
							//-----------LOG-------------------
						}

						

						//killare la comunicazione
						closesocket(ClientSocket);
						closesocket(ListenSocket);
						port->setPromiseExit();
						::CloseHandle(this->pipe);
						
						return;
					}

					

				} while (dati_rimanenti > 0);

				fclose(fp);

				//Indica che il file è stato inviato correttamente
				string ackFileOK("1\0");
				send(ClientSocket, ackFileOK.c_str(), 2, 0);

			}
			else {
				cout << "Percorso non creato" << endl;
			}
		}
}


/*
*Metodo di verifica e creazione del path locale
*/

bool TCP_Server::creaPath(string camminoTot, string& camm) {
	namespace fs = std::experimental::filesystem;

	stringstream inputString(camminoTot);
	string localpath("localPath.txt");
	string cammino, camminoParz;
	string line;
	if (exists_test(localpath)) {
		ifstream filestream(localpath);
		getline(filestream, camminoParz, '\0');
	}
	else {
		//Funzione che crea la cartella in cui salvare i file
		//identificando il percorso in USERPROFILE corrente
		string pth = hp->print();

		{
			lock_guard<mutex> lg(*mutSharedPath);
			if (*this->sharedPath == "")
				camminoParz.assign(pth).append("\\Documents\\fileTransfer");
			else
				camminoParz = *this->sharedPath;
		}

		if (!fs::is_directory(camminoParz))
			fs::create_directory(camminoParz);
	}

	while (getline(inputString, cammino, '\\')) {
		camminoParz.append("\\").append(cammino).append("\\");
		if (!fs::exists(camminoParz)) {
			fs::create_directory(camminoParz);
			is_directory = true;
			if (root == "") {
				root = camminoParz;
			}
		}
	}
	camm = camminoParz;
	return true;
}