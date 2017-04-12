#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/asio.hpp>
#include <memory>

#include <boost/bind.hpp>
using boost::asio::ip::tcp;

namespace rpc
{
	class client
	{
	public:
		client(boost::asio::io_service& io_service);
		void connect(const std::string& addr, short port);

		template<typename... Args>
		std::string call(const std::string & handler_name, Args&&... args);

		template<typename HandlerT, typename... Args>
		std::string async_call(const char* handler_name, HandlerT handler, Args&&... args);

		template <typename T>
		auto get_result(const std::string &buf);

	private:
		std::string call(const std::string& serial_str);

		template<typename HandlerT>
		std::string async_call(const std::string& json_str, HandlerT handler);

		template<typename ...Args>
		static std::string make_serial(const std::string &handler_name, Args && ...args);

		template <typename T>
		static void encode(T t, std::stringstream &ss, boost::archive::text_oarchive &oa);

	private:
		boost::asio::io_service& io_service_;
		tcp::socket socket_;
		enum { max_length = 8192 };
		char data_[max_length];
		char recv_data_[max_length];
	};

	template<typename HandlerT, typename... Args>
	std::string client::async_call(const char* handler_name, HandlerT handler, Args&&... args)
	{
		auto serial_str = make_serial(handler_name, std::forward<Args>(args)...);
		return async_call(serial_str, handler);
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

	template<typename HandlerT>
	inline std::string client::async_call(const std::string & serial_str, HandlerT handler)
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
		socket_.async_receive(boost::asio::buffer(&recv_json[0], len), handler);
		return recv_json;
	}

	template<typename ...Args>
	std::string client::call(const std::string &handler_name, Args && ...args)
	{
		auto serial_str = make_serial(handler_name, std::forward<Args>(args)...);
		return call(serial_str);
	}

	template<typename ...Args>
	static inline std::string client::make_serial(const std::string &handler_name, Args && ...args)
	{
		std::stringstream ss;
		boost::archive::text_oarchive oa(ss);
		encode(handler_name, ss, oa);
		encode(sizeof ...(args), ss,oa);
		std::initializer_list <int>{ (encode(args, ss, oa), 0)...};
		return ss.str();
	}

	template<typename T>
	static inline void client::encode(T t, std::stringstream & ss, boost::archive::text_oarchive & oa)
	{
		oa << t;
	}

}
