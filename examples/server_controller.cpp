#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>
#include <cstring>
#include <string>
#include <cassert>
#include <openabe/openabe.h>
#include <openabe/zsymcrypto.h>
using namespace oabe;
using namespace oabe::crypto;
using namespace std;

#define BUF_SIZE 1024

void error_handling(char *message);
void read_childproc(int sig);

std::string mpkparam = "AAAAFqpvyQeGOyICbZ29rFw+6s74bZxtcGsAAAHToQFBsgEEtLIBABYrNiM2C7hfU2U8ZVwX+GVM0OdcBuIj4hIjLPC2twfmCYVok4A7jzxbROZjaFu82qmRdY5IdDAmDFJd3zeKYKMJQYNyiN/Hx0fLT/7iSc5+1n/V+iTl+Y8laxbOY1kxqRz+1qcfSUCx0MJO05dz2hZoZAh+e6ZN5cWyut7fo+wTBAqmuS3Q1+ordLPzgfSeFChxwYo/UhDafl25cGEADvoVtnIqR8/8FAx2co1DoCz5PrVRkdii5D5Yxb5u/xpIzRF6ZyAR4rca1AUplr9bIRwXjmn8yZN6hHdFyZ9C6BCCEDh2+rW0HleGUmOxeYqAiBH2vkelLUZGbuB8FWY84MihAmcxoSSyoSECI7LiNuFqS8IDSCcFwqmWmneJdAD+7UtCBJZinvXdUbmhA2cxYaEksqEhAw2Z7n15ftNpsfjR5HsaJ1f9nEyITBLBw3BuNvpbjCVsoQJnMqFEs6FBAwIINf5u9ptuuMuZbmdq39aDAkrkJ02NEJEzWJOSZjOXCCPYGra1N4jUyz0dGgLUbK/MQAGuYVdSGDrZLHZVtTShAWuhJR0AAAAgWRCwJQBcFtxFRaR4eX7GFeVQlybDT21xQYPGsYGSasg=";

std::string mskparam = "AAAAFqpvyQC/De1L2WQAGcniiKHyeFVtc2sAAAB3oQVhbHBoYaEjsQAgCojxJbZh+cmJ85vRN8B9QptoCNO9tOfgKt/sII2uThKhA2cyYaFEs6FBAgnwkJIGhHv1yhBE+xqtogqC2ReeKRYHTK3SxLYM+n0EI82+iksjJ5verfLYITflKaNNmx7bMx7ZeNvs+H40IwM=";

int main(int args, char *argv[])
{
    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
    string ct, pt;
    std::string mpk;
    mpk = mpkparam;
    cpabe.importPublicParams(mpk);

    int serv_sock, clnt_sock;                //套接字文件描述符
    struct sockaddr_in serv_addr, clnt_addr; //描述地址相关的结构体
    socklen_t adr_sz;                        //保证和当前机器的int类型具有一致的字节长度，否则就会>破坏套接字层的填充。

    pid_t pid;
    struct sigaction act; //设置信号处理方式
    int str_len, state;
    char message[BUF_SIZE];
    if (args != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);           //将sa_mask成员的所有初始位初始化为0
    act.sa_flags = 0;                    //初始化
    state = sigaction(SIGCHLD, &act, 0); // 注册SIGCHLD处理器，在一个进程终止>或者停止时，将SIGCHLD信号发送给其父进程，按系统默认将忽略此信号，如果父进程希>望被告知其子系统的这种状态，则应捕捉此信号。防止产生僵尸进程

    serv_sock = socket(PF_INET /*IPV4协议*/, SOCK_STREAM, 0); //第二和第三割>>参数用于指定TCP方式
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4网络协议中采用的地址>族
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY就是指定地址>为0.0.0.0的地址, 这个地址事实上表示不确定地址, 或“所有地址”、“任意地址”。
    serv_addr.sin_port = htons(atoi(argv[1]));

    //向server套接字分配网络地址
    if (bind(serv_sock, (const sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error");
    }

    //进入等待连接的状态，连接队列的请求长度为5
    if (listen(serv_sock, 5) == -1)
    {
        error_handling("listen() error");
    }

    while (1)
    {
        adr_sz = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (sockaddr *)&clnt_addr, &adr_sz); //受理客户端的连接请求
        if (clnt_sock == -1)
            continue;
        else
        {
            puts("new client connected...");
        }
        pid = fork(); //创建进程
        if (pid == 0)
        {
            //子进程处理客户端的具体请求，同时返回
            //由于进程会全复制，包括serv_sock和clnt_sock（这种套接字文件描述>>符，但具体的套接字内容不会被复制，该信息被系统所有）都会被复制，
            close(serv_sock);

            while (1)
            {
                fgets(message, BUF_SIZE, stdin);

                pt = message;
                cpabe.encrypt("attr1 and attr2", pt, ct);
                char *message2 = (char *)ct.c_str();
                int len = ct.length();
                //  message2[len] =  '\0';
                cout << "The Ciphertext is:" << endl << ct << endl;
                cout << "The length of ciphertext:" << len << endl;
                write(clnt_sock, message2, len);
                // write(clnt_sock, message2, strlen(message2));

                // cout<<"The length of message2:"<<sizeof(message2)<<endl;
                //    printf("message2:\n%s\n",message2);
                memset(message2, 0, sizeof(message2));
                memset(message, 0, sizeof(message));
            }
            /*
            while ((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0) {
                write(clnt_sock, buf, str_len);     //回复客户端，等同于windows的send
            }
         */
            close(clnt_sock);
            puts("client disconnected......");
            return 0;
        }
        else
        {
            close(clnt_sock);
        }
    }

    close(clnt_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr); //向指定的文件写入一个字符串（不自动写入字符串结>>束标记符‘\0’）stderr:标准输出(设备)文件，对应终端的屏幕
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig)
{
    pid_t pid;
    int status;
    //    WNOHANG:若pid指定的子进程没有结束，则waitpid()函数返回0，不予以等待>。若结束，则返回该子进程的ID。
    pid = waitpid(-1 /*等待任何子进程*/, &status, WNOHANG); //waitpid()会暂时>停止目前进程的执行,直到有信号来到或子进程结束。
    printf("remove proc id: %d \n", pid);
}
