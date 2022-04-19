#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <intrin.h>

WSADATA data;
SOCKET sock;
SOCKADDR_IN adres;

unsigned char clientC;
SOCKET client[8];
unsigned char clientdata[8][1024];

const unsigned int sendTimeOut = 300;
const unsigned int recvTimeOut = 300;

typedef struct{
	char input[8];
	unsigned int rx[8];
	unsigned int ry[8];
	float x[8];
	float y[8];
	float vx[8];
	float vy[8];
}PLAYER;

PLAYER player;

void physics(){
	for(;;){
		int i2 = 0;
		for(int i = 0;i < clientC && i2 < 8;i2++){
			if(player.x[i2] && player.y[i2]){
				if(player.input[i] & 0x01){
					player.vx[i] += 0.35f;
				}
				if(player.input[i] & 0x02){
					player.vx[i] -= 0.35f;
				}
				if(player.input[i] & 0x04){
					player.vy[i] += 0.35f;
				}
				if(player.input[i] & 0x08){
					player.vy[i] -= 0.35f;
				}
				player.x[i] += player.vx[i];
				player.y[i] += player.vy[i];
				player.vx[i] /= 1.04f;
				player.vy[i] /= 1.04f;
			}
		}
		Sleep(15);
	}
}

void socketErrors(unsigned char id){

}

void communication(unsigned char *ID){
	unsigned char id = *ID;
	clientdata[id][0] = id;
	unsigned int pr = send(client[id],clientdata[id],1024,0);
	player.x[id] = 100;
	player.y[id] = 100;
	printf("clientID:%i initialized\n",id);
	for(;;){
		int rs = recv(client[id],clientdata[id],1024,0);
		if(rs == -1 || rs == 0){
			if(WSAGetLastError() == WSAECONNRESET){
				printf("clientID:%i disconnected\n",id);
				closesocket(client[id]);
				player.x[id] = 0;
				player.y[id] = 0;
				player.vx[id] = 0;
				player.vy[id] = 0;
				player.input[id] = 0;
				clientC--;
				return;
			}	
		}
		player.input[id] = clientdata[id][0];
		clientdata[id][0] = clientC;
		int i2 = 0;
		for(int i = 0;i < clientC && i2 < 8;i2++){
			if(player.x[i2] && player.y[i2]){
				clientdata[id][i*5+1] = (int)player.x[i];
				clientdata[id][i*5+2] = (int)player.x[i]>>8;
				clientdata[id][i*5+3] = (int)player.y[i];
				clientdata[id][i*5+4] = (int)player.y[i]>>8;
				clientdata[id][i*5+5] = i;
				i++;
			}
		}
		rs = send(client[id],clientdata[id],clientC*5+1,0);
		if(rs == -1 || rs == 0){
			if(WSAGetLastError() == WSAECONNRESET){
				printf("clientID:%i disconnected\n",id);
				closesocket(client[id]);
				player.x[id] = 0;
				player.y[id] = 0;
				player.vx[id] = 0;
				player.vy[id] = 0;
				player.input[id] = 0;
				clientC--;
				return;
			}
			
		}
	}
}

unsigned char searchServerSlot(){
	unsigned char r = 0;
	while(player.x[r] && player.y[r]){
		r++;
	}
	return r;
}

void main(){
	CreateThread(0,0,physics,0,0,0);
	WORD ver = MAKEWORD(2, 2);
	WSAStartup(ver, &data);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	adres.sin_family = AF_INET;
	adres.sin_port = htons(7778);
	adres.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(sock,(SOCKADDR*)&adres,sizeof(adres));
	for(;;){
		listen(sock,SOMAXCONN);
		SOCKET temp = accept(sock,0,0);
		unsigned char id = searchServerSlot();
		client[id] = temp;
		clientC++;
		CreateThread(0,0,communication,&id,0,0);
	}
}
