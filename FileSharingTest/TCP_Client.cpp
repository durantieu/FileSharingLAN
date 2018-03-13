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
	//è un file
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
	//mando al TCP_Listener il tipo file -> dovrei mandargli anche il mio MAC per permettergli di filtrare i mittenti prima di iniziare una connessione col server vera e propria.


	iResult = send(sock, send_buf.c_str(), (size_t)send_buf.size(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("Send function failed\n");
		closesocket(sock);
		WSACleanup();
		return;
	}

	//Ho richiesto la foto al TCP listener e muoio
	if (is_foto == 2) {
		iResult = recv(sock, recv_buf, 1, 0);  // aggiornamento 11/10/2017-> questa Recv è necessaria, infatti il TCP listener manda un ack... è necessario riceverlo.
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
	//(se il valore è 2 il Listener sgancia un thread Client per inviare una foto)
	//Teoricamente subito dopo la ricezione dell'ack qui sopra Il client Muore, ma dall'altra parte, simmetricamente ne nasce un altro con valore is_foto=3
	//che deve scrivere alla sua controparte TCP_Listener un pacchetto con valore != 2 (fin qua ci siamo)
	//2) aspetta che il listener (thread padre del server) gli notifichi la porta sulla quale il server sarà in ascolto
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
	//chiusura del socket

	closesocket(sock);
	recv_buf[5] = '\0';
	//cout << "(Client) Porta sulla quale connettersi al server: " << recv_buf << endl;

	string buffer(recv_buf);

	if (!buffer.compare("0")) { //controllo se la porta è valida -> se ricevo 0 significa che l'utente mi ha bloccato e non accetta connessioni da me
		cout << "Connessione rifiutata: l'utente ti ha bloccato" << endl;
		return;
	}


	//salvo la porta sulla quale il server è in ascolto
	string port_TCPserver;
	port_TCPserver.assign(recv_buf);

	//3) nuovo socket legato alla porta sulla quale il server sarà in ascolto e stabilire nuova connessione tcp 
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

		//invio delle info

		send(sock, data_to_send.c_str(), (size_t)strlen(data_to_send.c_str()), 0);
		//cout << "(Client) Info inviate al tcp server" << endl;
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
			recv_buf[1] = '\0';
			if (strcmp(recv_buf, "1")) {
				cout << "Errore lato server";
				return;
			}
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
		char* send_buf = new char[BUF_LEN];
		while (dati_rimasti > 0) {
			letto = fread(send_buf, sizeof(char), 50, fin);
			if (letto < 0) {
				printf("ERROR WHILE READING FILE\n");
			}
			if ((inviati = send(sock, send_buf, letto, 0))<0) {
				printf("--ERROR WHILE SENDING FILE--\n--ABORTING CLIENT--\n");
				return;
			}
			dati_rimasti -= inviati;
		}
		//cout << "file inviato" << endl;
		fclose(fin);
		free(send_buf);

		path_set.pop_front();

		char rbuf[2];
		recv(sock, rbuf, 2, 0);
		rbuf[1] = '\0';
		if (rbuf[0] == '1') {
			//cout << "file inviato correttamente" << endl;
		}

	}
	//6)  invia messaggio al server per chiudere la connessione
	if (is_foto == 3) {
		closesocket(sock);
		return;
	}
	string end_communication("end");
	end_communication.append("|");
	send(sock, end_communication.c_str(), (size_t)end_communication.size(), 0);
	closesocket(sock);
}

//navigazione file system
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
			//file_status è un oggetto in grado di darmi indicazioni sul tipo di file
			file_status s = status(path); //status(path) è una funzione che mi ritorna informazioni utili sull'oggetto path
										  //se è un file regolare, salva il percorso, se è una cartella o altro passa alla prossima iterazione 
			if (is_regular_file(s)) {
				v.push_back(st);
			}

		}
		flag_is_file = 1;
	}

	return v;
}