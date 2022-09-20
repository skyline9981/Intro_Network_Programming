#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<bits/stdc++.h>

using namespace std;

int tcp_sockfd=0;
int udp_sockfd=0;
struct sockaddr_in Info;
int len;
int num=0;

void send_register(string message,stringstream& ss);
void send_login(string message,stringstream& ss);
void send_logout(string message);
void send_whoami(string message);
void send_listuser(string message);
void send_exit(string message);



int main(int argc,char *argv[]){
	
	string ip = argv[1];
	string s = argv[2];
	stringstream ss;
	ss<<s;
	int port_num;
	ss>>port_num;

	
	tcp_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(tcp_sockfd==-1){
		cout<<"Fail to make TCP socket."<<endl;
	}
	
	udp_sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(udp_sockfd==-1){
		cout<<"Fail to make UDP socket."<<endl;
	}

	len=sizeof(Info);	
	bzero(&Info,sizeof(Info));
	Info.sin_family = AF_INET;	
	Info.sin_addr.s_addr = inet_addr((const char *)ip.c_str());
	Info.sin_port = htons(port_num);

	int err = connect(tcp_sockfd,(struct sockaddr *)&Info,sizeof(Info));
	if(err==-1){
		cout<<"Connection error"<<endl;
		return 0;
	}
	
	char recv_buffer[1000];	
	recvfrom(tcp_sockfd,(char *)recv_buffer,sizeof(recv_buffer),MSG_WAITALL,(struct sockaddr *)&Info,(socklen_t *)&len);
	puts(recv_buffer);

	//cout<<"********************************\n** Welcome to the BBS  server.**\n********************************"<<endl;
	while(1){
		cout<<"% ";
		string message;
       	getline(cin,message);
       	stringstream ss(message);
       	string temp;
       	ss>>temp;
       	if(temp == "register"){ //UDP
       		send_register(message,ss);
       	}else if(temp == "login"){ //TCP
       		send_login(message,ss);
       	}else if(temp == "logout"){ //TCP
       		send_logout(message);
       	}else if(temp == "whoami"){ //UDP
       		send_whoami(message);
       	}else if(temp == "list-user"){ //TCP
       		send_listuser(message);
       	}else if(temp == "exit"){ //TCP
       		send_exit(message);
       		break;
       	}else{
       		cout<<"command not found."<<endl;
       	}
	}


return 0;

}

void send_register(string message,stringstream& ss){
	char send_buffer[1000];
	strcpy(send_buffer,(char *)message.c_str());
	int c=0;
	string temp;
	while(ss>>temp){
		c++;
	}
	if(c!=3){
		cout<<"Usage: register <username> <email> <password>"<<endl;
		return ;
	}
	sendto(udp_sockfd,(char *)send_buffer,sizeof(send_buffer),MSG_CONFIRM,(struct sockaddr *)&Info,len);
	
	char recv_buffer[1000];	
	recvfrom(udp_sockfd,(char *)recv_buffer,sizeof(recv_buffer),MSG_WAITALL,(struct sockaddr *)&Info,(socklen_t *)&len);
	puts(recv_buffer);
}

void send_login(string message,stringstream& ss){
	char send_buffer[1000];
	
	int c=0;
	string temp;
	while(ss>>temp){
		c++;
	}
	if(c!=2){
		cout<<"Usage: login <username> <password>"<<endl;
		return ;
	}
	string s=message+" "+to_string(num);
	strcpy(send_buffer,(char *)s.c_str());
	send(tcp_sockfd,send_buffer,sizeof(send_buffer),0);
	
	char recv_buffer[1000];
	recv(tcp_sockfd,recv_buffer,sizeof(recv_buffer),0);
	if(recv_buffer[0]=='W'){ //Welcome
		stringstream ss2(recv_buffer);
		string w;
		ss2>>w;
		string name;
		ss2>>name;
		cout<<w<<" "<<name<<"."<<endl;
		ss2>>num;
	}else{ //Failed
		puts(recv_buffer);	
	}
}

void send_logout(string message){
	char send_buffer[1000];
	string temp=message+" "+to_string(num);
	strcpy(send_buffer, (char*)temp.c_str());
	send(tcp_sockfd,send_buffer,sizeof(send_buffer),0);
	
	char recv_buffer[1000];
	recv(tcp_sockfd,recv_buffer,sizeof(recv_buffer),0);
	puts(recv_buffer);
	num=0;
}

void send_whoami(string message){
	char send_buffer[1000];
	string temp=message+" "+to_string(num);
	strcpy(send_buffer,(char*)temp.c_str());
	sendto(udp_sockfd,(char *)send_buffer,sizeof(send_buffer),MSG_CONFIRM,(struct sockaddr *)&Info,len);
	
	char recv_buffer[1000];	
	recvfrom(udp_sockfd,(char *)recv_buffer,sizeof(recv_buffer),MSG_WAITALL,(struct sockaddr *)&Info,(socklen_t *)&len);
	puts(recv_buffer);
}

void send_listuser(string message){
	char send_buffer[1000];
	strcpy(send_buffer,(char *)message.c_str());
	send(tcp_sockfd,send_buffer,sizeof(send_buffer),0);
	
	char recv_buffer[1000];
	recv(tcp_sockfd,recv_buffer,sizeof(recv_buffer),0);
	puts(recv_buffer);
}

void send_exit(string message){
	char send_buffer[1000];
	string temp=message+" "+to_string(num);
	strcpy(send_buffer,(char *)temp.c_str());
	send(tcp_sockfd,send_buffer,sizeof(send_buffer),0);
}
