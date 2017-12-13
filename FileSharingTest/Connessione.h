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
		Utente* choose_user(string MAC);
		void file_transfer(string path, SOCKADDR_IN ip_utente);
		void be_invisible();
		void be_visible();
		void change_visibility(bool vs);
		void change_filepath(string path);
		void change_immagine_profilo(string foto_path);
		void blocco_utente(string MAC);
		bool getVisibilityUtenteAttivo();
		list<string> getUtentiConnessi();
	};
	
	class ConnWrapper {
	public:
		static Connessione* creaConnessione();
		static void modPrivata(Connessione* conn);
		static list<string> getUtentiConnessi(Connessione* conn);
		static void putInBlackList(Connessione* conn, string MAC);
		static void inviaFile(Connessione* conn, string file, string MAC);
		static void cambiaFilePath(Connessione* conn, string path);
		static void cambiaImmagine(Connessione* conn, string path);
	};
}


extern "C" __declspec(dllexport) connNmSpace::Connessione* creaConnessione();
extern "C" __declspec(dllexport) list<string>* getUtentiConnessi(connNmSpace::Connessione* conn);
extern "C" __declspec(dllexport) void modPrivata(connNmSpace::Connessione* conn);
extern "C" __declspec(dllexport) void putInBlackList(connNmSpace::Connessione* conn, string MAC);
extern "C" __declspec(dllexport) void inviaFile(connNmSpace::Connessione* conn, string file, string MAC);
extern "C" __declspec(dllexport) void cambiaFilePath(connNmSpace::Connessione* conn, string path);
extern "C" __declspec(dllexport) void cambiaImmagine(connNmSpace::Connessione* conn, string path);