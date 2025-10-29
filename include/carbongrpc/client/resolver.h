#ifndef RESOLVER_H
#define RESOLVER_H

#include <ares.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace monolith_grpc::client {
class Resolver {
public:

  enum class Status {
    kInProgress,
    kSuccess,
    kFailure
  };

  enum class QueryType {
    kNone,
    kHostname,
    kSRV
  };

  class Result {
  public:

    Result(ares_channel channel, const char* name, QueryType query_type);
    ~Result();

    [[nodiscard]] Status status();
    [[nodiscard]] int error();
    [[nodiscard]] QueryType query_type();
    [[nodiscard]] std::string request_name();

    // hostname result
    [[nodiscard]] std::string official_name();
    [[nodiscard]] std::vector<std::string> addresses();
    [[nodiscard]] std::vector<std::string> aliases();

    // srv result
    struct Srv {
      std::string host;
      int port{};
      int priority{};
      int weight{};
    };
    std::vector<Srv> srv_result();

  private:

    void Update();
    ares_channel channel_;
    bool need_destroy_channel_;
    Status status_;

    static void ares_cb_host(void* arg, int status, int timeouts, struct hostent* hostent);
    static void ares_cb_query(void* arg, int status, int timeouts, unsigned char* abuf, int alen);

    QueryType query_type_;
    std::string request_name_;
    int error_;

    // hostname result
    std::string official_name_;
    std::vector<std::string> addresses_;
    std::vector<std::string> aliases_;

    // srv result
    std::vector<Srv> srv_results_;
  };

  static void Initialize();
  [[nodiscard]] static std::unique_ptr<Result> Resolve(const char* name, QueryType query_type);
  static void SetServers(const char* servers);

private:

  static std::string servers_;
};
}  // namespace monolith_grpc

#endif
