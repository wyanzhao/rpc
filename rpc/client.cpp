#include "client.h"

rpc::client::client(boost::asio::io_service & io_service):
	io_service_(io_service),
	socket_(io_service),
	recv_buf_(""),
	if_connected_ (false),
	len_ (0)
{
}

rpc::client::~client()
{
	io_service_.stop();
	socket_.close();
}

bool rpc::client::connect(const std::string & addr, short port)
{
	auto ret = true;

	if (!if_connected_)
	{
		try
		{
			tcp::endpoint e(boost::asio::ip::address::from_string(addr), port);
			socket_.connect(e);

			if_connected_ = true;
			ret = true;
		}
		
		catch (const boost::system::system_error& ec)
		{
			std::cerr << ec.what() << std::endl;
			ret = false;
		}
	}

	return ret;
}

std::string rpc::client::get_async_data()
{
	return recv_buf_;
}





