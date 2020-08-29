#define SWRS_USES_HASH

#include "server.hpp"
#include <windows.h>
#include <shlwapi.h>

#include "swrs.h"
#include "fields.h"
//#include "address.h"

static Server wsserver;

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

static bool isCreated = false;
void comp_BattleOnCreate(void* This) {
	void* p1 = ACCESS_PTR(g_pbattleMgr, ADDR_BMGR_P1);
	void* p2 = ACCESS_PTR(g_pbattleMgr, ADDR_BMGR_P2);
	//wsserver.send(std::string("created: ") + std::to_string((int)p1) + "/" + std::to_string((int)p2));
	wsserver.send(std::string("characters ") + std::to_string((int)ACCESS_CHAR(p1, CF_CHARACTER_INDEX)) + " " + std::to_string((int)ACCESS_CHAR(p2, CF_CHARACTER_INDEX)));
	wsserver.send(getDeckBase(p1, "deck1"));
	wsserver.send(getDeckBase(p2, "deck2"));
}

// 0x2c ROUND BEFORE
// 0x20 GAME LOOP
// 0x1c ROUND LOOP
// 0x08 DESTROY 2

int __fastcall repl_BattleOnProcess(void* This) {
	if (!isCreated) {
		isCreated = true;
		comp_BattleOnCreate(This);
	}

	int ret = BattleOnProcess(This);

	// Get address to the player data based on data inside "Battle Manager"
	//void* p1 = ACCESS_PTR(battleManager, ADDR_BMGR_P1);
	//void* p2 = ACCESS_PTR(battleManager, ADDR_BMGR_P2);

	/* HEALTH DISPLAY */
	//ACCESS_<variable_type>() is both used for accessing and writing to the resource.
	//Character variables are in fields.h l.1 "Character class"
	//short p1Health = ACCESS_SHORT(p1, CF_CURRENT_HEALTH);
	//short p1Spirit = ACCESS_SHORT(p1, CF_CURRENT_SPIRIT);
	//short p2Health = ACCESS_SHORT(p2, CF_CURRENT_HEALTH);
	//short p2Spirit = ACCESS_SHORT(p2, CF_CURRENT_SPIRIT);

	return ret;	
}

void* __fastcall repl_BattleOnDestroy(void* This) {
	isCreated = false;

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

	wsserver.start();

	return true;
}

extern "C" __declspec(dllexport) void AtExit() {
	wsserver.stop();
	outlog.close();
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return true;
}