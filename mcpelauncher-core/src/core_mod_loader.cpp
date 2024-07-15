#include <dirent.h>
#include <elf.h>
#include <hybris/dlfcn.h>
#include <log.h>
#include <mcpelauncher/core_mod_loader.h>
#include <mcpelauncher/hook.h>

#include <queue>

#include "mcpelauncher/mod_loader.h"

void* CoreModLoader::loadMod(std::string const& path)
{
  void* handle = dlopen(path.c_str(), RTLD_LAZY);
  if (handle == nullptr)
  {
    Log::error("CoreModLoader", "Failed to load mod %s: %s", path.c_str(),
               dlerror());
    return nullptr;
  }

  const auto initFunc =
      reinterpret_cast<void (*)()>(dlsym(handle, "coremod_onLoad"));
  if (!initFunc)
  {
    Log::warn("CoreModLoader", "Mod %s does not have an onLoad function",
              path.c_str());
    return handle;
  }
  initFunc();

  return handle;
}

void CoreModLoader::loadModMulti(std::string const& path,
                                 std::string const& fileName,
                                 std::set<std::string>& otherMods)
{
  auto deps = getModDependencies(path + fileName);
  for (auto const& dep : deps)
  {
    if (otherMods.count(dep) > 0)
    {
      std::string modName = dep;
      otherMods.erase(dep);
      loadModMulti(path, modName, otherMods);
      otherMods.erase(dep);
    }
  }

  Log::info("CoreModLoader", "Loading mod: %s", fileName.c_str());
  void* mod = loadMod(path + fileName);
  if (mod != nullptr) mods.push_back(mod);
}

std::shared_ptr<CoreModLoader> CoreModLoader::m_Instance;
std::shared_ptr<CoreModLoader> CoreModLoader::getInstance()
{
  if (!m_Instance) m_Instance = std::make_shared<CoreModLoader>();

  return m_Instance;
}

void CoreModLoader::loadModsFromDirectory(std::string const& path)
{
  if (path.at(0) != '/')
    throw std::runtime_error("Path must be absolute!");

  DIR* dir = opendir(path.c_str());
  dirent* ent;
  if (dir == nullptr) return;
  Log::info("CoreModLoader", "Loading mods");
  std::set<std::string> modsToLoad;
  while ((ent = readdir(dir)) != nullptr)
  {
    if (ent->d_name[0] == '.') continue;
    std::string fileName(ent->d_name);
    const auto len = fileName.length();
    if (len < 4 || fileName[len - 3] != '.' || fileName[len - 2] != 's' ||
        fileName[len - 1] != 'o')
      continue;

    modsToLoad.insert(fileName);
  }
  closedir(dir);

  Log::info("CoreModLoader", "Found %li mods to load.", modsToLoad.size());
  while (!modsToLoad.empty())
  {
    auto it = modsToLoad.begin();
    const auto& fileName = *it;
    modsToLoad.erase(it);

    loadModMulti(path, fileName, modsToLoad);
  }
  Log::info("CoreModLoader", "Loaded %li mods", mods.size());
}

void CoreModLoader::onCreate(void* handle)
{
  for (const auto& mod : this->mods)
  {
    const auto func =
        reinterpret_cast<void (*)(void*)>(dlsym(mod, "coremod_onCreate"));
    if (func) func(handle);
  }
}

void CoreModLoader::onStart(GenericMinecraft *genericMinecraft)
{
  for (const auto& mod : this->mods)
  {
    const auto func =
        reinterpret_cast<void (*)(GenericMinecraft*)>(dlsym(mod, "coremod_onStart"));
    if (func) func(genericMinecraft);
  }
}

void CoreModLoader::onGUIRequested()
{
  for (const auto& mod : this->mods)
  {
    const auto func =
        reinterpret_cast<void (*)()>(dlsym(mod, "coremod_onGUIRequested"));
    if (func) func();
  }
}

void CoreModLoader::onKeyboardInput(int key, int action)
{
  for (const auto& mod : this->mods)
  {
    const auto func =
        reinterpret_cast<void (*)(int, int)>(dlsym(mod, "coremod_onKeyboardInput"));
    if (func) func(key, action);
  }
}


void CoreModLoader::onGameWindowCreated(std::shared_ptr<GameWindow> gameWindow)
{
   for (const auto& mod : this->mods)
  {
    const auto func =
        reinterpret_cast<void (*)(std::shared_ptr<GameWindow>)>(dlsym(mod, "coremod_onGameWindowCreated"));
    if (func) func(gameWindow);
  }
}

std::vector<std::string> CoreModLoader::getModDependencies(
    std::string const& path)
{
  Elf32_Ehdr header;
  FILE* file = fopen(path.c_str(), "r");
  if (file == nullptr)
  {
    Log::error("CoreModLoader", "getModDependencies: failed to open mod");
    return {};
  }
  if (fread(&header, sizeof(Elf32_Ehdr), 1, file) != 1)
  {
    Log::error("CoreModLoader", "getModDependencies: failed to read header");
    fclose(file);
    return {};
  }

  fseek(file, header.e_phoff, SEEK_SET);

  char phdr[header.e_phentsize * header.e_phnum];
  if (fread(phdr, header.e_phentsize, header.e_phnum, file) != header.e_phnum)
  {
    Log::error("CoreModLoader", "getModDependencies: failed to read phnum");
    fclose(file);
    return {};
  }

  // find dynamic
  const Elf32_Phdr* dynamicEntry = nullptr;
  for (int i = 0; i < header.e_phnum; i++)
  {
    Elf32_Phdr& entry =
        *reinterpret_cast<Elf32_Phdr*>(&phdr[header.e_phentsize * i]);
    if (entry.p_type == PT_DYNAMIC) dynamicEntry = &entry;
  }
  if (dynamicEntry == nullptr)
  {
    Log::error("CoreModLoader", "getModDependencies: couldn't find PT_DYNAMIC");
    fclose(file);
    return {};
  }
  const size_t dynamicDataCount = dynamicEntry->p_filesz / sizeof(Elf32_Dyn);
  Elf32_Dyn dynamicData[dynamicDataCount];
  fseek(file, (long)dynamicEntry->p_offset, SEEK_SET);
  if (fread(dynamicData, sizeof(Elf32_Dyn), dynamicDataCount, file) !=
      dynamicDataCount)
  {
    Log::error("CoreModLoader",
               "getModDependencies: failed to read PT_DYNAMIC");
    fclose(file);
    return {};
  }

  // find strtab
  size_t strtabOff = 0;
  size_t strtabSize = 0;
  for (int i = 0; i < dynamicDataCount; i++)
  {
    if (dynamicData[i].d_tag == DT_STRTAB)
    {
      strtabOff = dynamicData[i].d_un.d_val;
    }
    else if (dynamicData[i].d_tag == DT_STRSZ)
    {
      strtabSize = dynamicData[i].d_un.d_val;
    }
  }
  if (strtabOff == 0 || strtabSize == 0)
  {
    Log::error("CoreModLoader", "getModDependencies: couldn't find strtab");
    fclose(file);
    return {};
  }
  std::vector<char> strtab;
  strtab.resize(strtabSize);
  fseek(file, (long)strtabOff, SEEK_SET);
  if (fread(strtab.data(), 1, strtabSize, file) != strtabSize)
  {
    Log::error("CoreModLoader", "getModDependencies: failed to read strtab");
    fclose(file);
    return {};
  }
  std::vector<std::string> ret;
  for (int i = 0; i < dynamicDataCount; i++)
  {
    if (dynamicData[i].d_tag == DT_NEEDED)
      ret.emplace_back(&strtab[dynamicData[i].d_un.d_val]);
  }
  return ret;
}