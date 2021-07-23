#include "stdafx.h"
#include "ChatServer.h"
#include "Resource.h"
#include"Thread.h"
ChatServer::ChatServer() {
	cout << "begin chat with TCP\n";
	_IsConected = false;
	WSADATA wsaData;

	sockaddr_in local;

	int wsaret = WSAStartup(0x101, &wsaData);

	if (wsaret != 0)
	{
		return;
	}
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons((u_short)8080);
	_socListenClient = socket(AF_INET, SOCK_STREAM, 0);
	if (_socListenClient == INVALID_SOCKET)
	{
		return;
	}


	if (bind(_socListenClient, (sockaddr*)&local, sizeof(local)) != 0)
	{
		return;
	}


	if (listen(_socListenClient, 10) != 0)
	{
		return;
	}

	_IsConected = true;
	return;
}
ChatServer::~ChatServer()
{
	closesocket(_socListenClient);

	WSACleanup();

	if (_clientList.size() != 0)
	{
		for (auto client : _clientList)
		{
			delete client;
		}
	}
}
bool ChatServer::isConnected()
{
	return _IsConected;
}
void ChatServer::startListenClient()
{

	sockaddr_in from;
	int fromlen = sizeof(from);

	_socClient = accept(_socListenClient,
		(struct sockaddr*)&from, &fromlen);
	auto packet = new ClientPacket;
	packet->socket = _socClient;

	if (_socClient != INVALID_SOCKET)
	{
		SendMessage(_hwnd, WM_COMMAND, ID_USER_CONNECT, 0);
		_clientList.push_back(packet);

	}

	AfxBeginThread(recServerThread, (void*)_socClient);

}
int ChatServer::sendMessageClient(ClientPacket* client, WCHAR* message, int len)
{
	int iStat = 0;

	iStat = send(client->socket, (char*)message, len * 2 + 2, 0);
	if (iStat == -1)
		_clientList.remove(client);
	if (iStat == -1)
		return 1;

	return 0;

}
int ChatServer::recClient(SOCKET recSocket)
{
	WCHAR* message;
	WCHAR temp[4096];
	int iStat;
	int len;
	iStat = recv(recSocket, (char*)temp, 4096, 0);
	list<ClientPacket*>::iterator itl;
	for (itl = _clientList.begin(); itl != _clientList.end(); itl++)
	{
		if ((*itl)->socket == recSocket)
		{
			break;
		}
	}
	message = temp;
	switch (message[0])
	{
	case MessageType::SIGNUP:
	{

		WCHAR* username;
		WCHAR* password;

		username = message + 1;
		password = message + wcslen(message) + 1;

		auto user = new User;
		user->username = username;
		user->password = password;
		int result = signUp(user);
		if (result == true)
		{
			message[0] = MessageType::SU_SUCCESS;
			SendMessage(_hwnd, WM_COMMAND, ID_RESTORE, 0);
		}
		else
		{
			message[0] = MessageType::SU_FAILURE;
		}
		sendMessageClient((*itl), (WCHAR*)message, 1);
		break;
	}
	case MessageType::LOGIN:
	{


		WCHAR* username;
		WCHAR* password;
		username = message + 1;
		password = message + wcslen(message) + 1;
		User user;
		user.username = username;
		user.password = password;
		int result = logIn(recSocket, user);
		if (result == true)
		{
			message[0] = MessageType::LI_SUCCESS;
		}
		else
		{
			message[0] = MessageType::LI_FAILURE;
		}

		sendMessageClient((*itl), (WCHAR*)message, 1);
		break;
	}
	case MessageType::SF_ACCEPT:
	case MessageType::SF_CANCEL:
	{

		WCHAR* partner;
		partner = message + wcslen(message) + 4;
		partner += wcslen(partner) + 1;
		//buffer[len] = NULL;
		for (auto client : _clientList)
		{
			if (wcscmp(client->username.c_str(), partner) == 0)
			{
				sendMessageClient(client, (WCHAR*)message, iStat / 2);
				break;
			}
		}
		break;
	}
	case MessageType::STOP:
	case MessageType::CONTINUE:
	{
		/*
		* receive:	message = [FLAG | receiver | NULL | sender | NULL]
		* send:		message = [FLAG | receiver | NULL | sender | NULL]
		*/
		WCHAR* receiver = message + 1;
		for (auto client : _clientList)
		{
			if (wcscmp(client->username.c_str(), receiver) == 0)
			{
				sendMessageClient(client, (WCHAR*)message, iStat / 2);
				break;
			}
		}

		break;
	}
	}

	return 0;
}
void ChatServer::setHWND(HWND hwnd)
{
	_hwnd = hwnd;
}

bool ChatServer::signUp(User* user)
{
	for (auto userdata : _userData)
	{
		if (user->username == userdata->username)
		{
			return false;
		}
	}
	_userData.push_back(user);
	return true;
}

bool ChatServer::logIn(SOCKET socket, User user)
{
	for (auto userdata : _userData)
	{
		if (user.username == userdata->username && user.password == userdata->password)
		{
			for (auto client : _clientList)
			{
				if (client->socket == socket)
				{
					client->username = user.username;
					return true;
				}
			}
		}
	}
	return false;
}
void ChatServer::addUser(User* user)
{
	_userData.push_back(user);
}

list<User*>& ChatServer::getUser()
{
	return _userData;
}


//
