#include <Console/console.h>
#include <Process/process.h>
#include <json.hpp>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "imported/downloader.h"
#include "../shared/dumb_zipper.h"

using namespace Lunaris;

// Discord is easier to share stuff around.
const std::string download_path = "https://cdn.discordapp.com/attachments/950728646224646164/952771590079279114/latest.folder";

const std::string mytmpfile = "download.folder";
const std::string expected_ghdl = "runtime\\ghdl\\bin\\ghdl.exe";
const std::string expected_gtkw = "runtime\\gtkwave\\bin\\gtkwave.exe";

std::string getenv_custom(const std::string&);
// url, path
bool download_to(const std::string&, const std::string&);

std::vector<std::string> sliceup(const std::string&);

int main(int argc, char* argv[])
{
	if (argc == 1) {
		cout << console::color::AQUA << "========================================================";
		cout << console::color::AQUA << "=         GHDL & GTKW integrator tool by Lohk          =";
		cout << console::color::AQUA << "=                         2022                         =";
		cout << console::color::AQUA << "========================================================";
	}

	cout << console::color::DARK_GRAY << "[Setup] Getting needed information to work...";

	const std::string envpath = [] {const auto i = getenv_custom("APPDATA"); if (!i.empty()) return i + "\\GHDLGTKW_by_Lunaris\\"; return std::string{}; }();
	if (envpath.empty()) { cout << console::color::RED << "Can't read %appdata%, please try running on main disk instead!"; std::this_thread::sleep_for(std::chrono::seconds(60)); return 0; }

	// -1 == exit, 0 == ok, 1 == error, but ok
	const auto func_work = [&](std::string in) {
		if (in.find("gtkw") == 0) {
			if (in.find("gtkw ") == 0) {
				in.erase(in.begin(), in.begin() + 4);
				cout << console::color::YELLOW << "[GTKW] Calling 'gtkwave.exe " << in << "'...";
			}
			else {
				in.clear();
				cout << console::color::YELLOW << "[GTKW] Started immersive mode.";
				cout << console::color::YELLOW << "[GTKW] Each line is equivalent of GTKW %INPUT%";
				cout << console::color::YELLOW << "[GTKW] Type ~ to quit this immersive mode.";

				while (1) {
					std::cout << "$gtkw> ";
					std::string in;
					std::getline(std::cin, in);
					if (in == "~") break;

					const auto args = sliceup(in);

					Lunaris::process_sync proc(envpath + expected_gtkw, args, process_sync::mode::READWRITE);

					while (proc.has_read() || proc.is_running())
						if (const auto _r = proc.read(); !_r.empty()) cout << console::color::GOLD << _r;
				}

				cout << console::color::YELLOW << "[GTKW] Ended immersive mode.";
				return 0;
			}

			const auto args = sliceup(in);

			Lunaris::process_sync proc(envpath + expected_gtkw, args, process_sync::mode::READWRITE);

			while (proc.has_read() || proc.is_running())
				if (const auto _r = proc.read(); !_r.empty()) cout << console::color::GOLD << _r;

			cout << console::color::YELLOW << "[GTKW] Task ended.";
		}
		else if (in.find("ghdl") == 0) {
			if (in.find("ghdl ") == 0) {
				in.erase(in.begin(), in.begin() + 4);
				cout << console::color::YELLOW << "[GHDL] Calling 'ghdl.exe " << in << "'...";
			}
			else {
				in.clear();
				cout << console::color::YELLOW << "[GHDL] Started immersive mode.";
				cout << console::color::YELLOW << "[GHDL] Each line is equivalent of GHDL %INPUT%";
				cout << console::color::YELLOW << "[GHDL] Type ~ to quit this immersive mode.";

				while (1) {
					std::cout << "$ghdl> ";

					std::string in;
					std::getline(std::cin, in);
					if (in == "~") break;

					const auto args = sliceup(in);

					Lunaris::process_sync proc(envpath + expected_ghdl, args, process_sync::mode::READWRITE);

					while (proc.has_read() || proc.is_running())
						if (const auto _r = proc.read(); !_r.empty()) cout << console::color::GOLD << _r;
				}

				cout << console::color::YELLOW << "[GHDL] Ended immersive mode.";
				return 0;
			}

			const auto args = sliceup(in);

			Lunaris::process_sync proc(envpath + expected_ghdl, args, process_sync::mode::READWRITE);

			while (proc.has_read() || proc.is_running())
				if (const auto _r = proc.read(); !_r.empty()) cout << console::color::GOLD << _r;

			cout << console::color::YELLOW << "[GHDL] Task ended.";
		}
		else if (in.find("install") == 0) {
			cout << console::color::YELLOW << "[Install] Starting to download necessary stuff...";
			cout << console::color::YELLOW << "# Setting up basic path...";

			{
				std::error_code err{};
				std::filesystem::create_directories(envpath, err);
				if (err) { cout << console::color::RED << "# (ABORT) Error setting up main path: " << err.message(); return 1; }
			}

			cout << console::color::YELLOW << "# Downloading package...";

			if (!download_to(download_path, envpath + mytmpfile)) { cout << console::color::RED << "# (ABORT) Error downloading package. Please try again later or contact developer!"; return 1; }

			cout << console::color::YELLOW << "# Extracting funky package...";

			if (!DUMB::make_file_folder(envpath + mytmpfile, envpath,
				[](const DUMB::ongoing_info& info) { printf_s("[ %06.3lf%% ] \r", (100.0 * info.totalfiles.current / (info.totalfiles.total ? info.totalfiles.total : 1))); }))
			{
				cout << console::color::RED << "# (ABORT) Error extracting package. Please try again later or contact developer!"; return 1;
			}

			::remove((envpath + mytmpfile).c_str());

			cout << console::color::YELLOW << "# All good! Things should be installed now.";
		}
		else if (in.find("exit") == 0) {
			return -1;
		}
		else { // help
			if (in.find("help") != 0) cout << console::color::GOLD << "Command not found, showing help instead:";

			cout << console::color::YELLOW << "[Help] Commands available:";
			cout << console::color::YELLOW << "# " << console::color::LIGHT_PURPLE << "install: " << console::color::GRAY << "Get stuff needed for GHDL/GTKW to work, including themselves;";
			cout << console::color::YELLOW << "# " << console::color::LIGHT_PURPLE << "gtkw: " << console::color::GRAY << "Calls for gtkwave with your arguments, exactly as manually calling it on cmd. If no argument, immersive mode instead;";
			cout << console::color::YELLOW << "# " << console::color::LIGHT_PURPLE << "ghdl: " << console::color::GRAY << "Calls for gtkw with your arguments, exactly as manually calling it on cmd. If no argument, immersive mode instead;";
			cout << console::color::YELLOW << "# " << console::color::LIGHT_PURPLE << "exit: " << console::color::GRAY << "Closes the app.";
			cout << console::color::YELLOW << "# " << console::color::GREEN << "If you need more help, please consider joining the UTFPR unoficial Discord server at https://discord.gg/xrtz3VjAus or call @lohkdesgds on Twitter.";
		}
		return 0;
	};

	if (argc == 1) {
		cout << console::color::DARK_GRAY << "[Setup] Detected working environment: " << envpath;
		cout << console::color::DARK_GRAY << "[Setup] All good.";
	}
	else {
		cout << console::color::YELLOW << "[CmdLine] Detected scripting, calling all arguments as user input and exiting when done.";

		for (int a = 1; a < argc; ++a)
		{
			try {
				func_work(argv[a]);
			}
			catch (const std::exception& e) {
				cout << console::color::RED << "BAD EXCEPTION: " << e.what();
				cout << console::color::YELLOW << "Press enter to continue, if you want to.";
				while (getchar() != '\n');
			}
			catch (...) {
				cout << console::color::RED << "BAD EXCEPTION: UNKNOWN";
				cout << console::color::YELLOW << "Press enter to continue, if you want to.";
				while (getchar() != '\n');
			}
		}
		cout << console::color::YELLOW << "[CmdLine] Each argument was interpreted as input. Exiting the app.";

		return 0;
	}

	cout << console::color::YELLOW << "[Start] If you don't know what to do, please type 'help'";


	while (1) {
		std::cout << "> ";
		std::string in;
		std::getline(std::cin, in);

		try {
			func_work(in);
		}
		catch (const std::exception& e) {
			cout << console::color::RED << "BAD EXCEPTION: " << e.what();
		}
		catch (...) {
			cout << console::color::RED << "BAD EXCEPTION: UNKNOWN";
		}
	}

	cout << console::color::YELLOW << "[Exit] Closing...";

	return 0;
}

std::string getenv_custom(const std::string& nam)
{
	size_t req = 0;
	getenv_s(&req, nullptr, 0, nam.c_str());
	if (!req) return {};
	std::string buf;
	buf.resize(req, '\0');
	getenv_s(&req, buf.data(), buf.size(), nam.c_str());
	while (buf.size() && buf.back() == '\0') buf.pop_back();
	return buf;
}

bool download_to(const std::string& url, const std::string& sav)
{
	if (sav.empty() || url.empty()) return false;
	std::ofstream ofp(sav.c_str(), std::ios::binary | std::ios::out);
	if (!ofp) return false;

	downloader down;
	if (!down.get_store(url, [&](const char* data, const size_t len) { ofp.write(data, len); })) return false;

	return true;
}

std::vector<std::string> sliceup(const std::string& in)
{
	std::vector<std::string> args;
	{
		std::string token;
		bool on_ignore = false;
		for (const auto& i : in) {
			if (on_ignore) {
				on_ignore = false;
				continue;
			}
			switch (i) {
			case '\\':
				on_ignore = true;
				continue;
			case ' ':
				if (token.size()) args.push_back(token);
				token.clear();
				continue;
			default:
				token += i;
			}
		}
		if (token.size()) args.push_back(token);
	}
	return args;
}