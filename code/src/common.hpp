#ifndef COMMON_HPP
#define COMMON_HPP

#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include "codec.hpp"

using namespace std;

void ReadToEnd( fstream &f, string &s );

template<class T>inline bool getmax( T&a, const T&b );

template<class T>inline bool getmin( T&a, const T&b );

vector<int> select_write( const vector<int> &fds );

vector<int> select_read( const vector<int> &fds );

bool init_server( const int port, const int queueLen, int &server_fd );

bool send( const int fd, const void *buf, size_t len, bool &unexpected_error );

bool send_int( const int fd, const int v, bool &unexpected_error );

bool send_string( const int fd, const string &s, bool &unexpected_error );

bool receive( const int fd, void *buf, size_t len, bool &unexpected_error );

bool receive_int( const int fd, int &msg, bool &unexpected_error );

bool receive_string( const int fd, string &msg, bool &unexpected_error );

bool accept( const int sockfd, int &client_fd, uint32_t &client_ip, int &client_port );

bool try_connect( const int server_fd, int &error_number, const struct sockaddr_in &addr );

bool connect_to( const string &servername, const int port, int &server_fd, int &error_number, struct sockaddr_in &addr );

string ip_to_string( const uint32_t ip );

vector<string> split( const string &s, const char splitter );

#endif
