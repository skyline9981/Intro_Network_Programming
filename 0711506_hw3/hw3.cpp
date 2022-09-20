#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<bits/stdc++.h>
#include<sys/time.h>


using namespace std;

string chat_record = "";

bool has_filted=false;

vector<pair<string, string> >banword = {{"how","***"}, {"you","***"}, {"or","**"}, {"pek0","****"}, {"tea","***"},
										{"ha","**"}, {"kon","***"}, {"pain","****"}, {"Starburst Stream","****************"}};

string ReplaceString(string subject, const string& search, const string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
		has_filted = true;
    }
    return subject;
}

static const string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
			ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i){
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while((i++ < 3))
			ret += '=';

	}
	return ret;
}

string base64_decode(string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	string ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
			char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
			ret += char_array_3[i];
			i = 0;
		}
	}

	if(i){
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

  return ret;
}

struct a {
    unsigned char flag;
    unsigned char version;
    unsigned char payload[0];
} __attribute__((packed));

struct b {
    unsigned short len;
    unsigned char data[0];
} __attribute__((packed));


bool isDeg(string str){
	for(int i = 0; i < str.size(); i++){
		if(str.at(i) == '-' && str.size()>1)continue;
		if(str.at(i) > '9' && str.at(i) < '0')return false;
	}
	return true;
}

class USER{
public:
	string username="";
	string passwd="";
	int online = -1;
	int chat_port = -1;
	int chat_version = -1;
	bool banned = false;
	int message_rule = 0;
	vector<int> number;
	USER(string s1,string s2){
		username=s1;
		passwd=s2;
		online=-1;
	}
};

class info{
public:
	struct sockaddr_in client;
	int version = 0;
};

bool cmp(USER a,USER b){
	return a.username<b.username;
}

bool paircmp(pair<string,int> a,pair<string,int> b){
	return a.first<b.first;
}

int main(int argc,char *argv[]){

	stringstream stream_port;
	int port_num;
	int opt = 1;
	stream_port<<argv[1];
	stream_port>>port_num;
	

	char inputBuffer[10000] = {};
	
	vector<USER> vec;
	vector<info> allclient;

	int tcp_sockfd=0,udp_sockfd=0,maxfdp,nready;
	int new_socket,client_socket[32],max_client=32,activity,valread,sd;
	
	fd_set readfds;
	
	if((tcp_sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
		cout<<"tcp socket fail"<<endl;
	}

	if(setsockopt(tcp_sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&opt, sizeof(opt)) < 0){
		cout << "Setsockopt error." << endl;
	}

	for(int i=0;i<max_client;++i){
		client_socket[i]=0;
	}

	struct sockaddr_in serverInfo,clientInfo;
	int addrlen = sizeof(serverInfo);
	bzero(&serverInfo,sizeof(serverInfo));
	bzero(&clientInfo,sizeof(clientInfo));

	serverInfo.sin_family = AF_INET;
	//serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverInfo.sin_port = htons(port_num);

	bind(tcp_sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
	listen(tcp_sockfd, 32);

	if((udp_sockfd = socket(AF_INET,SOCK_DGRAM,0))<0){
		cout<<"udp socket fail"<<endl;
		return 0;
	}

	if(bind(udp_sockfd,(const struct sockaddr *)&serverInfo,sizeof(serverInfo))<0){
		cout<<"udp bind failed"<<endl;
		return 0;
	}

	puts("Waiting for connections ...");

	FD_ZERO(&readfds);
	//maxfdp=max(tcp_sockfd,udp_sockfd);
	maxfdp = tcp_sockfd;

	while(1){
		FD_SET(tcp_sockfd, &readfds); 
		FD_SET(udp_sockfd, &readfds);

		for(int i=0;i<max_client;++i){
			sd=client_socket[i];
			if(sd>0){
				FD_SET(sd,&readfds);
			}
			if(sd>maxfdp){
				maxfdp=sd;
			}
		}

		nready = select(maxfdp+1, &readfds, NULL, NULL, NULL);
		
		if(nready<0){
			cout<<"select error!"<<endl;
		}

		//TCP
		if(FD_ISSET(tcp_sockfd,&readfds)){
			new_socket = accept(tcp_sockfd,(struct sockaddr*)&serverInfo,(socklen_t*)&addrlen);
			cout<<"New connection."<<endl;
			const char s[] = "********************************\n** Welcome to the BBS server. **\n********************************\n% ";
			for(int i=0;i<max_client;++i){
				if(client_socket[i]==0){
					client_socket[i]=new_socket;
					cout<<"Adding to list of sockets as "<<i<<endl;
					sd = new_socket;
					cout << "sd: " << sd<<endl;
					getpeername(sd,(struct sockaddr*)&serverInfo,(socklen_t*)&addrlen);
					send(sd,(const char *)s,strlen(s),0);
					break;
				}
			}
		}

		for(int i=0;i<max_client;++i){
			sd = client_socket[i];


			if(FD_ISSET(sd,&readfds)){
				if((valread=read(sd,inputBuffer,sizeof(inputBuffer)))==0){
					getpeername(sd,(struct sockaddr*)&serverInfo,(socklen_t*)&addrlen);
					cout<<"Host disconnected, ip "<<inet_ntoa(serverInfo.sin_addr)<<", port "<<ntohs(serverInfo.sin_port)<<endl;
					client_socket[i]=0;
				}else{
					inputBuffer[valread]='\0';
					cout<<"Get from TCP socket "<<sd<<". Message: "<<inputBuffer<<endl;
					
					
					stringstream ss(inputBuffer);
					string temp;
					ss>>temp;
					if(temp=="exit"){
						if(ss >> temp){
							const char usage[]="Usage: exit\n";
							send(sd,(const char *)usage,strlen(usage),0);
						}else{
							bool canlogin = true;
							string s="Bye, ";
							for(int i=0;i<vec.size();++i){
								if(vec[i].online == sd){
									s+=vec[i].username;
									s+=".\n";
									vec[i].online = -1;
									canlogin=false;
									break;
								}
							}
							if(!canlogin){
								send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
							}
							close(sd);
							FD_CLR(sd,&readfds);
							client_socket[i]=0;
						}
						
						
					}else if(temp=="logout"){
						if(ss >> temp){
							const char usage[]="Usage: logout\n";
							send(sd,(const char *)usage,strlen(usage),0);
						}else{
							bool canlogin = true;
							string s="Bye, ";
							for(int i=0;i<vec.size();++i){
								if(vec[i].online == sd){
									s+=vec[i].username+".";
									vec[i].online = -1;
									vec[i].chat_port = -1;
									canlogin = false;
									break;
								}
							}
							const char fail[]="Please login first.\n";
							if(canlogin){
								send(sd,fail,strlen(fail),0);
							}else{
								s+="\n";
								send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
							}
						}
						
						
					}else if(temp=="register"){
						const char usage[]="Usage: register <username> <password>\n";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=2){
							cout<<"Usage: register <username> <password>\n";
							send(sd,(const char*)usage,strlen(usage),0);
							//continue;
						}else{
							stringstream ss1(inputBuffer);
							ss1>>tmp;
							string temp1,temp2;
							ss1>>temp1>>temp2;
							const char success[]="Register successfully.\n";
							const char fail[]="Username is already used.\n";
							bool rep=false;
							for(int i=0;i<vec.size();++i){
								if(vec[i].username==temp1){
									rep=true;
									break;
								}
							}
							if(rep){
								send(sd,(const char*)fail,strlen(fail),0);
							}else{
								USER U(temp1,temp2);
								vec.push_back(U);
								send(sd,(const char*)success,strlen(success),0);
							}
						}
						

					}else if(temp=="login"){ //login
						const char usage[]="Usage: login <username> <password>\n";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=2){
							cout<<"Usage: login <username> <password>\n";
							send(sd,(const char *)usage,strlen(usage),0);
							//continue;
						}else{
							stringstream ss1(inputBuffer);
							ss1>>tmp;
							string temp1,temp2;
							ss1>>temp1>>temp2;
							bool ok=false;
							const char fail1[]="Please logout first.\n";
							const char fail2[]="Login failed.\n";
							bool canlogin = true;
							bool ban = false;
							for(int i=0;i<vec.size();++i){
								if(vec[i].online == sd){
									canlogin = false;
									break;
								}
								if(vec[i].username == temp1 && vec[i].online !=-1){
									canlogin = false;
									break;
								}
							}
							if(!canlogin){
								send(sd,fail1,strlen(fail1),0);
							}else{
								for(int i=0;i<vec.size();++i){
									if(vec[i].username==temp1&&vec[i].passwd==temp2){
										if(vec[i].banned){
											string getout = "We don't welcome ";
											getout+=vec[i].username;
											getout+="!\n";
											const char* sendbuffer = getout.c_str();
											send(sd,(const char *)sendbuffer,strlen(sendbuffer),0);
											ok=true;
											break;
										}else{
											string s="Welcome, ";
											s+=temp1;
											s+=".\n";
											const char* sendbuffer = s.c_str();
											send(sd,(const char *)sendbuffer,strlen(sendbuffer),0);
											cout << sd << endl;
											vec[i].online = sd;
											ok=true;
											break;
										}
										
									}
								}
								if(!ok){
									send(sd,fail2,strlen(fail2),0);
								}
							}
							
						}
					}else if(temp == "enter-chat-room"){
						const char usage[]="Usage: enter-chat-room <port> <version>\n";
						string fail1="Port ";
						string fail2="Version ";
						const char fail3[]="Please login first.\n";
						string success="Welcome to public chat room.\nPort:";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=2){
							cout<<"Usage: login <username> <password>\n";
							send(sd,(const char *)usage,strlen(usage),0);
							//continue;
						}else{
							stringstream ss1(inputBuffer);
							ss1>>tmp;
							string temp1,temp2;
							ss1>>temp1>>temp2;
							bool canlogin = true;
							bool vaild1 = isDeg(temp1);
							bool vaild2 = isDeg(temp2);
							int udp_port=-1,chat_ver=0;
							if(vaild1){
								udp_port = atoi(temp1.c_str());
								cout << temp1 << " " << udp_port<<endl;
								if(udp_port<1 || udp_port > 65535){
									vaild1 = false;
								}
							}
							if(vaild2){
								chat_ver = atoi(temp2.c_str());
								if(chat_ver != 1 && chat_ver != 2){
									vaild2 = false;
								}
							}
							for(int i=0;i<vec.size();++i){
								if(vec[i].online == sd){
									canlogin = false;
									break;
								}
							}
							if(!vaild1){
								fail1 += temp1;
								fail1 += " is not valid.\n";
								send(sd,(const char *)fail1.c_str(),strlen(fail1.c_str()),0);
							}else if(!vaild2){
								fail2 += temp2;
								fail2 += " is not supported.\n";
								send(sd,(const char *)fail2.c_str(),strlen(fail2.c_str()),0);
							}else if(canlogin){
								send(sd,(const char *)fail3,strlen(fail3),0);
							}else{
								success += to_string(udp_port);
								success += "\nVersion:";
								success += to_string(chat_ver);
								success += "\n";
								success += chat_record;
								cout << success.c_str() << endl;
								for(int i=0;i<vec.size();++i){
									if(vec[i].online == sd){
										vec[i].chat_port = udp_port;
										vec[i].chat_version = chat_ver;
									}
								}
								send(sd,(const char *)success.c_str(),strlen(success.c_str()),0);
								info tmp_info;
								tmp_info.version = chat_ver;
								tmp_info.client = serverInfo;
								tmp_info.client.sin_port = htons(udp_port);
								allclient.push_back(tmp_info);
							}
						}
					}
					// else{
					// 	const char fail1[]="\n";
					// 	send(sd,fail1,strlen(fail1),0);
					// }
					const char some[]="% ";
					send(sd,some,strlen(some),0);
				}
			}	
		}

		//UDP
		if (FD_ISSET(udp_sockfd, &readfds)) { 
			int len = sizeof(clientInfo); 
			bzero(inputBuffer, sizeof(inputBuffer)); 
			printf("Get from UDP: \n"); 
			recvfrom(udp_sockfd,inputBuffer,sizeof(inputBuffer),0,(struct sockaddr*)&clientInfo,(socklen_t *)&len); 
			cout << inputBuffer << endl;
			cout << "flag: " << int(inputBuffer[0]) << endl;
			cout << "version: " << int(inputBuffer[1]) << endl;
			if(int(inputBuffer[0])!=1)continue;
			if(int(inputBuffer[1])!=1 && int(inputBuffer[1])!=2)continue;
			string name="";
			string msg="";
			int get_ver = 0;
			for(int i=0;i<allclient.size();i++){
				if(allclient[i].client.sin_port == clientInfo.sin_port){
					get_ver = allclient[i].version;
					break;
				}
			}
			
			if(get_ver == 2){
				int idx=0;
				for(int j=2;j<strlen(inputBuffer);j++){
					if(inputBuffer[j]!='\n'){
						name+=inputBuffer[j];
					}else{
						idx = j;
						break;
					}
				}
				for(int k=idx+1;k<strlen(inputBuffer);k++){
					if(inputBuffer[k]!='\n'){
						msg+=inputBuffer[k];
					}else{
						break;
					}
				}
				name = base64_decode(name);
				msg = base64_decode(msg);
			}else{
				uint16_t namelen= (inputBuffer[3] | uint16_t(inputBuffer[2]) << 8);
				cout << "namelen: " << namelen << endl;
				int idx=0;
				for(int j=4;j<=4+namelen;j++){
					name += inputBuffer[j];
					idx=j;
				}
				uint16_t msglen= (inputBuffer[idx+1] | uint16_t(inputBuffer[idx]) << 8);
				for(int j=idx+2;j<=idx+2+msglen;j++){
					msg += inputBuffer[j];
				}
			}

			for(int j=0;j<banword.size();j++){
				msg = ReplaceString(msg,banword[j].first,banword[j].second);
			}
			cout << name << ":" << msg << endl;
			if(get_ver==1){
				name.pop_back();
				msg.pop_back();
			}
			chat_record += name;
			chat_record += ":";
			chat_record += msg;
			chat_record += "\n";

			if(has_filted){
				cout << "fuck!!!!!!!!!!!!!!" << endl;
				for(int j=0;j<vec.size();j++){
					if(htons(vec[j].chat_port) == clientInfo.sin_port){
						vec[j].message_rule++;
						cout  << "times: " << vec[j].message_rule << endl;
						if(vec[j].message_rule==3){
							vec[j].banned=true;
						}
						break;
					}
				}
			}
			has_filted = false;
			
			string pkt2_name = base64_encode(reinterpret_cast<const unsigned char*>(name.c_str()), name.length());
			string pkt2_msg = base64_encode(reinterpret_cast<const unsigned char*>(msg.c_str()), msg.length());
			cout << pkt2_name << " " << pkt2_msg << endl;
			char packet2[10000];
			sprintf(packet2, "\x01\x02%s\n%s\n", pkt2_name.c_str(), pkt2_msg.c_str());
			//size_t payload2_size = 4 + strlen(pkt2_name.c_str()) + 
			
			uint16_t name_len = (uint16_t)strlen(name.c_str());
			cout << name_len << endl;
			uint16_t mesg_len = (uint16_t)strlen(msg.c_str());
			cout << mesg_len << endl;

			unsigned char packet1[4096];
			bzero(packet1, sizeof(packet1));
			struct a *pa = (struct a*) packet1;
			struct b *pb1 = (struct b*) (packet1 + sizeof(struct a));
			struct b *pb2 = (struct b*) (packet1 + sizeof(struct a) + sizeof(struct b) + name_len);
			pa->flag = 0x01;
			pa->version = 0x01;
			pb1->len = htons(name_len);
			memcpy(pb1->data, name.c_str(), name_len);
			pb2->len = htons(mesg_len);
			memcpy(pb2->data, msg.c_str(), mesg_len);
			
			size_t payload_size = 6+name_len+mesg_len;
			int banned_port=0;
			for(int j=0;j<allclient.size();j++){
				int clilen = sizeof(allclient[j].client);
				bool has_ban = false;
				for(int k=0;k<vec.size();k++){
					if(htons(vec[k].chat_port) == clientInfo.sin_port){
						if(vec[k].banned){
							has_ban = true;
							banned_port = vec[k].chat_port;
						}
						break;
					}
				}
				if(!has_ban){
					if(allclient[j].version == 1){
						sendto(udp_sockfd, packet1, payload_size, 0, (struct sockaddr*)&allclient[j].client, sizeof(allclient[j].client));
					}else{
						sendto(udp_sockfd, packet2, strlen(packet2), 0, (struct sockaddr*)&allclient[j].client, sizeof(allclient[j].client));
					}
				}else{
					if(allclient[j].client.sin_port != htons(banned_port)){
						if(allclient[j].version == 1){
							sendto(udp_sockfd, packet1, payload_size, 0, (struct sockaddr*)&allclient[j].client, sizeof(allclient[j].client));
						}else{
							sendto(udp_sockfd, packet2, strlen(packet2), 0, (struct sockaddr*)&allclient[j].client, sizeof(allclient[j].client));
						}
					}
				}
			}
		}
		for(int i=0;i<max_client;++i){
			sd = client_socket[i];
			for(int j=0;j<vec.size();j++){
				string str = "Bye, ";
				if(vec[j].banned && vec[j].online==sd){
					str+=vec[j].username;
					str+=".\n% ";
					send(sd,(const char*)str.c_str(),strlen(str.c_str()),0);
					vec[j].online = -1;
					vec[j].chat_port = -1;
				}
			}
		}
	}
	return 0;
}