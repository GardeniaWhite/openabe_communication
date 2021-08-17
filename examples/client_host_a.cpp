#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

#include <cstring>
#include <string>
#include <openabe/openabe.h>
#include <openabe/zsymcrypto.h>
using namespace oabe;
using namespace oabe::crypto;

#define BUF_SIZE 1024
void errorhandling(char *message);

std::string mpkparam = "AAAAFqpvyQeGOyICbZ29rFw+6s74bZxtcGsAAAHToQFBsgEEtLIBABYrNiM2C7hfU2U8ZVwX+GVM0OdcBuIj4hIjLPC2twfmCYVok4A7jzxbROZjaFu82qmRdY5IdDAmDFJd3zeKYKMJQYNyiN/Hx0fLT/7iSc5+1n/V+iTl+Y8laxbOY1kxqRz+1qcfSUCx0MJO05dz2hZoZAh+e6ZN5cWyut7fo+wTBAqmuS3Q1+ordLPzgfSeFChxwYo/UhDafl25cGEADvoVtnIqR8/8FAx2co1DoCz5PrVRkdii5D5Yxb5u/xpIzRF6ZyAR4rca1AUplr9bIRwXjmn8yZN6hHdFyZ9C6BCCEDh2+rW0HleGUmOxeYqAiBH2vkelLUZGbuB8FWY84MihAmcxoSSyoSECI7LiNuFqS8IDSCcFwqmWmneJdAD+7UtCBJZinvXdUbmhA2cxYaEksqEhAw2Z7n15ftNpsfjR5HsaJ1f9nEyITBLBw3BuNvpbjCVsoQJnMqFEs6FBAwIINf5u9ptuuMuZbmdq39aDAkrkJ02NEJEzWJOSZjOXCCPYGra1N4jUyz0dGgLUbK/MQAGuYVdSGDrZLHZVtTShAWuhJR0AAAAgWRCwJQBcFtxFRaR4eX7GFeVQlybDT21xQYPGsYGSasg=";

std::string mskparam = "AAAAFqpvyQC/De1L2WQAGcniiKHyeFVtc2sAAAB3oQVhbHBoYaEjsQAgCojxJbZh+cmJ85vRN8B9QptoCNO9tOfgKt/sII2uThKhA2cyYaFEs6FBAgnwkJIGhHv1yhBE+xqtogqC2ReeKRYHTK3SxLYM+n0EI82+iksjJ5verfLYITflKaNNmx7bMx7ZeNvs+H40IwM=";

int main(int argc, char *argv[])
{
    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
    string ct, pt;
    std::string mpk, msk;
    mpk = mpkparam;
    msk = mskparam;
    cpabe.importPublicParams(mpk);
    cpabe.importSecretParams(msk);
    cpabe.keygen("|attr1|attr2", "key0");
 
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len = 0, idx = 0, read_len = 0;

    if (argc != 3)
    {
        cout << "Usage : " << argv[0] << "<IP> <port> " << endl;
        exit(0);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        errorhandling("socket() error;");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)    {
        errorhandling("connect error!");
    }
    else
    {
        cout << "connected....." << endl;
    }
 
    printf("Message from server : \n");
    while (1)
    {
         str_len = read(sock, message, BUF_SIZE);
        cout<<"read length:"<<str_len<<endl;
       // message[str_len] = '\0';
       // printf("%s\n", message);
        char message2[BUF_SIZE];
        memset(message2, 0, sizeof(message2));
        for(int i=0;i<str_len;i++){
            message2[i]=message[i];
        }

        ct = message2;
        cout<<"Message from server(ciphertext):"<<endl<<ct<<endl;
        cout<<"the length of message:"<<ct.length()<<endl;
        cpabe.decrypt("key0", ct, pt);
        cout<<endl<<"the plaintext:"<<endl<<pt<<endl;
        ofstream outfile;
        outfile.open("key.txt");
        if (outfile.is_open())
        {
            outfile << pt;
        }
        outfile.close();
       // message[str_len] = '\0';
       // printf("%s\n", message);
        //printf("Message from server : %s\n", message);
       // memset(message2, 0, sizeof(message2));
        memset(message, 0, sizeof(message));
    }

    close(sock);

    close(sock);
    return 0;
}

void errorhandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
