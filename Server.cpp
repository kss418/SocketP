#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#pragma comment(lib,"ws2_32.lib")

const short PORT = 9000;
const int BUFSIZE = 512;
std::vector <SOCKET> part;

// 오류 시 출력, 종료 O
void err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char *)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// 오류 시 출력, 종료 X
void err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

void Server_Thread(SOCKET client_sock) {
	int ret; char buf[BUFSIZE], ID[BUFSIZE];
	while (1) {
		ret = recv(client_sock, ID, BUFSIZE, 0);
		if (ret == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (!ret) break;
		ID[ret] = '\0';

		ret = recv(client_sock, buf, BUFSIZE, 0);
		if (ret == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (!ret) break;
		buf[ret] = '\0';

		std::cout << buf << "\n";

		for (auto& other : part) {
			if (other == client_sock) continue;
			ret = send(other, ID, strlen(ID), 0);
			ret = send(other, buf, strlen(buf), 0);
			if (ret == SOCKET_ERROR) continue;
		}
	}

	closesocket(client_sock);
	return;
}

int b(SOCKET* server_sock) {
	sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET; // IPv4
	inet_pton(AF_INET, "10.21.20.36", &server_addr.sin_addr); // 주소 변환 후 삽입
	server_addr.sin_port = htons(PORT); // 바이트 정렬 변환
	return bind(*server_sock, (sockaddr*)&server_addr, sizeof(server_addr));
}

int main() {
	int ret;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// 소켓 생성
	SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock == INVALID_SOCKET) err_quit("socket()");

	// bind
	if(b(&server_sock) == SOCKET_ERROR) err_quit("bind()");

	// listen
	ret = listen(server_sock, SOMAXCONN);
	if (ret == SOCKET_ERROR) err_quit("listen()");

	//accept
	SOCKET client_sock;
	class sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);

	while (1) {
		client_sock = accept(server_sock, (sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()"); break;
		}
		part.push_back(client_sock);
		std::thread th(Server_Thread, client_sock);
		th.detach();
	}
	closesocket(server_sock);
	WSACleanup(); return 0;
}