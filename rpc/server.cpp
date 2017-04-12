#include "server.h"

rpc::server::server(short port, size_t size, size_t timeout_milli):
	io_service_pool_(size), timeout_milli_(timeout_milli),
	acceptor_(io_service_pool_.get_io_service(), tcp::endpoint(tcp::v4(), port))
{
	do_accept();
}

rpc::server::~server()
{
	io_service_pool_.stop();
	thd_->join();
}

void rpc::server::run()
{
	thd_ = std::make_shared<std::thread>([this] {io_service_pool_.run(); });
}

void rpc::server::remove_handler(std::string const & name)
{
	router::get().remove_handler(name);
}

void rpc::server::do_accept()
{
	conn_.reset(new connection(io_service_pool_.get_io_service(), timeout_milli_));
	
	acceptor_.async_accept(conn_->socket(), [this](boost::system::error_code ec)
	{
		if (ec)
		{
			//todo log
			std::cerr << "error code is " << ec << std::endl;
		}
		else
		{
			conn_->start();
		}

		do_accept();
	});
}
