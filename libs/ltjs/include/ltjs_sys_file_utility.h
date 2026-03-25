/*
LTJS: Source port of LithTech Jupiter System
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0
*/

// System: File utility

#ifndef LTJS_SYS_FILE_UTILITY_INCLUDED
#define LTJS_SYS_FILE_UTILITY_INCLUDED

namespace ltjs::sys {

int get_file_size(const char* path) noexcept;
int load_file(const char* path, void* buffer, int buffer_size) noexcept;
bool save_file(const char* path, const void* buffer, int buffer_size) noexcept;

} // namespace ltjs::sys

#endif // LTJS_SYS_FILE_UTILITY_INCLUDED
