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

int SN=1;

class USER{
public:
	string username="";
	string passwd="";
	int online = -1;
	vector<int> number;
	USER(string s1,string s2){
		username=s1;
		passwd=s2;
		online=-1;
	}
};

class Post{
public:
	int my_sn;
    string title;
    string author;
    string date;
    string content;
	string in_which_board;
	vector<string> comment;
	bool deleted = false;
	Post(string s1,string s2,string s3,string s4,string s5){
		title=s1;
		author=s2;
		date=s3;
		content=s4;
		in_which_board=s5;
	}
};

class Board{
public:
	int index=1;
    string name;
    string moderator;
	Board(int idx,string s1,string s2){
		index=idx;
		name=s1;
		moderator=s2;
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
	

	char inputBuffer[100000] = {};
	
	vector<USER> vec;
	vector<Board> all_board;
	vector<Post> all_posts;

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
			const char s[] = "********************************\n** Welcome to the BBS  server.**\n********************************\n% ";
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
					cout<<SN<<endl;
					stringstream ss(inputBuffer);
					string temp;
					ss>>temp;
					if(temp=="exit"){
						bool canlogin = true;
						string s="Bye, ";
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
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
						//close(sd);
						shutdown(sd,SHUT_RDWR);
					}else if(temp=="logout"){
						bool canlogin = true;
						string s="Bye, ";
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
								s+=vec[i].username+".";
								vec[i].online = -1;
								canlogin = false;
								break;
							}
						}
						const char fail[]="Please login first.\n%";
						if(canlogin){
							send(sd,fail,sizeof(fail),0);
						}else{
							s+="\n% ";
							send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
						}
						
					}else if(temp=="register"){
						const char usage[]="Usage: register <username> <password>\n% ";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=2){
							cout<<"Usage: register <username> <password>\n% ";
							send(sd,(const char*)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1,temp2;
		    			ss1>>temp1>>temp2;
		    			const char success[]="Register successfully.\n% ";
    					const char fail[]="Username is already used.\n% ";
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

					}else if(temp=="login"){ //login
						const char usage[]="Usage: login <username> <password>\n% ";
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
						const char fail1[]="Please logout first.\n% ";
						const char fail2[]="Login failed.\n% ";
						bool canlogin = true;
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
							send(sd,fail1,sizeof(fail1),0);
						}else{
							for(int i=0;i<vec.size();++i){
								if(vec[i].username==temp1&&vec[i].passwd==temp2){
									string s="Welcome, ";
									s+=temp1;
									s+=".\n% ";
									const char* sendbuffer = s.c_str();
									send(sd,(const char *)sendbuffer,strlen(sendbuffer),0);
									cout << sd << endl;
									vec[i].online = sd;
									ok=true;
									break;
								}
							}
							if(!ok){
								send(sd,fail2,sizeof(fail2),0);
							}
						}
						
					}else if(temp == "create-board"){
						const char usage[]="Usage: create-board <name>\n% ";
						const char fail1[]="Please login first.\n% ";
						const char fail2[]="Board already exists.\n% ";
						const char success[]="Create board successfully.\n% ";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=1){
							cout<<"Usage: create-board <name>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1;
						ss1>>temp1;
						bool canlogin = true;
						string moder_name;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
								canlogin = false;
								moder_name = vec[i].username;
								break;
							}
						}
						if(canlogin){
							send(sd,fail1,sizeof(fail1),0);
						}else{
							bool board_exist = false;
							for(int i=0;i<all_board.size();++i){
								if(all_board[i].name == temp1){
									board_exist = true;
								}
							}
							if(board_exist){
								send(sd,fail2,sizeof(fail2),0);
							}else{
								int idx=all_board.size()+1;
								Board new_board(idx,temp1,moder_name);
								all_board.push_back(new_board);
								send(sd,success,sizeof(success),0);
							}
						}
					}
					else if(temp == "create-post"){
						const char usage[]="Usage: create-post <board-name> --title <title> --content <content>\n% ";
						const char fail1[]="Please login first.\n% ";
						const char fail2[]="Board does not exist.\n% ";
						const char success[]="Create post successfully.\n% ";
						bool c= false,c1=false,c2=false,c3=false;
						string tmp;
						ss>>tmp;
						while(ss>>tmp){
							if(tmp=="--title"){
								c=true;
								c1=false;
							}else if(tmp=="--content"){
								c1= true;
								c=false;
							}else{
								if(c){
									c2=true;
								}else if(c1){
									c3=true;
								}
							}
						}
						if(!(c2 && c3)){
							cout<<"Usage: create-post <board-name> --title <title> --content <content>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						string temp1;
						ss1>>temp1;
						bool canlogin = true;
						string author_name;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
								canlogin = false;
								author_name = vec[i].username;
								break;
							}
						}
						if(canlogin){
							send(sd,fail1,sizeof(fail1),0);
							continue;
						}else{
							bool board_exist = false;
							string board_name="";
							for(int i=0;i<all_board.size();++i){
								if(all_board[i].name == temp1){
									board_exist = true;
									board_name=temp1;
									break;
								}
							}
							if(!board_exist){
								send(sd,fail2,sizeof(fail2),0);
								continue;
							}else{
								string opt1;
								ss1>>opt1;
								string txt1="",txt2="",tmpin;
								if(opt1 == "--title"){
									while(ss1>>tmpin){
										if(tmpin[0]!='-' && tmpin[1]!='-'){
											txt1+=tmpin;
											txt1 += " ";
										}else{
											break;
										}
									}
									
									while(ss1>>tmpin){
										if(tmpin[0]!='-' && tmpin[1]!='-'){
											txt2+=tmpin;
											txt2+=" ";
										}else{
											break;
										}
									}
								}else{
									while(ss1>>tmpin){
										if(tmpin[0]!='-' && tmpin[1]!='-'){
											txt2+=tmpin;
											txt2+=" ";
										}else{
											break;
										}
									}
									while(ss1>>tmpin){
										if(tmpin[0]!='-' && tmpin[1]!='-'){
											txt1+=tmpin;
											txt1 += " ";
										}else{
											break;
										}
									}
								}
								//txt1.back()='\0';
								//txt2.back()='\0';
								stringstream ss2(txt2);
								string final_txt2="",tmpin1;
								while(getline(ss2,tmpin1,'<')){
									stringstream ss3(tmpin1);
									while(getline(ss3,tmpin1,'>')){
										if(tmpin1!="br"){
											final_txt2+=tmpin1;
										}else{
											final_txt2+='\n';
										}
									}
								}
								cout<<final_txt2;
								time_t t = time(NULL);
								tm* timePtr = localtime(&t);
								string ti = "";
								stringstream d1, d2;
								d1<<((timePtr->tm_mon)+1);
								d2<<timePtr->tm_mday;
								ti+= d1.str()+"/"+d2.str();
								cout<<"date: "<<ti<<endl;
								Post p(txt1,author_name,ti,final_txt2,board_name);
								p.my_sn = SN;
								SN++;
								cout << p.date << endl;
								all_posts.push_back(p);
								send(sd,success,sizeof(success),0);
							}
						}

					}
					else if(temp=="list-board"){
						string s="Index Name Moderator\n";
						for(int i=0;i<all_board.size();++i){
							string temp1="";
							temp1 += to_string(all_board[i].index);
							temp1 += " ";
							temp1 += all_board[i].name;
							temp1 += " ";
							temp1 +=all_board[i].moderator;
							temp1 += "\n";
							cout<<temp1<<endl;
							s+=temp1;
						}
						s+="% ";
						send(sd,(const char *)s.c_str(),strlen(s.c_str()),0);
					}
					else if(temp == "list-post"){
						const char usage[]="Usage: list-post <board-name>\n% ";
						const char fail1[]="Board does not exist.\n% ";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=1){
							cout<<"read <post-S/N>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1 >> tmp;
						string which_board;
						ss1>>which_board;
						string s="S/N Title Author Date\n";
						bool board_exist = false;
						for(int i=0;i<all_board.size();i++){
							if(all_board[i].name==which_board){
								board_exist=true;
							}
						}
						if(!board_exist){
							send(sd,fail1,sizeof(fail1),0);
							continue;
						}
						for(int i=0;i<all_posts.size();++i){
							if(which_board == all_posts[i].in_which_board && !all_posts[i].deleted){
								string temp1="";
								temp1 += to_string(all_posts[i].my_sn);
								temp1 += " ";
								temp1 += all_posts[i].title;
								temp1 += " ";
								temp1 += all_posts[i].author;
								temp1 += " ";
								temp1 += all_posts[i].date;
								temp1 += "\n";
								s+=temp1;
							}
						}
						s+="% ";
						cout<<s<<endl;
						send(sd,s.c_str(),strlen(s.c_str()),0);
						
					}
					else if(temp == "read"){
						const char usage[]="Usage: read <post-S/N>\n% ";
						const char fail1[]="Post does not exist.\n% ";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=1){
							cout<<"read <post-S/N>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						int want_read;
						ss1>>want_read;
						string s="";
						bool canread = false;
						for(int i=0;i<all_posts.size();++i){
							if(all_posts[i].my_sn==want_read && !all_posts[i].deleted){
								s+="Author: ";
								s+=all_posts[i].author;
								s+="\n";
								s+="Title: ";
								s+=all_posts[i].title;
								s+="\n";
								s+="Date: ";
								s+=all_posts[i].date;
								s+="\n--\n";
								s+=all_posts[i].content;
								s+="\n--\n";
								for(int j=0;j<all_posts[i].comment.size();++i){
									s+=all_posts[i].comment[j];
								}
								s+="% ";
								canread = true;
							}
						}
						cout <<s<<endl;
						//cout << s.c_str()<<endl;
						if(canread){
							write(sd,(const char *)s.c_str(),strlen(s.c_str())*sizeof(char));
						}else{
							send(sd,fail1,sizeof(fail1),0);
						}
					}
					else if(temp == "delete-post"){
						const char usage[]="Usage: delete-post <post-S/N>\n% ";
						const char fail1[]="Please login first.\n% ";
						const char fail2[]="Post does not exist.\n% ";
						const char fail3[]="Not the post owner.\n% ";
						const char success[]="Delete successfully.\n% ";
						int c=0;
						string tmp;
						while(ss>>tmp){
							c++;
						}
						if(c!=1){
							cout<<"delete-post <post-S/N>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						int want_delete;
						ss1>>want_delete;
						bool canlogin = true;
						bool post_exist = false;
						bool is_author = false;
						string selfname;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
								canlogin = false;
								selfname = vec[i].username;
								break;
							}
						}
						if(canlogin){
							send(sd,fail1,sizeof(fail1),0);
							continue;
						}
						for(int i=0;i<all_posts.size();++i){
							if(all_posts[i].my_sn == want_delete && !all_posts[i].deleted){
								post_exist = true;
								if(all_posts[i].author == selfname){
									is_author= true;
									all_posts[i].deleted=true;
								}
							}
						}
						if(!post_exist){
							send(sd,fail2,sizeof(fail2),0);
							continue;
						}
						if(!is_author){
							send(sd,fail3,sizeof(fail3),0);
							continue;
						}
						send(sd,success,sizeof(success),0);
					}
					else if(temp == "update-post"){
						const char usage[]="update-post <post-S/N> --title/content <new>\n% ";
						const char fail1[]="Please login first.\n% ";
						const char fail2[]="Post does not exist.\n% ";
						const char fail3[]="Not the post owner.\n% ";
						const char success[]="Update successfully.\n% ";
						int c=0,intc=0;
						int inttmp;
						string tmp;
						while(ss>>inttmp){
							intc++;
							if(intc==1)break;
						}
						while(ss>>tmp){
							c++;
						}
						if(intc!=1 && c<2){
							cout<<"update-post <post-S/N> --title/content <new>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						int want_update;
						ss1>>want_update;
						bool canlogin = true;
						bool post_exist = false;
						bool is_author = false;
						string selfname;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
								canlogin = false;
								selfname = vec[i].username;
								break;
							}
						}
						if(canlogin){
							send(sd,fail1,sizeof(fail1),0);
							continue;
						}
						for(int i=0;i<all_posts.size();++i){
							if(all_posts[i].my_sn == want_update && !all_posts[i].deleted){
								post_exist = true;
								if(all_posts[i].author == selfname){
									is_author= true;
								}
							}
						}
						if(!post_exist){
							send(sd,fail2,sizeof(fail2),0);
							continue;
						}
						if(!is_author){
							send(sd,fail3,sizeof(fail3),0);
							continue;
						}
						string opt,temp1;
						ss1>>opt;
						if(opt=="--title"){
							string update_title ="";
							while(ss1>>temp1){
								update_title+=temp1;
								update_title+= " ";
							}
							//update_title.back()='\0';
							for(int i=0;i<all_posts.size();++i){
								if(all_posts[i].my_sn == want_update && !all_posts[i].deleted){
									all_posts[i].title=update_title;
								}
							}
							send(sd,success,sizeof(success),0);
							continue;
						}
						string update_content="";
						string tmps;
						while(getline(ss1,tmps,'<')){
							stringstream ss2;
							while(getline(ss2,tmps,'>')){
								if(tmps!="br"){
									update_content+=tmps;
								}
								else{
									update_content+='\n';
								}
							}
						}
						for(int i=0;i<all_posts.size();++i){
							if(all_posts[i].my_sn == want_update && !all_posts[i].deleted){
								all_posts[i].content=update_content;
							}
						}
						send(sd,success,sizeof(success),0);
						

					}
					else if(temp == "comment"){
						const char usage[]="comment <post-S/N> <comment>\n% ";
						const char fail1[]="Please login first.\n% ";
						const char fail2[]="Post does not exist.\n% ";
						const char success[]="Comment successfully.\n% ";
						int c=0,intc=0;
						int inttmp;
						string tmp;
						while(ss>>inttmp){
							intc++;
							if(intc==1)break;
						}
						while(ss>>tmp){
							c++;
						}
						if(intc!=1 && c<1){
							cout<<"comment <post-S/N> <comment>\n";
							send(sd,(const char *)usage,sizeof(usage),0);
							continue;
						}
						stringstream ss1(inputBuffer);
						ss1>>tmp;
						int want_comment;
						ss1>>want_comment;
						string selfname,temp1;
						bool canlogin = true;
						bool post_exist = false;
						for(int i=0;i<vec.size();++i){
							if(vec[i].online == sd){
								canlogin = false;
								selfname = vec[i].username;
								break;
							}
						}
						if(canlogin){
							send(sd,fail1,sizeof(fail1),0);
							continue;
						}
						string txt="";
						txt += selfname;
						txt += ": ";
						while(ss1>>temp1){
							txt+=temp1;
							txt += " ";
						}
						//txt.back()='\0';
						txt += "\n";
						for(int i=0;i<all_posts.size();++i){
							if(all_posts[i].my_sn == want_comment && !all_posts[i].deleted){
								post_exist = true;
								all_posts[i].comment.push_back(txt);
								send(sd,success,sizeof(success),0);
							}
						}
						if(!post_exist){
							send(sd,fail2,sizeof(fail2),0);
							continue;
						}
					}
					else{
						const char fail1[]="% ";
						send(sd,fail1,sizeof(fail1),0);
					}
				}
			}	
		}
	}
	return 0;
}