#pragma once

#include "Dipendenze.h"

class Porta {
private:
	int numero;
	string tipo_file;
	promise<bool> pExit;
	promise<bool> pStarted;
public:
	Porta(int port, promise<bool>& prom, promise<bool>& promE) {
		numero = port;
		pStarted = move(prom);
		pExit = move(promE);
	};
	string get_numero() {
		return to_string(numero);
	};
	bool getPromiseExit() {
		return pExit.get_future().get();
	}
	future<bool> getPromiseStarted() {
		return pStarted.get_future();
	}
	void setPromiseExit() {
		this->pExit.set_value(true);
	}
	void setPromiseStarted() {
		this->pStarted.set_value(true);
	}
};