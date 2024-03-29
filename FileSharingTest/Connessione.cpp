#include "Connessione.h"

namespace connNmSpace {

	condition_variable Connessione::cvar;
	mutex Connessione::mut;
	mutex Connessione::mut_on_exit;
	bool Connessione::exit_discoverer;

	void lanciaBatch(wstring index, wstring params) {

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		//wstring tmp = L"..\\..\\..\\PDSProjectBatch.cmd ";
		wstring tmp = L"C:\\Users\\duran\\source\\repos\\FileSharing\\PDSProjectBatch.cmd ";
		tmp.append(index).append(params);

		wchar_t* text = &tmp[0];

		CreateProcess(
			NULL
			, text
			, NULL
			, NULL
			, TRUE
			, CREATE_NO_WINDOW
			, NULL
			, NULL
			, &si
			, &pi
		);

		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	//Controlla se una directory esiste
	bool dirExists(const std::string& dirName_in)
	{
		DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!

		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   // this is a directory!

		return false;    // this is not a directory!
	}

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

	//Funzione di stampa del MAC address su file
	void Connessione::printMAC() {
		//system("getmac > mac.txt");
		wstring com = L"1";
		lanciaBatch(com, L"");
	}

	//Costruttore dell'oggetto connessione
	Connessione::Connessione(string dati) {
		printMAC();
		this->pipeNumInvio = 0;
		this->sync_utenti = new Sync_mappa(this->getMACaddress());
		mutSharedPath = new mutex();
		exit_discoverer = false;
		this->start(dati);
		this->connect();
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

		return "1192.168.43.163";
		//system("ipconfig > ip.txt");
		wstring com = L"2";
		lanciaBatch(com, L"");

		ifstream readFile;
		string buf, first, second;
		string mask("IPv4");
		string mask2("LAN"), mask3("Ethernet Ethernet"), mask4("Wi-Fi");
		string prefix("192.168");
		bool LANFound = false;

		readFile.open("ip.txt");
		while (getline(readFile, buf)) {
			stringstream str(buf);
			getline(str, first, ':');
			getline(str, second);

			if (first.find(mask2) != string::npos ||
				first.find(mask3) != string::npos ||
				first.find(mask4) != string::npos)
				LANFound = true;
			if (first.find(mask) != string::npos && LANFound == true && second.find(prefix) != string::npos) {
				readFile.close();
				//system("del ip.txt");
				com = L"3";
				lanciaBatch(com, L"");

				return second;
			}
		}
		readFile.close();
		//system("del ip.txt");
		com = L"3";
		lanciaBatch(com, L"");
		return "";
	}

	//dato l'indirizzo IP dell'host ritorna l'indirizzo IP broadcast della rete
	string Connessione::getBroadcastIP(string ip) {
		stringstream str(ip);
		string buf, ipRet;

		getline(str, buf, '.');
		ipRet.assign(buf).append(".");
		getline(str, buf, '.');
		ipRet.append(buf).append(".");
		getline(str, buf, '.');
		ipRet.append(buf).append(".");
		ipRet.append("255");

		//elimina il primo spazio della stringa
		ipRet.erase(0, 1);

		return ipRet;
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
		 //funzioni per recuperare l'indirizzo IP broadcast dal proprio IP
		string ownIP, broadcastIP;
		ownIP = getOwnIP();

		if (ownIP == "") {
			cout << "Host non connesso alla rete";
			return;
		}	

		//broadcastIP = getBroadcastIP(ownIP);
		broadcastIP = "192.168.43.255";
		/*ofstream of ("C:\\Users\\duran\\Desktop\\out.txt");
		of << broadcastIP<< endl;
		of.close();*/

		//Struttura del sender
		struct sockaddr_in Sender_addr;
		Sender_addr.sin_family = AF_INET;
		Sender_addr.sin_port = htons(PORTA_IN_ASCOLTO);
		Sender_addr.sin_addr.s_addr = inet_addr(broadcastIP.c_str());
		int i = 0;

		while (1) {

			{
				lock_guard<mutex> lg(mut_on_exit);
				if (exit_discoverer) {
					exit_discoverer = false;
					return;
				}
			}
			


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
	bool Connessione::start(string dati) {

		/*---------------------------------------------
		CREAZIONE DELL'ALBERO DEI DIRETTORI SE NON ESISTONO
		*/
		//system("echo %USERPROFILE% >> homedir.txt");
		wstring com = L"4";
		lanciaBatch(com, L"");
		ifstream fpp;
		string path_tmp;
		fpp.open("homedir.txt");
		getline(fpp, path_tmp, ' ');
		path_tmp.append("\\");
		fpp.close();
		//system("del homedir.txt");
		com = L"5";
		lanciaBatch(com, L"");

		path_tmp.append("FileSharing");

		this->homePath.assign(path_tmp);

		if (!dirExists(path_tmp)) {
			//creazione della cartella FileSharing
			wstring com(L"9");
			lanciaBatch(com, L"");
		}
		path_tmp.append("\\Immagini_utenti");
		if (!dirExists(path_tmp)) {
			//Creazione della cartella Immagini_utenti
			wstring com(L"10");
			lanciaBatch(com, L"");
		}

		/*FINE DELLA CREAZIONE DELL'ALBERO DEI DIRETTORI
		-----------------------------------------------------------
		*/

		string fileSharingPath;
		fileSharingPath.assign(path_tmp).append("\\Credenziali.txt");



		Utente *rawUser = static_cast<Utente*> (Utente::apri_utente(dati, fileSharingPath));

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
	string Connessione::file_transfer(string path, SOCKADDR_IN ip_utente) {

		/*--------------------------------------------------------
		CREAZIONE DELLA PIPE*/

		string pipeID("pipe");
		pipeID.append(to_string(pipeNumInvio));
		
		

		HANDLE transferPipe;
		string strPipeName("\\\\.\\pipe\\");
		strPipeName.append(pipeID);

		std::wstring stemp = s2ws(strPipeName);
		LPCWSTR pipeName = stemp.c_str();

		//creazione dell'oggetto pipe
		transferPipe = CreateNamedPipe(
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
		if (transferPipe== INVALID_HANDLE_VALUE)
		{
			cout << "INVALID_HANDLE_VALUE" << GetLastError() << endl;
			cin.get();
			return "";
		}

		pipeNumInvio = (pipeNumInvio + 1) % 100000;
	
		/*FINE CREAZIONE DELLA PIPE
		---------------------------------------------------------------
		*/

		TCP_Client* newClient = new TCP_Client(path, ip_utente, 1, getMACaddress(), NULL, NULL, transferPipe);
		thread *newThread = new thread(*newClient);

		newThread->detach();

		return pipeID;
	}

	//funzione per recuperare un utente dalla mappa dato il MAC address
	Utente* Connessione::choose_user(string MAC) {
		Utente* utente = this->sync_utenti->get_utente(MAC);
		return utente;
	}

	//killa il thread discoverer rendendo invisibili sulla rete
	void Connessione::be_invisible() {
		//TerminateThread(discoverer->native_handle(), NULL); -> non funzionava, ho messo un'uscita strutturata
		lock_guard<mutex> lg(mut_on_exit);
		exit_discoverer = true;
	}

	//lancia il thread discoverer rendendo visibili sulla rete
	void Connessione::be_visible() {
		this->discoverer = new thread(Connessione::discoverer_function, this);
		
		//non si dovrebbe fare una detach()?
		this->discoverer->detach();
	}

	//cambia lo stato della visibilit�, modificando anche il file credenziali.txt (da fare la funzione in Utente)
	void Connessione::change_visibility(bool vs) {

		//ofstream fstr("C:\\Users\\Mattia\\Desktop\\logDll.txt");
			
		if (!this->utente_attivo->get_visibility()) {

				this->be_visible();
				/*ofstream f("C:\\Users\\duran\\Desktop\\log1.txt");
				f << "now you are visible :"<<" visibility:"<< to_string(this->utente_attivo->get_visibility()) << endl;
				f.close();
				*/
				string input, first, second, credPath(homePath), tmpCredPath(homePath);
				credPath.append("\\Credenziali.txt");
				tmpCredPath.append("\\tmpCred.txt");

			ifstream readFile(credPath);
			ofstream tmpFile(tmpCredPath);

			if (readFile.is_open() && tmpFile.is_open()) {

					while (!readFile.eof()) {
						readFile >> input;
						stringstream input_stringstream(input);
						if (getline(input_stringstream, first, '|')) {
							getline(input_stringstream, second);
							if (first == "Visible") {
								second = "true";
								tmpFile << first << "|" << second;
								break;
							}
							
							if (first != "Visible")
								tmpFile << first << "|" << second  << endl;
						}
					}
				}

				readFile.close();
				tmpFile.close();

				if (!remove(credPath.c_str())) {
					if (!rename(tmpCredPath.c_str(), credPath.c_str())) {

					}
					
				}
				

				{
					lock_guard<mutex> lg(*mutSharedPath);
					this->utente_attivo->set_visibility(true);
				}
			}
			else {

				this->be_invisible();

				string input, first, second, credPath(homePath), tmpCredPath(homePath);
				credPath.append("\\Credenziali.txt");
				tmpCredPath.append("\\tmpCred.txt");
				ifstream readFile(credPath);
				ofstream tmpFile(tmpCredPath);

			if (readFile.is_open() && tmpFile.is_open()) {
				while (!readFile.eof()) {
					readFile >> input;
					stringstream input_stringstream(input);
					if (getline(input_stringstream, first, '|')) {
						getline(input_stringstream, second);
						if (first == "Visible") {
							second = "false";
							tmpFile << first << "|" << second;
							break;
						}
							
						if (first != "Visible")
							tmpFile << first << "|" << second << endl;
					}
				}
			}

				readFile.close();
				tmpFile.close();

				if (!remove(credPath.c_str())) {
					if (!rename(tmpCredPath.c_str(), credPath.c_str())) {
						/*ofstream f("C:\\Users\\duran\\Desktop\\log_v_i.txt");
						f << "v-->i succesful" << endl;
						f.close();*/
					}
					
				}


				{
					lock_guard<mutex> lg(*mutSharedPath);
					this->utente_attivo->set_visibility(false);
				}
			}
		
	}

	//metodo per cambiare la default path dei files
	void Connessione::change_filepath(string path) {

		{
			lock_guard<mutex> lg(*mutSharedPath);
			this->utente_attivo->set_filepath(path);
		}

		string input, first, second, credPath(homePath), tmpCredPath(homePath);
		credPath.append("\\Credenziali.txt");
		tmpCredPath.append("\\tmpCred.txt");
		ifstream readFile(credPath);
		ofstream tmpFile(tmpCredPath);

		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, '|')) {
					getline(input_stringstream, second);
					if (first == "Path_file_transfer") {
						second = path;
					}
					if (first != "Visible") {
						tmpFile << first << "|" << second << endl;
					}
					else {
						tmpFile << first << "|" << second;
					}
				}
			}
		}

		readFile.close();
		tmpFile.close();

		if (!remove(credPath.c_str())) {
			if (!rename(tmpCredPath.c_str(), credPath.c_str())) {
				/*ofstream f("C:\\Users\\duran\\Desktop\\log_i_v.txt");
				f << "i-->v succesful" << endl;
				f.close();*/
			}

		}
	}

	//metodo per cambiare l'immagine profilo
	void Connessione::change_immagine_profilo(string foto_path) {

		{
			lock_guard<mutex> lg(*mutSharedPath);
			this->utente_attivo->set_percorso_foto(foto_path);
		}

		string input, first, second, credPath(homePath), tmpCredPath(homePath);
		credPath.append("\\Credenziali.txt");
		tmpCredPath.append("\\tmpCred.txt");
		ifstream readFile(credPath);
		ofstream tmpFile(tmpCredPath);

		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, '|')) {
					getline(input_stringstream, second);
					if (first == "Path_immagine_profilo") {
						second = foto_path;
					}
					if (first != "Visible") {
						tmpFile << first << "|" << second << endl;
					}
					else {
						tmpFile << first << "|" << second;
						break;
					}

				}
			}
		}

		readFile.close();
		tmpFile.close();

		

		if (!remove(credPath.c_str())) {
			rename(tmpCredPath.c_str(), credPath.c_str());
			/*ofstream f("C:\\Users\\duran\\Desktop\\log.txt");
			f << "op succesful" << endl;
			f.close();*/
		}


	}

	//metodo per cambiare il nome
	void Connessione::change_name(string name) {
		{
			lock_guard<mutex> lg(*mutSharedPath);
			this->utente_attivo->set_nome(name);
		}

		
		string input, first, second, credPath(homePath), tmpCredPath(homePath);
		credPath.append("\\Credenziali.txt");
		tmpCredPath.append("\\tmpCred.txt");

		ifstream readFile(credPath);
		ofstream tmpFile(tmpCredPath);


		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, '|')) {
					getline(input_stringstream, second);
					if (first == "Nome") {
						second = name;
					}
					if (first != "Visible") {
						tmpFile << first << "|" << second << endl;
					}
					else {
						tmpFile << first << "|" << second;
						break;
					}
				}
			}
		}

		readFile.close();
		tmpFile.close();

		if (!remove(credPath.c_str())) {
			if (!rename(tmpCredPath.c_str(), credPath.c_str())) {
				/*ofstream f("C:\\Users\\duran\\Desktop\\log_i_v.txt");
				f << "i-->v succesful" << endl;
				f.close();*/
			}

		}
	}

	//metodo per cambiare se il programma accetta automaticamente tutti i file 
	void Connessione::change_accept(bool val) {
		string valore;
		if (val) {
			valore = "true";
		}
		else {
			valore = "false";
		}

		string input, first, second, credPath(homePath), tmpCredPath(homePath);
		credPath.append("\\Credenziali.txt");
		tmpCredPath.append("\\tmpCred.txt");

		ifstream readFile(credPath);
		ofstream tmpFile(tmpCredPath);


		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, '|')) {
					getline(input_stringstream, second);
					if (first == "accettaAutomaticamente") {
						second = valore;
					}
					if (first != "Visible") {
						tmpFile << first << "|" << second << endl;
					}
					else {
						tmpFile << first << "|" << second;
						break;
					}
				}
			}
		}

		readFile.close();
		tmpFile.close();

		if (!remove(credPath.c_str())) {
			if (!rename(tmpCredPath.c_str(), credPath.c_str())) {
				/*ofstream f("C:\\Users\\duran\\Desktop\\log_i_v.txt");
				f << "i-->v succesful" << endl;
				f.close();*/
			}

		}
	}

	//metodo per cambiare il cognome
	void Connessione::change_surname(string surname)
	{
		{
			lock_guard<mutex> lg(*mutSharedPath);
			this->utente_attivo->set_nome(surname);
		}

		
		string input, first, second, credPath(homePath), tmpCredPath(homePath);
		credPath.append("\\Credenziali.txt");
		tmpCredPath.append("\\tmpCred.txt");
		ifstream readFile(credPath);
		ofstream tmpFile(tmpCredPath);

		if (readFile.is_open() && tmpFile.is_open()) {
			while (!readFile.eof()) {
				readFile >> input;
				stringstream input_stringstream(input);
				if (getline(input_stringstream, first, '|')) {
					getline(input_stringstream, second);
					if (first == "Cognome") {
						second = surname;
					}
					if (first != "Visible") {
						tmpFile << first << "|" << second << endl;
					}
					else {
						tmpFile << first << "|" << second;
						break;
					}
				}
			}
		}

		readFile.close();
		tmpFile.close();


		if (!remove(credPath.c_str())) {
			if (!rename(tmpCredPath.c_str(), credPath.c_str())) {
				/*ofstream f("C:\\Users\\duran\\Desktop\\log_i_v.txt");
				f << "i-->v succesful" << endl;
				f.close();*/
			}

		}
	}


	//classe per il blocco-sblocco utente
	void Connessione::blocco_utente(string MAC) {
		//se la funzione check_identity torna false significa che il MAC non � presente in black list
		if (this->sync_utenti->check_identity(MAC) == false)
			this->sync_utenti->blocco_utente(MAC);
		else
			this->sync_utenti->sblocco_utente(MAC);
	}


	//getter della visibilit� dell'utente attivo
	bool Connessione::getVisibilityUtenteAttivo() {
		return this->utente_attivo->get_visibility();
	}

	//getter della lista degli utenti presenti attualmente in mappa
	vector<char*>* Connessione::getUtentiConnessi() {
		return this->sync_utenti->getUtenti();
	}

	const char* Connessione::getHomeDir() {
		return this->homePath.c_str();
	}

	/*
	*------------------------------------------------------
	*Funzioni della classe wrapper di Connessione
	*------------------------------------------------------
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

	vector<char*>* ConnWrapper::getUtentiConnessi(Connessione* conn) {
		return conn->getUtentiConnessi();
	}

	void ConnWrapper::putInBlackList(Connessione* conn, char* MAC) {
		string mc(MAC);
		conn->blocco_utente(mc);
	}

	string ConnWrapper::inviaFile(Connessione* conn, char* file, char* MAC) {
		string mc(MAC);
		string fle(file);
		Utente* user;
		user = conn->choose_user(mc);
		if(user == NULL)
			return "-1";
		else
			return conn->file_transfer(fle, user->get_ip());
	}

	void ConnWrapper::cambiaFilePath(Connessione* conn, char* path) {
		string pth(path);
		conn->change_filepath(pth);
	}

	void ConnWrapper::cambiaImmagine(Connessione* conn, char* path) {
		string pth(path);
		conn->change_immagine_profilo(pth);
	}

	void ConnWrapper::cambiaNome(Connessione* conn, char* nome) {
		string nme(nome);
		conn->change_name(nme);
	}

	void ConnWrapper::cambiaCognome(Connessione* conn, char* cognome) {
		string cgnme(cognome);
		conn->change_surname(cgnme);
	}

	const char* ConnWrapper::getHomeDir(Connessione* conn) {
		return conn->getHomeDir();
	}

	void ConnWrapper::cambiaAccettaDefault(connNmSpace::Connessione* conn, bool val) {
		return conn->change_accept(val);
	}

}


connNmSpace::Connessione* creaConnessione(char* info) {
	string dati(info);
	return connNmSpace::ConnWrapper::creaConnessione(dati);
}

vector<char*>* getUtentiConnessi(connNmSpace::Connessione* conn) {
	return  connNmSpace::ConnWrapper::getUtentiConnessi(conn);
}

void modPrivata(connNmSpace::Connessione* conn) {
	connNmSpace::ConnWrapper::modPrivata(conn);
}

void putInBlackList(connNmSpace::Connessione* conn, char* MAC) {
	
	connNmSpace::ConnWrapper::putInBlackList(conn, MAC);
}

void inviaFile(connNmSpace::Connessione* conn, char* file, char* MAC, char* pipeID) {
	string pipe;
	
	pipe = connNmSpace::ConnWrapper::inviaFile(conn, file, MAC);
	strcpy(pipeID, pipe.c_str());
}

void cambiaFilePath(connNmSpace::Connessione* conn, char* path) {
	connNmSpace::ConnWrapper::cambiaFilePath(conn, path);
}

void cambiaImmagine(connNmSpace::Connessione* conn, char* path) {
	connNmSpace::ConnWrapper::cambiaImmagine(conn, path);
}

void cambiaNome(connNmSpace::Connessione* conn, char* nome) {
	connNmSpace::ConnWrapper::cambiaNome(conn, nome);
}

void cambiaCognome(connNmSpace::Connessione* conn, char* cognome) {
	connNmSpace::ConnWrapper::cambiaCognome(conn, cognome);
}

const char* getHomeDir(connNmSpace::Connessione* conn) {
	return connNmSpace::ConnWrapper::getHomeDir(conn);
}

void cambiaAccettaDefault(connNmSpace::Connessione* conn, bool val) {
	return connNmSpace::ConnWrapper::cambiaAccettaDefault(conn, val);
}

void firstGetHomeDir(char* str) {
	//system("echo %USERPROFILE% > homedir.txt");
	wstring com = L"4";
	connNmSpace::lanciaBatch(com, L"");
	ifstream fpp;
	string path_tmp;
	fpp.open("homedir.txt");
	getline(fpp, path_tmp, ' ');
	path_tmp.append("\\");
	fpp.close();
	//system("del homedir.txt");
	com = L"5";
	connNmSpace::lanciaBatch(com, L"");

	path_tmp.append("FileSharing\\");

	strcpy(str, path_tmp.c_str());
}




//Aggiunto per gestire il Marshalling di vector (informazioni sugli utenti connessi)

/*MARSHALING COLLECTION VECTOR TO EXPORT IT IN C#*/


bool MarshalVector(connNmSpace::Connessione* conn, ItemListHandle hItems, char*** ItemsData, int* ItemsCounter ){

	
	auto online_users = connNmSpace::ConnWrapper::getUtentiConnessi(conn);
	

	hItems = reinterpret_cast<ItemListHandle>(online_users);
	*ItemsData = online_users->data();
	*ItemsCounter = online_users->size();

	return true;
}

bool deleteVector(ItemListHandle item) {

	auto it = reinterpret_cast<vector<char*>*>(item);
	delete it;

	return true;

}