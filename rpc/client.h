#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/asio.hpp>
#include <memory>

#include "utility.hpp"

namespace rpc
{
	using boost::asio::ip::tcp;

	class client
	{
	public:
		client(boost::asio::io_service& io_service);
		~client();
		bool connect(const std::string& addr, short port);

		template<typename... Args>
		std::string call(const std::string & handler_name, Args&&... args);

		template<typename HandlerT, typename... Args>
		void async_call(const char* handler_name, HandlerT &handler, Args&&... args);

		template <typename T>
		auto get_result(const std::string &buf);

		std::string get_async_data();
	private:
		inline std::string call(const std::string& serial_str);

		template<typename HandlerT>
		inline void async_call(const std::string& json_str, HandlerT &handler);

		template<typename ...Args>
		static std::string make_serial(const std::string &handler_name, Args && ...args);	
	private:
		boost::asio::io_service& io_service_;
		bool if_connected_;
		tcp::socket socket_;
		std::string recv_buf_;
		u_int len_;
	};

	template<typename HandlerT, typename... Args>
	void client::async_call(const char* handler_name, HandlerT &handler, Args&&... args)
	{
		auto serial_str = make_serial(handler_name, std::forward<Args>(args)...);
		try {
			async_call(serial_str, handler);
		}
		catch (const boost::system::system_error& ec)
		{
			std::cerr << ec.what() << std::endl;
		}
	}

	template<typename HandlerT>
	inline void client::async_call(const std::string & serial_str, HandlerT &handler)
	{
		len_ = (u_int)serial_str.length();
		
		std::vector<boost::asio::const_buffer> message;
		message.push_back(boost::asio::buffer(&len_, 4));
		message.push_back(boost::asio::buffer(serial_str));
		socket_.send(message);
		
		socket_.async_receive(boost::asio::buffer(&len_, 4), [&, this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				recv_buf_.resize(len_);
				socket_.async_receive(boost::asio::buffer(&recv_buf_[0], len_), handler);
			}
			else
			{
				std::cerr << "async receive head failed, code: " << ec << std::endl;
			}
		});
		
	}

	template <typename T>
	inline auto client::get_result(const std::string &buf)
	{
		int num_result;
		T result;
		std::stringstream ss (buf);
		boost::archive::text_iarchive ia(ss);

		ia >> num_result;
		ia >> result;
		return result;
	}

	template<typename ...Args>
	std::string client::call(const std::string &handler_name, Args && ...args)
	{
		std::string serial_str = make_serial(handler_name, std::forward<Args>(args)...);
		std::string buf = "";

		try
		{
			buf = call(serial_str);
		}
		catch (const boost::system::system_error& ec)
		{
			std::cerr << ec.what() << std::endl;
			buf = "";
		}

		return buf;
	}

	std::string client::call(const std::string & serial_str)
	{
		len_ = (u_int)serial_str.length();

		std::vector<boost::asio::const_buffer> message;
		message.push_back(boost::asio::buffer(&len_, 4));
		message.push_back(boost::asio::buffer(serial_str));
		socket_.send(message);

		socket_.receive(boost::asio::buffer(&len_, 4));
		recv_buf_.resize(len_);
		socket_.receive(boost::asio::buffer(&recv_buf_[0], len_));
		return recv_buf_;
	}

	template<typename ...Args>
	static inline std::string client::make_serial(const std::string &handler_name, Args && ...args)
	{
		std::stringstream ss;
		boost::archive::text_oarchive oa(ss);
		rpc::encode(handler_name, ss, oa);
		rpc::encode(sizeof ...(args), ss,oa);
		std::initializer_list <int>{ (encode(args, ss, oa), 0)...};
		return ss.str();
	}



}
