#ifdef _WIN32

#include "ltjs_logger.h"
#include <cassert>
#include <string>
#include <windows.h>

namespace ltjs {

namespace {

class LoggerConsoleSink final : public LoggerSink
{
public:
	LoggerConsoleSink();
	LoggerConsoleSink(const LoggerConsoleSink&) = delete;
	LoggerConsoleSink& operator=(const LoggerConsoleSink&) = delete;
	~LoggerConsoleSink() override = default;

	bool write(LoggerMessageType message_type, std::string_view message) override;
	bool flush() override;

private:
	HANDLE stderr_{};
	HANDLE stdout_{};
	std::wstring message_buffer_{};
};

// -------------------------------------

LoggerConsoleSink::LoggerConsoleSink()
{
	stderr_ = ::GetStdHandle(STD_ERROR_HANDLE);
	if (stderr_ == INVALID_HANDLE_VALUE)
	{
		assert(false && "GetStdHandle(STD_ERROR_HANDLE)");
		stderr_ = nullptr;
	}
	stdout_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdout_ == INVALID_HANDLE_VALUE)
	{
		assert(false && "GetStdHandle(STD_OUTPUT_HANDLE)");
		stdout_ = nullptr;
	}
}

bool LoggerConsoleSink::write(LoggerMessageType message_type, std::string_view message)
{
	if (message.empty())
	{
		assert(false && "Empty message.");
		return true;
	}
	HANDLE std_handle;
	switch (message_type)
	{
		case LoggerMessageType::info:
			std_handle = stdout_;
			break;
		case LoggerMessageType::warn:
		case LoggerMessageType::error:
			std_handle = stderr_;
			break;
		default:
			assert(false && "Unknown message type.");
			return false;
	}
	if (std_handle == nullptr)
	{
		return true;
	}
	if (message_buffer_.size() < message.size())
	{
		message_buffer_.resize(message.size());
	}
	const int written_size = ::MultiByteToWideChar(
		/* CodePage       */ CP_UTF8,
		/* dwFlags        */ 0,
		/* lpMultiByteStr */ message.data(),
		/* cbMultiByte    */ static_cast<int>(message.size()),
		/* lpWideCharStr  */ message_buffer_.data(),
		/* cchWideChar    */ static_cast<int>(message.size()));
	if (written_size == 0)
	{
		assert(false && "MultiByteToWideChar");
		return false;
	}
	const DWORD to_write_byte_count = static_cast<DWORD>(written_size * 2);
	DWORD written_byte_count;
	if (!::WriteFile(
		/* hFile                  */ std_handle,
		/* lpBuffer               */ message_buffer_.data(),
		/* nNumberOfBytesToWrite  */ to_write_byte_count,
		/* lpNumberOfBytesWritten */ &written_byte_count,
		/* lpOverlapped           */ nullptr))
	{
		assert(false && "WriteFile");
		return false;
	}
	if (written_byte_count != to_write_byte_count)
	{
		assert(false && "Partial write.");
		return false;
	}
	return true;
}

bool LoggerConsoleSink::flush()
{
	bool is_failed = false;
	if (stderr_ != nullptr)
	{
		if (!::FlushFileBuffers(stderr_))
		{
			assert(false && "FlushFileBuffers(STD_ERROR_HANDLE)");
			is_failed = true;
		}
	}
	if (stderr_ != nullptr)
	{
		if (!::FlushFileBuffers(stdout_))
		{
			assert(false && "FlushFileBuffers(STD_OUTPUT_HANDLE)");
			is_failed = true;
		}
	}
	return !is_failed;
}

} // namespace

// =====================================

LoggerSinkUPtr make_console_logger_sink()
{
	return std::make_unique<LoggerConsoleSink>();
}

} // namespace ltjs

#endif // _WIN32
