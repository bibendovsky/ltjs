/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// A substream wrapper for a stream

#ifndef LTJS_SUBSTREAM_INCLUDED
#define LTJS_SUBSTREAM_INCLUDED

#include "ltjs_stream.h"

namespace ltjs {

class Substream : public Stream
{
public:
	enum class SyncPositionOnRead
	{
		none,
		enable,
		disable
	};

	Substream() = default;
	Substream(
		Stream* stream_ptr,
		Position offset,
		SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable) noexcept;
	Substream(
		Stream* stream_ptr,
		Position offset,
		Position size,
		SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable) noexcept;
	Substream(const Substream& that) = default;
	Substream& operator=(const Substream& that) = default;
	~Substream() override;

	bool open(
		Stream* stream,
		Position offset,
		SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable) noexcept;
	bool open(
		Stream* stream,
		Position offset,
		Position size,
		SyncPositionOnRead sync_position_on_read = SyncPositionOnRead::enable) noexcept;

	bool is_open() const noexcept override;
	void close() noexcept override;
	int read(void* buffer, int buffer_size) noexcept override;
	int write(const void* buffer, int buffer_size) noexcept override;
	Position get_position() noexcept override;
	Position set_position(Position offset, Origin origin) noexcept override;
	Position get_size() noexcept override;

private:
	Stream* stream_ptr_{};
	Position begin_position_{};
	Position current_position_{};
	Position end_position_{};
	SyncPositionOnRead sync_position_on_read_{SyncPositionOnRead::none};

	bool impl_is_open() const noexcept;
	void impl_close() noexcept;
	bool open_internal(
		Stream* stream_ptr,
		Position offset,
		Position size,
		SyncPositionOnRead sync_position_on_read) noexcept;
};

} // namespace ltjs

#endif // LTJS_SUBSTREAM_INCLUDED
