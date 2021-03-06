#include "downloader.h"

namespace Lunaris {

	bool downloader::run_get_with_func(const std::string& url, std::function<void(const char*, const size_t)> f)
	{
		constexpr size_t max_block_size = 1536;

#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Downloader %p downloading '%s'", this, url.c_str());
#endif

#ifdef UNICODE
		HINTERNET connect = InternetOpen(L"Lunaris Downloader V6.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#else
		HINTERNET connect = InternetOpen("Lunaris Downloader V6.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#endif
		if (!connect) return false;
		HINTERNET OpenAddress = InternetOpenUrlA(connect, url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);
		if (!OpenAddress) return false;


		char DataReceived[max_block_size];
		DWORD NumberOfBytesRead = 0;
		TotalBytesRead = 0;

		while (InternetReadFile(OpenAddress, DataReceived, max_block_size, &NumberOfBytesRead)) {
			if (NumberOfBytesRead <= 0) break;
			f(DataReceived, NumberOfBytesRead);
			TotalBytesRead += NumberOfBytesRead;
		}
		if (NumberOfBytesRead > 0) f(DataReceived, NumberOfBytesRead);

		InternetCloseHandle(OpenAddress);
		InternetCloseHandle(connect);

#ifdef LUNARIS_VERBOSE_BUILD
		PRINT_DEBUG("Downloader %p downloaded '%s'", this, url.c_str());
#endif

		return true;
	}

	bool downloader::get(const std::string& url)
	{
		buf.clear();
		return run_get_with_func(url, [&](const char* feed, const size_t feed_siz) { buf.insert(buf.end(), feed, feed + feed_siz); });
	}

	bool downloader::get_store(const std::string& url, std::function<void(const char*, const size_t)> f)
	{
		buf.clear();
		return run_get_with_func(url, f);
	}

	size_t downloader::bytes_read() const
	{
		return TotalBytesRead;
	}

	const std::vector<char>& downloader::read() const
	{
		return buf;
	}

	void downloader::clear_buf()
	{
		buf.clear();
	}

	bool downloader::valid() const
	{
		return buf.size() != 0;
	}

	bool downloader::empty() const
	{
		return buf.empty();
	}

}