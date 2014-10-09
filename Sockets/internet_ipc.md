## Network IPC: Sockets

### 1.套接字描述符

套接字是通信端点的抽象，正如使用文件描述符访问文件，应用程序用套接字描述符访问套接字。

**-创建套接字：**

	#include <sys/socket.h>
	int socket(int domain, int type, int protocal);//成功返回套接字描述符，否则返回-1

参数`domain`确定通信的特性，包括地址格式。常用的通信域有：

* AF_INET：IPv4
* AF_INET6: IPv6
* AF_UNIX: UNIX
* AF_UPSPEC: 未指定（任何域）

参数`type`确定套接字的类型，进一步确定通信特征。常用的套接字类型有：

* SOCK_DGRAM：固定长度的，无链接的，不可靠的报文传递——默认是UDP
* SOCK_STREAM：有序的，可靠的，双向的，面向链接的字节流——默认是TCP

参数`protocal`通常是0，表示为给定的域和套接字类型选择默认协议，常见的套接字协议类型有：

* IPPROTO_IP
* IPPROTO_IPV6
* IPPROTO_TCP
* IPPROTO_UDP
* IPPROTO_ICMP

**-关闭套接字**

套接字通信是双向的，可以采用`shutdown`函数来禁止一个套接字的I/O

	#include <sys/socket.h>
	int shutdown(int sockfd, int how);//成功返回0，否则返回-1

`how`的可选项有：`SHUT_RD`（关闭读端），`SHUT_WR`（关闭写端），`SHUT_RDWR`（关闭读写端）

### 2.寻址

**-字节序**

由于不同主机的字节序可能不同（最大端／最小端），TCP／IP使用网络字节序，为了使应用程序不被字节序问题所困扰，有4个处理字节序和网络字节序之间实施转换的函数：

	#include <arpa/inet.h>
	uint32_t htonl(uint32_t hostint32); //返回：以网络字节序表示的32位整型. host to net long
	uint16_t htons(uint16_t hostint16); //返回：以网络字节序表示的16位整型。host to net short
	uint32_t ntohl(uint32_t netint32); //返回：以主机字节序表示的32位整型
	uint16_t ntohs(uint116_t netint16); //返回：以主机字节序表示的16位整型

**-getattrinfo**

为使不用格式的地址能够传入到套接字函数，地址会被强制转换成一个通用的地址结构`sockaddr`

在IPv4因特网域中，套接字地址结构如下：

	struct sockaddr_in {
	sa_family_t sin_family; /* address family */
	in_port_t sin_port; /* port number */
	struct in_addr sin_addr; /* IPv4 address */
	unsigned char sin_zero[8]; /* filler */
	...
	};
	struct in_addr {
	in_addr_t s_addr; /* IPv4 address */
	};

有时需要打印出能被人理解的地址格式，`inet_addr`和`inet_ntoa`可以用于二进制格式与点分十进制格式之间的相互转换，只是它们只支持IPv4

取得代之的是：（可用于IPv4和IPv6）

	#include <arpa/inet.h>
	const char *inet_ntop(int domain, const void *restrict addr,
			    char *restrict str, socklen_t size); //成功返回地址字符串指针，否则返回NULL
	int inet_pton(int domain, const char *restrict str, void *restrict addr); //成功返回1，格式无效返回0，出错返回-1


`getattrinfo`函数允许将一个主机名和一个服务名映射到一个地址：

	#include <sys/socket.h>
	#include <netdb.h>
	int getaddrinfo(const char *restrict host,
		       const char *restruct service,
		       const struct addrinfo *restrict hint,
		       struct addrinfo **restrict res);//成功返回0
	void freeaddrinfo(struct addrinfo *ai);

需要提供主机名，服务名，或者两者都提供。如果仅仅提供一个名字，另外一个必须是一个空指针。主机名可以是一个节点名或点分格式的主机地址

`getaddrinfo`返回一个链表结构`addrinfo`，`freeaddrinfo`用于释放一个或多个这种结构

`addrinfo`结构的定义至少包含如下成员:

	struct addrinfo {
	int ai_flags; //customize behavior
	int ai_family; //address family
	int ai_socktype; //socket type
	int ai_protocal; //protocal
	socklen_t ai_addrlen; //length in bytes of address
	struct sockaddr *ai_addr; //address
	char *ai_canonname; //canonical name of host
	struct addrinfo *ai_next; //next in list
	...
	};

`hint`是一个用于过滤地址的模板，包括`ai_family`，`ai_flags`，`ai_protocal`和`ai_socktype`字段。剩余的证书字段必须设置为0，指针字段必须为空。

常用的`ai_flags`有`AI_CANONAME`——意思是需要一个规范的名字

如果`getaddrinfo`失败，必须要使用`gai_strerror`将返回的错误码转换成错误消息

	#include <netdb.h>
	const char *gai_strerror(int error);//返回描述错误的字符串指针

`getnameinfo`函数将一个地址转换成一个主机名和一个服务名

	#include <sys/socket.h>
	#include <netdb.h>
	int getnameinfo(const struct sockaddr *restrict addr, socklen_t alen,
		       char *restrict host, socklen_t hostlen,
		       char *restrict service, socklen_t servlen, int flags);
	//成功返回0

`flags`参数提供了一些控制翻译的方式，比如

* NI_DGRAM——服务基于数据报
* NI_NUMERICSERV——返回端口号
* NI_NUMERICHOST——返回主机地址的数字形式

**-bind**

`bind`函数用来关联地址和套接字

	#include <sys/socket.h>
	int bind(int sockfd, const struct sockaddr *addr, socklen_t len);//成功返回0

对于使用的地址有以下的限制：

* 在进程正在运行的计算机上，指定的地址必须有效；不能指定一个其他机器的地址
* 地址必须和创建套接字时的地址族所支持的格式相匹配
* 地址中的端口号必须小于1024，除非该进程具有相应的权限
* 一般只能将一个套接字端点绑定要一个给定的地址上

可以调用`getsockname`函数来获得绑定到套接字上的地址

	#include <sys/socket.h>
	int getsockname(int sockfd, struct sockaddr *restrict addr,
		       socklen_t *restrict alenp); //成功返回0
	
调用之前，讲`alenp`设置为一个指向整数的指针，该证书指定缓冲区`sockaddr`的长度。返回时，该整数会被设置成返回地址的大小。

如果套接字已经和对等方连接，可以调用`getpeername`函数来找到对方的地址

	#include <sys/socket.h>
	int getpeername(int sockfd, struct sockaddr *restrict addr,
		       socklen_t *restrict alenp); //成功返回0

### 3.建立连接

处理面向连接的网络服务（TCP），客户端在请求服务时需要与服务端建立一个连接：

	#include <sys/socket.h>
	int connect(int sockfd, const struct sockaddr *addr, socklen_t len);//成功返回0，否则返回-1

`connect`函数还可以用于UDP，如果用`SICK_DGRAM`套接字调用`connect`，传送的报文的目标地址会被设置成`connect`调用中所指定的地址，这样每次传送报文时就不需要再提供地址。另外，仅能接收来自指定地址的报文
服务器则调用`listen`函数表示它愿意接受连接：

	#include <sys/socket.h> 
	int listen(int sockfd, int backlog); //成功返回0，否则返回-1

`int backlog`规定该进程可以提供入队的最大连接请求数量

一旦服务器调用了`listen`，所用的套接字就能接受连接请求，建立连接的函数为：

	#include <sys/socket.h>
	int accept(int sockfd, struct sockaddr *restrict addr,
		  socklen_t *restrict len); //成功返回套接字描述符，否则返回-1

这里`addr`表示客户端的地址，调用函数成功后，保存了客户端主机的相关信息

### 4.数据传输

用于数据传送的包括6个函数：

	#include <sys/socket.h>
	ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags); //成功返回字节数，否则返回-1
	ssize_t sendto(int sockfd, const void *buf, size)t nbytes, int flags,
		      const struct sockaddr *destaddr, socklen_t destlen); //成功返回发送的字节数，否则返回-1
	ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags); //成功返回发送的字节数，否则返回-1

`sendto`与`send`的区别：前者可以在无连接的套接字上指定一个目标地址。对于面向连接的套接字，目标地址是忽略的，因为连接中隐含了目标地址。对于无连接的套接字，除非先调用了`connect`设置了目标地址，否则不能使用`send`

	#include <sys/socket.h>
	ssize_t recv(int sockfd, void *buf, size_t nbytes, int flags); //返回数据的字节长度；若无可用数据或对等方已经按序结束，返回0，出错返回-1
	ssize_t recvfrom(int sockfd, void *restrict buf, size_t len, int flags,
		        struct sockaddr *restrict addr,
		        socklent_t *restrict addrlen); 
	ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);

`recvfrom`可以定位发送者

