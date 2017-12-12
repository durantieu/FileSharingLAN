#pragma once
#include "Sync_mappa.h"
//#include "Sync_Utente.h"
#include "TCP_Listener.h"
#include "TCP_Client.h"

#define PORTA_IN_ASCOLTO 5200
#define BUF_LENGTH 10000

namespace connNmSpace {

	class Connessione {
	private:
		unique_ptr<Utente> utente_attivo;
		Sync_mappa *sync_utenti;
		thread *discoverer;
		thread *listener;
		thread *updater;
		thread *TCP_listener;
		TCP_Listener* TCPList;
		string *sharedpath;
		string *sharedFotoPath;
		mutex *mutSharedPath;
		static condition_variable cvar;
		static mutex mut;
	public:
		Connessione();
		bool start();
		void connect();
		static void discoverer_function(Connessione *conn);
		static void listener_function(Sync_mappa *m1);
		static void updater_function(Sync_mappa *m1);
		static string getMACaddress(void);
		static void printMAC();
		Utente* choose_user();
		void file_transfer(string path, SOCKADDR_IN ip_utente);
		void be_invisible();
		void be_visible();
		void change_visibility(bool vs);
		void change_filepath(string path);
		void change_immagine_profilo(string foto_path);
		void blocca_utente(string utente);
		void sblocco_utente(string utente);
		bool getVisibilityUtenteAttivo();
		list<string> getUtentiConnessi();
	};
	
	class ConnWrapper {
	public:
		static __declspec(dllexport) Connessione* creaConnessione();
		static __declspec(dllexport) void modPrivata(Connessione* conn);
		static __declspec(dllexport) list<string> getUtentiConnessi(Connessione* conn);
	};
}

/*
Per fare il login dell'utente si potrebbe fare che il codice in C# effettua
login ed eventuale registrazione, e poi manda i dati al codice in C++
La gestione sarebbe semplificata e non ci sarebbe bisogno di comunicazione
strana tra i due linguaggi
*/

/*
*Non ho utilizzato una struct di stringhe per semplificare il 
marshalling dei dati; la stringa nella lista degli utenti avrà
la forma nome-cognome-path (o sostituendo - con $ o altre porcate)
*/
