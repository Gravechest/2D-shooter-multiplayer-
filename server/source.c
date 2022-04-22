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

unsigned char *map;

typedef struct{
	float x;
	float y;

	float vx;
	float vy;

	float deltax;
	float deltay;

	float sidex;
	float sidey;

	int stepx;
	int stepy;

	int side;

	int ix;
	int iy;
}RAY;

typedef struct{
	char input[8];
	unsigned short r[8];
	unsigned short cooldown[8];
	float x[8];
	float y[8];
	float vx[8];
	float vy[8];
}PLAYER;

typedef struct{
	unsigned char c;
	unsigned short p1x[16];
	unsigned short p1y[16];
	unsigned short p2x[16];
	unsigned short p2y[16];
	unsigned short tm[16];
}BULLET;

BULLET bullet;
PLAYER player;


RAY rayCreate(float x,float y,float rx,float ry){
	RAY ray;
	ray.x  		= x;
	ray.y  		= y;
	ray.vx		= rx;
	ray.vy 		= ry;
	
	ray.deltax  = fabsf(1.0/ray.vx);
	ray.deltay  = fabsf(1.0/ray.vy);

	if(ray.vx < 0.0){
		ray.stepx = -1;
		ray.sidex = (ray.x-(int)ray.x) * ray.deltax;
	}
	else{
		ray.stepx = 1;
		ray.sidex = ((int)ray.x + 1.0 - ray.x) * ray.deltax;
	}
	if(ray.vy < 0.0){
		ray.stepy = -1;
		ray.sidey = (ray.y-(int)ray.y) * ray.deltay;
	}
	else{
		ray.stepy = 1;
		ray.sidey = ((int)ray.y + 1.0 - ray.y) * ray.deltay;
	}
	ray.ix = ray.x;
	ray.iy = ray.y;
	return ray;
}

void rayItterate(RAY *ray){
    if(ray->sidex < ray->sidey){
		ray->ix += ray->stepx;
		ray->sidex += ray->deltax;
		ray->side = 0;
    }
    else{
		ray->iy += ray->stepy;
		ray->sidey += ray->deltay;
		ray->side = 1;
    }
}

float distance(float x1,float y1,float x2,float y2){
	float nx = x1-x2;
	float ny = y1-y2;
	return sqrtf(nx*nx+ny*ny);
}

void physics(){
	for(;;){
		int i2 = 0;
		for(int i = 0;i < bullet.c;i2++){
			if(bullet.tm[i2]){
				bullet.tm[i2]--;
				if(!bullet.tm[i2]){
					bullet.c--;
				}
				i++;
			}
		}
		i2 = 0;
		for(int i = 0;i < clientC && i2 < 8;i2++){
			if(player.x[i2] && player.y[i2]){
				if(player.input[i2] & 0x01){
					player.vx[i2] += 0.35f;
				}
				if(player.input[i2] & 0x02){
					player.vx[i2] -= 0.35f;
				}
				if(player.input[i2] & 0x04){
					player.vy[i2] += 0.35f;
				}
				if(player.input[i2] & 0x08){
					player.vy[i2] -= 0.35f;
				}
				if(!player.cooldown[i2]){
					if(player.input[i2] & 0x10){
						player.cooldown[i2] = 5;
						for(int i3 = 0;;i3++){
							if(!bullet.tm[i3]){
								bullet.p1x[i3] = player.x[i2] + cosf((float)player.r[i2]*0.0001f+3.14f)*70.0f;
								bullet.p1y[i3] = player.y[i2] + sinf((float)player.r[i2]*0.0001f+3.14f)*70.0f;
								RAY ray = rayCreate(player.x[i2],player.y[i2],cosf((float)player.r[i2]*0.0001f+3.14f),sinf((float)player.r[i2]*0.0001f+3.14f));
								for(;;){
									rayItterate(&ray);
									if(ray.ix==0||ray.iy==0||ray.ix==0xffff||ray.iy==0xffff){
										bullet.p2x[i3] = ray.ix;
										bullet.p2y[i3] = ray.iy;
										break;
									}
								}
								bullet.tm[i3] = 3;
								bullet.c++;
								break;
							}	
						}
					}
				}
				else{
					player.cooldown[i2]--;
				}
				player.x[i2] += player.vx[i2];
				player.y[i2] += player.vy[i2];
				player.vx[i2] /= 1.04f;
				player.vy[i2] /= 1.04f;
				i++;
			}
		}
		Sleep(15);
	}
}

void communication(unsigned char *ID){
	unsigned char id = *ID;
	clientdata[id][0] = id;
	send(client[id],clientdata[id],1,0);
	player.x[id] = 100;
	player.y[id] = 100;
	send(client[id],map,65536*4,0);
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
		player.r[id] = clientdata[id][1];
		player.r[id] +=clientdata[id][2]<<8;
		clientdata[id][0] = clientC;
		int i2 = 0;
		for(int i = 0;i < clientC && i2 < 8;i2++){
			if(player.x[i2] && player.y[i2]){
				clientdata[id][i*9+1] = (int)player.x[i2];
				clientdata[id][i*9+2] = (int)player.x[i2]>>8;
				clientdata[id][i*9+3] = (int)player.y[i2];
				clientdata[id][i*9+4] = (int)player.y[i2]>>8;
				clientdata[id][i*9+5] = (int)player.vx[i2];
				clientdata[id][i*9+6] = (int)player.vy[i2];
				clientdata[id][i*9+7] = player.r[i2];
				clientdata[id][i*9+8] = player.r[i2]>>8;
				clientdata[id][i*9+9] = i2;
				i++;
			}
		}
		int pP = clientC*9+1;
		clientdata[id][pP] = bullet.c;
		i2 = 0;
		for(int i = 0;i < bullet.c && i2 < 16;i2++){
			if(bullet.tm[i2]){
				clientdata[id][i*8+pP+1] = bullet.p1x[i2];
				clientdata[id][i*8+pP+2] = bullet.p1x[i2]>>8;
				clientdata[id][i*8+pP+3] = bullet.p1y[i2];
				clientdata[id][i*8+pP+4] = bullet.p1y[i2]>>8;
				clientdata[id][i*8+pP+5] = bullet.p2x[i2];
				clientdata[id][i*8+pP+6] = bullet.p2x[i2]>>8;
				clientdata[id][i*8+pP+7] = bullet.p2y[i2];
				clientdata[id][i*8+pP+8] = bullet.p2y[i2]>>8;
				i++;
			}
		}
		pP += bullet.c*8+1;
		rs = send(client[id],clientdata[id],pP,0);
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
	map = HeapAlloc(GetProcessHeap(),8,65536*4);
	for(int i = 0;i < 65536*4;i+=8){
		map[i] = 1;
		if(i % (256*4) > 253*4){
			i+=4;
		}
	}
	timeBeginPeriod(1);
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
