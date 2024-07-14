#pragma once

#include "../../client/base_service_client_impl.h"
#include "../common/unix_connection.h"

namespace simpleipc
{
  namespace client
  {

    class unix_service_client_impl : public base_service_client_impl
    {
     private:
      int fd = -1;
      std::string path;
      std::shared_ptr<unix_connection> connection;

     public:
      void open(std::string const& path) override;

      void send_message(rpc_message const& msg) override;

      void close() override;

      simpleipc::connection* get_connection() override
      {
        return connection.get();
      }
    };

  }  // namespace client
}  // namespace simpleipc