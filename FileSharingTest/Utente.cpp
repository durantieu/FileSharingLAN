#include "Utente.h"

void Utente::set_nome(string nome) {
	lock_guard<mutex> lg(*mtx);
	this->nome = nome;
}

void Utente::set_cognome(string cognome) {
	lock_guard<mutex> lg(*mtx);
	this->cognome = cognome;
}

void Utente::set_filepath(string path) {
	lock_guard<mutex> lg(*mtx);
	this->filepath = path;
}

void Utente::set_ip(SOCKADDR_IN ip_addr) {
	lock_guard<mutex> lg(*mtx);
	this->ip = ip_addr;
}

void Utente::set_percorso_foto(string percorso) {
	lock_guard<mutex> lg(*mtx);
	this->percorso_foto = percorso;
}

SOCKADDR_IN Utente::get_ip() {
	lock_guard<mutex> lg(*mtx);
	return this->ip;
}

string Utente::get_percorso_foto() {
	lock_guard<mutex> lg(*mtx);
	return this->percorso_foto;
}

string Utente::get_nome() {
	lock_guard<mutex> lg(*mtx);
	return this->nome;
}

string Utente::get_cognome() {
	lock_guard<mutex> lg(*mtx);
	return this->cognome;
}

void Utente::set_time_stamp(time_t time) {
	lock_guard<mutex> lg(*mtx);
	this->timestamp = time;
}

time_t Utente::get_time_stamp() {
	lock_guard<mutex> lg(*mtx);
	return this->timestamp;
}

string Utente::get_filepath() {
	lock_guard<mutex> lg(*mtx);
	return filepath;
}

string* Utente::get_filepathPointer() {
	lock_guard<mutex> lg(*mtx);
	return &this->filepath;
}

string* Utente::get_fotopathPointer() {
	lock_guard<mutex> lg(*mtx);
	return &this->percorso_foto;
}

void Utente::set_visibility(bool vs) {
	lock_guard<mutex> lg(*mtx);
	this->visible = vs;
}

bool Utente::get_visibility() {
	lock_guard<mutex> lg(*mtx);
	return this->visible;
}

/*
*La funzione sign_in ricerca il file "credenziali.txt" dove sono salvati i dati di login
*dell'utente; se il file non è presente, significa che non esiste nessun utente,
*quindi viene chiamata la funzione sign_up che si occupa di
*acquisire i dati dell'utente tramite la funzione acquisisci dati
*e salvarli all'interno del file "credenziali.txt".
*Dopo aver trovato il file "credenziali.txt" (o averlo creato), si caricano i dati
*nell'oggetto 
*/
Utente* Utente::apri_utente(string dati, string path) {
	Utente* utente = new Utente();
	stringstream Str(dati);
	string dato;
	int i = 0;
		
	utente->credPath = path;

	while (getline(Str, dato, '|')) {
		switch (i) {
		case 0: {
			utente->set_nome(dato);
			break;
		}
		case 1: {
			utente->set_cognome(dato);
			break;
		}
		case 2: {
			utente->set_percorso_foto(dato);
			break;
		}
		case 3: {
			utente->set_filepath(dato);
			break;
		}
		case 4: {
			if (dato == "true")
				utente->set_visibility(true);
			else
				utente->set_visibility(false);
		}
		default:
			break;
		}

		i++;
	}

	utente->set_time_stamp(0);
	
	return utente;
}

/*
*Funzione di lettura da file dei dati dell'utente e di acquisizione nella struttura dati
*/
bool Utente::sign_in() {

	ifstream readFile;
	string input, first, second;
	readFile.open(credPath);

	if (readFile.is_open()) {
		while (!readFile.eof()) {
			readFile >> input;
			stringstream input_stringstream(input);
			if (getline(input_stringstream, first, ':')) {
				getline(input_stringstream, second);
				if (first == "nome") {
					this->set_nome(second);
					continue;
				}
				if (first == "cognome") {
					this->set_cognome(second);
					continue;
				}
				if (first == "path") {
					this->set_percorso_foto(second);
				}
				if (first == "filepath") {
					this->set_filepath(second);
				}
				if (first == "visible") {
					if (!second.compare("true"))
						this->set_visibility(true);
					else
						this->set_visibility(false);
				}
			}
		}
		readFile.close();
		this->set_time_stamp(0); //da rivedere il valore da assegnare all'inizio
		return TRUE;
	}
	else {
		readFile.close();
		return FALSE;
	}
}

/*
*Funzione di acquisizione dei dati dall'utente e di scrittura su file
*/
bool Utente::sign_up() {
	string buf;
	ofstream writeFile;
	bool ok = FALSE;

	writeFile.open(credPath);

	if (!writeFile.is_open())
		return FALSE;

	while (!ok) {
		cout << "Inserisci il tuo nome" << endl;
		cin >> buf;

		if (buf.find(":") != -1) {
			cout << "Carattere \":\" non consentito" << endl;
			continue;
		}
		writeFile << "nome:" << buf << endl;
		ok = TRUE;
	}

	ok = FALSE;

	while (!ok) {
		cout << "Inserisci il tuo cognome" << endl;
		cin >> buf;

		if (buf.find(":") != -1) {
			cout << "Carattere \":\" non consentito" << endl;
			continue;
		}
		writeFile << "cognome:" << buf << endl;
		ok = TRUE;
	}

	ok = FALSE;

	while (!ok) {
		string com, path;
		cout << "Inserire foto?" << endl;
		cin >> com;

		if (com == "no") {
			ok = TRUE;
			set_percorso_foto("0");
			writeFile << "path:0" << endl;

		}
		if (com == "si") {
			cout << "Percorso:" << endl;
			cin >> path;

			set_percorso_foto(path);
			writeFile << "path:" << path << endl;
			ok = TRUE;
		}
	}

	ok = false;

	while (!ok) {
		string path;
		cout << "Inserire il nuovo path:" << endl;
		cin >> path;
		set_filepath(path);
		writeFile << "filepath:" << path << endl;
		ok = true;
	}

	ok = false;

	while (!ok) {
		string com;
		cout << "Vuoi essere visibile?" << endl;
		cin >> com;

		if (com == "no") {
			ok = TRUE;
			set_visibility(false);
			writeFile << "visible:false";

		}
		if (com == "si") {
			ok = TRUE;
			set_visibility(true);
			writeFile << "visible:true";
		}
	}

	writeFile.close();
	return TRUE;

}