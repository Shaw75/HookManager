#pragma once
#include <Windows.h>
#include <mutex>
#include <ranges>
#include <unordered_map>
#include "detours/detours.h"

#ifdef _WIN64
#pragma comment(lib, "[x64]detours.lib")
#else
#pragma comment(lib, "[x86]detours.lib")
#endif
//HookManager
class HookManager {
   public:
	   template<typename Fn,typename Handler>
	   static auto Install(Fn func,Handler handler) -> bool {
		   if (reinterpret_cast<void*>(func) != nullptr && reinterpret_cast<void*>(handler) != nullptr) {
			   if (Enable(func, handler)) {
				   std::lock_guard map(lock);
				   holderMap[reinterpret_cast<void*>(handler)] = reinterpret_cast<void*>(func);
				   return true;
			   }
		   }
		   return false;
	   }
	   template<typename Fn>
	   static auto GetOrigin(Fn handler) noexcept -> Fn {
		   std::lock_guard map(lock);
		   if (!holderMap.contain(reinterpret_cast<void*>(handler))) return nullptr;
		   return holderMap[reinterpret_cast<void*>(handler)];
	   }

	   template<typename RType, typename... Params>
	   static auto Call(RType(*handler)(Params...), Params... params) -> RType {
		   auto origin = GetOrigin(handler);
		   if (origin != nullptr) return origin(params...);

		   return RType();
	   }

	   template<typename RType, typename... Params>
	   static auto Ccall(RType(__cdecl* handler)(Params...), Params... params) -> RType {
		   auto origin = GetOrigin(handler);
		   if (origin != nullptr) return origin(params...);

		   return RType();
	   }

	   template<typename RType, typename... Params>
	   static auto Scall(RType(__stdcall* handler)(Params...), Params... params) -> RType {
		   auto origin = GetOrigin(handler);
		   if (origin != nullptr) return origin(params...);

		   return RType();
	   }

	   template<typename RType, typename... Params>
	   static auto Fcall(RType(__fastcall* handler)(Params...), Params... params) -> RType {
		   auto origin = GetOrigin(handler);
		   if (origin != nullptr) return origin(params...);

		   return RType();
	   }

	   template<typename RType, typename... Params>
	   static auto Vcall(RType(__vectorcall* handler)(Params...), Params... params) -> RType {
		   auto origin = GetOrigin(handler);
		   if (origin != nullptr) return origin(params...);

		   return RType();
	   }

	   static auto DetachAll() -> void {
		   
		   for (const auto key : holderMap) Disable(key);
		   holderMap.clear();
	   }


   private:
	   static std::mutex lock;
	   static std::unordered_map<void*, void*> holderMap;
	   
	   template <typename Fn>
	   static auto Disable(Fn handler) -> void {

		   Fn origin = GetOrigin(handler);
		   DetourTransactionBegin();
		   DetourUpdateThread(GetCurrentThread());
		   DetourDetach(&reinterpret_cast<PVOID&>(origin), handler);
		   DetourTransactionCommit();
	   }

	   template<typename Fn>
	   static auto Enable(Fn &func,Fn handler) -> void {
		   DetourTransactionBegin();
		   DetourUpdateThread(GetCurrentThread());
		   const bool ret = DetourAttach(&reinterpret_cast<PVOID&>func, handler);
		   DetourTransactionCommit();
		   retunr ret;
	   }




};
