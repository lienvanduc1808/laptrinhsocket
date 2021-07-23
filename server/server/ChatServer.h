#pragma once

#define _AFXDLL
#include "Definition.h"
class ChatServer {
public:
	ChatServer();
	~ChatServer();
	bool isConnected();
	void startListenClient();
	int sendMessageClient(ClientPacket* client, WCHAR* message, int len);
	int recClient(SOCKET recSocket);
	void setHWND(HWND hwnd);
	bool signUp(User* user);
	bool logIn(SOCKET socket, User user);
	void addUser(User* user);
	list<User*>& getUser();//
private:
	bool _IsConected;
	SOCKET _socListenClient;
	list<ClientPacket*> _clientList;
	SOCKET _socClient;
	HWND _hwnd;
	list<User*> _userData;
};
