/**
* Project ISA
* Client implementation
* Author: Kozhevnikov Dmitrii
* Login: xkozhe00
*/

#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>
#include <iostream>
#include <stddef.h>
#include <cstdlib>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>

#define IPADDR "127.0.0.1"
#define PORT 32323
#define BUFSIZE 100000

using namespace std;

// structure for storing input data
struct CLI {
    string login;
    string password;
    string recipient;
    string subject;
    string body;
    string id;
    string command;
};

// base64 table
static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
* base64_encode - Base64 encode
* @src: Data to be encoded
* @len: Length of the data to be encoded
* @out_len: Pointer to output length variable, or %NULL if not used
* Returns: Allocated buffer of out_len bytes of encoded data,
* or empty string on failure
* 
* Function from https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
*/
string base64encode(const unsigned char *src, size_t len) {
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4*((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return std::string(); /* integer overflow */

    string outStr;
    outStr.resize(olen);
    out = (unsigned char*)&outStr[0];

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}

/**
* error - Print errors and exit from program
* @message: The text of the error message
* @exitCode: Exit code from the program
*/
void error(const char *message, int exitCode) {
    cerr << message << endl;
    exit(exitCode);
}

/**
* generateMessage - Generating a message from the client to the server
* @cli: Structure with data that the client will send to the server
* Returns: a string to be passed as a message to the server
*/
string generateMessage(struct CLI *cli) {
    string encodedPassword;                                                                                           // user password

    // command "login" and "register"
    if (!cli->command.compare("register") || !cli->command.compare("login")) {
        int passwordLength = 0;
        encodedPassword = base64encode((const unsigned char *)cli->password.c_str(), cli->password.length());
        return "(" + cli->command + " \"" + cli->login + "\" \"" + encodedPassword + "\"" + ")";

    // command "logout" 
    } else if (!cli->command.compare("logout")) {
        ifstream file("token-login");
        if (!file.is_open()) {
            cout << "Not logged in" << endl;
            exit(0);
        }
        getline(file, encodedPassword); 
        return "(" + cli->command + encodedPassword + " )";

    // command "fetch"
    } else if (!cli->command.compare("fetch")) {
        ifstream file("token-login");
        if (!file.is_open()) {
            cout << "Not logged in" << endl;
            exit(0);
        }
        getline(file, encodedPassword); 
        return "(" + cli->command + " " + encodedPassword + " " + cli->id + ")";

    // command "send"
    } else if (!cli->command.compare("send")) {
        ifstream file("token-login");
        if (!file.is_open()) {
            cout << "Not logged in" << endl;
            exit(0);
        }
        getline(file, encodedPassword); 
        return "(" + cli->command + " " + encodedPassword + " \"" + cli->recipient + "\" \"" + cli->subject + "\" \"" + cli->body + "\")";

    // command "list"
    } else if (!cli->command.compare("list")) {
        ifstream file("token-login");
        if (!file.is_open()) {
            cout << "Not logged in" << endl;
            exit(0);
        }
        getline(file, encodedPassword); 
        return "(" + cli->command + " " + encodedPassword + ")";
    }
    return "";
}

/**
* connectionToServer - creating a connection between the client and the server and further processing the response from the server
* @portNumber: Port number
* @address: Input address 
* @cli: Structure with data that the client will send to the server
* Returns: 0 - if the response was successful, 2 - in case of an incorrect response from the server
*/
int connectionToServer (int portNumber, string address, CLI cli) {
    int sockfd, connfd;
	struct sockaddr_in servAddr;
	struct sockaddr_in6 servAddr6;

    // ipv4
    if (inet_pton(AF_INET, address.c_str(), &(servAddr.sin_addr)) > 0) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
            error("ERROR: bad socket\n", 3);
        }

        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(portNumber);

        if (connect(sockfd, (const struct sockaddr *) &servAddr, sizeof(servAddr))) {
            error("ERROR: bad connect\n", 3);
        }

    // ipv6
    } else if (inet_pton(AF_INET6, address.c_str(), &(servAddr6.sin6_addr)) > 0) {
        if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) <= 0) {
            error("ERROR: bad socket\n", 3);
        }

        servAddr6.sin6_family = AF_INET6;
        servAddr6.sin6_port = htons(portNumber);

        if (connect(sockfd, (const struct sockaddr *) &servAddr6, sizeof(servAddr6))) {
            error("ERROR: bad connect\n", 3);
        }
    } else {
        cerr << "tcp-connect: host not found" << endl << "hostname: " << address << endl << "port: " << portNumber << endl;
    }
    
    char buffer[BUFSIZE];
    bzero(buffer, sizeof(buffer));
    bzero(&servAddr, sizeof(servAddr));
    bzero(&servAddr6, sizeof(servAddr6));

    string clientMessage = generateMessage(&cli);   // create message
    //cout << "Client message: " << clientMessage << endl;

    // message send
    int bytestx = send(sockfd, clientMessage.c_str(), clientMessage.length(), 0);
    if (bytestx < 0) {
        error("ERROR in send", 2);
    }

    // response from server
    int bytesrx = recv(sockfd, buffer, BUFSIZE, 0);

    // processing the response from the server
    if (bytesrx < 0) {
        error("ERROR in recv", 2);
    } else {
        string odpoved = buffer;

        // command "login" or "register"
        if (!cli.command.compare("register") || !cli.command.compare("login")) {

            if (odpoved.find("ok") == 1){
                cout << "SUCCESS: ";
            } else {
                cout << "ERROR: ";
            }
            odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
            cout << odpoved.substr(0, odpoved.find("\"")) << endl;
            if (!cli.command.compare("login")) {
                ofstream file;
                file.open("token-login");
                file << odpoved.erase(0, odpoved.find("\"") + 2).substr(0, odpoved.length() - 1);
                file.close();
            }

        // command "logout"
        } else if (!cli.command.compare("logout")) {
            if (odpoved.find("ok") == 1){
                cout << "SUCCESS: ";
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                cout << odpoved.substr(0, odpoved.find("\"")) << endl;
                remove("token-login");
            } else {
                cout << "ERROR: ";
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                cout << odpoved.substr(0, odpoved.find("\"")) << endl;
            } 

        // command "fetch"
        } else if (!cli.command.compare("fetch")) {
            if (odpoved.find("ok") == 1) {
                cout << "SUCCESS:" << endl << endl << "From: ";
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                string fromUser = odpoved.substr(0, odpoved.find("\""));
                cout << fromUser << endl;
                
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                string sub = odpoved.substr(0, odpoved.find("\""));
                cout << "Subject:" << sub << endl << endl;
                
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                string mes = odpoved.substr(0, odpoved.find("\""));
                cout << mes << endl;
                
            } else {
                cout << "ERROR: ";
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                cout << odpoved.substr(0, odpoved.find("\"")) << endl;
            }

        // command"send    
        } else if (!cli.command.compare("send")) {
            if (odpoved.find("ok") == 1){
                cout << "SUCCESS: ";
            } else {
                cout << "ERROR: ";
            }
            odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
            cout << odpoved.substr(0, odpoved.find("\"")) << endl;

        // command "list"
        } else if (!cli.command.compare("list")) {
            if (odpoved.find("ok") == 1){
                int count = 1;
                cout << "SUCCESS: " << endl;
                
                while (1) {
                    odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                    string fromUser = odpoved.substr(0, odpoved.find("\""));

                    if (fromUser == ")))" || fromUser == "(ok ())") {
                        break;
                    }
                    cout << count << ":" << endl;
                    cout << "  From: " << fromUser << endl; 

                    odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                    odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                    string sub = odpoved.substr(0, odpoved.find("\""));
                    cout << "  Subject: " << sub << endl;
                    count++;
                    odpoved = odpoved.erase(0, odpoved.find("\"") + 1);

                }
            } else {
                cout << "ERROR: ";
                odpoved = odpoved.erase(0, odpoved.find("\"") + 1);
                cout << odpoved.substr(0, odpoved.find("\"")) << endl;
            }
        }
    }
    //printf("Message from server: %s\n", buffer);

    close(sockfd);
    return 0;
}

/**
* main - Main function; processes incoming arguments for correctness. In case of correct input - call the necessary command
*/
int main(int argc, char *argv[]) {

    if (argc == 1) {
        printf("client: expects <command> [<args>] ... on the command line, given 0 arguments\n");
        return 0;
    }
    
    int c;
    // start arguments control structure
    int optIdx;
    int port = PORT;
    string address = IPADDR;
    int counter = 0;
    while(1) {
        char *portString = NULL;
        counter++;
        static struct option long_opt[] = {
        {"help", no_argument, 0, 'h'},
        {"port", 1, 0, 'p'},
        {"address", 1, 0, 'a'},
        {0,0,0,0},
        };

        c = getopt_long (argc, argv, "hp:a:", long_opt, &optIdx);

        if (c == -1) {
            break;
        }

        switch(c){
            case 0:
                printf("unknown argument: %s", long_opt[optIdx].name);
                return 1;
                
            case 'h':
                cout << ("usage: client [ <option> ... ] <command> [<args>] ... \n<option> is one of \n\n\t-a <addr>, --address <addr>\n\t\tServer hostname or address to connect to\n\t-p <port>, --port <port>\n\t\tServer port to connect to\n\t--help, -h\n\t\tShow this help\n\t--\n\t\tDo not treat any remaining argument as a switch (at this level)\n\nMultiple single-letter switches can be combined after\none `-`. For example, `-h-` is the same as `-h --`.\nSupported commands:\n\tregister <username> <password>\n\tlogin <username> <password>\n\tlist\n\tsend <recipient> <subject> <body>\n\tfetch <id>\n\tlogout\n") << endl;
                exit(0);
            case 'p':
                counter++;
                port = strtol(optarg, &portString, 10);

                if (*portString) {
                    error("Port number is not a string", 1);
                }
                break;
            case 'a':
                counter++;
                address = optarg;
                break;
            case '?':
                error("unknown argument", 1);
            default:
                error("unknown argument", 1); 
        }
    }

    int fetchId = 0;
    struct CLI cli;
    if (argv[counter] == NULL) {
        cout << "client: expects <command> [<args>] ... on the command line, given 0 arguments" << endl;
        exit(0);
    } else {
        while (argv[counter] != NULL) {
        
            if (!strcmp(argv[counter], "list")) {
                counter++;
                if (argv[counter] != NULL) {
                  cout << "list" << endl;
                  exit(2);  
                } else {
                    cli.command = "list";
                    connectionToServer(port, address, cli);
                }
            } else if (!strcmp(argv[counter], "logout")) {
                counter++;
                if (argv[counter] != NULL) {
                  cout << "logout" << endl;
                  exit(2);  
                } 
                cli.command = "logout";
                connectionToServer(port, address, cli);
                
            } else if (!strcmp(argv[counter], "fetch")) {
                counter++;
                if (argv[counter] == NULL) {
                    cout << "fetch <id>" << endl;
                    exit(2);
                }
                cli.command = "fetch";
                char *pEnd;
                fetchId = strtol(argv[counter], &pEnd, 10);
                if (*pEnd) {
                    error("ERROR: id as is not a number", 1);
                }
                cli.id = argv[counter];
                counter++;
                if (argv[counter] != NULL) {
                    cout << "fetch <id>" << endl;
                    exit(2);
                }
                connectionToServer(port, address, cli);
            } else if (!strcmp(argv[counter], "login")) {
                counter++;
                if (argv[counter] == NULL) {
                    cout << "login <username> <password>" << endl;
                    exit(2);
                }
                cli.command = "login";
                cli.login = argv[counter];
                counter++;
                if (argv[counter] == NULL) {
                    cout << "login <username> <password>" << endl;
                    exit(2);
                }
                cli.password = argv[counter];
                counter++;
                if (argv[counter] != NULL) {
                    cout << "login <username> <password>" << endl;
                    exit(2);
                }
                connectionToServer(port, address, cli);
            } else if (!strcmp(argv[counter], "register")) {
                counter++;
                if (argv[counter] == NULL) {
                    cout << "register <username> <password>" << endl;
                    exit(2);
                }
                cli.command = "register";
                cli.login = argv[counter];
                counter++;
                if (argv[counter] == NULL) {
                    cout << "register <username> <password>" << endl;
                    exit(2);
                }
                cli.password = argv[counter];
                counter++;
                if (argv[counter] != NULL) {
                    cout << "register <username> <password>" << endl;
                    exit(2);
                }
                connectionToServer(port, address, cli);
            } else if (!strcmp(argv[counter], "send")) {
                counter++;
                if (argv[counter] == NULL) {
                    cout << "send <recipient> <subject> <body>" << endl;
                    exit(2);
                }
                cli.command = "send";
                cli.recipient = argv[counter];
                counter++;
                if (argv[counter] == NULL) {
                    cout << "send <recipient> <subject> <body>" << endl;
                    exit(2);
                }
                cli.subject = argv[counter];
                counter++;
                if (argv[counter] == NULL) {
                    cout << "send <recipient> <subject> <body>" << endl;
                    exit(2);
                }
                cli.body = argv[counter];
                counter++;
                if (argv[counter] != NULL) {
                    cout << "send <recipient> <subject> <body>" << endl;
                    exit(2);
                }
                connectionToServer(port, address, cli);
            } else {
                error("unknown command", 1);
            }
        }
    }
    return 0;
}