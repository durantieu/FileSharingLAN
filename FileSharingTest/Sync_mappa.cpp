#include "Sync_Mappa.h"

int Sync_mappa::cerca_e_inserisci(string chiave, string nome, string cognome, time_t timeStamp, string flag_foto, SOCKADDR_IN ip) {
	map<string, Utente>::iterator it;

	m.lock();

	it = utenti.find(chiave);

	if (it == utenti.end()) {
		//utente non presente--> inserisci utente in mappa 
		Utente u;
		u.set_nome(nome);
		u.set_cognome(cognome);
		u.set_time_stamp(timeStamp);
		u.set_percorso_foto(flag_foto); //in questo momento il valore può essere o 0=utente non ha foto / 1=utente ha la foto ma è da scaricare
		u.set_ip(ip);
		utenti.insert(pair<string, Utente>(chiave, u));
		cout << "inserito " << nome << endl;
		m.unlock();

		return 1;
	}
	else {
		if ((!it->second.get_percorso_foto().compare("0") && !flag_foto.compare("1")) || (!flag_foto.compare("2"))) {
			//foto da scaricare -> Sgancia un Client con valore tipo_file = 2.

			it->second.set_percorso_foto("1"); //il percorso foto può valere: 0-> non ha foto; 1->ce l'ha ma è da scaricare; 2->sto scaricando la foto (qui non dovrei mai leggerlo come valore)
			this->scarica_foto(it->second.get_ip());

		}
		else {
			if (it->second.get_percorso_foto().compare("0") && !flag_foto.compare("0")) { //quando qualcuno non vuole più che la sua foto si veda

				string command;

				if (it->second.get_percorso_foto().compare("1")) {
					command.append("del ").append(it->second.get_percorso_foto()); //esiste un percorso foto in mappa utenti
					system(command.c_str());
				}
				//gestire con eccezione percorso non valido
				it->second.set_percorso_foto("0");
			}
			else {

			}
		}
		it->second.set_time_stamp(timeStamp);
		m.unlock();
		//utente già presente--> non inserire
		return 0;
	}
}

int Sync_mappa::scorri_e_aggiorna() {
	//acquisisco il mutex
	m.lock();

	//scorro la mappa in cerca di elementi scaduti
	for (auto it = utenti.begin(); it != utenti.end();) {
		if (difftime(time(NULL), it->second.get_time_stamp()) > EXPIRED) {
			utenti.erase(it++);
		}
		else {
			if (!it->second.get_percorso_foto().compare("1")) {
				scarica_foto(it->second.get_ip());
				it->second.set_percorso_foto("2"); // sto scaricando la foto
			}
			++it;
		}
	}

	//rilascio il mutex
	m.unlock();

	return 0;
}

/*
*Funzione per inviare la foto
*/
void Sync_mappa::scarica_foto(SOCKADDR_IN ip) {
	TCP_Client* newClient = new TCP_Client("", ip, 2, MAC, NULL, NULL);
	thread* newThread = new thread(*newClient);

	newThread->detach();
}

void Sync_mappa::stampa_utenti() {
	m.lock();
	for (auto it = utenti.begin(); it != utenti.end(); ++it) {
		cout << "Utente 1: ";
		cout << "Nome: " << it->second.get_nome() << endl << "Cognome " << it->second.get_cognome() << endl;
	}
	m.unlock();
}

//ritorna il nome di un utente dato un MAC address
string Sync_mappa::get_nome(string MAC) {
	m.lock();
	auto user = utenti.find(MAC);
	string nome = user->second.get_nome();
	nome.append(" ").append(user->second.get_cognome());
	m.unlock();
	return nome;
}

//ritorna un utente dato un MAC address
Utente* Sync_mappa::get_utente(string MAC) {
	m.lock();
	auto user = utenti.find(MAC);
	m.unlock();
	return &(user->second);
}


vector<Utente*> Sync_mappa::estrai_utente(string utente) {
	vector<Utente*> v;
	m.lock();
	string nome;
	string cognome;
	cout << utente;
	stringstream ss(utente);
	getline(ss, nome, ' ');
	getline(ss, cognome, '\0');
	int corrispondenze = 0;
	for (auto it = utenti.begin(); it != utenti.end(); ++it) {
		if (nome == it->second.get_nome()) {
			if (cognome == it->second.get_cognome()) {
				corrispondenze++;
				v.push_back(&(it->second));
			}
		}
	}

	m.unlock();
	return v;
}
/*
*	Questo metodo aggiunge entry al file black list che definisce quali
*	MAC address bloccare
*/
void Sync_mappa::blocco_utente(string MAC) {

	lock_guard<mutex> lk(m_blacklist);
	auto bloccato = this->utenti.find(MAC);
	Utente u = bloccato->second;
	string MAC1 = bloccato->first;
	ofstream f;
	f.open("black_list.txt");
	f << MAC1.append("\n").c_str();
	f.close();
}

//Funzione inutilizzata (credo) vedere se si può eliminare
void Sync_mappa::blocco(string utente) {

	string MAC;
	string nome;
	string cognome;
	int corrispondenze = 0;
	stringstream ss(utente);
	getline(ss, nome, ' ');
	getline(ss, cognome, ' ');
	vector<string> v;
	string str;
	m.lock();
	for (auto it = utenti.begin(); it != utenti.end(); ++it) {
		if (!nome.compare(it->second.get_nome())) {
			if (!cognome.compare(it->second.get_cognome())) {
				corrispondenze++;
				str.assign(it->first).append(" ").append(it->second.get_nome()).append(" ").append(it->second.get_cognome()).append(" ");
				v.push_back(str);
			}
		}
	}
	if (corrispondenze > 1) {
		cout << "Multiple occurrences found: choose which one block " << endl;
		for (auto it = v.begin(); it != v.end(); it++) {
			cout << it->back() << endl;
		}
		int choose;
		cin >> choose;
		string temp = v.at(choose);
		stringstream ss(temp);
		getline(ss, MAC, ' ');
		blocco_utente(MAC);
	}
	else if (corrispondenze <= 0) {
		cout << "ERROR: user not found" << endl;
	}
	else {
		string temp = v.back();
		stringstream ss(temp);
		getline(ss, MAC, ' ');
		blocco_utente(MAC);
	}
	m.unlock();

	return;
}

/*
* metodo di eliminazione di un utente dalla black list
*/
void Sync_mappa::sblocco_utente(string MAC) {

	lock_guard<mutex> lk(m_blacklist);
	fstream f;
	char* str = new char[MAC.size()];
	f.open("black_list.txt");
	while (f.good()) {
		f.getline(str, 20);
		if (!strcmp(str, MAC.c_str())) {
			long pos = f.tellp();
			f.seekp(pos - MAC.size());
			f.write("", 0);
		}
	}
	f.close();
}

/*
Controllo se nella black list è presente un certo MAC address
*/
bool Sync_mappa::check_identity(string MAC) {
	fstream f;
	lock_guard<mutex> lk(m_blacklist);
	f.open("black_list.txt");
	if (f.is_open()) {
		char* temp = new char[MAC.size()];
		while (f.good()) {
			f.getline(temp, 20);
			if (!MAC.compare(temp)) // MAC bloccato
				return true;
		}
	}
	
	return false;
}


//Funzione che ritorna la lista di utenti connessi in lista di stringhe
vector<char*>* Sync_mappa::getUtenti() {
	vector<char*>* listaUtenti = new vector<char*>();
	map<string, Utente>::iterator it;

	m.lock();

	for (it = this->utenti.begin(); it != this->utenti.end(); ++it) {
		string tmp;
		string bloccato;

		bool blocked = this->check_identity(it->first);
		if (blocked == false)
			bloccato = "libero";
		else
			bloccato = "bloccato";

		tmp.assign(it->first).append("-").append(it->second.get_nome()).append("-").
			append(it->second.get_cognome()).append("-").
			append(*(it->second.get_fotopathPointer())).append("-").
			append(bloccato).append("-");

		char *cstr = new char[tmp.length() + 1];
		strcpy(cstr, tmp.c_str());

		listaUtenti->push_back(cstr);

	}

	m.unlock();

	return listaUtenti;
}
