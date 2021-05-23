//
//  problem3.c
//  ECE4436A - Assignment 1 - Problem 3
//
//  Created by Gareth Cross on 2012-10-04.
//  Copyright (c) 2012 Gareth Cross. MIT License.
// g++ simplehttp.cpp -o simplehttp -lpthread
// g++ -std=c++11 simplehttp.cpp -o simplehttp -lpthread -Wall -lcrypto -lrelic -lrelic_ec -lopenabe -L/usr/local/lib -I${ZROOT}/deps/root/include -I${ZROOT}/root/include -L${ZROOT}/deps/root/lib -L${ZROOT}/root/lib

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <regex.h>
#include <signal.h>


#include <sys/types.h>
// #include <dlfcn.h>

#include <iostream>
#include <cstring>
#include <string>
#include <openabe/openabe.h>
#include <openabe/zsymcrypto.h>
using namespace oabe;
using namespace oabe::crypto;
using namespace std;


std::string mpkparam = "AAAAFqpvyQeGOyICbZ29rFw+6s74bZxtcGsAAAHToQFBsgEEtLIBABYrNiM2C7hfU2U8ZVwX+GVM0OdcBuIj4hIjLPC2twfmCYVok4A7jzxbROZjaFu82qmRdY5IdDAmDFJd3zeKYKMJQYNyiN/Hx0fLT/7iSc5+1n/V+iTl+Y8laxbOY1kxqRz+1qcfSUCx0MJO05dz2hZoZAh+e6ZN5cWyut7fo+wTBAqmuS3Q1+ordLPzgfSeFChxwYo/UhDafl25cGEADvoVtnIqR8/8FAx2co1DoCz5PrVRkdii5D5Yxb5u/xpIzRF6ZyAR4rca1AUplr9bIRwXjmn8yZN6hHdFyZ9C6BCCEDh2+rW0HleGUmOxeYqAiBH2vkelLUZGbuB8FWY84MihAmcxoSSyoSECI7LiNuFqS8IDSCcFwqmWmneJdAD+7UtCBJZinvXdUbmhA2cxYaEksqEhAw2Z7n15ftNpsfjR5HsaJ1f9nEyITBLBw3BuNvpbjCVsoQJnMqFEs6FBAwIINf5u9ptuuMuZbmdq39aDAkrkJ02NEJEzWJOSZjOXCCPYGra1N4jUyz0dGgLUbK/MQAGuYVdSGDrZLHZVtTShAWuhJR0AAAAgWRCwJQBcFtxFRaR4eX7GFeVQlybDT21xQYPGsYGSasg=";

std::string mskparam = "AAAAFqpvyQC/De1L2WQAGcniiKHyeFVtc2sAAAB3oQVhbHBoYaEjsQAgCojxJbZh+cmJ85vRN8B9QptoCNO9tOfgKt/sII2uThKhA2cyYaFEs6FBAgnwkJIGhHv1yhBE+xqtogqC2ReeKRYHTK3SxLYM+n0EI82+iksjJ5verfLYITflKaNNmx7bMx7ZeNvs+H40IwM=";



/*
 *  Simple HTTP Proxy for GET requests
 *
 *  Description of program:
 *
 *  A simple multi-threaded HTTP Proxy and cache. The main thread waits for incoming connections
 *  on port 8888 using accept(). Once a connection is made, a new thread is dispatched to handle
 *  the request. A binary-tree cache is used to store the result of GET requests. If a resource 
 *  can be found in the cache, no requests to external servers are made. If the resource cannot
 *  be located, then a new outbound GET request is performed and the result is sent back to the
 *  client. Threading is achieved using POSIX threads, and a mutex is used to protect the cache
 *  from threading errors.
 *
 *  NOTE: No caps are placed on cache size, # of threads or # of open sockets.
 *
 *  This program has been tested on Mac OSX 10.8, and Ubuntu 12.04 LTS.
 *  Compile with: gcc simplehttp.c -lpthread
 */

/**
 *  @brief Destroys all global variables during shutdown
 */
void cleanup_globals();

/**
 *  @brief System signal handler for SIGINT
 *  @param signum Constant (only handles SIGINT)
 */
void signal_handler(int signum);

/**
 *  @brief Threadable function to handle an incoming request
 *  @param param Pointer to client_state object
 *  @return Always returns 0
 */
void *request_handler(void *param);

/**
 *  @brief Send a GET request for a resource at a foreign server
 *  @param hostname Hostname of destination server
 *  @param request Full body of HTTP request, terminated by a 0
 *  @param length Pointer to size_t, contains size of data returned by the method
 *  @return Returns data downloaded, or 0 on error
 */
void *make_request(char *hostname, char *request, char *resourceName, size_t *length,ssize_t *bufferlength,size_t *indexlength);

/**
 *  @brief Debug mode regcomp
 *  @param ALL Parameters equivalent to regcomp(), except errors are written to stdout
 */
int regcomp_d(regex_t *__restrict, const char *__restrict, int);

/**
 *  @struct client_state
 *  @brief Represents a connected client
 */
struct client_state
{
    int fd;           /**<  socket for the client */
    pthread_t thread; /**<  thread the client is being handled on */
};

/**
 *  @struct cache
 *  @brief Represents an entry in the proxy cache
 */
struct cache
{
    char *resourceName;  /**<  name of the resource in the cache (complete URL) */
    void *data;          /**<  data downloaded from that object */
    size_t dataLength;   /**<  length of data */
    struct cache *left;  /**<  left branch in cache tree */
    struct cache *right; /**<  right branch in cache tree */
};

/**
 *  @brief Creates a new instance of cache entry
 *  @param name Name of resource (http://mysite.com/mycoolfile.html)
 *  @param data Downloaded data
 *  @param length Length of memory pointed to by 'data'
 *  @return Pointer to new struct cache
 */
struct cache *cache_init(char *name, void *data, size_t length);

/**
 *  @brief Append an entry to the cache tree
 *  @param tree Root node of the cache tree
 *  @param entry The entry to append
 */
void cache_append(struct cache *tree, struct cache *entry);

/**
 *  @brief Search for data in the cache
 *  @param tree Root node of the cache tree
 *  @param name Name of the resource
 *  @param length Pointer to size_t, contains the length of the data found
 *  @return Pointer to data, if it exists. 0 otherwise
 */
void *cache_get_data(struct cache *tree, char *name, size_t *length);

/**
 *  @brief Recursively destroys cache entry
 *  @param entry Pointer to top of cache tree
 */
void cache_free(struct cache *entry);

/*
 *  Global variables
 */
int listenSocket = 0;          //  server listener socket
struct cache *serverCache = 0; //  root node of the cache
pthread_mutex_t cacheMutex;    //  mutex to protect the cache

int main(int argc, const char *argv[])
{
    //  create a socket to listen for incoming IPv4 TCP/IP connections
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listenSocket < 0)
    {
        printf("Unable to create a socket to listen on, exiting\n");
        return 1;
    }

    struct sockaddr_in socketAddress;
    memset(&socketAddress, 0, sizeof(socketAddress));

    //  we want a socket for TCP/IP, port 8888 on any incoming interface
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(8888);
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    //  bind the socket to the incoming port
    if (bind(listenSocket, (const struct sockaddr *)&socketAddress, sizeof(socketAddress)) < 0)
    {
        printf("Unable to bind to port 8888, exiting\n");
        return 1;
    }

    //  listen indicates that this is a passive socket
    //  SOMAXCONN indicates we will allow the maximum number of incoming connections to queue
    if (listen(listenSocket, SOMAXCONN) < 0)
    {
        printf("Unable to listen to socket, exiting\n");
        return 1;
    }

    printf("Accepting client connections...\n");
    signal(SIGINT, signal_handler); //  handle Ctrl-C on command line

    //  initialize cache mutex
    pthread_mutex_init(&cacheMutex, 0);

    //  now we wait indefinitely for incoming connection requests
    while (1)
    {
        socklen_t socketLength = sizeof(socketAddress);
        int client = accept(listenSocket, (struct sockaddr *)&socketAddress, &socketLength);
        if (client < 0)
        {
            printf("Error while accepting client, exiting\n");
            break;
        }

        //  retrieve the IP address of the client
        char ipBuffer[INET_ADDRSTRLEN];
        const char *ptr = inet_ntop(socketAddress.sin_family, &socketAddress.sin_addr, ipBuffer, sizeof(ipBuffer)); //#include <arpa/inet.h>  const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);

        //  DNS lookup to obtain the client hostname, if possible
        char hostname[255];
        if (!getnameinfo((struct sockaddr *)&socketAddress, sizeof(socketAddress), hostname, sizeof(hostname), 0, 0, NI_NAMEREQD))
        {
            printf("Incoming connection from %s [%s]\n", hostname, ipBuffer);
        }
        else
        {
            printf("Incoming connection from %s\n", ptr);
        }

        /*
         *  Create new thread to handle this request.
         *  Note: We do not cap the # of threads in this example - a real implementation would draw them from a fixed-size pool.
         */
        struct client_state *state = (client_state *)malloc(sizeof(struct client_state));
        state->fd = client;

        pthread_attr_t attributes;
        pthread_attr_init(&attributes);
        int status = pthread_create(&state->thread, &attributes, request_handler, state);
        pthread_attr_destroy(&attributes);

        //  failed thread, just drop the client
        if (status != 0)
        {
            printf("Failed to create handler thread, dropping client\n");
            close(client);
            continue;
        }

        //  thread stack can be destroyed when the thread exits, we cannot use 'join' after this point
        pthread_detach(state->thread);
    }

    cleanup_globals();
    return 0;
}

void cleanup_globals()
{
    //  destroy all global variables
    pthread_mutex_lock(&cacheMutex);
    cache_free(serverCache);
    serverCache = 0;
    pthread_mutex_unlock(&cacheMutex);

    pthread_mutex_destroy(&cacheMutex);
    close(listenSocket);
}

void signal_handler(int signum)
{
    switch (signum)
    {
    case SIGINT:
        cleanup_globals();
        exit(0);
    default:
        break;
    }
}

void send_and_close(struct client_state *state, void *data, size_t dataLength)
{
    send(state->fd, data, dataLength, 0);
    close(state->fd);
    free(state);
}

void extract_match(char *dest, const char *src, regmatch_t match)
{
    regoff_t o;
    for (o = 0; o < match.rm_eo - match.rm_so; o++)
    {
        dest[o] = src[o + match.rm_so];
    }
    dest[o] = 0;
}

void *request_handler(void *param)
{
    char *internalError = "HTTP/1.1 500 Internal Error\r\n";
    char *badRequest = "HTTP/1.1 400 Bad Request\r\n";
    char *notFound = "HTTP/1.1 404 Not Found\r\n";

    struct client_state *state = (struct client_state *)param;
    printf("Handler for client: %i\n", state->fd);

    //  configure safe timeout values
    struct timeval timeout;
    timeout.tv_sec = 5;  //seconds
    timeout.tv_usec = 0;  //microseconds
    setsockopt(state->fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(state->fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    //  read the request header, allowing up to 8kB in the request
    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    size_t count = recv(state->fd, buffer, sizeof(buffer) - 1, 0); //  leave space for 0 terminator
   
    if (count <= 0)
    {
        printf("Client disconnected (%zu)\n", count);
        close(state->fd);
        free(state);
        return 0;
    }
    

    //  this regexp will match a GET request
    regex_t reg;
    if (regcomp_d(&reg, "GET +([^ \t]+) +HTTP/[0-9.]+ *[\r|\n]", REG_ICASE | REG_EXTENDED))
    {
        send_and_close(state, internalError, strlen(internalError));
        return 0;
    }

    //  find a single match
    regmatch_t matches[2];
    regexec(&reg, buffer, 2, matches, 0);
    regfree(&reg);

    if (matches[0].rm_so != 0 || matches[0].rm_eo == 0)
    {
        printf("Invalid GET request:\n%s\nDropping client\n", buffer);
        send_and_close(state, badRequest, strlen(badRequest));
        return 0;
    }

    //  this is a get request, let's pull out the target address
    char requestTarget[8192];
    extract_match(requestTarget, buffer, matches[1]);
    printf("Processing request: %s\n", requestTarget); //  http://192.168.204.135:6379/

    //  hit the cache for the resource
    size_t dataLength;
    void *data;
    string ct, pt2, pt;

    //  we use a mutex to ensure threaded access is synchronized
    pthread_mutex_lock(&cacheMutex);
    data = cache_get_data(serverCache, requestTarget, &dataLength);
    pthread_mutex_unlock(&cacheMutex);

    //  cache contained no resource, so we lookup the host and make a request
    if (data == 0 || dataLength == 0)
    {
        //  a regexp to find the hostname
        if (regcomp_d(&reg, "Host:[ \t]+([^ \t\r\n]+)[\r|\n]", REG_ICASE | REG_EXTENDED))
        { //(https?://)+([^ /]+)
            send_and_close(state, internalError, strlen(internalError));
            return 0;
        }

        regexec(&reg, buffer, 2, matches, 0);
        regfree(&reg);
        if (matches[0].rm_eo == 0)
        {
            printf("Malformed request, dropping client\n");
            send_and_close(state, badRequest, strlen(badRequest));
            return 0;
        }

        //  extract the hostname
        char hostname[8192];
        extract_match(hostname, buffer, matches[1]);

        char hostname_ip[8192];
        regoff_t i;
        for (i = 0; hostname[i] != ':'; i++)
        {
            hostname_ip[i] = hostname[i];
        }
        hostname_ip[i] = '\0';
 
        // printf("Connecting to host: %s\n", hostname); //  192.168.204.135:6379
        printf("Connecting to host: %s\n", hostname_ip);
        
        // 
        ssize_t index[10000];//array to save read buffer length
        size_t ilength;
        size_t recvlength;          

        // data = make_request(hostname, buffer, requestTarget, &dataLength);  //buffer: full request   data: response
       void *recvdata = make_request(hostname_ip, buffer, requestTarget, &recvlength,index,&ilength);
        
    //decrypt
    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
//    string ct, pt2, pt;
    std::string mpk, msk;
    mpk = mpkparam;
    msk = mskparam;
    cpabe.importPublicParams(mpk);
    cpabe.importSecretParams(msk);
    cpabe.keygen("|attr1|attr2", "key0");
   
    ssize_t temp = 0;
    string cipher =(char *)recvdata ;
    
    for(size_t j=0;j<=ilength;j++){
//       cout<<"index[j]:"<<index[j]<<endl;
//       memcpy(cipher+temp,(char *)data ,index[j]);
       ct = cipher.substr(0 + temp, index[j]);
       temp += (index[j]);
       cpabe.decrypt("key0", ct, pt2);      
       pt += pt2;

    }        
    data = (void *)pt.c_str();
    dataLength = pt.length();
   
        cout<<"request_handler make_request dataLength:"<<dataLength<<endl;

        cout << endl<< "request_handler data:" << endl << (char *)data << endl;

        if (data == 0)
        {
            //  couldn't load resource, mark this as a 404
            send_and_close(state, notFound, strlen(notFound));
            return 0;
        }

        //  Cache the data we just downloaded for future requests
        pthread_mutex_lock(&cacheMutex);

        printf("Caching resource for %s\n", requestTarget);
        struct cache *entry = cache_init(requestTarget, data, dataLength);
        
        cout<<"data2:"<<endl<<(char *)data<<endl;
        //  Cache has not been initialized
        if (!serverCache)
        {
            serverCache = entry;
        }
        else
        {
            //  Append
            cache_append(serverCache, entry);
        }

        pthread_mutex_unlock(&cacheMutex);
    }
    else
    {
        printf("Using cached resource for %s\n", requestTarget);
    }
    cout<<"data3:"<<endl<<(char *)data<<endl;
    //  we downloaded some data, send it back to the client and finish
    write(state->fd, data, dataLength);
  
    ShutdownOpenABE();
    close(state->fd);
    free(state);
    
    return 0;
}

void *make_request(char *hostname, char *request, char *resourceName, size_t *length,ssize_t *bufferlength,size_t *indexlength)
{
    //  resolve the hostname to an IP
    struct addrinfo hints; //  ai_family, ai_socktype, ai_protocol
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result = 0;
    // if (getaddrinfo(hostname, "80", &hints, &result) != 0)
    if (getaddrinfo(hostname, "6379", &hints, &result) != 0)
    {
        printf("Unable to resolve hostname %s\n", hostname);
        return 0;
    }

    //  we assume first result is a TCP/IPv4 address
    int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == 0)
    {
        printf("Unable to open a socket\n");
        freeaddrinfo(result);
        return 0;
    }

    long t = time(0);

    //  connect to server
    if (connect(sock, result->ai_addr, result->ai_addrlen) != 0)
    {
        printf("Unable to connect to %s\n", hostname);
        close(sock);
        freeaddrinfo(result);
        return 0;
    }
    freeaddrinfo(result);

    printf("Time to connect: %lis\n", time(0) - t);

    
    //encrypt request before send it to server
    InitializeOpenABE();
    OpenABECryptoContext cpabe("CP-ABE");
    string ct, pt1;//, pt2,pt ;
    std::string mpk, msk;
    mpk = mpkparam;
    cpabe.importPublicParams(mpk);
    pt1 = request;
    cpabe.encrypt("attr1 and attr2", pt1, ct);
    char *cipher_request = (char *)ct.c_str();
    

    //send to server ???
    //  write the request
   // if (write(sock, request, strlen(request)) != strlen(request))
    if (write(sock, cipher_request, strlen(cipher_request)) != strlen(cipher_request))
    {
        printf("Failed to write request\n");
        close(sock);
        return 0;
    }

    // cout << endl<< "make_request write request:" << endl<< request << endl;
   cout<<endl<<"make_request write cipher_request:"<<endl<<cipher_request<<endl;

    char buffer[8192]; //  storage for read() operations
    void *data = 0;    //  final data downloaded
    size_t count = 0; 
   // char socket_buffer[8192];

    //  we use a polling time of 500ms for any read events
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.events = POLLIN | POLLRDNORM | POLLRDBAND;
    pfd.fd = sock;

    //  while we can read...
    printf("Downloading...\n");
    ssize_t index[10000]; //array to save readLength
    size_t i = 0;         
   // ssize_t sumL = 0;
   
    while (poll(&pfd, 1, 500) > 0)
    {
      
        ssize_t L = read(sock, buffer, sizeof(buffer));
   /*     cout<<endl<<"make_request read buffer:"<<endl<<buffer<<endl;
        cout<<"make_request strlen(buffer):"<<strlen(buffer)<<endl;
        cout<<"make_request L:"<<L<<endl;
   */   

   /*     //decrypt
        ssize_t readLength = read(sock, socket_buffer, sizeof(socket_buffer));
        cout<<"make_request read socket_buffer:"<<endl<<socket_buffer<<endl;
        
        msk = mskparam;
        ct = (char *)socket_buffer;
        cpabe.importSecretParams(msk);
        cpabe.keygen("|attr1|attr2", "key0");
        cpabe.decrypt("key0", ct, pt2);
        char *buffer = (char *)pt2.c_str(); 
        ssize_t L = (ssize_t)(pt2.length());
        cout<<"make_request read length:"<<readLength<<endl;
        cout<<"plaintext length:"<<L<<endl;
        cout<<"plaintext:"<<endl<<pt2<<endl;
       */        

       if (L > 0)
        {
            if (!data)
            {
                data = malloc((unsigned long)L);
            }
            else
            {
                data = realloc(data, count + L);
            }

            // memcpy(&data[count], buffer, L);
            memcpy((char *)data + count, buffer, L);
            
            count += (size_t)L;

            index[i] = L;
            i++;          
        }   
        else
            break;
    }
    *indexlength = (i-1);
//    memcpy(bufferlength,index,i);  
    for(size_t j=0;j<i;j++){
       bufferlength[j]=index[j];
  //     cout<<"bufferlength:"<<bufferlength[j]<<endl;  
    }
/*    
    cout<<endl<<"cipher:"<<endl<<(char *)data<<endl;
    cout<<endl<<"count:"<<count<<endl;    
    
    ssize_t temp = 0;
    string cipher =(char *)data ;
    
    for(ssize_t j=0;j<i;j++){
       cout<<"index[j]:"<<index[j]<<endl;
//       memcpy(cipher+temp,(char *)data ,index[j]);
       ct = cipher.substr(0 + temp, index[j]);
       temp += (index[j]);
       
       msk = mskparam;
       cpabe.importSecretParams(msk);
       cpabe.keygen("|attr1|attr2", "key0");
       cpabe.decrypt("key0", ct, pt2);      
       pt += pt2;

    }
    cout<<endl<<"pt:"<<endl<<pt<<endl;
    size_t len = (size_t)(pt.length());
    cout<<endl<<"len:"<<len<<endl;
       
   void *plaintext ;
   char plain[10000];
//    char *plaintext;
    memcpy(plain,pt.c_str(),len);
//    memcpy(plaintext+0,plain,len);
    cout<<endl<<"plain:"<<endl<<plain<<endl;
    memcpy((char *)plaintext,plain,len);
*/    
//    cout<<endl<<"plaintext:"<<endl<<(char *)plaintext<<endl;
    

    ShutdownOpenABE();
    //  cleanup socket
    close(sock);
    printf("Downloaded %zu bytes from %s\n", count, resourceName);

/*
       //decrypt
         // InitializeOpenABE();
         // OpenABECryptoContext cpabe("CP-ABE");
         // string ct, pt1, pt2;
         // std::string mpk, msk;
         // mpk = mpkparam;
         msk = mskparam;
         ct = (char *)data;

         // cpabe.importPublicParams(mpk);
         cpabe.importSecretParams(msk);
         cpabe.keygen("|attr1|attr2", "key0");
         cpabe.decrypt("key0", ct, pt2);
         char *plaintext_data = (char *)pt2.c_str();
         data = plaintext_data;
*/       

        //  done
     *length = count;
  //  *length = len;
    
   // cout << endl<< "make_request data:" << endl<< (char *)data << endl;
//    cout<<endl<<"plaintext:"<<endl<<(char *)response<<endl;    
       
     return data;
//    return plaintext;
}

//  dumps errors to the console
int regcomp_d(regex_t *__restrict arg0, const char *__restrict arg1, int arg2)
{
    int error = regcomp(arg0, arg1, arg2);
    if (error != 0)
    {
        char errorBuffer[255];
        regerror(error, arg0, errorBuffer, sizeof(errorBuffer));
        printf("REGEX error: %s\n", errorBuffer);
    }
    return error;
}

struct cache *cache_init(char *name, void *data, size_t length) //  assume these parameters are non-nil
{
    struct cache *C = (cache *)malloc(sizeof(struct cache));
    C->data = data;
    C->dataLength = length;
    C->left = 0;
    C->right = 0;

    //  copy name
    C->resourceName = (char *)malloc(strlen(name));
    memcpy(C->resourceName, name, strlen(name));

    return C;
}

void cache_append(struct cache *tree, struct cache *entry)
{
    //  We try to append 'entry' to 'tree', or its children

    int compare = strcmp(tree->resourceName, entry->resourceName);
    if (compare > 0)
    {
        if (tree->right != 0)
        {
            //  need to keep going right
            cache_append(tree->right, entry);
        }
        else
        {
            //  found an empty space for this entry
            tree->right = entry;
        }
    }
    else
    {
        //  need to keep going left
        if (tree->left != 0)
        {
            cache_append(tree->left, entry);
        }
        else
        {
            tree->left = entry;
        }
    }

    //  for now we ignore the case where the entry is already in the cache
}

void *cache_get_data(struct cache *tree, char *name, size_t *length)
{
    //  go down the tree, looking for an entry that matches name

    if (tree == 0)
    {
        *length = 0;
        return 0;
    }

    int compare = strcmp(tree->resourceName, name);
    if (compare == 0)
    {
        //  found data
        *length = tree->dataLength;
        return tree->data;
    }
    else if (compare > 0)
    {
        //  branch right
        return cache_get_data(tree->right, name, length);
    }

    //  branch left
    return cache_get_data(tree->left, name, length);
}

void cache_free(struct cache *entry)
{
    //  destroy all resources
    if (entry == 0)
        return;

    cache_free(entry->left);
    cache_free(entry->right);

    free(entry->resourceName);
    free(entry->data);

    free(entry);
}

