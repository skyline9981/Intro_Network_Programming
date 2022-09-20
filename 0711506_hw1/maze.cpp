#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<sys/time.h>	/* includes <time.h> unsafely */
#include	<time.h>		/* old system? */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		/* for Unix domain sockets */
# include	<sys/select.h>	/* for convenience */
# include	<sys/param.h>	/* OpenBSD prereq for sysctl.h */
# include	<sys/sysctl.h>
# include	<poll.h>		/* for convenience */
# include	<strings.h>		/* for convenience */
# include	<sys/ioctl.h>
#include<string.h>
#include<bits/stdc++.h>

using namespace std;


struct poi{
    int x,y;
};

int step = 0;
int aa = 0;
int row=101,m=101;
poi parent[105][105];
vector<string>maze(1000);
int dic[4][2] = {{1,0},{0,1},{0,-1},{-1,0} };
vector<poi>ans(100000);
vector<string> road(100000);

bool in(int x,int y){
    if(x>=0 && y>=0 && x<row && y<m){
        return true;
    }else{
        return false;
    }
}

void make_ans(int x,int y){
    if(parent[x][y].x != x || parent[x][y].y != y){
        make_ans(parent[x][y].x,parent[x][y].y);
    }
    ans[step].x = x;
    ans[step].y = y;
    step++;
}

void bfs(int sx,int sy){
    poi start;
    start.x = sx;
    start.y = sy;
    queue<poi>que;
    que.push(start);
    parent[sx][sy].x = sx;
    parent[sx][sy].y = sy;
    while(!que.empty()){
        poi from;
        from = que.front();
        que.pop();
        for(int i=0;i<4;i++){
            poi to;
            to.x = from.x + dic[i][0];
            to.y = from.y + dic[i][1];
            if(in(to.x,to.y) && parent[to.x][to.y].x == -1 && maze[to.x][to.y] == '.'){
                que.push(to);
                parent[to.x][to.y].x = from.x;
                parent[to.x][to.y].y = from.y;
            }
        }
    }
}


int main(int argc, char **argv){
    if(argc != 3){
        cout<< '\n';
        return 0;
    }
    int sockfd,n,cnt = 0;
    struct sockaddr_in servaddr;
    struct hostent *hptr;
    int portnum;
    stringstream pot(argv[2]);
    pot>>portnum;
    struct in_addr		**pptr;
	struct in_addr		*inetaddrp[2];
	struct in_addr		inetaddr;
    struct hostent		*hp;
	struct servent		*sp;
    char readbuffer[100000];
    bool start_input = false;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portnum);
    memset(parent,-1,sizeof(parent));
    if ( (hp = gethostbyname(argv[1])) == NULL) {
		if (inet_aton(argv[1], &inetaddr) == 0) {
            cout<<"error"<<endl;
		} 
        else {
			inetaddrp[0] = &inetaddr;
			inetaddrp[1] = NULL;
			pptr = inetaddrp;
		}
	} 
    else {
		pptr = (struct in_addr **) hp->h_addr_list;
	}
    for ( ; *pptr != NULL; pptr++) {
		memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
		if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == 0)
			break;		/* success */
		close(sockfd);
	}


    int now = 0,startx,starty,endx = 0,endy = 0;
    bool input_end = false,cal_end = false;
    while((n = read(sockfd,readbuffer,sizeof(readbuffer)))>=0){
        readbuffer[n]=0;
        stringstream ss(readbuffer);
        string s1,s2,s3;
        printf("%s",readbuffer);
        
        

        
        if(readbuffer[0] == 'B'){
            exit(0);
        }
    }
    

}



//11*7
//79*21
//101*101


/*
for(int i=0;i<step-1;i++){
    if(ans[i+1].x-ans[i].x == 0){
        if(ans[i+1].y-ans[i].y == 1){
            road[i] = "D\n";
        }
        else{
            road[i] = "A\n";
        }
    }
    else if(ans[i+1].y-ans[i].y == 0){
        if(ans[i+1].x-ans[i].x == 1){
            road[i] = "S\n";
        }
        else{
            road[i] = "W\n";
        }
    }
}
*/
            