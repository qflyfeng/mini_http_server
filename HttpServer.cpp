#include <iostream>
#include <process.h>
#include <winsock.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;
//最大路由路径 
#define MAX_FILENAME 64
//请求方式 
#define MAX_METHOD 20
//用户来访信息
#define MAX_USER_AGNET 256
//保存所有信息
#define MAX_ALL_INFO  1024
//response all
#define MAX_RESPONSE MAX_ACCEPT+MAX_SERVER+MAX_DATE+MAX_TYPE+MAX_FILE
//accept 信息 
#define MAX_ACCEPT 128
//server 信息
#define MAX_SERVER 128 
//date
#define MAX_DATE 64
//content type 
#define MAX_TYPE 128 
//file size
#define MAX_FILE 1024000
//state
#define MAX_STATE 64
//head
#define MAX_HEAD 1024
//请求头类 
class ResponseHead
{
	public:
		ResponseHead();
		void setState(char* st);
		void setServerinfo(char* st);
		void setDate(char* st);
		void setType(char* st);
		void combine();
		void sendinfo(SOCKET& user);
		int getCont();
		char* getState();
		bool readFile(char* filename);
	private:
		int cont_lenth;
		char head[MAX_HEAD];
		char state[MAX_STATE];
		char response[MAX_RESPONSE];
		char serverinfo[MAX_SERVER];
		char date[MAX_DATE];
		char type[MAX_TYPE];
		char file[MAX_FILE];
};
ResponseHead::ResponseHead():cont_lenth(0)
{
	memset(response,0,sizeof(response));
	memset(serverinfo,0,sizeof(serverinfo));
	memset(date,0,sizeof(date));
	memset(type,0,sizeof(type));
	memset(file,0,sizeof(file));
	memset(state,0,sizeof(state));
	memset(head,0,sizeof(head));
	strcpy(serverinfo,"Mini_Http");
	strcpy(head,
		"HTTP/1.0 %s \r\n"
		"Accept-Ranges: bytes\r\n"
		"Server: %s \r\n"
		"Date: %s \r\n"
		"Content-Length: %d \r\n"
		"connection: Close\r\n"
		"Content-type: %s \r\n\r\n"
	);
}
char* ResponseHead::getState()
{
	return state;
}
int ResponseHead::getCont()
{
	return cont_lenth;
}
void ResponseHead::setState(char* st)
{
	strcpy(state,st);
}
void ResponseHead::setServerinfo(char* st)
{
	strcpy(serverinfo,st);
}
void ResponseHead::setDate(char* st)
{
	strcpy(date,st);
}
void ResponseHead::setType(char* st)
{
	strcpy(type,st);
}
bool ResponseHead::readFile(char* filename)
{
	int len=0;
    fstream file1;
    file1.open(filename,ios::in | ios::binary);
    if(!file1.is_open())
    	return false;
    file1.read(file,MAX_FILE);
    cont_lenth=file1.gcount();
    file1.close();
	return true;
}
void ResponseHead::combine()
{
	sprintf(response,head,state,serverinfo,date,cont_lenth,type);
	memcpy(&response[strlen(response)],file,sizeof(file));
}
void ResponseHead::sendinfo(SOCKET& user)
{
	send(user,response,sizeof(response),0);
	sleep(2); 
	closesocket(user);
}
class RequestHead
{
	private:
		char request[MAX_ALL_INFO];
		char menthod[MAX_METHOD];
		char filename[MAX_FILENAME];
		char user_agent[MAX_USER_AGNET];
		char accept[MAX_ACCEPT];
	public:
		RequestHead();
		RequestHead(char *requestinfo);
		void recv_info(SOCKET user_socket);
		void cat_info();
		void init();
		char* getFilename()
		{
			return filename;
		}
		char* getUser_agent()
		{
			return user_agent;
		}
		char* getMenthod()
		{
			return menthod;
		}
		char* getAccept()
		{
			return accept;
		} 
		void show()
		{
			
			cout<<request<<endl<<"method"<<menthod<<endl<<"filename:"<<filename<<endl<<"user_agnet:"<<user_agent<<endl<<"accept:"<<accept<<endl;
		}
};
RequestHead::RequestHead()
{
	init();
}
RequestHead::RequestHead(char *requestinfo)
{
	init();
	memcpy(request,requestinfo,MAX_ALL_INFO);
}
void RequestHead::init()
{
	memset(request,0,sizeof(request));
	memset(menthod,0,sizeof(menthod));
	memset(filename,0,sizeof(filename));
	memset(user_agent,0,sizeof(user_agent));
	memset(accept,0,sizeof(accept));
}	
void RequestHead::cat_info()
{
	int i=0;
	int j=0;
	int n=0;
	char temp[1024];
	memset(temp,0,sizeof(temp));
	while(request[i++]==' ');
	i--;
	while(i<MAX_ALL_INFO && request[i]!='/')
	{
		menthod[j++]=request[i];
		i++;
	}
	menthod[j]='\n';
	j=0;
	i++;
	while(i<MAX_ALL_INFO && request[i]!=' ')
	{
		filename[j++]=request[i];
		i++;
	}
	while(i<MAX_ALL_INFO)
	{
		j=0;
		if(i<MAX_ALL_INFO && request[i]=='\n')
		{
			j=0;
			if(request[i]=='\n')
				i++;
			memset(temp,0,sizeof(temp));
			while(i<MAX_ALL_INFO && request[i]!=':')
				temp[j++]=request[i++];
			temp[j]='\n';
			
			i+=2;
			char st[10][20]={{"Accept\n"},{"User-Agent\n"}};
			if(strcmp(temp,st[0])==0)
			{
				j=0;
				while(i<MAX_ALL_INFO && request[i]!='/')
					accept[j++]=request[i++];
				accept[j]='\n';
			}
			else if(strcmp(temp,st[1])==0)
			{
				j=0;
				while(i<MAX_ALL_INFO && request[i]!='\n')
					user_agent[j++]=request[i++];
				user_agent[j]='\n';
			}
		}
		else
		i++;
	}	
}
//接收头部信息 
void RequestHead::recv_info(SOCKET user_socket)
{
	char temp;//循环接收客户端请求
	int j=0;
	int i=0;
	while(1)
	{
		i=recv(user_socket,&temp, sizeof(temp), 0);
		if(i==1)
		{
			if(temp=='\r')
			{
				recv(user_socket,&temp, sizeof(temp), 0);
				request[j++]=temp;
				if(temp=='\n')
				{
					recv(user_socket,&temp, sizeof(temp), 0);
					if(temp=='\r')
						break;
					else
						request[j++]=temp;
				}
			}
			else
			{
				request[j++]=temp;	
			}		
		}
		else
			break;	 
	}
	request[j]='\n';
	//结束标志\n\n 
}
class user
{
	private:
		ResponseHead response;
		RequestHead request;
		SOCKET user_socket;
	public:
		user();
		user(SOCKET& user1); 
		void respose();
		RequestHead& get_request();
		ResponseHead& get_response();
};
ResponseHead& user::get_response()
{
	return response;
}
RequestHead& user::get_request()
{
	return request;
}
user::user()
{
	
}
user::user(SOCKET& user1):user_socket(user1)
{
	
}
void user::respose()
{	
	if(!response.readFile(request.getFilename()))
	{
		response.setType("text/html");
		response.setState("404 Not Found");
		response.readFile("404.html");
	}
	else if(strcmp(request.getAccept(),"text\n")==0)
	{
		response.readFile(request.getFilename());
		response.setType("text/html");
	}
	else if(strcmp(request.getAccept(),"image\n")==0)
	{
		response.readFile(request.getFilename());
		response.setType("image/jpeg");
	}
	response.setDate("2016,10,15");
	response.combine();
	//cout<<request.getMenthod()<<"请求资源:"<< request.getFilename()<<endl<<"请求结果:"<<response.getState()<<endl<<"发送消息:"<<response.getCont()<<"字节"<<endl; 
	response.sendinfo(user_socket);
}
class HttpServer
{
	private:
		int port;
		string ip;
		SOCKET servSock;
	public:
		~HttpServer();
		HttpServer();
		void listen_socket();
		friend void deal_user(void* user1);
};
HttpServer::HttpServer()
{
	ip="127.0.0.1";
	port=9999;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);	
	//创建套接字 
	//AF_INET-->IPV6
	//SOCK_STREAM--->tcp(socket类型)
	//IPPROTO_TCP--->tcp(传输协议)
    servSock = socket(PF_INET, SOCK_STREAM, 0);
    //INVALID_SOCKET创建套接字出现错误 
	if(servSock == INVALID_SOCKET)
	{
		cout<<"套接字创建失败"<<endl;
		WSACleanup();
		exit(0);
	}
	else
	{
		cout<<"创建套接字"<<servSock<<endl;
	}
	//绑定套接字
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));  //每个字节都用0填充
    sockAddr.sin_family = PF_INET;  //使用IPv4地址
    sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());  //具体的IP地址
	sockAddr.sin_port = htons(port);  //端口
    if(bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
       //(绑定失败)
        WSACleanup();
        cout<<"绑定套接字失败"<<endl; 
        exit(0);
    } 
}
void deal_user(void *user1)
{
	user* user_x=new user(*(SOCKET*)user1);
	user_x->get_request().recv_info(*(SOCKET*)user1);
	user_x->get_request().cat_info();
	user_x->respose();
//	sleep(3);
	delete user_x;
}
void HttpServer::listen_socket()
{
	//进入监听状态
    cout<<"正在监听ip:"<<ip.c_str()<<"端口:"<<port<<endl;
    listen(servSock, 30);
	int nSize = sizeof(SOCKADDR);
	SOCKADDR clntAddr;
	SOCKET user;
	sockaddr_in sin;
	unsigned int x;
	while(1)
	{
		user = accept(servSock,&clntAddr, &nSize);
		if(user)
		{
			memcpy(&sin, &clntAddr, sizeof(sin));
			//printf("\n%s连接\n",inet_ntoa(sin.sin_addr));
			_beginthread(deal_user,0,&user);
			usleep(15000); 
		}	
	} 
}
HttpServer::~HttpServer()
{
	//终止 DLL 的使用
    WSACleanup();

}
int main()
{
    HttpServer a;
    a.listen_socket();
	return 0;
}

