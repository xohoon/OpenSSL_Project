#include <iostream>
#include <winsock.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

void init() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
}

void close() {
	ERR_free_strings();
	EVP_cleanup();
	WSACleanup();
}

int main() {
	init();

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddress;
	int addressLength = sizeof(serverAddress);
	
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(2959);

	bind(sockfd, (struct sockaddr*) & serverAddress, addressLength);

	listen(sockfd, 10);

	/* SSL ��ü �ʱ�ȭ */
	SSL_CTX* sslContext = SSL_CTX_new(SSLv23_server_method());
	SSL_CTX_set_options(sslContext, SSL_OP_SINGLE_DH_USE);

	/* ����Ű�� ����Ű �ʱ�ȭ */
	SSL_CTX_use_certificate_file(sslContext, "./cert.pem", SSL_FILETYPE_PEM);
	SSL_CTX_use_PrivateKey_file(sslContext, "./key.pem", SSL_FILETYPE_PEM);

	while (true) {
		int fd = accept(sockfd, (struct sockaddr*) & serverAddress, &addressLength);

		/* SSL ��� ó�� */
		SSL* ssl = SSL_new(sslContext);
		SSL_set_fd(ssl, fd);
		SSL_accept(ssl);

		/* SSL �Է� */
		char input[4096] = { 0 };
		SSL_read(ssl, (char*)input, 4096);
		
		/* SSL ��� */
		char output[4096] = { 0 };
		int length = wsprintfA(output, "[Echo]: %s\n", input);
		SSL_write(ssl, output, length);

		SSL_free(ssl);
		closesocket(fd);
	}
	SSL_CTX_free(sslContext);

	close();
	return 0;

}