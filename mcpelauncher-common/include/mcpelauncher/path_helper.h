#pragma once

#include <functional>
#include <string>
#include <vector>

class PathHelper
{
 private:
  static std::string const appDirName;

  struct PathInfo
  {
    std::string appDir;
    std::string homeDir;
    std::string dataHome;
    std::vector<std::string> dataDirs;
    std::string cacheHome;
    std::string overrideDataDir;
    std::string overrideCacheDir;
    std::string gameDir;

    PathInfo();

    void findAppleDirectories();
  };

  static PathInfo pathInfo;

  static std::string findUserHome();

  static std::string findAppDir();

 public:
  static bool fileExists(std::string const& path);

  static std::string getParentDir(std::string const& path);

  static std::string getWorkingDir();

  static std::string findDataFile(std::string const& path);

  static void findAllDataFiles(std::string const& path,
                               std::function<void(std::string)> f);

  static std::string getPrimaryDataDirectory()
  {
    if (!pathInfo.overrideDataDir.empty()) return pathInfo.overrideDataDir;
    return pathInfo.dataHome + "/" + appDirName + "/";
  }

  static std::string getCacheDirectory()
  {
    if (!pathInfo.overrideCacheDir.empty()) return pathInfo.overrideCacheDir;
    return pathInfo.cacheHome + "/" + appDirName + "/";
  }

  static std::string getGameDir()
  {
    if (!pathInfo.gameDir.empty()) return pathInfo.gameDir;
    return getPrimaryDataDirectory();
  }

  static std::string findGameFile(std::string const& path)
  {
    if (!pathInfo.gameDir.empty()) return pathInfo.gameDir + path;
    return findDataFile(path);
  }

  static void setGameDir(std::string gameDir)
  {
    if (!gameDir.empty() && gameDir[gameDir.length() - 1] != '/')
      gameDir += '/';
    pathInfo.gameDir = std::move(gameDir);
  }

  static void setDataDir(std::string dataDir)
  {
    if (!dataDir.empty() && dataDir[dataDir.length() - 1] != '/')
      dataDir += '/';
    pathInfo.overrideDataDir = std::move(dataDir);
  }

  static void setCacheDir(std::string cacheDir)
  {
    if (!cacheDir.empty() && cacheDir[cacheDir.length() - 1] != '/')
      cacheDir += '/';
    pathInfo.overrideCacheDir = std::move(cacheDir);
  }

  static std::string getIconPath()
  {
    return findGameFile("assets/icon.png");
  }
};
