#include "../rpc/client.h"
#include <iostream>
#include <string>
#include <thread>
using namespace std;

void callback(const boost::system::error_code &err, std::size_t)
{
	cout << "hello\n";
}

void async()
{
	boost::asio::io_service io_service;
	std::string buf;
	buf.reserve(1024);

	rpc::client c(io_service);
	
	c.connect("127.0.0.1", 9002);

	buf = c.call("add", 1, 2);
	cout <<  c.get_result<int> (buf) << endl;

	
}

int main()
{
	async();
	getchar();
	return 0;
}