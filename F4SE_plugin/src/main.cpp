#include "RE/Bethesda/Actor.hpp"
#include "RE/Bethesda/PlayerCharacter.hpp"
#include "RE/Bethesda/ProcessLists.hpp"
#include "RE/Bethesda/UI.hpp"

#include <atomic>
#include <chrono>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

std::atomic<bool> g_isAudioActive = false;

struct MonsterData
{
	bool found = false;
	float hp = -100.0f;
	float scale = 1.0f;
	float speed = 0.0f;
	float combat = 0.0f;
	float attack = 0.0f;
};

void SendOSCFloat(SOCKET sock, sockaddr_in& dest, const char* address, float value)
{
	char buffer[1024];
	int offset = 0;

	int addrLen = static_cast<int>(strlen(address));
	strcpy_s(buffer + offset, 1024 - offset, address);
	offset += addrLen;
	do {
		buffer[offset++] = 0;
	} while (offset % 4 != 0);

	strcpy_s(buffer + offset, 1024 - offset, ",f");
	offset += 2;
	do {
		buffer[offset++] = 0;
	} while (offset % 4 != 0);

	unsigned int floatBits;
	memcpy(&floatBits, &value, sizeof(float));
	floatBits = htonl(floatBits);
	memcpy(buffer + offset, &floatBits, 4);
	offset += 4;

	sendto(sock, buffer, offset, 0, (sockaddr*)&dest, sizeof(dest));
}

bool IsGamePaused()
{
	auto ui = RE::UI::GetSingleton();
	if (ui != nullptr) {
		return (ui->menuMode > 0);
	}
	return false;
}

MonsterData GetNearestMonsterData()
{
	MonsterData result;

	auto player = RE::PlayerCharacter::GetSingleton();
	auto processLists = RE::ProcessLists::GetSingleton();
	auto av = RE::ActorValue::GetSingleton();

	if (player != nullptr && processLists != nullptr && av != nullptr && av->health != nullptr) {
		RE::Actor* nearestHostile = nullptr;
		float minDistance = 1500.0f;
		auto playerPos = player->GetPosition();

		for (auto& handle : processLists->highActorHandles) {
			auto actorPtr = handle.get();
			RE::Actor* actor = actorPtr.get();

			if (actor != nullptr && actor != player) {
				if (!actor->IsDead(true) && actor->GetHostileToActor(player)) {
					float currentHP = actor->GetActorValue(*(av->health));
					if (currentHP > 0.0f) {
						float dist = playerPos.GetDistance(actor->GetPosition());
						if (dist < minDistance) {
							minDistance = dist;
							nearestHostile = actor;
						}
					}
				}
			}
		}

		if (nearestHostile != nullptr) {
			float currentHP = nearestHostile->GetActorValue(*(av->health));
			float maxHP = nearestHostile->GetBaseActorValue(*(av->health));

			if (maxHP > 0.0f) {
				result.found = true;
				result.hp = currentHP / maxHP;
				result.scale = static_cast<float>(nearestHostile->refScale) / 100.0f;

				static RE::Actor* lastActor = nullptr;
				static auto lastPos = nearestHostile->GetPosition();
				static auto lastTime = std::chrono::steady_clock::now();
				static float smoothedSpeed = 0.0f;

				auto currentPos = nearestHostile->GetPosition();
				auto now = std::chrono::steady_clock::now();
				float dt = std::chrono::duration<float>(now - lastTime).count();

				if (nearestHostile == lastActor && dt > 0.001f && dt < 0.5f) {
					float rawSpeed = currentPos.GetDistance(lastPos) / dt;
					smoothedSpeed = smoothedSpeed * 0.7f + rawSpeed * 0.3f;
					if (smoothedSpeed < 1.0f)
						smoothedSpeed = 0.0f;
				}
				else {
					smoothedSpeed = 0.0f;
				}

				lastActor = nearestHostile;
				lastPos = currentPos;
				lastTime = now;

				result.speed = smoothedSpeed;
				result.combat = nearestHostile->IsInCombat() ? 1.0f : 0.0f;
				result.attack = (nearestHostile->meleeAttackState > 0) ? 1.0f : 0.0f;
			}
		}
	}
	return result;
}

void AudioSenderLoop()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		WSACleanup();
		return;
	}

	sockaddr_in dest = {};
	dest.sin_family = AF_INET;
	dest.sin_port = htons(9000);
	inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

	bool uKeyPressedPrevious = false;
	bool wasTargetPresent = false;

	while (true) {
		bool uKeyPressedNow = (GetAsyncKeyState(0x55) & 0x8000) != 0;
		if (uKeyPressedNow && !uKeyPressedPrevious) {
			g_isAudioActive = !g_isAudioActive;
		}
		uKeyPressedPrevious = uKeyPressedNow;

		if (g_isAudioActive.load()) {
			bool paused = false;
			MonsterData data;

			__try {
				paused = IsGamePaused();
				if (!paused) {
					data = GetNearestMonsterData();
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				data.found = false;
			}

			if (!paused) {
				if (data.found) {
					SendOSCFloat(sock, dest, "/synth/hp", data.hp);
					SendOSCFloat(sock, dest, "/synth/scale", data.scale);
					SendOSCFloat(sock, dest, "/synth/speed", data.speed);
					SendOSCFloat(sock, dest, "/synth/combat", data.combat);
					SendOSCFloat(sock, dest, "/synth/attack", data.attack);
					wasTargetPresent = true;
				}
				else if (wasTargetPresent) {
					SendOSCFloat(sock, dest, "/synth/hp", -100.0f);
					wasTargetPresent = false;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

F4SE_EXPORT constinit auto F4SEPlugin_Version = []() noexcept {
	auto data = F4SE::PluginVersionData();
	data.AuthorName("Aleksandra");
	data.PluginName("MyAudioPlugin");
	data.PluginVersion(1);
	data.UsesAddressLibrary(true);
	data.IsLayoutDependent(false);
	data.UsesSigScanning(false);
	data.HasNoStructUse(false);
	data.CompatibleVersions({ REL::Version(1, 10, 163, 0) });
	return data;
}();

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = "MyAudioPlugin";
	a_info->version = 1;
	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);
	std::thread(AudioSenderLoop).detach();
	return true;
}
