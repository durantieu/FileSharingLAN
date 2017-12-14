#include "Connessione.h"


namespace connNmSpace {

	condition_variable Connessione::cvar;
	mutex Connessione::mut;

	Connessione::Connessione(string dati) {
		printMAC();
		this->sync_utenti = new Sync_mappa(this->getMACaddress());
		mutSharedPath = new mutex();

		this->start(dati);
		this->connect();
	}

	void Connessione::printMAC() {
		system("getmac > mac.txt");
	}

	/*
	*Funzione che ritorna il MAC address del PC (relativo alla nic connessa alla rete)
	*/
	string Connessione::getMACaddress(void) {
		ifstream readFile;
		string buf, MAC, state;
		int index = 0;

		readFile.open("mac.txt");

		while (std::getline(readFile, buf)) {

			if (index > 2) {
				istringstream lineStream(buf);
				getline(lineStream, MAC, ' ');
				getline(lineStream, state, '\0');

				int i;
				for (i = 0; i < (int)state.size(); i++) {
					if (state[i] != ' ')
						break;
				}
				if (state[i] == '\\') {
					readFile.close();
					//cout << "mac address:" << MAC << endl;
					return MAC;
				}
			}
			index++;
		}

		readFile.close();
		return "";
	}

	//ritorna il proprio indirizzo IP
	string Connessione::getOwnIP() {
		system("ipconfig > ip.txt");

		ifstream readFile;
		string buf, first, second;
		string mask("IPv4");
		string mask2("LAN");
		bool LANFound = false;

		readFile.open("ip.txt");
		while (getline(readFile, buf)) {
			stringstream str(buf);
			getline(str, first, ':');
			getline(str, second);

			if (first.find(mask2) != string::npos)
				LANFound = true;
			if (first.find(mask)!= string::npos && LANFound == true) {
				readFile.close();
				system("del ip.txt");
				return second;
			}	
		}
		readFile.close();
		system("del ip.txt");
		return "";
	}

	//dato l'indirizzo IP dell'host ritorna l'indirizzo IP broadcast della rete
	string Connessione::getBroadcastIP(string ip) {
		stringstream str(ip);
		string buf, ipRet;

		cout << ip << endl;
		getline(str, buf, '.');
		ipRet.assign(buf).append(".");
		getline(str, buf, '.');
		ipRet.append(buf).append(".");
		getline(str, buf, '.');
		ipRet.append(buf).append(".");
		ipRet.append("255");

		//elimina il primo spazio della stringa
		ipRet.erase(0, 1);

		ofstream f; 
		f.open("C:\\Users\\duran\\Desktop\\ciao.txt");
		f << ipRet;
		f.close();


		return ipRet;
	}

	/*
	*Discoverer, thread sganciato dal metodo connessione::start()
	*si occupa di inviare pacchetti UDP in braodcast ogni n secondi;
	*se viene impostata la modalità privata è necessario che questo thread venga
	*chiuso, per poi eventualmente essere riaperto impostando
	*nuovamente la modalità pubblica
	*/
	void Connessione::discoverer_function(Connessione *conn) {

		//cout << "Discoverer launched" << endl;

		string nome = conn->utente_attivo.get()->get_nome();
		string cognome = conn->utente_attivo.get()->get_cognome();
		string MAC = getMACaddress();
		string flag_foto = "0";

		if (MAC == "") {
			cout << "NIC non trovata" << endl;
		}



		string outputStr;
		outputStr.assign(MAC).append(" ").append(nome).append(" ")\
			.append(cognome).append(" ");


		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock == INVALID_SOCKET)
		{
			perror("socket creation");
			return;
		}

		BOOL enabled = TRUE;
		if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(BOOL)) < 0)
		{
			perror("broadcast options");
			closesocket(sock);
			return;
		}
		 //funzioni per recuperare l'indirizzo IP broadcast dal proprio IP
		string ownIP, broadcastIP;
		ownIP = getOwnIP();
		if (ownIP == "") {
			cout << "Host non connesso alla rete";
			return;
		}	

		broadcastIP = getBroadcastIP(ownIP);
		

		//Struttura del sender
		struct sockaddr_in Sender_addr;
		Sender_addr.sin_family = AF_INET;
		Sender_addr.sin_port = htons(PORTA_IN_ASCOLTO);
		Sender_addr.sin_addr.s_addr = inet_addr(broadcastIP.c_str());

		int i = 0;

		while (1) {

			outputStr.assign(MAC).append(" ").append(nome).append(" ")\
				.append(cognome).append(" ");
			Sleep(3000);
			if (i == 5) {
				if (conn->utente_attivo->get_percorso_foto().compare("0") != 0)
					flag_foto.assign("2"); // si, ho una foto, chiedimela 
				else
					flag_foto.assign("0"); // non ho foto da mandare

				i = 0;
			}
			else {
				if (!conn->utente_attivo->get_percorso_foto().compare("0")) {
					flag_foto.assign("0"); //non ho foto da mandare
				}
				else {
					flag_foto.assign("1"); // si ho una foto, non chiedermela però
				}
			}
			outputStr.append(flag_foto).append(" ");
			//Converto il timeStamp in formato stringa per poterlo inviare
			std::ostringstream oss;
			time_t tst = time(NULL);
			oss << tst;
			string tStamp = oss.str();
			outputStr.append(tStamp).append(" ");

			const char* stringa = outputStr.c_str();
			if (sendto(sock, stringa, strlen(stringa) + 1, 0, (sockaddr *)&Sender_addr, sizeof(Sender_addr)) < 0) {
				cout << "Errore nell'invio del pacchetto UDP" << endl;
			}
			else
				//cout << "Pacchetto UDP inviato" << endl;

				i++;
		}

		closesocket(sock);

		WSACleanup();
		return;
	}

	/*
	*modulo che si occupa di ascoltare i pacchetti che viaggiano in LAN e salvarli nella mappa
	*/
	void Connessione::listener_function(Sync_mappa *m1) {

		//cout << "Listener launched" << endl;

		SOCKET listen_socket;
		SOCKADDR_IN sender_address, recv_address;

		short port = PORTA_IN_ASCOLTO; // numero di porta sulla quale ascoltare

		int iResult = 0;
		char *buf = new char[BUF_LENGTH];
		int buflen = BUF_LENGTH;
		int wsa_startup; //variabile per il controllo versione 

		WORD wVersionRequested = 0x0202;
		WSADATA wsaData;

		int slen = sizeof(sockaddr);

		//controllo versione 
		wsa_startup = WSAStartup(wVersionRequested, &wsaData);
		if (wsa_startup != NO_ERROR) {
			cout << "Errore WSA_STARTUP()" << endl;
		}

		//apro il socket
		listen_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (listen_socket < 0) {
			cout << "Impossibile aprire il socket" << endl;
		}

		//inizializzo la struttura con i dettagli per il socket
		recv_address.sin_family = AF_INET;
		recv_address.sin_port = htons(port);
		recv_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		//bindo il socket alla porta

		::bind(listen_socket, (sockaddr *)&recv_address, sizeof(sockaddr));

		while (1) {

			iResult = recvfrom(listen_socket, buf, buflen, 0, (sockaddr*)&sender_address, &slen);
			//cout << buf << endl;
			if (iResult == SOCKET_ERROR) {

				wprintf(L"recvfrom fallita con errore: %d\n", WSAGetLastError());
				//cout << "Errore in ricezione" << endl;
				continue;
			}
			else {
				//codice per l'aggiunta di nuove entry nella struttura dati.
				SOCKADDR_IN ip = sender_address;
				//ip.assign(sender_address.sin_addr.S_un.S_addr);
				string str;
				str.assign(buf);
				string nome;
				string cognome;
				string MAC_address;
				string flag_foto;
				string timeStamp;
				stringstream input(str);
				string tStamp;
				/*
				Parsing del contenuto del pacchetto nel formato:

				(MAC_ADDRESS' 'NOME' 'COGNOME' 'FLAG_FOTO' 'TIMESTAMP' ')

				*/
				getline(input, MAC_address, ' ');
				getline(input, nome, ' ');
				getline(input, cognome, ' ');
				getline(input, flag_foto, ' ');
				getline(input, tStamp, ' ');

				// Riconverto la stringa contente il timeStamp del pacchetto in formato time_t per vedere se il pacchetto è scaduto
				std::istringstream stream(tStamp);
				time_t tm;
				stream >> tm;
				double k;
				if ((k = difftime(time(NULL), tm)) > 1000) { //Se il pacchetto è scaduto, scartalo
					cout << difftime(time(NULL), tm) << "seconds passed: packet expired" << endl;
					continue;
				}
				if (MAC_address.compare(getMACaddress()) != 0) {
					//acquisizione mutex
					lock_guard<mutex> lock(Connessione::mut);

					m1->cerca_e_inserisci(MAC_address, nome, cognome, time(NULL), flag_foto, ip);

					Connessione::cvar.notify_one();
					//una volta uscito il mutex è rilasciato
				}
			}
		}
		iResult = closesocket(listen_socket);
		if (iResult == SOCKET_ERROR) {
			cout << "Errore in chiusura del socket" << endl;
		}
		WSACleanup();
	}

	/*
	*modulo che si occupa di aggiornare la mappa utenti per identificare gli utenti disconnessi
	*/
	void Connessione::updater_function(Sync_mappa *m1) {

		//cout << "Updater launched" << endl;

		while (1) {
			unique_lock<mutex> lk(Connessione::mut);
			Connessione::cvar.wait_for(lk, 100ms);
			m1->scorri_e_aggiorna();
			lk.unlock();
		}
	}

	/*
	*modulo di apertura dell'utente con login o registrazione
	*/
	bool Connessione::start(string dati) {
		Utente *rawUser = static_cast<Utente*> (Utente::apri_utente(dati));

		if (rawUser == NULL) {
			cout << "Impossibile aprire nuovo utente...uscita" << endl;
			return FALSE;
		}

		unique_ptr<Utente> user(rawUser);
		this->utente_attivo = move(user);

		this->sharedpath = this->utente_attivo->get_filepathPointer();
		this->sharedFotoPath = this->utente_attivo->get_fotopathPointer();
		this->TCPList = new TCP_Listener(this->sync_utenti, getMACaddress(), this->sharedpath, this->mutSharedPath, this->sharedFotoPath);

		return TRUE;
	}

	/*
	*Funzione di sgancio dei thread discoverer, listener, updater e TCP_listener
	*/
	void Connessione::connect() {
		if (this->utente_attivo->get_visibility() == true)
			this->be_visible();
		else
			cout << "Sei invisibile sulla rete" << endl;
		this->listener = new thread(Connessione::listener_function, this->sync_utenti);
		this->updater = new thread(Connessione::updater_function, this->sync_utenti);

		this->TCP_listener = new thread(*TCPList);
	}

	/*
	*Funzione per trasferire un file ad un utente
	*/
	void Connessione::file_transfer(string path, SOCKADDR_IN ip_utente) {
		TCP_Client* newClient = new TCP_Client(path, ip_utente, 1, getMACaddress(), NULL, NULL);
		thread *newThread = new thread(*newClient);

		newThread->detach();
	}

	//funzione per recuperare un utente dalla mappa dato il MAC address
	Utente* Connessione::choose_user(string MAC) {
		Utente* utente = this->sync_utenti->get_utente(MAC);
		return utente;
	}

	//killa il thread discoverer rendendo invisibili sulla rete
	void Connessione::be_invisible() {
		TerminateThread(discoverer->native_handle(), NULL);
	}

	//lancia il thread discoverer rendendo visibili sulla rete
	void Connessione::be_visible() {
		this->discoverer = new thread(Connessione::discoverer_function, this);
	}

	//cambia lo stato della visibilità, modificando anche il file credenziali.txt (da fare la funzione in Utente)
	void Connessione::change_visibility(bool vs) {
		if (vs == true) {
			if (this->utente_attivo->get_visibility() == false) {
				this->be_visible();
				ifstream readFile;
				ofstream tmpFile;
				string input, first, second;
				readFile.open("credenziali.txt");
				tmpFile.open("tmpCred.txt");

				if (readFile.is_open() && tmpFile.is_open()) {
					while (!readFile.eof()) {
						readFile >> input;
						stringstream input_stringstream(input);
						if (getline(input_stringstream, first, ':')) {
							getline(input_stringstream, second);
							if (first == "visible") {
								second = "false";
							}
							tmpFile << first << ":" << second;
							if (first != "visible")
								tmpFile << endl;
						}
					}
				}

				readFile.close();
				tmpFile.close();
				system("del credenziali.txt");
				system("rename tmpCred.txt credenziali.txt");
				this->utente_attivo->set_visibility(true);
			}
		}
		else {
			if (this->utente_attivo->get_visibility() == true) {
				this->be_invisible();
				ifstream readFile;
				ofstream tmpFile;
				string input, first, second;
				readFile.open("credenziali.txt");
				tmpFile.open("tmpCred.txt");

				if (readFile.is_open() && tmpFile.is_open()) {
					while (!readFile.eof()) {
						readFile >> input;
						stringstream input_stringstream(input);
						if (getline(input_stringstream, first, ':')) {
							getline(input_stringstream, second);
							if (first == "visible") {
								second = "false";
							}
							tmpFile << first << ":" << second;
							if (first != "visible")
								tmpFile << endl;
						}
					}
				}

				readFile.close();
				tmpFile.close();
				system("del credenziali.txt");
				system("rename tmpCred.txt credenziali.txt");
				this->utente_attivo->set_visibility(false);
			}
		}
	}

	//classe per cambiare la default path dei files
	void Connessione::change_filepath(string path) {

		{
			lock_guard<mutex> lg(*mutSharedPath);
			this->utente_attivo->set_filepath(path);
		}

		ifstream readFile;
		ofstream tmpFile;
		string input, first, second;
		readFile.open("credenziali.txt");
		tmpFile.open("tmpCred.txt");

		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, ':')) {
					getline(input_stringstream, second);
					if (first == "filepath") {
						second = path;
					}
					if (first != "visible") {
						tmpFile << first << ":" << second << endl;
					}
					else {
						tmpFile << first << ":" << second;
					}
				}
			}
		}

		readFile.close();
		tmpFile.close();
		system("del credenziali.txt");
		system("rename tmpCred.txt credenziali.txt");
	}

	void Connessione::change_immagine_profilo(string foto_path) {

		{
			lock_guard<mutex> lg(*mutSharedPath);
			this->utente_attivo->set_percorso_foto(foto_path);
		}

		ifstream readFile;
		ofstream tmpFile;
		string input, first, second;
		readFile.open("credenziali.txt");
		tmpFile.open("tmpCred.txt");

		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, ':')) {
					getline(input_stringstream, second);
					if (first == "path") {
						second = foto_path;
					}
					if (first != "visible") {
						tmpFile << first << ":" << second << endl;
					}
					else {
						tmpFile << first << ":" << second;
					}
				}
			}
		}

		readFile.close();
		tmpFile.close();
		system("del credenziali.txt");
		system("rename credenziali.txt tmpCred.txt");

	}

	//classe per il blocco-sblocco utente
	void Connessione::blocco_utente(string MAC) {
		//se la funzione check_identity torna false significa che il MAC non è presente in black list
		if (this->sync_utenti->check_identity(MAC) == false)
			this->sync_utenti->blocco_utente(MAC);
		else
			this->sync_utenti->sblocco_utente(MAC);
	}


	//getter della visibilità dell'utente attivo
	bool Connessione::getVisibilityUtenteAttivo() {
		return this->utente_attivo->get_visibility();
	}

	//getter della lista degli utenti presenti attualmente in mappa
	list<string> Connessione::getUtentiConnessi() {
		return this->sync_utenti->getUtenti();
	}



	/*
	*
	*Funzioni della classe wrapper di Connessione
	*
	*/
	Connessione* ConnWrapper::creaConnessione(string dati) {
		return new Connessione(dati);
	}

	void ConnWrapper::modPrivata(Connessione* conn) {
		if (conn->getVisibilityUtenteAttivo() == true)
			conn->change_visibility(false);
		else
			conn->change_visibility(true);
	}

	list<string> ConnWrapper::getUtentiConnessi(Connessione* conn) {
		return conn->getUtentiConnessi();
	}

	void ConnWrapper::putInBlackList(Connessione* conn, string MAC) {
		conn->blocco_utente(MAC);
	}

	void ConnWrapper::inviaFile(Connessione* conn, string file, string MAC) {
		Utente* user = conn->choose_user(MAC);
		conn->file_transfer(file, user->get_ip());
	}

	void ConnWrapper::cambiaFilePath(Connessione* conn, string path) {
		conn->change_filepath(path);
	}

	void ConnWrapper::cambiaImmagine(Connessione* conn, string path) {
		conn->change_immagine_profilo(path);
	}


}


connNmSpace::Connessione* creaConnessione(char* info) {
	string dati(info);
	ofstream f;
	f.open("C:\\Users\\duran\\Desktop\\l.txt");
	f<<dati;
	f.close();
	return connNmSpace::ConnWrapper::creaConnessione(dati);
}

list<string>* getUtentiConnessi(connNmSpace::Connessione* conn) {
	return  &(connNmSpace::ConnWrapper::getUtentiConnessi(conn));
}

void modPrivata(connNmSpace::Connessione* conn) {
	connNmSpace::ConnWrapper::modPrivata(conn);
}

void putInBlackList(connNmSpace::Connessione* conn, string MAC) {
	connNmSpace::ConnWrapper::putInBlackList(conn, MAC);
}

void inviaFile(connNmSpace::Connessione* conn, string file, string MAC) {
	connNmSpace::ConnWrapper::inviaFile(conn, file, MAC);
}

void cambiaFilePath(connNmSpace::Connessione* conn, string path) {
	connNmSpace::ConnWrapper::cambiaFilePath(conn, path);
}

void cambiaImmagine(connNmSpace::Connessione* conn, string path) {
	connNmSpace::ConnWrapper::cambiaImmagine(conn, path);
}

