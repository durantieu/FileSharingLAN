#pragma once

#include "Utente.h"
#include "TCP_Client.h"

#define EXPIRED 10

class Sync_mappa {
private:
	map<string, Utente> utenti;
	mutex m, m_blacklist;
	string MAC;
public:
	Sync_mappa(string MAC) {
		this->MAC = MAC;
	};
	//metodi di lettura e scrittura sincronizzati con il mutex
	int cerca_e_inserisci(string chiave, string nome, string cognome, time_t timeStamp, string flag_foto, SOCKADDR_IN ip);
	int scorri_e_aggiorna();
	void scarica_foto(SOCKADDR_IN ip);
	void stampa_utenti();
	vector<Utente*> estrai_utente(string utente);
	string get_nome(string MAC);
	void blocco_utente(string MAC); //funzioni per la black list
	void blocco(string utente); //funzioni per la black list
	void sblocco_utente(string MAC); //funzione black list
	bool check_identity(string MAC); //funzione black list
	list<string> getUtenti();
};
