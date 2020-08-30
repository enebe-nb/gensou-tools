#define SWRS_USES_HASH

#include "server.hpp"
#include <windows.h>
#include <shlwapi.h>

#include "swrs.h"
#include "fields.h"
//#include "address.h"

static Server wsserver;
static std::mutex dataLock;
static bool isCreated = false;
static std::string lastCharacters("characters 21 21");
static std::string lastDeck1("deck1");
static std::string lastDeck2("deck2");

static DWORD orig_BattleOnProcess;
#define BattleOnProcess(p) Ccall(p, orig_BattleOnProcess, int, ())()
static DWORD orig_BattleOnDestroy;
#define BattleOnDestroy(p) Ccall(p, orig_BattleOnDestroy, void*, ())()

#define ADDR_BMGR_P1 0x0C
#define ADDR_BMGR_P2 0x10

std::string getDeckBase(void* player, std::string buffer) {
	short** deckData = (short**)ACCESS_PTR(player, CF_DECK_BASE);
	for (int i = 0; i < 20; ++i) {
		buffer += " " + std::to_string(deckData[i/8][i%8]);
	}
	return buffer;
}

void onSocketOpen(websocketpp::connection_hdl conn) {
	std::cout << "socket open" << std::endl;
	std::lock_guard<std::mutex> lock(dataLock);
	if (!isCreated) return;
	wsserver.send(conn, lastCharacters);
	wsserver.send(conn, lastDeck1);
	wsserver.send(conn, lastDeck2);
}

void comp_BattleOnCreate(void* This) {
	void* p1 = ACCESS_PTR(g_pbattleMgr, ADDR_BMGR_P1);
	void* p2 = ACCESS_PTR(g_pbattleMgr, ADDR_BMGR_P2);
	
	dataLock.lock();
	lastCharacters = std::string("characters ") + std::to_string((int)ACCESS_CHAR(p1, CF_CHARACTER_INDEX)) + " " + std::to_string((int)ACCESS_CHAR(p2, CF_CHARACTER_INDEX));
	lastDeck1 = getDeckBase(p1, "deck1");
	lastDeck2 = getDeckBase(p2, "deck2");
	dataLock.unlock();

	wsserver.send(lastCharacters);
	wsserver.send(lastDeck1);
	wsserver.send(lastDeck2);
}

// 0x2c ROUND BEFORE
// 0x20 GAME LOOP
// 0x1c ROUND LOOP
// 0x08 DESTROY 2

int __fastcall repl_BattleOnProcess(void* This) {
	dataLock.lock();
	if (!isCreated) {
		isCreated = true;
		dataLock.unlock();
		comp_BattleOnCreate(This);
	} else dataLock.unlock();

	int ret = BattleOnProcess(This);
	return ret;	
}

void* __fastcall repl_BattleOnDestroy(void* This) {
	dataLock.lock();
	isCreated = false;
	dataLock.unlock();

	return BattleOnDestroy(This);
}

/* Entry point of the module */
extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

std::ofstream outlog;

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	outlog.open("outlog.txt");
	std::cout.set_rdbuf(outlog.rdbuf());
	std::cout << "starting dll" << std::endl;
	std::cout.flush();

	DWORD old;
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_WRITECOPY, &old);
	orig_BattleOnProcess = TamperDword(vtbl_CBattleManager + 0x0c, union_cast<DWORD>(repl_BattleOnProcess));
	orig_BattleOnDestroy = TamperDword(vtbl_CBattleManager + 0x08, union_cast<DWORD>(repl_BattleOnDestroy));
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	wsserver.start(onSocketOpen);

	return true;
}

extern "C" __declspec(dllexport) void AtExit() {
	wsserver.stop();
	outlog.close();
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return true;
}