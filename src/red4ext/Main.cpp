#include "RED4ext/RTTITypes.hpp"
#include "Utils.hpp"
#include "Version.hpp"
#include "stdafx.hpp"
#include <RED4ext/RED4ext.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct ClassNameAddr {
  std::string name;
  uintptr_t address;
  ClassNameAddr(std::string name, uintptr_t address) : name(name), address(address){};
};

bool GetVFTRVA(RED4ext::CGameApplication *app) {

  auto rtti = RED4ext::CRTTISystem::Get();

  //// auto types = RED4ext::DynArray<RED4ext::CBaseRTTIType*>(new RED4ext::Memory::DefaultAllocator());
  //// rtti->GetNativeTypes(types);
  auto classes = RED4ext::DynArray<RED4ext::CClass *>(new RED4ext::Memory::DefaultAllocator());
  auto types = RED4ext::DynArray<RED4ext::CBaseRTTIType *>(new RED4ext::Memory::DefaultAllocator());
  rtti->GetClasses(nullptr, classes);
  rtti->GetNativeTypes(types);

  std::map<std::string, uintptr_t> classNameToAddr;

  for (const auto cls : classes) {
    if (cls && cls->name != "None") { // && !cls->flags.isAbstract) {
      if (cls->name == "inkInputKeyIconManager" || cls->name == "exEntitySpawner")
        continue;
      auto name = cls->name.ToString();
      auto instance = (RED4ext::ISerializable *)cls->AllocMemory();
      cls->ConstructCls(instance);
      if (instance) {
        classNameToAddr.insert_or_assign(std::format("{}_VFT", name), *reinterpret_cast<uintptr_t *>(instance));
        // RED4ext::CClass * base = cls;
        // while (base->parent) {
        //   base = base->parent;
        // }
        // if (base->GetName() == "ISerializable") {
          // RED4ext::CClass * ptr = instance->GetNativeType();
          // classNameToAddr.insert_or_assign(std::format("{}_Class_p", name), reinterpret_cast<uintptr_t>(&ptr));
          // classNameToAddr.insert_or_assign(std::format("{}_Class_VFT", name), *reinterpret_cast<uintptr_t *>(cls));
        // }
      }
      auto cls_ptr = reinterpret_cast<uintptr_t>(cls);
      classNameToAddr.insert_or_assign(std::format("{}_Class_VFT", name), *reinterpret_cast<uintptr_t *>(cls));
      classNameToAddr.insert_or_assign(std::format("{}_Class", name), cls_ptr);
      auto data_start = reinterpret_cast<uintptr_t*>(0x3221000 + RED4ext::RelocBase::GetImageBase());
      auto data_end = reinterpret_cast<uintptr_t*>(0x4830000 + RED4ext::RelocBase::GetImageBase());
      while (data_start < data_end) {
        if (*data_start == cls_ptr) {
          classNameToAddr.insert_or_assign(std::format("{}_Class_p", name), reinterpret_cast<uintptr_t>(data_start));
          break;
        }
        data_start++;
      }
    }
  }
  for (const auto &type : types) {
    if (type && type->GetName() != "None" && type->GetType() == RED4ext::ERTTIType::Fundamental) {
      auto name = type->GetName().ToString();
      // if (!classNameToAddr.contains(std::format("{}_Class", name))) {
        classNameToAddr.insert_or_assign(std::format("{}_Type_VFT", name), *reinterpret_cast<uintptr_t *>(type));
        classNameToAddr.insert_or_assign(std::format("{}_Type", name), reinterpret_cast<uintptr_t>(type));
      // }
    }
  }

  std::vector<ClassNameAddr> classNameAddrs;

  std::for_each(classNameToAddr.begin(), classNameToAddr.end(),
                [&](const auto &item) { classNameAddrs.emplace_back(ClassNameAddr(item.first, item.second)); });

  std::sort(classNameAddrs.begin(), classNameAddrs.end(),
            [](ClassNameAddr const &a, ClassNameAddr const &b) { return a.name < b.name; });

  std::stringstream header;
  std::stringstream enum_;
  std::stringstream json;
  std::stringstream ida;

  header << "#pragma once" << std::endl << "// This file was generated automatically" << std::endl << std::endl;
  enum_ << "#pragma once" << std::endl << "// This file was generated automatically" << std::endl << std::endl;
  enum_ << "enum class RTTI_VFT : long {" << std::endl;
  json << "[" << std::endl;

  // {
  //   auto name = "resourceGameDepot_VFT";
  //   auto rva = *reinterpret_cast<uintptr_t *>(app->unk118) - RED4ext::RelocBase::GetImageBase();
  //   header << std::format("#define {}_Addr 0x{:X}", name, rva) << std::endl;
  //   enum_ << std::format(" {} = 0x{:X},", name, rva) << std::endl;
  // }

  // {
  //   auto name = "CGameApplication_unk110_VFT";
  //   auto rva = *reinterpret_cast<uintptr_t *>(app->unk110) - RED4ext::RelocBase::GetImageBase();
  //   header << std::format("#define {}_Addr 0x{:X}", name, rva) << std::endl;
  //   enum_ << std::format(" {} = 0x{:X},", name, rva) << std::endl;
  // }

  header << "// From the RTTISystem" << std::endl;
  header << std::format("// Base address: 0x{:X}", RED4ext::RelocBase::GetImageBase()) << std::endl;
  enum_ << "// From the RTTISystem" << std::endl;
  enum_ << std::format("// Base address: 0x{:X}", RED4ext::RelocBase::GetImageBase()) << std::endl;

  bool first = true;

  for (auto &pair : classNameAddrs) {
    auto rva = pair.address - RED4ext::RelocBase::GetImageBase();
    if (pair.address > RED4ext::RelocBase::GetImageBase() && rva < 0x4830000) { // where pdata starts
      header << std::format("#define {}_Addr 0x{:X}", pair.name, rva) << std::endl;
      enum_ << std::format(" {} = 0x{:X},", pair.name, rva) << std::endl;
      if (first) {
        first = false;
      } else {
        json << "," << std::endl;
      }
      json << std::format("\t{{\n\t\t\"symbol\": \"{}\",\n\t\t\"offset\": \"{:X}\" \n\t}}", pair.name, rva);
      ida << std::format("// START_DECL VTABLE {}\n{}\n// END_DECL\n", rva, pair.name);
    } else {
      header << std::format("// {}'s address 0x{:X} not within expected values", pair.name, pair.address) << std::endl;
      // enum_ << std::format("  // {} rva 0x{:X} not within expected values", pair.name, rva) << std::endl;
    }
  }

  // classes.Clear();

  enum_ << "};" << std::endl;
  json << std::endl << "]";

  std::ofstream headerFile(
      "C:/Users/Jack/Documents/cyberpunk/flight_control/deps/red4ext.sdk/include/RED4ext/Addresses-VFT.hpp");
  headerFile << header.rdbuf();
  headerFile.close();

  std::ofstream enumFile(
      "C:/Users/Jack/Documents/cyberpunk/flight_control/deps/red4ext.sdk/include/RED4ext/Enum-VFT.hpp");
  enumFile << enum_.rdbuf();
  enumFile.close();

  std::ofstream jsonFile(
      "C:/Program Files (x86)/Steam/steamapps/common/Cyberpunk 2077/bin/x64/cyberpunk2077_classes.json");
  jsonFile << json.rdbuf();
  jsonFile.close();

  std::ofstream idaFile(
      "C:/Users/Jack/Documents/cyberpunk/IDA_Cyberpunk_2077/vfts.c");
  idaFile << ida.rdbuf();
  idaFile.close();
  

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
  aInfo->name = L"VFT Ripper";
  aInfo->author = L"Jack Humbert";
  aInfo->version = RED4EXT_SEMVER(MOD_VER_MAJOR, MOD_VER_MINOR, MOD_VER_PATCH);
  aInfo->runtime = RED4EXT_RUNTIME_LATEST;
  aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports() { return RED4EXT_API_VERSION_LATEST; }
