#include "memory.h"
#include <Windows.h>
#include <thread>
#include <iostream>

namespace client {
    namespace dll {
        constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1874050;
        constexpr std::ptrdiff_t dwEntityList = 0x1A1F730;
    }

    namespace C_BaseEntity {
        constexpr std::ptrdiff_t m_CBodyComponent = 0x38;
        constexpr std::ptrdiff_t m_NetworkTransmitComponent = 0x40;
        constexpr std::ptrdiff_t m_nLastThinkTick = 0x320;
        constexpr std::ptrdiff_t m_pGameSceneNode = 0x328;
        constexpr std::ptrdiff_t m_pRenderComponent = 0x330;
        constexpr std::ptrdiff_t m_pCollision = 0x338;
        constexpr std::ptrdiff_t m_iMaxHealth = 0x340;
        constexpr std::ptrdiff_t m_iHealth = 0x344;
        constexpr std::ptrdiff_t m_iTeamNum = 0x3E3;
    }

    namespace C_CSPlayerPawnBase {
        constexpr std::ptrdiff_t m_iIDEntIndex = 0x1458;
    }
}

void Shoot()
{
    std::cout << "[+] Shooting!\n";
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void WaitForExit()
{
    std::cout << "[*] Press Enter to exit...\n";
    std::cin.get();
}

int main()
{
    const Memory memory("cs2.exe");
    const uintptr_t clientBase = memory.GetModuleAddress("client.dll");

    if (!clientBase)
    {
        std::cout << "[!] Failed to find client.dll\n";
        WaitForExit();
        return 1;
    }

    std::cout << "[*] client.dll base address -> 0x" << std::hex << clientBase << std::dec << "\n";

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!GetAsyncKeyState(VK_MENU))
            continue;

        const uintptr_t localPawn = memory.Read<uintptr_t>(clientBase + client::dll::dwLocalPlayerPawn);
        if (!localPawn)
        {
            std::cout << "[!] Failed to get local player pawn\n";
            continue;
        }

        const int localHealth = memory.Read<int>(localPawn + client::C_BaseEntity::m_iHealth);
        if (localHealth <= 0)
        {
            std::cout << "[*] Local player is dead\n";
            continue;
        }

        const BYTE localTeam = memory.Read<BYTE>(localPawn + client::C_BaseEntity::m_iTeamNum);
        const int crosshairEntityIndex = memory.Read<int>(localPawn + client::C_CSPlayerPawnBase::m_iIDEntIndex);

        if (crosshairEntityIndex <= 0)
        {
            std::cout << "[*] Invalid crosshair entity index: " << crosshairEntityIndex << "\n";
            continue;
        }

        const uintptr_t entityList = memory.Read<uintptr_t>(clientBase + client::dll::dwEntityList);
        const uintptr_t listEntry = memory.Read<uintptr_t>(entityList + 0x8 * (crosshairEntityIndex >> 9) + 0x10);
        const uintptr_t entity = memory.Read<uintptr_t>(listEntry + 120 * (crosshairEntityIndex & 0x1FF));

        if (!entity)
        {
            std::cout << "[!] Invalid Entity bro lock in\n";
            continue;
        }

        const int entityHealth = memory.Read<int>(entity + client::C_BaseEntity::m_iHealth);
        if (entityHealth <= 0)
        {
            std::cout << "[*] Valid entity but bros dead\n";
            continue;
        }

        const BYTE entityTeam = memory.Read<BYTE>(entity + client::C_BaseEntity::m_iTeamNum);
        if (localTeam == entityTeam)
        {
            std::cout << "[*] Entity is on the same team :p \n";
            continue;
        }

        std::cout << "[+] Valid entity found. SHOOT THAT MF\n";
        Shoot();
    }

    WaitForExit();
    return 0;
}
