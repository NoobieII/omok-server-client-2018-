#include "server.hpp"

#include <iostream>

int main(){
	try{
		std::string ip, port;
		
		std::cout << "Enter the ip and port of the server as 2 strings:\n";
		std::cin >> ip >> port;
		
		Server::get_instance()->start_accept(ip, port);
		
		boost::asio::io_service &io = Server::get_instance()->get_io_service();
		
		for(;;){
			io.run();
		}
	}
	catch(std::exception &e){
		std::cerr << e.what() << "\n";
	}
	
	return 0;
}