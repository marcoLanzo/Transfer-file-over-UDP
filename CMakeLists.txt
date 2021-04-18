cmake_minimum_required(VERSION 3.5)
project(IIW2 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")



add_executable(IIW2 Server/Server.c Server/ServerSender.h Server/timerlist.h)
add_executable(Server Server/Server.c)
add_executable(Client Client/Client.c Client/clientfileoperation.h Client/ClientReceiver.h Client/timeoutlist.h)