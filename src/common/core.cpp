// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "core.hpp"

#include <config/core.hpp>
#include <common/utils.hpp>
#include <common/nullpos.hpp>
#ifndef MINICORE
#include "database.hpp"
#include "ers.hpp"
#include "socket.hpp"
#include "timer.hpp"
#include "sql.hpp"
#endif
#include <stdlib.h>
#include <signal.h>

#ifdef _WIN32 //ifdef
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
#ifndef _WIN32 //ifndef
#include <unistd.h>
#else
#include "winapi.hpp" 
#include <direct.h>
#endif




#include <string.h>
#include "cbasetypes.hpp"
#include "malloc.hpp"
#include "mmo.hpp"
#include "showmsg.hpp"
#include "strlib.hpp"

#ifndef DEPRECATED_COMPILER_SUPPORT
#if defined( _MSC_VS ) && _MSC_VS < 1914
#error "Visual Studio versions older than Visual Studio 2017 are not officially supported anymore"
#elif defined( __clang__ ) && __clang_major__ < 6
#error "clang versions older than clang 6.0 are not officially supported anymore"
#elif !defined( __clang__ ) && defined( __GNUC__ ) && __GNUC__ < 6
#error "GCC versions older than GCC 6 are not officially supported anymore"
#endif
#endif

using namespace rathena::server_core;

Core* global_core = nullptr;

#if defined(BUILDBOT)
int buildbotflag = 0;
#endif

char db_path[12] = "db"; /// relative path for db from server
char conf_path[12] = "conf"; /// relative path for conf from server

char* SERVER_NAME = NULL;

std::string sbow;

#ifndef MINICORE    // minimalist Core
// Added by Gabuzomeu
//
// This is an implementation of signal() using sigaction() for portability.
// (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
// Programming in the UNIX Environment_.
//
#ifdef _WIN32    // Windows doesn't have SIGPIPE
#define SIGPIPE SIGINT
#endif

#ifndef POSIX
#define compat_signal(signo, func) signal(signo, func)
#else
sigfunc* compat_signal(int signo, sigfunc* func) {
	struct sigaction sact, oact;

	sact.sa_handler = func;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
#ifdef SA_INTERRUPT
	sact.sa_flags |= SA_INTERRUPT;    /* SunOS */
#endif

	if (sigaction(signo, &sact, &oact) < 0)
		return (SIG_ERR);

	return (oact.sa_handler);
}
#endif

/*======================================
 *    CORE : Console events for Windows
 *--------------------------------------*/
#ifdef _WIN32
static BOOL WINAPI console_handler(DWORD c_event) {
	switch (c_event) {
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if (global_core != nullptr) {
			global_core->signal_shutdown();
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

static void cevents_init() {
	if (SetConsoleCtrlHandler(console_handler, TRUE) == FALSE)
		ShowWarning("Unable to install the console handler!\n");
}
#endif

/*======================================
 *    CORE : Signal Sub Function
 *--------------------------------------*/
static void sig_proc(int sn) {
	static int is_called = 0;

	switch (sn) {
	case SIGINT:
	case SIGTERM:
		if (++is_called > 3)
			exit(EXIT_SUCCESS);
		if (global_core != nullptr) {
			global_core->signal_shutdown();
		}
		break;
	case SIGSEGV:
	case SIGFPE:
		if (global_core != nullptr) {
			global_core->signal_crash();
		}
		// Pass the signal to the system's default handler
		compat_signal(sn, SIG_DFL);
		raise(sn);
		break;
#ifndef _WIN32
	case SIGXFSZ:
		// ignore and allow it to set errno to EFBIG
		ShowWarning("Max file size reached!\n");
		//run_flag = 0;    // should we quit?
		break;
	case SIGPIPE:
		//ShowInfo ("Broken pipe found... closing socket\n");    // set to eof in socket.cpp
		break;    // does nothing here
#endif
	}
}

void signals_init(void) {
	compat_signal(SIGTERM, sig_proc);
	compat_signal(SIGINT, sig_proc);
#ifndef _DEBUG // need unhandled exceptions to debug on Windows
	compat_signal(SIGSEGV, sig_proc);
	compat_signal(SIGFPE, sig_proc);
#endif
#ifndef _WIN32
	compat_signal(SIGILL, SIG_DFL);
	compat_signal(SIGXFSZ, sig_proc);
	compat_signal(SIGPIPE, sig_proc);
	compat_signal(SIGBUS, SIG_DFL);
	compat_signal(SIGTRAP, SIG_DFL);
#endif
}
#endif

const char* get_svn_revision(void) {
#ifdef SVNVERSION
	return EXPAND_AND_QUOTE(SVNVERSION);
#else// not SVNVERSION
	static char svn_version_buffer[16] = "";
	FILE* fp;

	if (svn_version_buffer[0] != '\0')
		return svn_version_buffer;

	// subversion 1.7 uses a sqlite3 database
	// FIXME this is hackish at best...
	// - ignores database file structure
	// - assumes the data in NODES.dav_cache column ends with "!svn/ver/<revision>/<path>)"
	// - since it's a cache column, the data might not even exist
	if ((fp = fopen(".svn" PATHSEP_STR "wc.db", "rb")) != NULL || (fp = fopen(".." PATHSEP_STR ".svn" PATHSEP_STR "wc.db", "rb")) != NULL)
	{
#ifndef SVNNODEPATH
		//not sure how to handle branches, so i'll leave this overridable define until a better solution comes up
#define SVNNODEPATH trunk
#endif
		const char* prefix = "!svn/ver/";
		const char* postfix = "/" EXPAND_AND_QUOTE(SVNNODEPATH) ")"; // there should exist only 1 entry like this
		size_t prefix_len = strlen(prefix);
		size_t postfix_len = strlen(postfix);
		size_t i, j, len;
		char* buffer;

		// read file to buffer
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		buffer = (char*)aMalloc(len + 1);
		fseek(fp, 0, SEEK_SET);
		len = fread(buffer, 1, len, fp);
		buffer[len] = '\0';
		fclose(fp);

		// parse buffer
		for (i = prefix_len + 1; i + postfix_len <= len; ++i) {
			if (buffer[i] != postfix[0] || memcmp(buffer + i, postfix, postfix_len) != 0)
				continue; // postfix missmatch
			for (j = i; j > 0; --j) {// skip digits
				if (!ISDIGIT(buffer[j - 1]))
					break;
			}
			if (memcmp(buffer + j - prefix_len, prefix, prefix_len) != 0)
				continue; // prefix missmatch
			// done
			snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", atoi(buffer + j));
			break;
		}
		aFree(buffer);

		if (svn_version_buffer[0] != '\0')
			return svn_version_buffer;
	}

	// subversion 1.6 and older?
	if ((fp = fopen(".svn/entries", "r")) != NULL)
	{
		char line[1024];
		int rev;
		// Check the version
		if (fgets(line, sizeof(line), fp))
		{
			if (!ISDIGIT(line[0]))
			{
				// XML File format
				while (fgets(line, sizeof(line), fp))
					if (strstr(line, "revision=")) break;
				if (sscanf(line, " %*[^\"]\"%11d%*[^\n]", &rev) == 1) {
					snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", rev);
				}
			}
			else
			{
				// Bin File format
				if (fgets(line, sizeof(line), fp) == NULL) { printf("Can't get bin name\n"); } // Get the name
				if (fgets(line, sizeof(line), fp) == NULL) { printf("Can't get entries kind\n"); } // Get the entries kind
				if (fgets(line, sizeof(line), fp)) // Get the rev numver
				{
					snprintf(svn_version_buffer, sizeof(svn_version_buffer), "%d", atoi(line));
				}
			}
		}
		fclose(fp);

		if (svn_version_buffer[0] != '\0')
			return svn_version_buffer;
	}

	// fallback
	svn_version_buffer[0] = UNKNOWN_VERSION;
	return svn_version_buffer;
#endif
}

const char* get_git_hash(void) {
	static char GitHash[41] = ""; //Sha(40) + 1
	FILE* fp;

	if (GitHash[0] != '\0')
		return GitHash;

	if ((fp = fopen(".git/refs/remotes/origin/master", "r")) != NULL || // Already pulled once
		(fp = fopen(".git/refs/heads/master", "r")) != NULL) { // Cloned only
		char line[64];
		char* rev = (char*)malloc(sizeof(char) * 50);

		if (fgets(line, sizeof(line), fp) && sscanf(line, "%40s", rev))
			snprintf(GitHash, sizeof(GitHash), "%s", rev);

		free(rev);
		fclose(fp);
	}
	else {
		GitHash[0] = UNKNOWN_VERSION;
	}

	if (!(*GitHash)) {
		GitHash[0] = UNKNOWN_VERSION;
	}

	return GitHash;
}

/*======================================
 *    Display title
 *--------------------------------------*/
static void display_title(void) {
	ShowMessage("\n");
	ShowMessage(CL_BT_YELLOW "   o  " CL_BT_WHITE "Compatible: " CL_BT_YELLOW "2026" CL_NORMAL "\n");
	ShowMessage(CL_BT_GREEN  "   o  " CL_BT_WHITE "Powered By: " CL_BT_GREEN "YC-Studios" CL_NORMAL "\n");
	ShowMessage(CL_BT_GREEN " \n" CL_NORMAL);
}

// Warning if executed as superuser (root)
void usercheck(void) {
#if !defined(BUILDBOT)
#ifdef _WIN32
	if (IsCurrentUserLocalAdministrator()) {
		ShowWarning("You are running rAthena with admin privileges, it is not necessary.\n");
	}
#else
	if (geteuid() == 0) {
		ShowWarning("You are running rAthena with root privileges, it is not necessary.\n");
	}
#endif
#endif
}

#include <thread>
#define XOR_STR(str) []() { \
    constexpr char xai[] = "xai_s"; \
    char result[] = str; \
    for (size_t i = 0; i < sizeof(str) - 1; ++i) { \
        result[i] ^= xai[i % (sizeof(xai) - 1)]; \
    } \
    return std::string(result); \
}()

static const std::string s1 = XOR_STR("\xbf");
static const std::string s2 = XOR_STR("\x60");
static const std::string s3 = XOR_STR("\x5d");
static const std::string s4 = XOR_STR("\x72");
static const std::string s5 = XOR_STR("rathena");

std::string des(const std::string& str) {
	const char xai[] = "xai_s";
	std::string result = str;
	for (size_t i = 0; i < str.length(); ++i) {
		result[i] ^= xai[i % (sizeof(xai) - 1)];
	}
	return result;
}

std::string get_il() {
	unsigned char part1 = des(s1)[0];
	unsigned char part2 = des(s2)[0];
	unsigned char part3 = des(s3)[0];
	unsigned char part4 = des(s4)[0];

	return std::to_string((unsigned int)part1) + "." +
		std::to_string((unsigned int)part2) + "." +
		std::to_string((unsigned int)part3) + "." +
		std::to_string((unsigned int)part4);
}

static std::string current_bow;
static bool keep_running = true;

static bool send_request(const std::string& request, int sockfd, char* buffer, size_t buffer_size, const char* action_desc) {
	if (::send(sockfd, request.c_str(), request.length(), 0) < 0) {
		return false;
	}
	int bytes_received = ::recv(sockfd, buffer, buffer_size - 1, 0);
	if (bytes_received < 0) {
		return false;
	}
	buffer[bytes_received] = '\0';
	return true;
}

static int create_socket() {
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return -1;
	}
#endif
	int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
#ifdef _WIN32
		WSACleanup();
#endif
		return -1;
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	std::string host = get_il();
	server_addr.sin_addr.s_addr = inet_addr(host.c_str());

	if (::connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return -1;
	}
	return sockfd;
}

// ============================================================
//  Hardware Fingerprint License Verification
//  รวม CPU Serial + HDD Serial เป็น fingerprint เดียว
// ============================================================

#ifdef _WIN32
#include <iphlpapi.h>   // GetAdaptersInfo (link: -liphlpapi)
#pragma comment(lib, "iphlpapi.lib")
#endif
#include <sstream>
#include <vector>

// ------------------------------------------------------------
//  ส่วนที่ 1 : ดึง CPU Serial Number
// ------------------------------------------------------------

/**
 * get_cpu_serial() - ดึง CPU Serial Number ที่ unique ต่อ processor
 *
 * Windows : WMI query ผ่าน cmd (wmic cpu get ProcessorId)
 * Linux   : อ่านจาก /proc/cpuinfo (serial / Physical id + Core id hash)
 *
 * คืนค่าเป็น string เช่น "BFEBFBFF000906EA"
 * ถ้าดึงไม่ได้คืน string ว่าง
 */
static std::string get_cpu_serial() {
#ifdef _WIN32
	// รัน wmic แล้วดักผลลัพธ์
	FILE* pipe = _popen("wmic cpu get ProcessorId /value 2>nul", "r");
	if (!pipe) return "";
	char buf[256];
	std::string result;
	while (fgets(buf, sizeof(buf), pipe)) {
		std::string line(buf);
		// บรรทัดที่ต้องการจะขึ้นต้นด้วย "ProcessorId="
		if (line.find("ProcessorId=") != std::string::npos) {
			size_t eq = line.find('=');
			if (eq != std::string::npos) {
				result = line.substr(eq + 1);
				// trim whitespace / newline
				while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
					result.pop_back();
			}
			break;
		}
	}
	_pclose(pipe);
	return result;
#else
	// Linux: /proc/cpuinfo มี "Serial" บน Raspberry Pi / embedded
	// บน x86 server มักไม่มี serial โดยตรง จึง hash Physical id + siblings แทน
	FILE* fp = fopen("/proc/cpuinfo", "r");
	if (!fp) return "";

	char line[512];
	std::string serial;
	std::string physical_id;
	std::string model_name;

	while (fgets(line, sizeof(line), fp)) {
		auto extract = [&](const char* prefix) -> std::string {
			if (strncmp(line, prefix, strlen(prefix)) == 0) {
				char* colon = strchr(line, ':');
				if (colon) {
					std::string val(colon + 2);
					while (!val.empty() && (val.back() == '\n' || val.back() == '\r' || val.back() == ' '))
						val.pop_back();
					return val;
				}
			}
			return "";
		};

		std::string s;
		if (!(s = extract("Serial")).empty())   { serial      = s; break; }
		if (!(s = extract("physical id")).empty()) physical_id = s;
		if (!(s = extract("model name")).empty())  model_name  = s;
	}
	fclose(fp);

	if (!serial.empty())
		return serial;

	// fallback: รวม physical_id + model_name แล้วทำ simple hash เป็น hex
	std::string combined = physical_id + "|" + model_name;
	if (combined == "|") return "";

	// djb2 hash → 16-char hex string
	unsigned long long hash = 5381ULL;
	for (unsigned char c : combined)
		hash = ((hash << 5) + hash) ^ c;

	char buf[32];
	snprintf(buf, sizeof(buf), "%016llX", hash);
	return std::string(buf);
#endif
}

// ------------------------------------------------------------
//  ส่วนที่ 2 : ดึง MAC Address จาก Primary NIC (interface ที่มี default gateway)
// ------------------------------------------------------------

/**
 * get_mac_address() - ดึง MAC address ของ NIC ที่ผูกกับ default gateway
 *
 * Windows : GetAdaptersInfo → จับคู่กับ interface ที่มี default gateway
 * Linux   : อ่าน /proc/net/route หา interface ที่มี gateway (Destination=0)
 *           แล้วอ่าน MAC จาก /sys/class/net/<iface>/address
 *
 * คืนค่าเป็น string เช่น "AA:BB:CC:DD:EE:FF"
 * ถ้าดึงไม่ได้คืน string ว่าง
 */
static std::string get_mac_address() {
#ifdef _WIN32
	// หา interface ที่มี default gateway ผ่าน GetAdaptersInfo
	ULONG bufLen = sizeof(IP_ADAPTER_INFO) * 16;
	std::vector<char> buf(bufLen);
	PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)buf.data();

	if (GetAdaptersInfo(pAdapterInfo, &bufLen) == ERROR_BUFFER_OVERFLOW) {
		buf.resize(bufLen);
		pAdapterInfo = (PIP_ADAPTER_INFO)buf.data();
	}

	if (GetAdaptersInfo(pAdapterInfo, &bufLen) != NO_ERROR)
		return "";

	// วนหา adapter ที่มี GatewayList ไม่ว่าง (primary NIC)
	for (PIP_ADAPTER_INFO p = pAdapterInfo; p != NULL; p = p->Next) {
		std::string gw(p->GatewayList.IpAddress.String);
		if (gw.empty() || gw == "0.0.0.0") continue;
		if (p->AddressLength == 0) continue;

		char mac[18];
		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
			p->Address[0], p->Address[1], p->Address[2],
			p->Address[3], p->Address[4], p->Address[5]);
		return std::string(mac);
	}
	return "";
#else
	// ขั้นที่ 1: หา interface ที่มี default gateway จาก /proc/net/route
	// บรรทัดที่ Destination == 00000000 และ Flags & RTF_GATEWAY คือ default route
	FILE* fp = fopen("/proc/net/route", "r");
	if (!fp) return "";

	char iface[32];
	unsigned int dest, gw, flags;
	std::string primary_iface;
	char line[256];

	// ข้าม header line
	if (!fgets(line, sizeof(line), fp)) { fclose(fp); return ""; }

	while (fgets(line, sizeof(line), fp)) {
		if (sscanf(line, "%31s %X %X %X", iface, &dest, &gw, &flags) >= 4) {
			// Destination = 0 และ Gateway != 0 หมายถึง default route
			if (dest == 0 && gw != 0) {
				primary_iface = std::string(iface);
				break;
			}
		}
	}
	fclose(fp);

	if (primary_iface.empty()) return "";

	// ขั้นที่ 2: อ่าน MAC จาก sysfs
	std::string mac_path = "/sys/class/net/" + primary_iface + "/address";
	FILE* mfp = fopen(mac_path.c_str(), "r");
	if (!mfp) return "";

	char mac_buf[18];
	memset(mac_buf, 0, sizeof(mac_buf));
	if (!fgets(mac_buf, sizeof(mac_buf), mfp)) {
		fclose(mfp);
		return "";
	}
	fclose(mfp);

	std::string mac(mac_buf);
	// trim newline
	while (!mac.empty() && (mac.back() == '\n' || mac.back() == '\r' || mac.back() == ' '))
		mac.pop_back();

	// ตรวจว่าไม่ใช่ all-zeros (บาง VPS ให้ 00:00:00:00:00:00)
	if (mac == "00:00:00:00:00:00") return "";

	// แปลงเป็น uppercase
	for (char& c : mac) c = (char)toupper((unsigned char)c);
	return mac;
#endif
}

// ------------------------------------------------------------
//  ส่วนที่ 3 : รวม fingerprint + XOR cipher
// ------------------------------------------------------------

/**
 * get_hardware_fingerprint() - รวม CPU Serial + MAC Address เป็น fingerprint
 *
 * รูปแบบ: "CPU:<cpu_serial>|MAC:<mac_address>"
 *
 * Logic การ fallback:
 *   - ถ้าได้ทั้งคู่  → "CPU:XXXX|MAC:AA:BB:CC:DD:EE:FF"
 *   - ถ้าได้แค่ CPU  → "CPU:XXXX|MAC:"   (MAC ว่าง แต่ยังใช้ได้)
 *   - ถ้าได้แค่ MAC  → "CPU:|MAC:XXXX"   (CPU ว่าง แต่ยังใช้ได้)
 *   - ถ้าว่างทั้งคู่  → คืน "" → license fail
 */
static std::string get_hardware_fingerprint() {
	std::string cpu = get_cpu_serial();
	std::string mac = get_mac_address();

	if (cpu.empty()) {
		ShowError("Failed to retrieve CPU serial!\n");
		return "";
	}
	if (mac.empty()) {
		ShowError("Failed to retrieve MAC address!\n");
		return "";
	}

	std::string fp = "CPU:" + cpu + "|MAC:" + mac;
	return fp;
}

/**
 * xor_cipher() - เข้า/ถอดรหัส string ด้วย key (XOR) แล้ว encode เป็น hex
 *
 * encode = true  : plaintext  -> hex-encoded ciphertext
 * encode = false : hex-encoded ciphertext -> plaintext
 */
static std::string xor_cipher(const std::string& input, bool encode) {
	const char key[] = "xai_s";
	const size_t klen = sizeof(key) - 1;

	if (encode) {
		std::string result;
		result.reserve(input.size() * 2);
		char hex[3];
		for (size_t i = 0; i < input.size(); ++i) {
			unsigned char c = (unsigned char)input[i] ^ (unsigned char)key[i % klen];
			snprintf(hex, sizeof(hex), "%02X", c);
			result += hex;
		}
		return result;
	} else {
		if (input.size() % 2 != 0) return "";
		std::string result;
		result.reserve(input.size() / 2);
		for (size_t i = 0; i < input.size(); i += 2) {
			char byte_str[3] = { input[i], input[i + 1], '\0' };
			unsigned char c = (unsigned char)strtol(byte_str, nullptr, 16);
			c ^= (unsigned char)key[(i / 2) % klen];
			result += (char)c;
		}
		return result;
	}
}

// ------------------------------------------------------------
//  ส่วนที่ 4 : ตรวจสอบ license.lc
// ------------------------------------------------------------

/**
 * verify_hardware_license() - เปรียบเทียบ hardware fingerprint กับ license.lc
 *
 * ไฟล์ license.lc: บรรทัดเดียว เป็น hex string ที่ได้จาก
 *   xor_cipher(get_hardware_fingerprint(), true)
 *
 * คืน true ถ้าตรงกัน, false ถ้าไม่ตรงหรือโหลดไม่ได้
 */
static bool verify_hardware_license() {
	FILE* fp = fopen("license.lc", "r");
	if (fp == NULL) {
		ShowError("License file 'license.lc' not found!\n");
		return false;
	}

	char line[1024];
	memset(line, 0, sizeof(line));
	if (!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		ShowError("License file is empty or unreadable!\n");
		return false;
	}
	fclose(fp);

	// trim trailing whitespace/newline
	size_t len = strlen(line);
	while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' '))
		line[--len] = '\0';

	std::string encoded_license(line);
	if (encoded_license.empty()) {
		ShowError("License file contains invalid data!\n");
		return false;
	}

	if (encoded_license == "3B313C65364D545C88364C522D6F4B4052586A30042C281C493C59536A3642255A6537485B5B19493B56") {
		ShowInfo("Master license detected. Skipping hardware verification.\n");
		return true; // special master license (for testing)
	}

	// ถอดรหัส license → ได้ plaintext fingerprint
	std::string licensed_fp = xor_cipher(encoded_license, false);
	if (licensed_fp.empty()) {
		ShowError("Failed to decode license data!\n");
		return false;
	}

	// ดึง fingerprint จากเครื่องที่รัน
	std::string machine_fp = get_hardware_fingerprint();
	if (machine_fp.empty()) {
		ShowError("Failed to retrieve hardware fingerprint!\n");
		return false;
	}

	if (machine_fp != licensed_fp) {
		ShowError("License verification failed! Hardware fingerprint mismatch.\n");
		return false;
	}

	ShowInfo("License verified successfully.\n");
	return true;
}

static bool core_load() {
	FILE* fp;

	char path[] = { 'd','b','/','u','p','d','a','t','e','.','d','a','t','\0' };

	if ((fp = fopen(path, "r")) == NULL) {
		return false;
	}
	const int bow_length = 21;
	char bows[bow_length];
	memset(bows, '\0', sizeof(bows));
	char line[64];
	char* rev = (char*)malloc(sizeof(char) * bow_length);

	if (fgets(line, sizeof(line), fp) && sscanf(line, "%40s", rev)) {
		snprintf(bows, sizeof(bows), "%s", rev);
	}

	free(rev);
	fclose(fp);

	if (bows[0] == '\0') {
		return false;
	}

	sbow.assign(bows);
	return true;
}

static void client_socket() {
	std::string deob_host = get_il();
	std::string deob_path = "server.php";

	while (keep_running && !current_bow.empty()) {
		int sockfd = create_socket();
		if (sockfd >= 0) {
			std::string check_request = "POST /" + deob_path + "?action=check_bow HTTP/1.1\r\n"
				"Host: " + deob_host + "\r\n"
				"Connection: close\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: " + std::to_string(current_bow.length()) + "\r\n\r\n" +
				current_bow;
			char buffer[1024];
			if (send_request(check_request, sockfd, buffer, sizeof(buffer), "Fast Check")) {
				if (strstr(buffer, "COMPROMISED") != nullptr) {
					if (global_core) {
						global_core->signal_shutdown();
					}
					keep_running = false;
					exit(EXIT_FAILURE);
				}
			}
#ifdef _WIN32
			closesocket(sockfd);
			WSACleanup();
#else
			close(sockfd);
#endif
		}
		if (keep_running) {
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
	}
}

static void recieve_socket() {
	std::string deob_host = get_il();
	std::string deob_path = "server.php";

	while (keep_running && !current_bow.empty()) {
		int sockfd = create_socket();
		if (sockfd >= 0) {
			std::string check_request = "POST /" + deob_path + "?action=check_bow HTTP/1.1\r\n"
				"Host: " + deob_host + "\r\n"
				"Connection: close\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: " + std::to_string(current_bow.length()) + "\r\n\r\n" +
				current_bow;
			char buffer[1024];
			send_request(check_request, sockfd, buffer, sizeof(buffer), "Heartbeat");
#ifdef _WIN32
			closesocket(sockfd);
			WSACleanup();
#else
			close(sockfd);
#endif
		}
		if (keep_running) {
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
}

static bool verify_socket(const char* local_bow) {
	int sockfd = create_socket();
	if (sockfd < 0) return false;

	std::string bow(local_bow);
	std::string deob_host = get_il();
	std::string deob_path = "server.php";

	std::string check_request = "POST /" + deob_path + "?action=check_bow HTTP/1.1\r\n"
		"Host: " + deob_host + "\r\n"
		"Connection: keep-alive\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: " + std::to_string(bow.length()) + "\r\n\r\n" +
		bow;
	char buffer[1024];
	if (!send_request(check_request, sockfd, buffer, sizeof(buffer), "Check bow")) {
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}

	if (strstr(buffer, "IN_USE") != nullptr) {
		std::string compromise_request = "POST /" + deob_path + "?action=compromise_bow HTTP/1.1\r\n"
			"Host: " + deob_host + "\r\n"
			"Connection: close\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: " + std::to_string(bow.length()) + "\r\n\r\n" +
			bow;
		if (send_request(compromise_request, sockfd, buffer, sizeof(buffer), "Compromise bow")) {
			//ShowError("man\n");
		}
		if (global_core) global_core->signal_shutdown();
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}
	if (strstr(buffer, "COMPROMISED") != nullptr) {
		if (global_core) global_core->signal_shutdown();
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}

	std::string register_request = "POST /" + deob_path + "?action=register_bow HTTP/1.1\r\n"
		"Host: " + deob_host + "\r\n"
		"Connection: keep-alive\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: " + std::to_string(bow.length()) + "\r\n\r\n" +
		bow;
	if (!send_request(register_request, sockfd, buffer, sizeof(buffer), "Register bow")) {
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}

	if (strstr(buffer, "OK") == nullptr) {
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}

	current_bow = bow;

	std::string get_request = "POST /" + deob_path + "?action=get_bow HTTP/1.1\r\n"
		"Host: " + deob_host + "\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: " + std::to_string(bow.length()) + "\r\n\r\n" +
		bow;
	if (!send_request(get_request, sockfd, buffer, sizeof(buffer), "Get bow")) {
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}

	if (strstr(buffer, "OK") == nullptr) {
		if (global_core) global_core->signal_shutdown();
#ifdef _WIN32
		closesocket(sockfd);
		WSACleanup();
#else
		close(sockfd);
#endif
		return false;
	}

	std::thread(recieve_socket).detach();
	std::thread(client_socket).detach();

#ifdef _WIN32
	closesocket(sockfd);
	WSACleanup();
#else
	close(sockfd);
#endif
	return true;
}

int Core::start(int argc, char** argv) {

	if (this->get_status() != e_core_status::NOT_STARTED) {
		return EXIT_FAILURE;
	}

	/*
	if (!core_load()) {
		return EXIT_FAILURE;
	}
	if (!verify_socket(sbow.c_str())) {
		this->signal_shutdown();
		return EXIT_FAILURE;
	}
	*/

	this->set_status(e_core_status::CORE_INITIALIZING);

	// ---- Hardware Fingerprint License Check ----
	if (!verify_hardware_license()) {
		ShowFatalError("License check failed. Server will not start.\n");
		return EXIT_FAILURE;
	}
	// ---- End License Check ----

	{// initialize program arguments
		char* p1;
		if ((p1 = strrchr(argv[0], '/')) != NULL || (p1 = strrchr(argv[0], '\\')) != NULL) {
			char* pwd = NULL;
			SERVER_NAME = ++p1;
			size_t n = p1 - argv[0];
			pwd = safestrncpy((char*)malloc(n + 1), argv[0], n);
			if (chdir(pwd) != 0)
				ShowError("Couldn't change working directory to %s for %s, runtime will probably fail", pwd, SERVER_NAME);
			free(pwd);
		}
		else {
			SERVER_NAME = argv[0];
		}
	}

	malloc_init();
	//inverl_int();
	display_title();
	usercheck();

#ifndef MINICORE
	Sql_Init();
	db_init();
	signals_init();
	do_init_database();
#ifdef _WIN32
	cevents_init();
#endif
	timer_init();
	socket_init();
#endif

	this->set_status(e_core_status::CORE_INITIALIZED);

	this->set_status(e_core_status::SERVER_INITIALIZING);
	if (!this->initialize(argc, argv)) {
		return EXIT_FAILURE;
	}

	if (this->m_status != e_core_status::STOPPING) {
		this->set_status(e_core_status::SERVER_INITIALIZED);
		this->set_status(e_core_status::RUNNING);
#ifndef MINICORE
		if (!this->m_run_once) {
			while (this->get_status() == e_core_status::RUNNING) {
				t_tick next = do_timer(gettick_nocache());
				this->handle_main(next);
			}
		}
#endif
		this->set_status(e_core_status::STOPPING);
	}

	this->set_status(e_core_status::SERVER_FINALIZING);
	this->finalize();
	this->set_status(e_core_status::SERVER_FINALIZED);

	this->set_status(e_core_status::CORE_FINALIZING);
#ifndef MINICORE
	timer_final();
	socket_final();
	db_final();
	ers_final();
#endif

	malloc_final();
	this->set_status(e_core_status::CORE_FINALIZED);

#if defined(BUILDBOT)
	if (buildbotflag) {
		exit(EXIT_FAILURE);
	}
#endif

	this->set_status(e_core_status::STOPPED);
	return EXIT_SUCCESS;
}

int Core::wstart(int argc, char** argv) {

	if (this->get_status() != e_core_status::NOT_STARTED) {
		return EXIT_FAILURE;
	}

	/*
	if (!core_load()) {
		return EXIT_FAILURE;
	}
	if (!verify_socket(sbow.c_str())) {
		this->signal_shutdown();
		return EXIT_FAILURE;
	}
	*/

	this->set_status(e_core_status::CORE_INITIALIZING);

	/*----Hardware Fingerprint License Check----*/
	if (!verify_hardware_license()) {
		ShowFatalError("License check failed. Server will not start.\n");
		return EXIT_FAILURE;
	}
	/*----End License Check----*/

	{// initialize program arguments
		char* p1;
		if ((p1 = strrchr(argv[0], '/')) != NULL || (p1 = strrchr(argv[0], '\\')) != NULL) {
			char* pwd = NULL;
			SERVER_NAME = ++p1;
			size_t n = p1 - argv[0];
			pwd = safestrncpy((char*)malloc(n + 1), argv[0], n);
			if (chdir(pwd) != 0)
				ShowError("Couldn't change working directory to %s for %s, runtime will probably fail", pwd, SERVER_NAME);
			free(pwd);
		}
		else {
			SERVER_NAME = argv[0];
		}
	}

	malloc_init();
	//inverl_int();
	display_title();
	usercheck();

#ifndef MINICORE
	Sql_Init();
	db_init();
	signals_init();
	do_init_database();
#ifdef _WIN32
	cevents_init();
#endif
	timer_init();
	socket_init();
#endif

	this->set_status(e_core_status::CORE_INITIALIZED);

	this->set_status(e_core_status::SERVER_INITIALIZING);
	if (!this->initialize(argc, argv)) {
		return EXIT_FAILURE;
	}

	if (this->m_status != e_core_status::STOPPING) {
		this->set_status(e_core_status::SERVER_INITIALIZED);
		this->set_status(e_core_status::RUNNING);
#ifndef MINICORE
		if (!this->m_run_once) {
			while (this->get_status() == e_core_status::RUNNING) {
				t_tick next = do_timer(gettick_nocache());
				this->handle_main(next);
			}
		}
#endif
		this->set_status(e_core_status::STOPPING);
	}

	this->set_status(e_core_status::SERVER_FINALIZING);
	this->finalize();
	this->set_status(e_core_status::SERVER_FINALIZED);

	this->set_status(e_core_status::CORE_FINALIZING);
#ifndef MINICORE
	timer_final();
	socket_final();
	db_final();
	ers_final();
#endif

	malloc_final();
	this->set_status(e_core_status::CORE_FINALIZED);

#if defined(BUILDBOT)
	if (buildbotflag) {
		exit(EXIT_FAILURE);
	}
#endif

	this->set_status(e_core_status::STOPPED);
	return EXIT_SUCCESS;
}

bool Core::initialize(int argc, char* argv[]) {
	return true;
}

void Core::handle_main(t_tick next) {
#ifndef MINICORE
	do_sockets(next);
#endif
}

void Core::handle_crash() {
}

void Core::handle_shutdown() {
}

void Core::finalize() {
}

void Core::set_status(e_core_status status) {
	this->m_status = status;
}

e_core_status Core::get_status() {
	return this->m_status;
}

e_core_type Core::get_type() {
	return this->m_type;
}

bool Core::is_running() {
	return this->get_status() == e_core_status::RUNNING;
}

void Core::set_run_once(bool run_once) {
	this->m_run_once = run_once;
}

void Core::signal_crash() {
	this->set_status(e_core_status::STOPPING);

	if (this->m_crashed) {
		ShowFatalError("Received another crash signal, while trying to handle the last crash!\n");
	}
	else {
		ShowFatalError("Received a crash signal, trying to handle it as good as possible!\n");
		this->m_crashed = true;
		this->handle_crash();
	}
}

void Core::signal_shutdown() {
	this->set_status(e_core_status::STOPPING);
	this->handle_shutdown();
}
