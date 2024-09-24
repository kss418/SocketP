#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#pragma comment(lib,"ws2_32.lib")

const short PORT = 9000;
const int BUFSIZE = 512;

// 오류 시 출력, 종료 O
void err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
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

void recv_Thread(SOCKET server_sock) {
	int ret; char buf[BUFSIZE];
	while (1) {
		ret = recv(server_sock, buf, BUFSIZE, 0);
		if (ret == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (!ret) break;
		for (int i = 0; i < ret - 1; i++) std::cout << buf[i];
		std::cout << " : ";

		ret = recv(server_sock, buf, BUFSIZE, 0);
		if (ret == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (!ret) break;
		buf[ret] = '\0';

		std::cout << buf << "\n";
	}
	return;
}

int main() {
	int ret; char ID[BUFSIZE];
	
	std::cout << "ID �Է� : ";
	if (fgets(ID, BUFSIZE + 1, stdin) == NULL) return 0;
	int ID_len = (int)strlen(ID);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// ���� ����
	SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock == INVALID_SOCKET) err_quit("socket()");

	// connect
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // IPv4
	inet_pton(AF_INET, "10.21.20.36", &server_addr.sin_addr); // �ּ� ��ȯ �� ����
	server_addr.sin_port = htons(PORT); // ����Ʈ ���� ��ȯ
	ret = connect(server_sock, (sockaddr*)&server_addr, sizeof(server_addr));
	if (ret == SOCKET_ERROR) err_quit("connect()");

	std::thread hrecv(recv_Thread, server_sock);
	hrecv.detach();

	char buf[BUFSIZE + 1];
	int len;
	while (1) {
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n') buf[len - 1] = '\0';

		ret = send(server_sock, ID, ID_len, 0);
		if (ret == SOCKET_ERROR) {
			err_display("send()"); break;
		}

		ret = send(server_sock, buf, len, 0);
		if (ret == SOCKET_ERROR) {
			err_display("send()"); break;
		}
	}

	closesocket(server_sock);
	WSACleanup(); return 0;
}