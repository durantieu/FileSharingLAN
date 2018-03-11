#include "TCP_Server.h"

/*
*Check if file exists on filesystem
*/
bool exists_test(std::string name) {
	ifstream f(name.c_str());
	return f.good();
}

//Costruttore dell'oggetto TCPserver
TCP_Server::TCP_Server(Porta* port, int tipo, mutex* mutPorts, HomePrinter* hp, string* path, mutex* mutSharedPath, string nome_sender) {
	this->port = port;
	this->tipo_file = tipo;
	this->mutPorts = mutPorts;
	this->hp = hp;
	this->mutSharedPath = mutSharedPath;
	this->sharedPath = path;
	this->nome_sender = nome_sender;
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



	//cout << port->get_numero() << endl;

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


	//gestione dell'invio della foto
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed with error: " << WSAGetLastError() << endl;
		closesocket(ClientSocket);
		closesocket(ListenSocket);
		port->setPromiseExit();
		return;

	}


	else {

		//cout << "Preparazione alla ricezione delle informazioni" << endl;

		while (1) {
			bool accettato = false;
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

			//estrapolazione dei parametri del file da acquisire
			//sockBuf contiene le info del file
			stringstream str(sockBuf);
			string path, ext, size, nomefile, nomeFoto;
			getline(str, path, '|');
			if (path != "end") {
				getline(str, nomefile, '|');
				getline(str, ext, '|');
				getline(str, size, '|');
				getline(str, nomeFoto, '|'); //nome foto è il MAC address


			}
			if (tipo_file != 3 && accettato == false) {
				/*string risposta;
				cout << "L'utente " << nome_sender << " vuole inviarti dei file: " << nomefile << "accettare? S/N" << endl;
				cin >> risposta;

				if (!risposta.compare("N")) {
				string ack;
				ack.assign("0"); //ack negativo -> dico al client chiudi la connessione
				send(ClientSocket, ack.c_str(), 1, 0);
				return;
				}*/
				accettato = true;
			}
			//
			//Send Ack al Client che ho ricevuto le informazioni relative al file che devo ancora ricevere
			string ack;
			ack.assign("1");
			send(ClientSocket, ack.c_str(), 1, 0);

			if (ClientSocket == INVALID_SOCKET) {
				cout << "accept failed with error: " << WSAGetLastError() << endl;
			}


			if (tipo_file == 3) {

				system("echo %USERPROFILE% >> homedir.txt");
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

			//condizione di terminazione della ricezione dei files
			if (path == "end") {
				cout << "Files finiti" << endl;
				//notificare che il thread è terminato
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

				while (true) {
					tmpNome = nomefile;
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
				do {
					byte_letti = recv(ClientSocket, sockBuf, recvbuflen, 0);
					fwrite(sockBuf, byte_letti, sizeof(char), fp);
					dati_rimanenti -= byte_letti;
				} while (dati_rimanenti > 0);

				fclose(fp);

				send(ClientSocket, ack.c_str(), 1, 0);

			}
			else {
				cout << "Percorso non creato" << endl;
				//Il percorso non è stato creato, da gestire
			}
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
		}
	}
	camm = camminoParz;
	return true;
}