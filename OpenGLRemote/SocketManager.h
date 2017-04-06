#pragma once


#include <boost/lexical_cast.hpp>

#include "server_ws.cpp"
#include <windows.h>
#include <tchar.h>
#include <iostream>  
#include <fstream>    

#include <algorithm>
#include <minmax.h>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <stdio.h>
using namespace Gdiplus;
using namespace std;
#pragma comment (lib,"Gdiplus.lib")
#include "DataPackage.h"

using namespace std;

typedef SimpleWeb::SocketServer<SimpleWeb::WS> WsServer;

class SocketManager
{
public:
	SocketManager(DataPackage* d) {
		data = d;
	};

	void StartServer();

private:

	std::vector<std::string> split(const std::string &s, char delim);
	DataPackage *data;
};

