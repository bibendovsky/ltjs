#ifndef LTJS_LOGGER_INCLUDED
#define LTJS_LOGGER_INCLUDED

#include <format>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>

namespace ltjs {

enum class LoggerMessageType
{
	none = 0,
	info,
	warn,
	error
};

class LoggerSink
{
public:
	LoggerSink() = default;
	virtual ~LoggerSink() = default;

	virtual bool write(LoggerMessageType message_type, std::string_view message) = 0;
	virtual bool flush() = 0;
};

class Logger
{
public:
	Logger() = default;
	virtual ~Logger() = default;

	void info();
	void warn();
	void error();

	void info(std::string_view message);
	void warn(std::string_view message);
	void error(std::string_view message);

	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void info(std::format_string<TArgs...> format, TArgs&&... args)
	{
		static thread_local std::string message_buffer{};
		message_buffer.clear();
		message_buffer.reserve(1024);
		std::vformat_to(std::back_inserter(message_buffer), format.get(), std::make_format_args(args...));
		info(message_buffer);
	}

	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void warn(std::format_string<TArgs...> format, TArgs&&... args)
	{
		static thread_local std::string message_buffer{};
		message_buffer.clear();
		message_buffer.reserve(1024);
		std::vformat_to(std::back_inserter(message_buffer), format.get(), std::make_format_args(args...));
		warn(message_buffer);
	}

	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void error(std::format_string<TArgs...> format, TArgs&&... args)
	{
		static thread_local std::string message_buffer{};
		message_buffer.clear();
		message_buffer.reserve(1024);
		std::vformat_to(std::back_inserter(message_buffer), format.get(), std::make_format_args(args...));
		error(message_buffer);
	}

private:
	virtual void log_message(LoggerMessageType message_type, std::string_view message) = 0;
};

// =====================================

using LoggerSinkUPtr = std::unique_ptr<LoggerSink>;

LoggerSinkUPtr make_file_logger_sink(const char* file_path);
LoggerSinkUPtr make_console_logger_sink();

// -------------------------------------

using LoggerUPtr = std::unique_ptr<Logger>;

LoggerUPtr make_logger(const char* logger_name, const char* file_path);

} // namespace ltjs

#endif // LTJS_LOGGER_INCLUDED
