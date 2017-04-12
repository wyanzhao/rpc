#include "client.h"

rpc::client::client(boost::asio::io_service & io_service):
	io_service_(io_service),
	socket_(io_service)
{

}

void rpc::client::connect(const std::string & addr, short port)
{
	tcp::endpoint e(boost::asio::ip::address::from_string(addr), port);
	socket_.connect(e);
}

std::string rpc::client::call(const std::string & serial_str)
{
	u_int len = (u_int)serial_str.length();

	std::vector<boost::asio::const_buffer> message;
	message.push_back(boost::asio::buffer(&len, 4));
	message.push_back(boost::asio::buffer(serial_str));
	socket_.send(message);

	message.clear();
	socket_.receive(boost::asio::buffer(&len, 4));
	std::string recv_json;
	recv_json.resize(len);
	socket_.receive(boost::asio::buffer(&recv_json[0], len));
	return recv_json;
}
