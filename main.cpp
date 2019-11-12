//============================================================================
// Name        : thread.cpp
// Author      : Samet Yildiz
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <random>
#include <unistd.h>
#include <vector>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/ip.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <array>
using namespace std;

struct token
{
	uint8_t ch[65536];

};

vector<token> buffer;
vector<int> packetSize;
vector<uint8_t> consume_buffer;


sem_t full, empty;
pthread_mutex_t mutex;

void* produce(void* arg){
	int i = 0;
	int packet_size;
	int sock = socket (PF_INET, SOCK_RAW, IPPROTO_UDP);
	if(sock == -1)
	{
		//socket creation failed, may be because of non-root privileges
		perror("Failed to create socket");
		exit(1);
	}
	while(1){
		usleep(1);
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		token samet;
		packet_size = recvfrom(sock , samet.ch , 65536 , 0 , NULL, NULL);
		if (packet_size == -1) {
			printf("Failed to get packets\n");
		}
		buffer.push_back(samet);
		packetSize.push_back(packet_size);
		cout << "pushed - " << packet_size  << endl;
		i++;
		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}
}

void* consume(void* arg){
	struct sockaddr_in source_socket_address, dest_socket_address;
	while(1){
		usleep(1);
		sem_wait(&full);
		pthread_mutex_lock(&mutex);
		token samet;
		samet = buffer.front();
		buffer.erase(buffer.begin());
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
		struct iphdr *ip_packet = (struct iphdr *)samet.ch;

		memset(&source_socket_address, 0, sizeof(source_socket_address));
		source_socket_address.sin_addr.s_addr = ip_packet->saddr;
		memset(&dest_socket_address, 0, sizeof(dest_socket_address));
		dest_socket_address.sin_addr.s_addr = ip_packet->daddr;
		consume_buffer.push_back((uint8_t)ntohs(ip_packet->id));


		if(!strcmp((char *)inet_ntoa(source_socket_address.sin_addr), "127.0.0.1")){
		        printf("Incoming Packet: \n");
		        printf("Packet Size (bytes): %d\n",ntohs(ip_packet->tot_len));
		        printf("Source Address: %s\n", (char *)inet_ntoa(source_socket_address.sin_addr));
		        printf("Destination Address: %s\n", (char *)inet_ntoa(dest_socket_address.sin_addr));
		        printf("Identification: %d\n\n", ntohs(ip_packet->id));
		        for (int i = 0; i < packetSize.front(); i++) {
		          printf("0x%.2X-", samet.ch[i]);
		        }
		        packetSize.erase(packetSize.begin());
		        printf("\n");
		      }

		cout << "consumed - "  << consume_buffer[consume_buffer.size()]  << endl;
		if(consume_buffer.size()==65535){
			consume_buffer.clear();
		}
	}
}

int main(){
	pthread_t producer, consumer;
	sem_init(&empty, 0, 65535);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&producer, NULL, produce, NULL);

	pthread_create(&consumer, NULL, consume, NULL);
	pthread_exit(NULL);
}
