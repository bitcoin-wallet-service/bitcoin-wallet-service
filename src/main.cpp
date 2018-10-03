// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <btcnode.h>
#include <shutdown.h>
#include <utils.h>

#ifndef WIN32
#include <signal.h>
#endif

#include "fcgio.h"

#include <univalue.h>

static std::string DEFAULT_DB = "leveldb";

#ifndef WIN32
static void registerSignalHandler(int signal, void(*handler)(int))
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signal, &sa, nullptr);
}
#endif

static void HandleSIGTERM(int)
{
    requestShutdown();
}

int main(int argc, char* argv[])
{
    // parse arguments
    g_args.ParseParameters(argc, argv);

    InitLogging();

    // // create datadir if required
    // if (!isDir(GetDataDir()))
    // {
    //     fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", g_args.GetArg("-datadir", "").c_str());
    //     exit(1);
    // }
    // CreateDir(GetDataDir());
    // Backup the stdio streambufs
    std::streambuf * cin_streambuf  = std::cin.rdbuf();
    std::streambuf * cout_streambuf = std::cout.rdbuf();
    std::streambuf * cerr_streambuf = std::cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        std::cin.rdbuf(&cin_fcgi_streambuf);
        std::cout.rdbuf(&cout_fcgi_streambuf);
        std::cerr.rdbuf(&cerr_fcgi_streambuf);
        
        std::cout << "Content-type: text/html\r\n"
             << "\r\n"
             << "<html>\n"
             << "  <head>\n"
             << "    <title>Hello, World!</title>\n"
             << "  </head>\n"
             << "  <body>\n"
             << "    <h1>Hello, World4!</h1>\n"
             << "  </body>\n"
             << "</html>\n";
        
        char * content_length_str = FCGX_GetParam("CONTENT_LENGTH", request.envp);
        unsigned long content_length = strtol(content_length_str, &content_length_str, 10);
    
        char * content_buffer = new char[content_length];
        std::cin.read(content_buffer, content_length);
        
        UniValue uniRequest;
        uniRequest.read(content_buffer);
    
        std::cout << uniRequest.write();
        std::cout << "\n";
        break;
    }
    
    // restore stdio streambufs
    std::cin.rdbuf(cin_streambuf);
    std::cout.rdbuf(cout_streambuf);
    std::cerr.rdbuf(cerr_streambuf);
//
// #ifndef WIN32
//     // Clean shutdown on SIGTERM
//     registerSignalHandler(SIGTERM, HandleSIGTERM);
//     registerSignalHandler(SIGINT, HandleSIGTERM);
//
//     // Ignore SIGPIPE, otherwise it will bring the daemon down if the client closes unexpectedly
//     signal(SIGPIPE, SIG_IGN);
// #endif
}
