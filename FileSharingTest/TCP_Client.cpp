#include "TCP_Client.h"

#define BUF_LEN 1024

struct info {
	string percorso;
	string nome_file;
	string estensione;
	string size;
};

using namespace std::tr2::sys;

TCP_Client::TCP_Client(string path, SOCKADDR_IN ip, int is_foto, string MAC, mutex* mt, string* percorso_foto, HANDLE pipe) {
	this->cammino = path;
	this->ip = ip;
	this->is_foto = is_foto;
	this->MAC = MAC;
	this->path_foto = percorso_foto;
	this->mt_path_foto = mt;
	this->pipeHandle = pipe;
}

void TCP_Client::operator()() {

	string bufferPipe;
	char* bufferPipeRec = new char[1024];

	//1) send to tcp listener -> notifica dell'invio di un file
	WSADATA wsadata;
	size_t iResult;
	SOCKET sock;
	SOCKADDR_IN /*client_addr,*/ server_addr;

	//short port;
	string send_buf;
	char* recv_buf = new char[BUF_LEN];
	int recv_buf_len = BUF_LEN;

	HANDLE f;
	string percorso_assoluto; //percorso assoluto per aprire la handle al file da inviare

	fd_set fs;
	struct timeval tv;
	int flag_is_file = 0;
	deque<struct info> path_set;
	int total_size;
	if (is_foto == 3) {
		{
			lock_guard<mutex> lk(*mt_path_foto);
			cammino.assign(*path_foto);
		}
		path_set = navigazione_fs(cammino, flag_is_file);
	}
	if (is_foto == 1) {
		path_set = navigazione_fs(cammino, flag_is_file); //in path set ci dovrebbero essere i percorsi relativi del file, l'estensione e il size
		if (path_set.size() == 0) {
			cout << "Directory vuota" << endl;
			return;
		}
	}

	total_size = calcoloTotalSize(path_set);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0) {
		printf("WsaStartup failed\n");
		exit(0);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(TCP_LISTENER_PORT);
	server_addr.sin_addr = ip.sin_addr;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		printf("ERROR1: %d", WSAGetLastError());
		WSACleanup();
		exit(-2);
	}
	iResult = connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if (iResult == SOCKET_ERROR) {
		printf("ERROR2: %d", WSAGetLastError());
		closesocket(sock);
		sock = INVALID_SOCKET;
		return;
	}
	//� un file
	if (is_foto == 1) {
		send_buf.assign("1").append(" ").append(MAC).append(" ").append("\0");
	}
	//voglio la foto
	else if (is_foto == 2) {
		send_buf.assign("2").append(" ").append(MAC).append(" ").append("\0");
	}
	//invio di foto
	else if (is_foto == 3) {
		send_buf.assign("3").append(" ").append(MAC).append(" ").append("\0");
	}
	else {
		cout << "Protocol Error" << endl;
		return;
	}
	//mando al TCP_Listener il tipo file 

	iResult = send(sock, send_buf.c_str(), (size_t)send_buf.size(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("Send function failed\n");
		closesocket(sock);
		WSACleanup();
		return;
	}

	//Ho richiesto la foto al TCP listener e muoio
	if (is_foto == 2) {
		iResult = recv(sock, recv_buf, 1, 0);
		if (recv_buf[0] == '1') {
			closesocket(sock);
			return;
		}
		if (recv_buf[0] == '2') {
			cout << "Richiesta foto negata: utente bloccato" << endl;
		}
		cout << "errore in ricezione di ack" << endl;
		closesocket(sock);
		return;
	}
	//(se il valore � 2 il Listener sgancia un thread Client per inviare una foto)
	//Teoricamente subito dopo la ricezione dell'ack qui sopra Il client Muore, ma dall'altra parte, simmetricamente ne nasce un altro con valore is_foto=3
	//che deve scrivere alla sua controparte TCP_Listener un pacchetto con valore != 2 (fin qua ci siamo)
	//2) aspetta che il listener (thread padre del server) gli notifichi la porta sulla quale il server sar� in ascolto
	FD_ZERO(&fs);
	FD_SET(sock, &fs);
	tv.tv_sec = 100;
	tv.tv_usec = 0;
	if (!select(0, &fs, NULL, NULL, &tv)) {
		//Session Expired
		//situazione da gestire
		cout << "timeout per la ricezione della porta su cui inviare i dati scaduto." << endl;
		closesocket(sock);
		return;
	}
	//porta su cui connettersi con il server
	iResult = recv(sock, recv_buf, 5, 0);



	if (is_foto == 1) {
		ConnectNamedPipe(this->pipeHandle, NULL);

		bufferPipe = "OK\0";
		//inoltro il messaggio alla GUI
		WriteFile(this->pipeHandle, bufferPipe.c_str(), 3, 0, NULL);
	}



	//chiusura del socket

	closesocket(sock);
	recv_buf[5] = '\0';
	//cout << "(Client) Porta sulla quale connettersi al server: " << recv_buf << endl;

	string buffer(recv_buf);

	//salvo la porta sulla quale il server � in ascolto
	string port_TCPserver;
	port_TCPserver.assign(recv_buf);

	//3) nuovo socket legato alla porta sulla quale il server sar� in ascolto e stabilire nuova connessione tcp 
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		printf("ERROR: %d", WSAGetLastError());
		WSACleanup();
		exit(-2);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(port_TCPserver.c_str()));
	server_addr.sin_addr = ip.sin_addr;

	iResult = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)); //connect al server (figlio di TCP_Listener)
	if (iResult == SOCKET_ERROR) {
		closesocket(sock);
		sock = INVALID_SOCKET;
		printf("ERROR\n");
		return;
	}
	int i = 0;

	int total_dati_rimasti = total_size;

	if (is_foto == 1) {

		//attendo HELLO dalla GUI
		ReadFile(this->pipeHandle, bufferPipeRec, 1024, 0, NULL);

		//Invio di HELLO message
		send(sock, bufferPipeRec, (size_t)strlen(bufferPipeRec), 0);

		char* bufOK = new char[1024];
		recv(sock, bufOK, 1024, 0);
		bufferPipe = bufOK;

		WriteFile(this->pipeHandle, bufferPipe.c_str(), 3, 0, NULL);

		if (bufferPipe.find("X") != string::npos) {
			//Connessione rifiutata, chiudere tutto
			closesocket(sock);
			::CloseHandle(this->pipeHandle);
			return;
		}
		else {
			ReadFile(this->pipeHandle, bufferPipeRec, 1024, 0, NULL);
		}
	}

	while (!path_set.empty()) {
		string data_to_send;
		struct info temp;
		//4) invio informazioni di servizio del file quali -tipo: +file  +foto; 
		//   se file: +percorso +estensione +size
		//   se foto: +NULL     +estensione +size
		//se non differenziamo: foto: +percorso +estensione +size
		//   se cartella vuota: il client impedisce l'invio.
		//preparazione info da inviare
		temp = path_set.front();

		data_to_send.assign(temp.percorso);
		data_to_send.append("|");
		data_to_send.append(temp.nome_file);
		data_to_send.append("|");
		data_to_send.append(temp.estensione);
		data_to_send.append("|");
		data_to_send.append(temp.size);
		data_to_send.append("|");
		data_to_send.append(MAC);
		data_to_send.append("|");
		data_to_send.append(to_string(total_size));
		data_to_send.append("|");

		//invio delle info
		send(sock, data_to_send.c_str(), (size_t)strlen(data_to_send.c_str()), 0);
		i++;

		//4b) attesa ack
		FD_ZERO(&fs);
		FD_SET(sock, &fs);
		tv.tv_sec = 1000;
		tv.tv_usec = 0;
		if (!select(0, &fs, NULL, NULL, &tv)) {
			//Session Expired
			//situazione da gestire
			return;
		}
		else {
			recv(sock, recv_buf, recv_buf_len, 0);
		}
		//5)invio del file 
		//5a) identifico il percorso del file
		if (flag_is_file == 0) {
			percorso_assoluto.assign(cammino);
		}
		else {
			percorso_assoluto.assign(cammino).append(temp.percorso.substr(temp.percorso.find('\\'), temp.percorso.size())).append(temp.nome_file).append(temp.estensione);
		}
		//5b) apro la handle al file
		FILE* fin = fopen(percorso_assoluto.c_str(), "rb");
		if (fin == NULL) {
			cout << "Errore apertura file" << endl;
			return;
		}
		int dati_rimasti = atoi(temp.size.c_str());
		int size_file = atoi(temp.size.c_str());;
		int letto, inviati;
		float percentuale = 0;
		int percentualePrec = 0;
		char* send_buf = new char[65536/*BUF_LEN*/];

		while (dati_rimasti > 0) {
			letto = fread(send_buf, sizeof(char), 65000, fin);
			if (letto < 0) {
				printf("ERROR WHILE READING FILE\n");
			}
			if ((inviati = send(sock, send_buf, letto, 0)) < 0) {
				printf("--ERROR WHILE SENDING FILE--\n--ABORTING CLIENT--\n");
				return;
			}

			dati_rimasti -= inviati;

			total_dati_rimasti -= inviati;

			// dati verso pipe per il file:
			if (is_foto == 1) {
				char* bufPipe = new char[1024];

				//ricezione di ACK-Dati dal TCP_server
				recv(sock, recv_buf, recv_buf_len, 0);
				string recvBufStr(recv_buf);

				percentuale = (((float)total_size - (float)total_dati_rimasti) / (float)total_size) * 100;

				//Invio di % alla GUI-A
				string buffer("|");
				buffer.append(to_string((int)percentuale)).append("||");

				if ((int)percentuale < 9)
					buffer.append("|");

				WriteFile(this->pipeHandle, buffer.c_str(), 5, 0, NULL);
				percentualePrec = percentuale;

				char* bufACKA = new char[1024];
				//Ricezione di ACK-A dalla pipe
				ReadFile(this->pipeHandle, bufACKA, 1024, 0, NULL);

				bufACKA[1] = '\0';
				//Invio di ACK-A a TCP-Server
				string bufAckAStr(bufACKA);

				send(sock, bufACKA, 2, 0);

				if (bufferPipe.find("X") != string::npos) {
					closesocket(sock);
					::CloseHandle(this->pipeHandle);
					fclose(fin);

					return;
				}

				//ricezione di ACK-B dal TCP_server
				recv(sock, recv_buf, recv_buf_len, 0);
				recvBufStr = recv_buf;

				WriteFile(this->pipeHandle, recvBufStr.c_str(), recvBufStr.length(), 0, NULL);
				if (recvBufStr.find("X") != string::npos) {
					closesocket(sock);
					::CloseHandle(this->pipeHandle);
					fclose(fin);
					return;
				}
				else {
					//Ricezione di ACK-ACK-B dalla pipe
					ReadFile(this->pipeHandle, bufPipe, 1024, 0, NULL);
				}

			}
		}

		fclose(fin);
		free(send_buf);

		path_set.pop_front();

		char rbuf[2];
		recv(sock, rbuf, 2, 0);
		rbuf[1] = '\0';
		if (rbuf[0] == '1') {
			cout << "file inviato correttamente" << endl;
		}
	}

	if (is_foto == 1) {
		string buffer("|");
		buffer.append("100").append("|");
		WriteFile(this->pipeHandle, buffer.c_str(), buffer.length(), 0, NULL);
		::CloseHandle(this->pipeHandle);
	}

	//6)  invia messaggio al server per chiudere la connessione
	if (is_foto == 3) {
		closesocket(sock);
		return;
	}

	string end_communication("end");
	end_communication.append("|");
	send(sock, end_communication.c_str(), (size_t)end_communication.size(), 0);
	recv(sock, bufferPipeRec, 1024, 0);
	closesocket(sock);
}



//navigazione file 
//pusha dentro vector<struct info> tutti i percorsi relativi a partire dalla root dei file da inviare
deque<struct info> TCP_Client::navigazione_fs(string root, int& flag_is_file) {

	deque<struct info> v;
	path folder(root);
	path null_ext("");
	recursive_directory_iterator start(folder), end;
	file_status fstatus = status(root);

	if (is_regular_file(folder)) {
		struct stat stat_buf;
		struct info inf;
		inf.nome_file = folder.filename().string();
		inf.nome_file = inf.nome_file.substr(0, inf.nome_file.find_last_of("."));

		inf.percorso = "";
		inf.estensione = folder.extension().string();
		int rc = stat(root.c_str(), &stat_buf);
		if (rc != 0) {
			printf("error opening file\n");
			exit(-1);
		}
		inf.size = to_string(stat_buf.st_size);
		flag_is_file = 0;
		v.push_back(inf);
	}
	else {

		string str;
		int counter = 0;
		stringstream ss(root);
		while (getline(ss, str, '\\')) {
			counter++;
		}

		for (; start != end; ++start) {
			struct info st;
			size_t pos;
			string relative_path, ext;
			struct _stat *stat_buf = new struct _stat();

			auto path_senza_ext = start->path();
			path_senza_ext.replace_extension(null_ext);
			auto path = path_senza_ext.string();

			//Estraggo il percorso relativo a partire dalla cartella root
			stringstream ss1(path);
			int j = 0;
			while (getline(ss1, relative_path, '\\')) {
				j++;
				if (j == counter - 1) {
					getline(ss1, relative_path, '\0');
					break;
				}
			}


			string tmp2;
			stringstream ss2(relative_path);

			while (getline(ss2, tmp2, '\\'));

			pos = relative_path.find(tmp2);
			string tmp3 = relative_path.substr(0, pos);
			ext = start->path().extension().string();
			st.percorso.assign(tmp3);
			st.nome_file.assign(tmp2);
			st.estensione.assign(ext);
			string tmp = path;
			tmp.append(ext);
			int rc = _stat(tmp.c_str(), stat_buf);
			if (rc != 0) {
				printf("error opening file\n");
				exit(-1);
			}
			st.size = to_string(stat_buf->st_size);
			path.append(ext);
			//file_status � un oggetto in grado di darmi indicazioni sul tipo di file
			file_status s = status(path); //status(path) � una funzione che mi ritorna informazioni utili sull'oggetto path
										  //se � un file regolare, salva il percorso, se � una cartella o altro passa alla prossima iterazione 
			if (is_regular_file(s)) {
				v.push_back(st);
			}

		}
		flag_is_file = 1;
	}

	return v;
}


int TCP_Client::calcoloTotalSize(deque<struct info> paths) {
	int total_size = 0;
	for each(struct info i in paths) {
		total_size += atoi(i.size.c_str());
	}
	return total_size;
}