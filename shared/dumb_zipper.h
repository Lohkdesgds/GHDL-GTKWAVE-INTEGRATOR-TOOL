#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <thread>
#include <functional>

namespace DUMB {

	struct dumb {
		struct fileinfo {
			size_t len = 0;
			std::string path;
		}; // 1234 path\\to\\file.txt

		std::vector<fileinfo> filelist;
	}; // multiple lines of fileinfo, then \nSTART_DATA\n

	struct ongoing_info {
		std::string extra;
		struct {
			size_t current = 0;
			size_t total = 0;
		} filebytes, totalfiles;

		ongoing_info() = default;
		ongoing_info(const std::string& s) : extra(s) {}
		ongoing_info(const size_t fbc, const size_t fbt, const size_t tfc, const size_t tft, const std::string& s = {}) : filebytes{ fbc, fbt }, totalfiles{ tfc, tft }, extra(s) {}

		bool is_file_begin() const { return filebytes.current == 0; }
		bool is_file_end() const { return filebytes.current == filebytes.total; }
		bool is_list_begin() const { return totalfiles.current == 0; }
		bool is_list_end() const { return totalfiles.current == totalfiles.total; }
	};

	const std::string start_key = "START_DATA";
	const std::string start_avoid = ".\\";

	namespace impl {

		inline bool do_write(FILE* f, const char* b, size_t r)
		{
			while (r) {
				const size_t wr = ::fwrite(b, sizeof(char), r, f);
				if (wr == 0 || wr > r) return false;
				r -= wr;
				b += wr;
			}
			return true;
		}

		inline size_t do_read(FILE* f, char* b, size_t r)
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

		inline void recursive_read(dumb& d, const std::string& p)
		{
			for (auto const& e : std::filesystem::recursive_directory_iterator{ p })
			{
				if (e.is_regular_file() && e.file_size() != 0) {
					std::string wrk = e.path().string();
					if (wrk.empty()) continue;
					while (std::find(start_avoid.begin(), start_avoid.end(), wrk.front()) != start_avoid.end()) wrk.erase(wrk.begin()); // clearup
					d.filelist.push_back(dumb::fileinfo{ static_cast<size_t>(e.file_size()), wrk });
				}
			}
		}

		inline bool recover_from_file(dumb& d, FILE* fp)
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
	}

	// folder to read, file to become
	inline bool make_folder_file(const std::string& fold, const std::string& pathu, const std::function<void(const ongoing_info&)> sout = {})
	{
		FILE* fp = nullptr;
		FILE* eac = nullptr;

		const auto autoprint = [&](const ongoing_info& s) { if (sout) sout(s); };
		const auto autoprinterr = [&](const std::string& s) { if (eac) fclose(eac); if (fp) fclose(fp); autoprint(s); };

		dumb gen;
		impl::recursive_read(gen, fold);

		if (gen.filelist.size() == 0) { autoprinterr("No data found to generate stuff. ERROR."); return false; }

		if (fopen_s(&fp, pathu.c_str(), "wb")) { autoprinterr("Can't open output file. ERROR."); return false; }

		for (const auto& i : gen.filelist) {
			const std::string _tmp = std::to_string(i.len) + " " + i.path + "\n";
			if (!impl::do_write(fp, _tmp.data(), _tmp.size())) { autoprinterr("Can't write file. FATAL ERROR"); return false; }
		}
		if (!impl::do_write(fp, start_key.data(), start_key.size())) { autoprinterr("Can't write file. FATAL ERROR"); return false; }

		size_t _extcount = 0;
		for (const auto& i : gen.filelist) {
			++_extcount;

			autoprint(ongoing_info{ 0, i.len, _extcount, gen.filelist.size(), i.path });
			//autoprint("[" + std::to_string(_extcount) + "/" + std::to_string(gen.filelist.size()) + "][0/" + std::to_string(i.len) + " byte(s)] " + i.path);

			if (fopen_s(&eac, i.path.c_str(), "rb") || eac == nullptr) { autoprinterr("Can't open '" + i.path + "' to read. FATAL ERROR"); return false; }

			char minbuf[1 << 12]{};
			size_t to_read = i.len;
			size_t redd = 0, tott = 0;
			while (to_read != 0 && (redd = impl::do_read(eac, minbuf, to_read > sizeof(minbuf) ? sizeof(minbuf) : to_read)) > 0) {
				if (!impl::do_write(fp, minbuf, redd)) { autoprinterr("Can't write file. FATAL ERROR"); return false; }
				tott += redd;
				if (redd > to_read) { autoprinterr("File was read too much? FATAL ERROR"); return false; }
				to_read -= redd;

				autoprint(ongoing_info{ tott, i.len, _extcount, gen.filelist.size(), i.path });
				//autoprint("[" + std::to_string(_extcount) + "/" + std::to_string(gen.filelist.size()) + "][" + std::to_string(tott) + "/" + std::to_string(i.len) + " byte(s)] " + i.path);
			}
			//autoprint("\n");

			fclose(eac);
			eac = nullptr;
		}

		fclose(fp);
		return true;
	}

	// file to read, folder offset, folder is auto (based on file)
	inline bool make_file_folder(const std::string& pathu, std::string folder_off = {}, const std::function<void(const ongoing_info&)> sout = {})
	{
		FILE* fp = nullptr;
		FILE* eac = nullptr;
		if (folder_off.size() && folder_off.back() != '\\') folder_off += '\\';

		const auto autoprint = [&](const ongoing_info& s) { if (sout) sout(s); };
		const auto autoprinterr = [&](const std::string& s) { if (eac) fclose(eac); if (fp) fclose(fp); autoprint(s); };

		dumb gen;

		if (fopen_s(&fp, pathu.c_str(), "rb")) { autoprinterr("Can't open input file. ERROR."); return false; }

		if (!impl::recover_from_file(gen, fp)) { autoprinterr("Failed reading file. ERROR."); return false; }
		if (gen.filelist.size() == 0) { autoprinterr("File seems empty (index). ERROR."); return false; }

		if (folder_off.size())
			for (auto& e : gen.filelist) e.path.insert(e.path.begin(), folder_off.cbegin(), folder_off.cend());

		size_t _extcount = 0;
		for (const auto& i : gen.filelist) {
			++_extcount;

			autoprint(ongoing_info{ 0, i.len, _extcount, gen.filelist.size(), i.path });
			//autoprint("[" + std::to_string(_extcount) + "/" + std::to_string(gen.filelist.size()) + "][0/" + std::to_string(i.len) + " byte(s)] " + i.path);

			{
				size_t pathlim = i.path.rfind('\\');
				if (pathlim != std::string::npos) {
					std::error_code err{};
					std::filesystem::create_directories(i.path.substr(0, pathlim), err);
					if (err) { autoprinterr("Error on filesystem: " + err.message()); return false; }
				}
			}

			if (fopen_s(&eac, i.path.c_str(), "wb") || eac == nullptr) { autoprinterr("Can't open '" + i.path + "' to write. FATAL ERROR"); return false; }

			char minbuf[1 << 12]{};
			size_t to_read = i.len;
			size_t redd = 0, tott = 0;
			while (to_read != 0 && (redd = impl::do_read(fp, minbuf, to_read > sizeof(minbuf) ? sizeof(minbuf) : to_read)) > 0) {
				if (!impl::do_write(eac, minbuf, redd)) { autoprinterr("Can't write file. FATAL ERROR"); return false; }
				tott += redd;
				if (redd > to_read) { autoprinterr("File was read too much? FATAL ERROR"); return false; }
				to_read -= redd;

				autoprint(ongoing_info{ tott, i.len, _extcount, gen.filelist.size(), i.path });
				//autoprint("[" + std::to_string(_extcount) + "/" + std::to_string(gen.filelist.size()) + "][" + std::to_string(tott) + "/" + std::to_string(i.len) + " byte(s)] " + i.path);
			}
			//autoprint("\n");

			fclose(eac);
			eac = nullptr;
		}

		fclose(fp);
		return true;
	}

}