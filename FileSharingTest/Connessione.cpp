#include "Connessione.h"


namespace connNmSpace {

	condition_variable Connessione::cvar;
	mutex Connessione::mut;

	Connessione::Connessione() {
		printMAC();
		this->sync_utenti = new Sync_mappa(this->getMACaddress());
		mutSharedPath = new mutex();

		this->start();
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

	/*
	*Discoverer, thread sganciato dal metodo connessione::start()
	*si occupa di inviare pacchetti UDP in braodcast ogni n secondi;
	*se viene impostata la modalit� privata � necessario che questo thread venga
	*chiuso, per poi eventualmente essere riaperto impostando
	*nuovamente la modalit� pubblica
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

		//Struttura del sender
		struct sockaddr_in Sender_addr;
		Sender_addr.sin_family = AF_INET;
		Sender_addr.sin_port = htons(PORTA_IN_ASCOLTO);
		Sender_addr.sin_addr.s_addr = inet_addr("192.168.43.255");

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
					flag_foto.assign("1"); // si ho una foto, non chiedermela per�
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

				// Riconverto la stringa contente il timeStamp del pacchetto in formato time_t per vedere se il pacchetto � scaduto
				std::istringstream stream(tStamp);
				time_t tm;
				stream >> tm;
				double k;
				if ((k = difftime(time(NULL), tm)) > 1000) { //Se il pacchetto � scaduto, scartalo
					cout << difftime(time(NULL), tm) << "seconds passed: packet expired" << endl;
					continue;
				}
				if (MAC_address.compare(getMACaddress()) != 0) {
					//acquisizione mutex
					lock_guard<mutex> lock(Connessione::mut);

					m1->cerca_e_inserisci(MAC_address, nome, cognome, time(NULL), flag_foto, ip);

					Connessione::cvar.notify_one();
					//una volta uscito il mutex � rilasciato
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
	bool Connessione::start() {
		Utente *rawUser = static_cast<Utente*> (Utente::apri_utente());

		if (rawUser == NULL) {
			cout << "Impossibile aprire nuovo utente...uscita" << endl;
			return FALSE;
		}

		unique_ptr<Utente> user(rawUser);
		this->utente_attivo = move(user);

		this->sharedpath = this->utente_attivo->get_filepathPointer();
		this->sharedFotoPath = this->utente_attivo->get_fotopathPointer();
		this->TCPList = new TCP_Listener(this->sync_utenti, getMACaddress(), this->sharedpath, this->mutSharedPath, this->sharedFotoPath);

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

	Utente* Connessione::choose_user() {
		string utente;
		vector<Utente*> scelta;
		cout << "Scegli un Utente (nome e cognome separati da spazio): " << endl;

		this->sync_utenti->stampa_utenti();
		string tmp;
		cin >> utente;
		cin >> tmp;
		utente.append(" ").append(tmp);
		scelta = this->sync_utenti->estrai_utente(utente);

		if (scelta.size() == 0) {
			cout << "Nessun utente presente in rete" << endl;
			return NULL;
		}
		else {
			if (scelta.size() == 1) {
				return scelta.back();
			}
			else {
				cout << "Multiple occurrences found: choose which one" << endl;

				for (auto it = scelta.begin(); it != scelta.end(); it++) {
					cout << (*it)->get_nome() << endl;
				}
				int i;
				cin >> i;
				return scelta.at(i);
			}
		}
	}

	//killa il thread discoverer rendendo invisibili sulla rete
	void Connessione::be_invisible() {
		TerminateThread(discoverer->native_handle(), NULL);
	}

	//lancia il thread discoverer rendendo visibili sulla rete
	void Connessione::be_visible() {
		this->discoverer = new thread(Connessione::discoverer_function, this);
	}

	//cambia lo stato della visibilit�, modificando anche il file credenziali.txt (da fare la funzione in Utente)
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

	//classi wrapper per il blocco-sblocco utente
	void Connessione::blocca_utente(string utente) {
		this->sync_utenti->blocco(utente);
		return;
	}

	void Connessione::sblocco_utente(string utente) {
		this->sync_utenti->blocco(utente);
		return;
	}

	//getter della visibilit� dell'utente attivo
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

	Connessione* ConnWrapper::creaConnessione() {
		return new Connessione();
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
}
