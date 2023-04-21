#include <RED4ext/RED4ext.hpp>
#include "Utils.hpp"
#include "Version.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <map>
#include "stdafx.hpp"


struct ClassNameAddr {
  std::string name;
  uintptr_t address;
  ClassNameAddr(std::string name, uintptr_t address) : name(name), address(address) {};
};


bool GetVFTRVA(RED4ext::CGameApplication * app) {

 auto rtti = RED4ext::CRTTISystem::Get();

//// auto types = RED4ext::DynArray<RED4ext::CBaseRTTIType*>(new RED4ext::Memory::DefaultAllocator());
//// rtti->GetNativeTypes(types);
 auto classes = RED4ext::DynArray<RED4ext::CClass *>(new RED4ext::Memory::DefaultAllocator());
 rtti->GetClasses(nullptr, classes);

 std::map<std::string, uintptr_t> classNameToAddr;

 for (const auto &cls : classes) {
    if (cls && cls->name != "None" && !cls->flags.isAbstract) {
      if (cls->name == "inkInputKeyIconManager")
        continue;
      auto name = cls->name.ToString();
      auto instance = cls->AllocMemory();
      cls->ConstructCls(instance);
      if (instance) {
        classNameToAddr.insert_or_assign(name, *reinterpret_cast<uintptr_t *>(instance));
      }
      classNameToAddr.insert_or_assign(std::format("{}_Class", name), *reinterpret_cast<uintptr_t *>(cls));
    }
 }

  std::vector<ClassNameAddr> classNameAddrs;

 std::for_each(classNameToAddr.begin(), classNameToAddr.end(), [&](const auto &item) {
   classNameAddrs.emplace_back(ClassNameAddr(item.first, item.second));
 });

  std::sort(classNameAddrs.begin(), classNameAddrs.end(), [](ClassNameAddr const &a, ClassNameAddr const &b) {
    return a.name < b.name;
  });

  std::stringstream header;
  std::stringstream enum_;

  header << "#pragma once" << std::endl << "// This file was generated automatically" << std::endl << std::endl;
  enum_ << "#pragma once" << std::endl << "// This file was generated automatically" << std::endl << std::endl;
  enum_ << "enum class RTTI_VFT : long {" << std::endl;

  {
    auto name = "resourceGameDepot";
    auto rva = *reinterpret_cast<uintptr_t *>(app->unk118) - RED4ext::RelocBase::GetImageBase();
    header << std::format("#define {}_VFT_Addr 0x{:X}", name, rva) << std::endl;
    enum_ << std::format(" {} = 0x{:X},", name, rva) << std::endl;
  }

  {
    auto name = "CGameApplication_unk110";
    auto rva = *reinterpret_cast<uintptr_t *>(app->unk110) - RED4ext::RelocBase::GetImageBase();
    header << std::format("#define {}_VFT_Addr 0x{:X}", name, rva) << std::endl;
    enum_ << std::format(" {} = 0x{:X},", name, rva) << std::endl;
  }

  header << "// From the RTTISystem" << std::endl;
  enum_ << "// From the RTTISystem" << std::endl;

  for (const auto &pair : classNameAddrs) {
     auto rva = pair.address - RED4ext::RelocBase::GetImageBase();
     if (pair.address > RED4ext::RelocBase::GetImageBase() && rva < 0x4700000) {
       header << std::format("#define {}_VFT_Addr 0x{:X}", pair.name, rva) << std::endl;
       enum_ << std::format(" {} = 0x{:X},", pair.name, rva) << std::endl;
     } else {
       header << std::format("// {} has no VFT", pair.name) << std::endl;
       enum_ << std::format("  // {} has no VFT", pair.name) << std::endl;
     }
 }

// classes.Clear();

 enum_ << "};" << std::endl;

 std::ofstream headerFile("Addresses-VFT.hpp");
 headerFile << header.rdbuf();
 headerFile.close();

 std::ofstream enumFile("Enum-VFT.hpp");
 enumFile << enum_.rdbuf();
 enumFile.close();

 DebugBreak();
 return true;
}


RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason,
                                        const RED4ext::Sdk *aSdk) {
  switch (aReason) {
  case RED4ext::EMainReason::Load: {
    // Attach hooks, register RTTI types, add custom states or initalize your
    // application. DO NOT try to access the game's memory at this point, it
    // is not initalized yet.

    Utils::CreateLogger();
    spdlog::info("Starting up VFT Ripper v{}.{}.{}", MOD_VER_MAJOR, MOD_VER_MINOR, MOD_VER_PATCH);
    auto ptr = GetModuleHandle(nullptr);
    spdlog::info("Base address: {}", fmt::ptr(ptr));
    auto modPtr = aHandle;
    spdlog::info("Mod address: {}", fmt::ptr(modPtr));


    RED4ext::GameState initState;

    initState.OnEnter = &GetVFTRVA;
    initState.OnUpdate = nullptr;
    initState.OnExit = nullptr;
    aSdk->gameStates->Add(aHandle, RED4ext::EGameStateType::Running, &initState);

    break;
  }
  case RED4ext::EMainReason::Unload: {
    // Free memory, detach hooks.
    // The game's memory is already freed, to not try to do anything with it.

    spdlog::info("Shutting down");
    spdlog::shutdown();
    break;
  }
  }

  return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo *aInfo) {
  aInfo->name = L"Let There Be Flight";
  aInfo->author = L"Jack Humbert";
  aInfo->version = RED4EXT_SEMVER(MOD_VER_MAJOR, MOD_VER_MINOR, MOD_VER_PATCH);
  aInfo->runtime = RED4EXT_RUNTIME_LATEST;
  aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports() { return RED4EXT_API_VERSION_LATEST; }
