#pragma once

#include <log.h>
#include <minecraft/Xbox.h>

#include <memory>

class XboxLiveGameInterface
{
 public:
  static XboxLiveGameInterface& getInstance();
  virtual std::string getCllXToken(bool refresh) = 0;

  virtual std::string getCllXTicket(std::string const& xuid) = 0;
};

class XboxLiveDefaultGameInterface : public XboxLiveGameInterface
{
 protected:
  static const char* const TAG;

 public:
  std::string getCllXToken(bool refresh) override;

  std::string getCllXTicket(std::string const& xuid) override;
};