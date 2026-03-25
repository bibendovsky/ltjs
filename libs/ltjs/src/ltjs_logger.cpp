/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// Logger

#include "ltjs_logger.h"
#include "ltjs_sys_time.h"
#include <cassert>
#include <algorithm>
#include <mutex>
#include "SDL3/SDL.h"

namespace ltjs {

namespace {

class FileLoggerSink final : public LoggerSink
{
public:
	FileLoggerSink(const char* file_path);
	FileLoggerSink(const FileLoggerSink&) = delete;
	FileLoggerSink& operator=(const FileLoggerSink&) = delete;
	~FileLoggerSink() override;

	bool write(LoggerMessageType message_type, std::string_view message) override;
	bool flush() override;

private:
	SDL_IOStream* sdl_io_stream_{};

	bool is_open() const noexcept;
};

// -------------------------------------

FileLoggerSink::FileLoggerSink(const char* file_path)
	:
	sdl_io_stream_{SDL_IOFromFile(file_path, "wb")}
{
	assert(sdl_io_stream_ != nullptr);
}

FileLoggerSink::~FileLoggerSink()
{
	[[maybe_unused]] const bool is_closed = SDL_CloseIO(sdl_io_stream_);
	assert(is_closed);
}

bool FileLoggerSink::write(LoggerMessageType message_type, std::string_view message)
{
	if (!is_open())
	{
		return false;
	}
	return SDL_WriteIO(sdl_io_stream_, message.data(), message.size()) == message.size();
}

bool FileLoggerSink::flush()
{
	if (!is_open())
	{
		return false;
	}
	return SDL_FlushIO(sdl_io_stream_);
}

bool FileLoggerSink::is_open() const noexcept
{
	return sdl_io_stream_ != nullptr;
}

// =====================================

class LoggerImpl final : public Logger
{
public:
	LoggerImpl(const char* logger_name, const char* file_path);
	~LoggerImpl() override = default;

private:
	std::mutex mutex_{};
	std::string logger_name_{};
	LoggerSinkUPtr console_sink_{};
	LoggerSinkUPtr file_sink_{};
	std::string message_buffer_{};

	void log_message(LoggerMessageType message_type, std::string_view message) override;
};

// -------------------------------------

LoggerImpl::LoggerImpl(const char* logger_name, const char* file_path)
	:
	logger_name_{logger_name},
	console_sink_{make_console_logger_sink()},
	file_sink_{make_file_logger_sink(file_path)}
{
	message_buffer_.reserve(1024);
}

void LoggerImpl::log_message(LoggerMessageType message_type, std::string_view message)
{
	std::lock_guard state_sentinel{mutex_};
	message_buffer_.clear();
	// Date/Time.
	const sys::TimeNsOpt time_ns_opt = sys::get_current_time_ns();
	const sys::DateTimeOpt date_time_opt = sys::time_ns_to_date_time_local(time_ns_opt.value());
	const sys::DateTime date_time = date_time_opt.value_or(sys::DateTime{});
	// Message type.
	char type_char;
	switch (message_type)
	{
		case LoggerMessageType::info:
			type_char = 'I';
			break;
		case LoggerMessageType::warn:
			type_char = 'W';
			break;
		case LoggerMessageType::error:
			type_char = 'E';
			break;
		default:
			type_char = '?';
			assert(false && "Unknown message type.");
			break;
	}
	// Format it.
	std::format_to(std::back_inserter(message_buffer_),
		"[{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}] [{}] [{}] {}\n",
		date_time_opt->year,
		date_time_opt->month,
		date_time_opt->day,
		date_time_opt->hour,
		date_time_opt->minute,
		date_time_opt->second,
		date_time_opt->nanosecond / 1'000'000,
		logger_name_,
		type_char,
		message);
	//
	[[maybe_unused]] const bool console_written = console_sink_->write(message_type, message_buffer_);
	assert(console_written);
	[[maybe_unused]] const bool console_flushed = console_sink_->flush();
	assert(console_flushed);
	//
	[[maybe_unused]] const bool file_written = file_sink_->write(message_type, message_buffer_);
	assert(file_written);
	[[maybe_unused]] const bool file_flushed = file_sink_->flush();
	assert(file_flushed);
}

} // namespace

// =====================================

void Logger::info()
{
	info(std::string_view{});
}

void Logger::warn()
{
	warn(std::string_view{});
}

void Logger::error()
{
	error(std::string_view{});
}

void Logger::info(std::string_view message)
{
	log_message(LoggerMessageType::info, message);
}

void Logger::warn(std::string_view message)
{
	log_message(LoggerMessageType::warn, message);
}

void Logger::error(std::string_view message)
{
	log_message(LoggerMessageType::error, message);
}

// =====================================

LoggerUPtr make_logger(const char* logger_name, const char* file_path)
{
	return std::make_unique<LoggerImpl>(logger_name, file_path);
}

// -------------------------------------

LoggerSinkUPtr make_file_logger_sink(const char* file_path)
{
	return std::make_unique<FileLoggerSink>(file_path);
}

} // namespace ltjs
