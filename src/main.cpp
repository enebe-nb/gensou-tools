#define SWRS_USES_HASH

#include "server.hpp"
#include "events.hpp"
#include <windows.h>
#include <vector>

#include "swrs.h"

static Server wsserver;
static bool isCreated = false;

static DWORD orig_BattleOnCreate;
static DWORD orig_BattleOnDestroy;
static DWORD orig_BattleOnRoundEvent;

#define LPLAYERID (*(int*)0x00899D10)
#define RPLAYERID (*(int*)0x00899D30)
#define LPLAYERPTR (*(void**)(((int)g_pbattleMgr) + 0x0c))
#define RPLAYERPTR (*(void**)(((int)g_pbattleMgr) + 0x10))

std::vector<int> getDeckBase(void* player) {
    std::vector<int> deck(20);
    short** deckData = *(short***)(((int)player) + 0x5A0);
    for (int i = 0; i < 20; ++i) {
        deck[i] = deckData[i/8][i%8];
    }
    return deck;
}

Event::BattleBegin::BattleBegin() : Event("{\"type\":\"battleBegin\"}"_json) {
    data["left"]["character"] = LPLAYERID;
    data["left"]["deck"] = getDeckBase(LPLAYERPTR);
    data["right"]["character"] = RPLAYERID;
    data["right"]["deck"] = getDeckBase(RPLAYERPTR);
}

Event::BattleEnd::BattleEnd() : Event("{\"type\":\"battleEnd\"}"_json) {}

Event::RoundEnd::RoundEnd() : Event("{\"type\":\"roundEnd\"}"_json) {
    data["left"]["character"] = LPLAYERID;
    data["left"]["rounds"] = *(((char*)LPLAYERPTR) + 0x573);
    data["right"]["character"] = RPLAYERID;
    data["right"]["rounds"] = *(((char*)RPLAYERPTR) + 0x573);
}

void onSocketOpen(websocketpp::connection_hdl conn) {
    if (isCreated) wsserver.send(conn, Event::BattleBegin());
}

void __fastcall repl_BattleOnCreate(void* This, void* edx, int arg) {
    Ccall(This, orig_BattleOnCreate, void, (int))(arg);
    isCreated = true;

    wsserver.send(Event::BattleBegin());
}

void __fastcall repl_BattleOnRoundEvent(void* This, void* edx, int id) {
    Ccall(This, orig_BattleOnRoundEvent, void, (int))(id);

    if (id == 3 || id == 5) wsserver.send(Event::RoundEnd());
}

// 0x2c ROUND END (not really)
// 0x20 GAME LOOP
// 0x1c ROUND LOOP
// 0x08 DESTROY 2
// 0x34 BATTLE EVENT

void* __fastcall repl_BattleOnDestroy(void* This) {
    isCreated = false;
    wsserver.send(Event::BattleEnd());

    return Ccall(This, orig_BattleOnDestroy, void*, ())();
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
    return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
    DWORD old;
    ::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_WRITECOPY, &old);
    orig_BattleOnCreate = TamperDword(vtbl_CBattleManager + 0x04, union_cast<DWORD>(repl_BattleOnCreate));
    orig_BattleOnDestroy = TamperDword(vtbl_CBattleManager + 0x08, union_cast<DWORD>(repl_BattleOnDestroy));
    orig_BattleOnRoundEvent = TamperDword(vtbl_CBattleManager + 0x34, union_cast<DWORD>(repl_BattleOnRoundEvent));
    ::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);
    ::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

    wsserver.start(onSocketOpen);
    return true;
}

extern "C" __declspec(dllexport) void AtExit() {
    wsserver.stop();
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return true;
}