#pragma once

#include "array_ref.h"
#include "common.h"
#include <string>

#if _MSC_VER >= 1800
#define WIDE_PATH_IO
#endif

namespace goldfish { namespace stream
{
	namespace details
	{
		class file_handle
		{
		public:
			file_handle(const char* path, const char* mode)
			{
#if _MSC_VER >= 1800
				if (auto errorno = fopen_s(&m_fp, path, mode))
#else
				errno = 0;
				m_fp = fopen(path, mode);
				if (m_fp == NULL || errno)
#endif
					throw io_exception_with_error_code{ "Error during file open", errno };
			}
#ifdef WIDE_PATH_IO
			file_handle(const wchar_t* path, const wchar_t* wmode)
			{
				if (auto error = _wfopen_s(&m_fp, path, wmode))
					throw io_exception_with_error_code{ "Error during file open", error };
			}
#endif
			file_handle(const std::string& path, const char* mode)
				: file_handle(path.c_str(), mode)
			{}
#ifdef WIDE_PATH_IO
			file_handle(const std::wstring& path, const wchar_t* wmode)
				: file_handle(path.c_str(), wmode)
			{}
#endif

			file_handle(file_handle&& rhs)
				: m_fp(rhs.m_fp)
			{
				rhs.m_fp = nullptr;
			}
			~file_handle()
			{
				if (m_fp)
					fclose(m_fp);
			}
			file_handle(const file_handle&) = delete;
			file_handle& operator = (const file_handle&) = delete;

			FILE* get() const { return m_fp; }
		private:
			FILE* m_fp;
		};
	}

	class file_reader
	{
	public:
		file_reader(const char* path)
			: m_file(path, "rb")
		{}
#ifdef WIDE_PATH_IO
		file_reader(const wchar_t* path)
			: m_file(path, L"rb")
		{}
#endif
		file_reader(const std::string& path)
			: m_file(path, "rb")
		{}
#ifdef WIDE_PATH_IO
		file_reader(const std::wstring& path)
			: m_file(path, L"rb")
		{}
#endif
		size_t read_partial_buffer(buffer_ref data)
		{
			auto cb = fread(data.data(), 1 /*size*/, data.size() /*count*/, m_file.get());
			if (cb != data.size())
			{
				if (auto error = ferror(m_file.get()))
					throw io_exception_with_error_code{ "Error during file read", error };
			}
			return cb;
		}
	private:
		details::file_handle m_file;
	};

	class file_writer
	{
	public:
		file_writer(const char* path)
			: m_file(path, "wb")
		{}
#ifdef WIDE_PATH_IO
		file_writer(const wchar_t* path)
			: m_file(path, L"wb")
		{}
#endif
		file_writer(const std::string& path)
			: m_file(path, "wb")
		{}
#ifdef WIDE_PATH_IO
		file_writer(const std::wstring& path)
			: m_file(path, L"wb")
		{}
#endif

		void write_buffer(const_buffer_ref data)
		{
			if (fwrite(data.data(), 1 /*size*/, data.size() /*count*/, m_file.get()) != data.size())
				throw io_exception_with_error_code{ "Error during file write", ferror(m_file.get()) };
		}
		void flush() { }
	private:
		details::file_handle m_file;
	};
}}
