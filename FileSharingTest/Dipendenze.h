#pragma once
#define _WINSOCKAPI_  
#include <fstream>
#include <iostream>
#include <thread>
#include <Windows.h>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <winsock.h>
#include <mutex>
#include <ws2tcpip.h>
#include <condition_variable>
#include <future>
#include "mswsock.h"
#include <experimental/filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <deque>
#include <conio.h>
#include <tchar.h>

#define TCP_LISTENER_PORT 50000
#define TCP_SERVER_PORT 10000
using namespace std;