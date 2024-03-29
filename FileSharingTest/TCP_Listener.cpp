#include "TCP_Listener.h"
#include "TCP_Server.h"

//costruttore di TCP_Listener
TCP_Listener::TCP_Listener(Sync_mappa *m, string MAC, string* path, mutex* mut, string* path_picture) {
	activePorts = new map<int, future<bool>>;
	for (int i = 0; i < 50; i++)
		actives[i] = false;
	m1 = m;
	this->MAC = MAC;
	this->mutShared = mut;
	this->sharedFilePath = path;
	this->sharedFotoPath = path_picture;
	this->pipeNumRic = 100001;

	string strPipeName("\\\\.\\pipe\\mainPipe");

	std::wstring stemp = s2ws(strPipeName);
	LPCWSTR pipeName = stemp.c_str();

	//creazione dell'oggetto pipe
	mainPipe = CreateNamedPipe(
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
	if (mainPipe == INVALID_HANDLE_VALUE)
	{
		cout << "INVALID_HANDLE_VALUE" << GetLastError() << endl;
		cin.get();
		return;
	}
}

void TCP_Listener::operator()() {
	//ciclo infinito in cui si mette in ricezione
	//aspetta una richiesta di un Client -> Sgancia thread (TCP_server) per servire il Client facendo sapere al nuovo thread creato su quale porta avverr� la comunicazione
	//TCP_Server::TCP_thread()
	//aspetto la promise del thread creato e appena arriva, notifico al client la porta
	//torna in ascolto di richieste pendenti

	//Fare il popup di non ricezione

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char* recvbuf = (char*)calloc(1024, sizeof(char));
	int recvbuflen = DEFAULT_BUFLEN;

	//Funzione bloccante! finch� il C# non si connette la dll rimane ferma
	ConnectNamedPipe(mainPipe, NULL);

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	// Resolve the server address and port
	iResult = getaddrinfo(NULL, to_string(TCP_LISTENER_PORT).c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
	}

	// Create a SOCKET for connecting 
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
	}

	// Setup the TCP listening socket
	iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
	}
	sockaddr_in* Client_IP = new sockaddr_in();

	while (1) {
		// Il TCP listener accetta una comunicazione dal client (coda in listen)

		int dim = sizeof(struct sockaddr_in);
		ClientSocket = accept(ListenSocket, (sockaddr*)Client_IP, &dim);
		if (ClientSocket == INVALID_SOCKET)
			cout << "accept failed with error: " << WSAGetLastError() << endl;

		//Ricezione del tipo di file da ricevere
		// se il tipo_file == 2 -> devo sganciare un Client perch� qualcuno vuole una foto
		// Altrimenti devo sganciare un Server e prepararmi alla ricezione di un file o di una foto Profilo.
		iResult = recv(ClientSocket, recvbuf, 50 * sizeof(char), 0);
		string value, identity; //identity is MAC
		string nome;

		stringstream stream(recvbuf);

		getline(stream, value, ' ');
		getline(stream, identity, ' ');

		nome.assign(m1->get_nome(identity));
		if (!nome.compare(""))
			nome.assign("Utente sconosciuto");


		if (iResult > 0) {
			//cout << "Bytes ricevuti: " << iResult << endl;


			string str(recvbuf);
			if (!value.compare("2")) { //quando str == 2 L'host che ci ha contattato vuole la foto-> entra qui dentro quindi invio l'ack al Client che mi ha contattato e lancio un nuovo Client 
									   //ack per notifica richiesta foto accettata

				string ack;
				ack.assign("1");
				send(ClientSocket, ack.c_str(), 1, 0);


				//cout << "Sgancio client per accontentare richiesta foto" << endl;
				TCP_Client* newClient = new TCP_Client("", *Client_IP, 3, MAC, mutShared, sharedFotoPath, nullptr); //La task di questo Client � inviare una foto...prima contatter� il Listener dall'altra parte con valore=3 -> poi contatter� il server creato dal listener dall'altra parte
				thread *newThread = new thread(*newClient);


				newThread->detach();
				continue;
			}
			else {
				if (m1->check_identity(identity) && value != "3") {
					cout << "Errore: Utente bloccato" << endl;
					string ack;
					ack.assign("0"); // Porta invalida -> chiusura di connessione!
					send(ClientSocket, ack.c_str(), 1, 0);
					closesocket(ClientSocket);
					continue;
				}

			
				int t_file_to_receive = atoi(recvbuf); // Se == 1 -> Il server ricever� un file, Se == 3 il Server ricever� una foto.
				if (t_file_to_receive != 1 && t_file_to_receive != 3) {
					cout << "Errore: Tipo file da ricevere non valido.\nValore Segnalato: " << t_file_to_receive << endl;
				}
				string portNumber = this->port_number_calculator();
				if (this->TCP_throw_thread(portNumber, atoi(recvbuf), nome) == false) {
					cout << "Impossibile lanciare nuovo TCP_Server" << endl;
				}
				else {

					//Invio al Client della porta su cui il TCP_server � in attesa oppure il messaggio Connessione rifiutata
					char* bufPipe = new char[1024];
					if (value == "1") {
						string buffer("|");
						buffer.append(identity).append("||")
							.append(m1->get_nome(identity)).append("||")
							.append(to_string(pipeNumRic)).append("|");

						pipeNumRic = (pipeNumRic + 1) % 200000;

						//Comunico alla GUI che l'utente vuole inviare un file ed il nome della pipe da aprire
						WriteFile(this->mainPipe, buffer.c_str(), buffer.length(), 0, NULL);
						//Attendo la risposta dalla GUI
						ReadFile(this->mainPipe, "OK", 2, 0, NULL);
					}

					iSendResult = send(ClientSocket, portNumber.c_str(), 5, 0);
					if (iSendResult == SOCKET_ERROR)
						cout << "send failed with error: " << WSAGetLastError() << "(Error in sending to Client port number where Server is listening to)" << endl;
				}
			}
		}
		if (iResult < 0)
			cout << "recv failed with error: " << WSAGetLastError() << endl;
	}
}

/*
*Calcola la prima porta libera
*/
string TCP_Listener::port_number_calculator() {
	int i;
	//lock_guard<mutex> lg(*mutPorts);
	map<int, future<bool>>::iterator iter;

	for (iter = this->activePorts->begin(); iter != this->activePorts->end(); ) {
		auto val = iter->second.wait_for(chrono::milliseconds(1));
		if (val == future_status::ready) {
			actives[iter->first - TCP_SERVER_PORT] = false;
			this->activePorts->erase(iter);
			if (activePorts->empty()) {
				break;
			}

			iter = this->activePorts->begin();
		}
		else {
			++iter;
		}
	}

	for (i = 0; i < 50; i++) {
		if (actives[i] == false)
			break;
	}

	if (i == 50) {
		cout << "Porte non sufficienti" << endl;
		return to_string(-1);
	}

	return to_string(i + TCP_SERVER_PORT);
}

/*
*TCP_throw_thread crea un oggetto TCP thread e lo lancia su un nuovo thread
*/
bool TCP_Listener::TCP_throw_thread(string port, int tipo_file, string nome) {

	promise<bool> proStart; //promise che permette di comunicare quando il server � pronto
	promise<bool> proExit; //promise che permette al server di comunicare la porta come libera

	future<bool> fut = proStart.get_future();
	future<bool> futE = proExit.get_future();

	activePorts->insert(pair<int, future<bool>>(atoi(port.c_str()), move(futE)));
	actives[stoi(port) - TCP_SERVER_PORT] = true;

	Porta* porta = new Porta(stoi(port), move(proStart), move(proExit));
	HomePrinter* hp = new HomePrinter();
	
	TCP_Server* newServer = new TCP_Server(move(porta), tipo_file, this->mutPorts, hp, sharedFilePath, mutShared, nome, pipeNumRic);

	thread *newThread = new thread(*newServer);

	auto val = fut.wait_for(chrono::milliseconds(2000));

	//se la promise � stata settata il TCP_listener fa la detach dal thread,
	//altrimenti killa il thread server
	if (val == future_status::ready) {
		newThread->detach();
		return true;
	}
	else {
		TerminateThread(newThread, NULL);
		cout << "lanciare il Server" << endl;
		return false;
	}
}


