/* 
 * CryptHook
 * Secure TCP/UDP wrapper
 * www.chokepoint.net
 * Tested with AES algorithm
 * Example:
 * $ LD_PRELOAD=crypthook.so CH_KEY=omghax ncat -l -p 5000
 * $ LD_PRELOAD=crypthook.so CH_KEY=omghax ncat localhost 5000
 * Packet Format:
 * [algo][len][iv][hmac][payload]
 */
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <string.h>
#include <iomanip>

#include <iostream>
#include <cstring>
#include <string>
#include <openabe/openabe.h>
#include <openabe/zsymcrypto.h>
using namespace oabe;
using namespace oabe::crypto;
using namespace std;

/* Use these to link to actual functions */
static ssize_t (*old_recv)(int sockfd, void *buf, size_t len, int flags);
static ssize_t (*old_send)(int sockfd, void *buf, size_t len, int flags);
static ssize_t (*old_recvfrom)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
static ssize_t (*old_sendto)(int sockfd, void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

#define MAX_LEN 100000

// #define PACKET_HEADER 0x17 // Packet Identifier added to each header

// 1 byte packet identifier
// 12 bytes IV
// 16 bytes MAC
// #define HEADER_SIZE 8192

std::string mpkparam = "AAAAFqpvyQeGOyICbZ29rFw+6s74bZxtcGsAAAHToQFBsgEEtLIBABYrNiM2C7hfU2U8ZVwX+GVM0OdcBuIj4hIjLPC2twfmCYVok4A7jzxbROZjaFu82qmRdY5IdDAmDFJd3zeKYKMJQYNyiN/Hx0fLT/7iSc5+1n/V+iTl+Y8laxbOY1kxqRz+1qcfSUCx0MJO05dz2hZoZAh+e6ZN5cWyut7fo+wTBAqmuS3Q1+ordLPzgfSeFChxwYo/UhDafl25cGEADvoVtnIqR8/8FAx2co1DoCz5PrVRkdii5D5Yxb5u/xpIzRF6ZyAR4rca1AUplr9bIRwXjmn8yZN6hHdFyZ9C6BCCEDh2+rW0HleGUmOxeYqAiBH2vkelLUZGbuB8FWY84MihAmcxoSSyoSECI7LiNuFqS8IDSCcFwqmWmneJdAD+7UtCBJZinvXdUbmhA2cxYaEksqEhAw2Z7n15ftNpsfjR5HsaJ1f9nEyITBLBw3BuNvpbjCVsoQJnMqFEs6FBAwIINf5u9ptuuMuZbmdq39aDAkrkJ02NEJEzWJOSZjOXCCPYGra1N4jUyz0dGgLUbK/MQAGuYVdSGDrZLHZVtTShAWuhJR0AAAAgWRCwJQBcFtxFRaR4eX7GFeVQlybDT21xQYPGsYGSasg=";

std::string mskparam = "AAAAFqpvyQC/De1L2WQAGcniiKHyeFVtc2sAAAB3oQVhbHBoYaEjsQAgCojxJbZh+cmJ85vRN8B9QptoCNO9tOfgKt/sII2uThKhA2cyYaFEs6FBAgnwkJIGhHv1yhBE+xqtogqC2ReeKRYHTK3SxLYM+n0EI82+iksjJ5verfLYITflKaNNmx7bMx7ZeNvs+H40IwM=";

static int encrypt_data(char *in, int len, char *out)
{
    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
    //    OpenPKEContext pke;

    string ct, pt1, pt2;
    //    cpabe.generateParams();
    std::string mpk, msk;
    //    cpabe.exportPublicParams(mpk);
    //    cpabe.exportSecretParams(msk);

    // mpkparam = mpk;
    mpk = mpkparam;
    //     msk = mskparam;
    //     cout << "encrypt mpk1:" << mpk << endl;
    //     cout<<"encrypt msk:"<<msk<<endl;
    cpabe.importPublicParams(mpk);
    // cout << "encrypt mpk:" << mpk << endl;

    unsigned char outbuf[MAX_LEN];
    unsigned char temp[MAX_LEN];

    unsigned char *step;
    // int tmplen = 0, outlen = 0;
    int outlen = 0;

    // copy plain text message into temp
    memset(temp, 0x00, MAX_LEN);
    memcpy(temp, in, len);

    // cpabe.keygen("|attr1|attr2", "key0");
    //    pke.keygen("user0");

    pt1 = (char *)temp; //不能用pt1 = in;

    cpabe.encrypt("attr1 and attr2", pt1, ct);
    //    pke.encrypt("user0", pt1, ct);
    //    cpabe.decrypt("key0", ct, pt2);
    cout << "the length of encrypt plaintext:" << pt1.length() << endl;
    cout << "ct:" << endl<< ct << endl;

    char *cipher = (char *)ct.c_str();
    //    char *plain = (char *)pt2.c_str();

    // outlen = ct.length();
    outlen = strlen(cipher);
    //    outlen = strlen(plain);

    memcpy(outbuf, cipher, outlen);
    //    memcpy(outbuf, plain, outlen);

    // // Add header information
    // out[0] = PACKET_HEADER; //1 byte, Packet Identifier
    ////printf("%s","out[0]:");
    ////printf("%x\n",out[0]);

    // // Pack full packet length
    // out[1] = (0xff00 & (outlen + HEADER_SIZE)) >> 8; //取len + HEADER_SIZE 高8位
    // out[2] = (0xff & (outlen + HEADER_SIZE));
    // step = (unsigned char *)&out[3];

    step = (unsigned char *)&out[0];
    // cout << "encrypt step:" << endl << step << endl;//输出为空，没有内容

    memcpy(step, outbuf, outlen);
    // cout << "encrypt outbuf step:" << endl<< step << endl;//输出为密文
    cout << "encrypt outlen(ciphertext length):" << endl<< outlen << endl;
    // cout << "encrypt out:" << endl<< out << endl;  //没有内容输出

    ShutdownOpenABE();

    // return outlen + tmplen + HEADER_SIZE;
    // return outlen + HEADER_SIZE;
    return outlen;
}

static int decrypt_data(char *in, int len, char *out)
{

    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
    string ct, pt1, pt2, pt;
    //    cpabe.generateParams();
    //    cpabe.keygen("|attr1|attr2", "key0");
    //    pke.keygen("user0");

    std::string mpk, msk;
    //     cpabe.exportPublicParams(mpk);
    mpk = mpkparam;
    msk = mskparam;
    //    cout << "decrypt mpk1:"<<endl << mpkparam << endl;

    //    cout <<"decrypt mpk:"<<endl<< mpk << endl;

    unsigned char outbuf[MAX_LEN];

    char *step;

    // int tmplen = 0, outlen = 0;
    int outlen = 0;
    memset(outbuf, 0x00, MAX_LEN);

    // cout << "decrypt_data in:" << endl<< in << endl;//输出没有内容
    // header information
    // step = in + 3; // First three bytes are header info / length only
    step = in;
    ct = step;

    //    pt1 = step;
    cout << "the ct will be decrypted:" << endl<< step << endl;

    cout << endl<< "ct length:" << ct.length() << endl;
    //    cout<<endl<<"ct length:"<<ct.length()<<endl;
    //    cpabe.encrypt("attr1 and attr2", pt1, ct);
    //    cout<<"ct:"<<endl<<ct<<endl;
    cpabe.importPublicParams(mpk);
    cpabe.importSecretParams(msk);
    cpabe.keygen("|attr1|attr2", "key0");
    cpabe.decrypt("key0", ct, pt2);
    //    cpabe.importPublicParams(mpk);
    //    cpabe.decrypt("key0", pt2, pt);
    //    pke.decrypt("user0", ct, pt2);
    char *plaintext = (char *)pt2.c_str();
    outlen = strlen(plaintext);
    memcpy(outbuf, plaintext, outlen);
    cout << "the length of decrypt plaintext:" << outlen << endl;
    cout << "decrypt len:" << len << endl;
    memcpy(out, outbuf, outlen);
    // memcpy(out, outbuf, outlen + tmplen);
    ShutdownOpenABE();
    //     return len;
    return outlen; // len = 明文长度+1
}

/* Hook recv and decrypt the data before returning to the program */
ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    char outbuf[MAX_LEN];
    unsigned char temp[MAX_LEN];

    int outlen, ret, packet_len;

    memset(outbuf, 0x00, MAX_LEN);
    memset(temp, 0x00, MAX_LEN);

    if (!old_recv)
        old_recv = (ssize_t(*)(int, void *, size_t, int))dlsym(RTLD_NEXT, "recv");

    if (sockfd == 0) // Y U CALL ME W/ SOCKFD SET TO ZERO!?!?
        return old_recv(sockfd, buf, len, flags);

    ret = old_recv(sockfd, (void *)temp, MAX_LEN, flags);
    // ret = old_recv(sockfd, (void *)temp, 3, MSG_PEEK); //   ??

    if (ret < 1)
    { // Nothing to decrypt
        return ret;
    }
    // //     printf("%s","recv temp[0]:");
    // //     printf("%x\n",temp[0]);
    // if (temp[0] != PACKET_HEADER)
    // {
    //     fprintf(stderr, "[!] Client not using CryptHook\n");
    //     return 0;
    // }
    // // Unpack the full message length
    // packet_len = (temp[1] << 8) + temp[2];

    // ret = old_recv(sockfd, (void *)temp, packet_len, flags);

    // outlen = decrypt_data((char *)temp, ret - HEADER_SIZE, &outbuf[0]);

    outlen = decrypt_data((char *)temp, ret, &outbuf[0]);

    memcpy((void *)buf, (void *)outbuf, (size_t)outlen);

    return outlen;
}

/* Hook recvfrom and decrypt the data before returning to the program */
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    char outbuf[MAX_LEN];
    unsigned char temp[MAX_LEN];

    int outlen, ret, packet_len;

    memset(outbuf, 0x00, MAX_LEN);
    memset(temp, 0x00, MAX_LEN);

    if (!old_recvfrom)
        old_recvfrom = (ssize_t(*)(int, void *, size_t, int, sockaddr *, socklen_t *))dlsym(RTLD_NEXT, "recvfrom");

    if (sockfd == 0) // Y U CALL ME W/ SOCKFD SET TO ZERO!?!?
        return old_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);

    ret = old_recvfrom(sockfd, (void *)temp, MAX_LEN, flags, src_addr, addrlen);
    // Grab first three bytes to see if its our protocol and the length
    // ret = old_recvfrom(sockfd, (void *)temp, 3, MSG_PEEK, src_addr, addrlen);

    if (ret < 1)
    { // Nothing to decrypt
        return ret;
    }
    // //     printf("%s","recvfrom temp[0]:");
    // //     printf("%x\n",temp[0]);
    // if (temp[0] != PACKET_HEADER)
    // {
    //     fprintf(stderr, "[!] Client not using same crypto algorithm\n");
    //     return 0;
    // }
    // // Unpack the full message length
    // packet_len = (temp[1] << 8) + temp[2];

    // ret = old_recvfrom(sockfd, (void *)temp, packet_len, flags, src_addr, addrlen);

    // outlen = decrypt_data((char *)temp, ret - HEADER_SIZE, &outbuf[0]);
    outlen = decrypt_data((char *)temp, ret , &outbuf[0]);

    memcpy((void *)buf, (void *)outbuf, (size_t)outlen);

    return outlen;
}

/* Hook send and encrypt data first */
ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    char outbuf[MAX_LEN];
    int outlen;

    memset(outbuf, 0x00, MAX_LEN);

    if (!old_send)
        old_send = (ssize_t(*)(int, void *, size_t, int))dlsym(RTLD_NEXT, "send");

    outlen = encrypt_data((char *)buf, len, &outbuf[0]);
    if (outlen == 0)
        return 0;

    // Send the encrypted data
    old_send(sockfd, (void *)outbuf, outlen, flags);

    return len;
}

/* Hook send and encrypt data first */
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    char outbuf[MAX_LEN];
    int outlen;

    memset(outbuf, 0x00, MAX_LEN);

    if (!old_sendto)
        old_sendto = (ssize_t(*)(int, void *, size_t, int, const sockaddr *, socklen_t))dlsym(RTLD_NEXT, "sendto");

    outlen = encrypt_data((char *)buf, len, &outbuf[0]);

    if (outlen == 0)
        return 0;

    // Send the encrypted data
    old_sendto(sockfd, (void *)outbuf, outlen, flags, dest_addr, addrlen);

    return len;
}

// LD_PRELOAD=./abehook.so ncat -l -p 5000
// LD_PRELOAD=./abehook.so ncat 127.0.0.1 5000

//LD_PRELOAD=./abehook.so python3 -m http.server 9000
//LD_PRELOAD=./abehook.so curl http://127.0.0.1:9000

// cc abehook.c -o abehook.so -Wall -shared -fpic -ldl -lcrypto -L/usr/local/lib

// g++ -std=c++11 abehook.cpp -o abehook.so -Wall -shared -fpic -ldl -lcrypto -lrelic -lrelic_ec -lopenabe -L/usr/local/lib -I${ZROOT}/deps/root/include -I${ZROOT}/root/include -L${ZROOT}/deps/root/lib -L${ZROOT}/root/lib
