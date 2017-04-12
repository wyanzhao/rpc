#pragma once
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <thread>

#pragma once
namespace rpc {
	class io_service_pool : boost::noncopyable
	{
	private:
		using io_service_ptr = std::shared_ptr<boost::asio::io_service>;
		using work_ptr = std::shared_ptr<boost::asio::io_service::work>;

		/// The pool of io_services.
		std::vector<io_service_ptr> io_services_;

		/// The work that keeps the io_services running.
		std::vector<work_ptr> work_;

		/// The next io_service to use for a connection.
		std::size_t next_io_service_;
	public:
		explicit io_service_pool(std::size_t pool_size);
		~io_service_pool();

		void run();
		void stop();
		boost::asio::io_service& get_io_service();
	};
}