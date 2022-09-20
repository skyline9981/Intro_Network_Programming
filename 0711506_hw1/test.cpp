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

class USER{
public:
	string username="";
	string passwd="";
	bool online;
	vector<int> number;
	USER(string s1,string s2){
		username=s1;
		passwd=s2;
		online=false;
	}
};

class Mesbox{
public:
	string sender="";
	string recver="";
	string message="";
	bool isrecv;
	Mesbox(string s1,string s2,string s3){
		sender=s1;
		recver=s2;
		message=s3;
		isrecv=false;
	}
};

bool cmp(USER a,USER b){
	return (a.username<b.username);
}

bool paircmp(pair<string,int> a,pair<string,int> b){
	return a.first<b.first;
}

int main(int argc,char *argv[]){

	stringstream stream_port;
	int port_num;
	stream_port<<argv[1];
	stream_port>>port_num;
	

	char inputBuffer[10000] = {};
	
	vector<USER> vec;
	vector<Mesbox> mail;

	int tcp_sockfd=0,udp_sockfd=0,maxfdp,nready;
	int new_socket,client_socket[32],max_client=32,activity,valread,sd;
	
	fd_set readfds;
	
	if((tcp_sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
		cout<<"tcp socket fail"<<endl;
	}

	for(int i=0;i<max_client;++i){
		client_socket[i]=0;
	}

	struct sockaddr_in serverInfo,clientInfo;
	int addrlen = sizeof(serverInfo);
	bzero(&serverInfo,sizeof(serverInfo));
	bzero(&clientInfo,sizeof(clientInfo));

	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverInfo.sin_port = htons(port_num);

	bind(tcp_sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
	listen(tcp_sockfd,32);
	puts("Waiting for connections ...");

	FD_ZERO(&readfds);
	maxfdp=max(tcp_sockfd,udp_sockfd);

	while(1){
		FD_SET(tcp_sockfd, &readfds); 
		
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
			const char s[] = "********************************\n** Welcome to the BBS  server.**\n********************************\n%";
			for(int i=0;i<max_client;++i){
				if(client_socket[i]==0){
					client_socket[i]=new_socket;
					cout<<"Adding to list of sockets as "<<i<<endl;
					sd = new_socket;
					getpeername(sd,(struct sockaddr*)&serverInfo,(socklen_t*)&addrlen);
					send(sd,(const char *)s,sizeof(s),0);
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
						bool canlogin = true;
						string s="Bye, ";
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								s+=vec[i].username;
								s+=".";
								vec[i].online = false;
								canlogin=false;
								break;
							}
						}
						if(!canlogin){
							send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
						}
						close(sd);
					}else if(temp=="logout"){
						bool canlogin = true;
						string s="Bye, ";
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								s+=vec[i].username+".";
								vec[i].online = false;
								canlogin = false;
								break;
							}
						}
						const char fail[]="Please login first.\n%";
						if(canlogin){
							send(sd,fail,sizeof(fail),0);
						}else{
							s+="\n%";
							send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
						}
						
					}else if(temp=="list-user"){
						string s="";
						sort(vec.begin(),vec.end(),cmp);
						for(int i=0;i<vec.size();++i){
							if(i==vec.size()-1){
								s+=(vec[i].username);
							}else{
								s+=(vec[i].username+"\n");
							}
						}
						s+="\n%";
						send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
					}else if(temp=="register"){
						const char usage[]="Usage: register <username> <password>\n%";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=2){
							cout<<"Usage: register <username> <password>\n%";
							send(sd,(const char*)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1,temp2;
		    			ss1>>temp1>>temp2;
		    			const char success[]="Register successfully.\n%";
    					const char fail[]="Username is already used.\n%";
		    			bool rep=false;
		    			for(int i=0;i<vec.size();++i){
		    				if(vec[i].username==temp1){
		    					rep=true;
		    					break;
		    				}
		    			}
		    			if(rep){
		    				send(sd,(const char*)fail,sizeof(fail),0);
		    			}else{
		    				USER U(temp1,temp2);
		    				vec.push_back(U);
		    				send(sd,(const char*)success,sizeof(success),0);
		    			}

					}else if(temp=="whoami"){
						bool canlogin = true;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								canlogin = false;
								break;
							}
						}
		    			const char fail[]="Please login first.\n%";
		    			if(canlogin){
		    				send(sd,(const char*)fail,sizeof(fail),0);
		    			}else{
			    			for(int i=0;i<vec.size();++i){
			    				if(vec[i].online){
			    					string s=vec[i].username;
			    					s+="\n%";
			    					send(sd,(const char*)s.c_str(),strlen(s.c_str()),0);
			    					break;
			    				}
			    			}
							
		    			}
					}
					else if(temp=="send"){
						const char usage[]="Usage: send <username> <message>\n%";
						const char fail1[]="Please login first.\n%";
						const char fail2[]="User not existed.\n%";
						int c=0,valid=0,valid1=0,valid2=0;
						string tmp,se,re;
						while(ss>>tmp){
							c++;
							if(c>=2){
								valid=1;
								break;
							}
						}
						if(!valid){
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								se=vec[i].username;
								valid1=1;
								break;
							}
						}
						if(!valid1){
							send(sd,(const char *)fail1,sizeof(fail1),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1;
						ss1>>temp1;
						for(int i=0;i<vec.size();++i){
							if(vec[i].username==temp1){
								re=vec[i].username;
								valid2=1;
								break;
							}
						}
						if(!valid2){
							send(sd,(const char *)fail2,sizeof(fail2),0);
							continue;
						}
						string inmessage;
						getline(ss1,tmp,'"');
						getline(ss1,inmessage,'"');
						Mesbox mes(se,re,inmessage);
						mail.push_back(mes);
						const char ccc[] = "%";
						send(sd,(const char *)ccc,strlen(ccc),0);
					}else if(temp=="list-msg"){
						const char fail1[]="Please login first.\n%";
						const char fail2[]="Your message box is empty.\n%";
						int valid1=0;
						string myself;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								myself=vec[i].username;
								valid1=1;
								break;
							}
						}
						if(!valid1){
							send(sd,(const char *)fail1,strlen(fail1),0);
							continue;
						}
						vector< pair<string,int> > numofmes;
						for(int i=0;i<mail.size();++i){
							if(!mail[i].isrecv && mail[i].recver==myself){
								int invec=0;
								for(int j=0;j<numofmes.size();++j){
									if(numofmes[j].first==mail[i].sender){
										invec=1;
										numofmes[j].second++;
										break;
									}
								}
								if(!invec){
									numofmes.push_back(make_pair(mail[i].sender,1));
								}
							}
						}
						if(numofmes.empty()){
							send(sd,(const char *)fail2,strlen(fail2),0);
							continue;
						}
						sort(numofmes.begin(),numofmes.end(),paircmp);
						string outmes="";
						for(int i=0;i<numofmes.size();++i){
							if(i==numofmes.size()-1){
								outmes+=to_string(numofmes[i].second);
								outmes+=" message from ";
								outmes+=numofmes[i].first;
								outmes+=".\n%";
							}else{
								outmes+=to_string(numofmes[i].second);
								outmes+=" message from ";
								outmes+=numofmes[i].first;
								outmes+=".\n";
							}
						}
						send(sd,(const char *)outmes.c_str(),strlen(outmes.c_str()),0);
					}else if(temp=="receive"){
						const char usage[]="Usage: receive <username>\n%";
						const char fail1[]="User not existed.\n%";
						const char fail2[]="%";
						int c=0,valid1=0,valid2=0;
						string tmp,myself;
						while(ss>>tmp){
							c++;
						}
						if(c!=1){
							send(sd,(const char *)usage,strlen(usage),0);
							continue;
						}
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								myself=vec[i].username;
								break;
							}
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1;
						ss1>>temp1;
						for(int i=0;i<vec.size();++i){
							if(vec[i].username==temp1){
								valid1=1;
								break;
							}
						}
						if(!valid1){
							send(sd,(const char *)fail1,sizeof(fail1),0);
							continue;
						}
						string outmes="";
						for(int i=0;i<mail.size();++i){
							if(!mail[i].isrecv){
								if(mail[i].sender==temp1 && mail[i].recver==myself){
									outmes=mail[i].message;
									mail[i].isrecv=1;
									valid2=1;
									break;
								}
							}
						}
						if(!valid2){
							send(sd,(const char *)fail2,strlen(fail2),0);
							continue;
						}
						outmes+="\n%";
						send(sd,(const char *)outmes.c_str(),strlen(outmes.c_str()),0);
					}else if(temp=="login"){ //login
						const char usage[]="Usage: login <username> <password>\n%";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=2){
							cout<<"Usage: login <username> <password>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1,temp2;
						ss1>>temp1>>temp2;
						bool ok=false;
						const char fail1[]="Please logout first.\n%";
						const char fail2[]="Login failed.\n%";
						bool canlogin = true;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online){
								canlogin = false;
								cout<<vec[i].username<<'\n';
								break;
							}
						}
						if(!canlogin){
							send(sd,fail1,sizeof(fail1),0);
						}else{
							for(int i=0;i<vec.size();++i){
								if(vec[i].username==temp1&&vec[i].passwd==temp2){
									string s="Welcome, ";
									s+=temp1;
									s+=".\n%";
									const char* sendbuffer = s.c_str();
									send(sd,(const char *)sendbuffer,strlen(sendbuffer),0);
									vec[i].online=true;
									ok=true;
									break;
								}
							}
							if(!ok){
								send(sd,fail2,sizeof(fail2),0);
							}
						}
						
					}
				}
			}	
		}
	}
	return 0;
}