#include "auth_server.hpp"

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <iostream>

#include "auth_service_impl.hpp"

AuthServer::~AuthServer()
{
    if (server_)
    {
        server_->Shutdown();
    }
}

void AuthServer::Run(const std::string& server_address)
{
    AuthServiceImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    server_ = builder.BuildAndStart();
    if (!server_)
    {
        std::cerr << "Failed to start server on " << server_address << std::endl;
        return;
    }

    std::cout << "Server listening on " << server_address << std::endl;
    server_->Wait();
}