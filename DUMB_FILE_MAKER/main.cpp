#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <thread>
#include "../shared/dumb_zipper.h"

struct dumb {
	struct fileinfo {
		size_t len = 0;
		std::string path;
	}; // 1234 path\\to\\file.txt

	std::vector<fileinfo> filelist;
}; // multiple lines of fileinfo, then \nSTART_DATA\n

const std::string start_key = "START_DATA";

bool do_write(FILE*, const char*, size_t);
size_t do_read(FILE*, char*, size_t);

// read paths and insert to dumb&
void recursive_read(dumb&, const std::string&);
// you have a file, so you want its data. This offsets to last part automatically.
bool recover_from_file(dumb&, FILE*);


int main()
{
	printf_s("Stupid file generator\n1. Pack; 2. Unpack\n> ");
	int choice = getchar();
	while (getchar() != '\n');

	switch (choice)
	{
	case '1': // pack stuff in
	{
		std::string pathu, fold;
		printf_s("File output: ");
		std::getline(std::cin, pathu);
		printf_s("Folder path source: ");
		std::getline(std::cin, fold);

		printf_s("\n\nPath source: %s\nFinal file: %s\n\n", fold.c_str(), pathu.c_str());

		DUMB::make_folder_file(fold, pathu, [](const DUMB::ongoing_info& str) { std::cout << "[" << str.totalfiles.current << "/" << str.totalfiles.total << "][" << str.filebytes.current << "/" + str.filebytes.total << " byte(s)] " << str.extra << "\r"; });

		printf_s("END.\n");
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return 0;
	}
		break;
	case '2':
	{
		std::string pathu;
		printf_s("File input: ");
		std::getline(std::cin, pathu);

		DUMB::make_file_folder(pathu, {}, [](const DUMB::ongoing_info& str) { std::cout << "[" << str.totalfiles.current << "/" << str.totalfiles.total << "][" << str.filebytes.current << "/" + str.filebytes.total << " byte(s)] " << str.extra << "\r"; });

		printf_s("END.\n");
		std::this_thread::sleep_for(std::chrono::seconds(5));
		return 0;
	}
		break;
	default:
		printf_s("Wrong one.\n");
		break;
	}
}

bool do_write(FILE* f, const char* b, size_t r)
{
	while (r) {
		const size_t wr = ::fwrite(b, sizeof(char), r, f);
		if (wr == 0 || wr > r) return false;
		r -= wr;
		b += wr;
	}
	return true;
}

size_t do_read(FILE* f, char* b, size_t r)
{
	size_t readd = 0;
	while (r) {
		const size_t wr = ::fread_s(b, r, sizeof(char), r, f);
		if (wr != r) return readd > 0 ? readd : 0;
		r -= wr;
		b += wr;
		readd += wr;
	}
	return readd;
}

void recursive_read(dumb& d, const std::string& p)
{
	for (auto const& e : std::filesystem::recursive_directory_iterator{ p })
	{
		if (e.is_regular_file() && e.file_size() != 0) {
			d.filelist.push_back(dumb::fileinfo{ static_cast<size_t>(e.file_size()), e.path().string()});
		}
	}
}

bool recover_from_file(dumb& d, FILE* fp)
{
	std::string lilbuf;
	while (lilbuf != start_key && !feof(fp)) {
		int val = fgetc(fp);
		if (val < 0) return false;
		switch (val) {
		case '\n':
		{
			char minbuf[512]{};
			size_t len = 0;
			if (sscanf_s(lilbuf.c_str(), "%zu %512[^\n]", &len, minbuf, static_cast<unsigned>(sizeof(minbuf))) != 2) { printf_s("Something went wrong while reading index. ABORT!\n"); return false; }

			if (len == 0 || strnlen(minbuf, sizeof(minbuf)) == 0) { printf_s("Something went wrong while reading index. ABORT!\n"); return false; }

			d.filelist.push_back({ len, std::string(minbuf) });

			lilbuf.clear();
		}
			break;
		default:
			lilbuf += static_cast<char>(val);
			break;
		}
	}
	return !feof(fp);
}