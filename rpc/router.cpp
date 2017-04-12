#include "router.h"

rpc::invoker_function::invoker_function(const std::function<void(token_parser&, std::string&)>& function, std::size_t size):
	function_ (function),
	param_size_ (size)
{
}

void rpc::invoker_function::operator()(token_parser & parser, std::string & result)
{
	function_(parser, result);
}

const std::size_t rpc::invoker_function::param_size() const
{
	return param_size_;
}

rpc::router & rpc::router::get()
{
	static router instance;
	return instance;
}

void rpc::router::remove_handler(std::string const & name)
{  
	this->map_invokers_.erase(name);
}

void rpc::router::route(const std::string &text, std::size_t length, const std::function<void(const char*)>& callback)
{
	assert(callback);
	token_parser parser(text);
						
	std::string result = "";
	std::string func_name = parser.get<std::string>();
	std::size_t num_parament = parser.get <std::size_t>();

	auto it = map_invokers_.find(func_name);
	if (it == map_invokers_.end())
	{
		result = rpc::get_serial(result_code::EXCEPTION, "unknown function: " + func_name);
		callback(result.c_str());
		return;
	}
		
		
	if (it->second.param_size() != num_parament)
	{
		result = get_serial(result_code::EXCEPTION, "parameter number is not match" + func_name);
		callback(result.c_str());
		return;
	}
		

	it->second(parser, result);
	callback(result.c_str());
}
