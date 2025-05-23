/*
 * 程序名：demo2.cpp，此程序用于演示socket通信的服务端
 */
#include "../_public.h"
using namespace std;
using namespace idc;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout
            << "Using:./demo2 通讯端口\nExample:./demo2 5005\n\n"; // 端口大于1024，不与其它的重复。
        cout << "注意：运行服务端程序的Linux系统的防火墙必须要开通5005端口。\n";
        cout << "      如果是云服务器，还要开通云平台的访问策略。\n\n";
        return -1;
    }

    // 第1步：创建服务端的socket。
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        return -1;
    }

    // 第2步：把服务端用于通信的IP和端口绑定到socket上。
    struct sockaddr_in servaddr; // 用于存放服务端IP和端口的数据结构。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // 指定协议。
    servaddr.sin_addr.s_addr =
        htonl(INADDR_ANY); // 服务端任意网卡的IP都可以用于通讯。
    servaddr.sin_port =
        htons(atoi(argv[1])); // 指定通信端口，普通用户只能用1024以上的端口。
    // 绑定服务端的IP和端口。
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("bind");
        close(listenfd);
        return -1;
    }

    // 第3步：把socket设置为可连接（监听）的状态。
    if (listen(listenfd, 5) != 0) {
        perror("listen");
        close(listenfd);
        return -1;
    }

    // 第4步：受理客户端的连接请求，如果没有客户端连上来，accept()函数将阻塞等待。
    int clientfd = accept(listenfd, 0, 0);
    if (clientfd == -1) {
        perror("accept");
        close(listenfd);
        return -1;
    }

    cout << "客户端已连接。\n";

    // 第5步：与客户端通信。
    while (true) {
        // char buffer[1024];
        // memset(buffer,0,sizeof(buffer));
        // if (recv(clientfd,buffer,sizeof(buffer),0)<=0) break;
        string buffer;
        if (tcpread(clientfd, buffer) == false) {
            break;
        }
        cout << "接收：" << buffer << endl;
    }

    // 第6步：关闭socket，释放资源。
    close(listenfd); // 关闭服务端用于监听的socket。
    close(clientfd); // 关闭客户端连上来的socket。
}
