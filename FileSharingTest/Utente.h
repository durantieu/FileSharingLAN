#pragma once
#include "Dipendenze.h"

class Utente {
private:
	string nome;
	string cognome;
	string credPath;
	time_t timestamp;
	SOCKADDR_IN ip;
	mutex* mtx;
protected:
	string percorso_foto; //0 = lui non ha caricato foto ; 1 = ha la foto ma da scaricare ; 2=foto da cambiare; other -> ho la foto di quell'utente
	bool visible;
	string filepath;
public:
	Utente() { mtx = new mutex(); };
	string get_nome();
	string get_cognome();
	time_t get_time_stamp();
	string get_filepath();
	string* get_filepathPointer();
	string* get_fotopathPointer();
	SOCKADDR_IN get_ip();
	virtual string get_percorso_foto();
	void set_nome(string nome);
	void set_cognome(string cognome);
	void set_time_stamp(time_t time);
	void set_filepath(string path);
	void set_ip(SOCKADDR_IN ip_addr);
	void set_visibility(bool vis);
	bool get_visibility();
	void set_percorso_foto(string percorso);
	static Utente* apri_utente(string dati, string path);
	bool sign_in();
	bool sign_up();
};
