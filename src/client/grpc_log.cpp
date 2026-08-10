// Copyright © 2025 CCP ehf.
#include "carbongrpc/client/grpc_log.h"

#include "carbongrpc/client/stream_status_log.h"

#include <list>
#include <mutex>
#include <string>

#include <absl/log/log_sink_registry.h>

namespace monolith_grpc::client {

class LogSink : public absl::LogSink
{
public:
	void Send(const absl::LogEntry& entry) override
	{
		auto severityEnumType = entry.log_severity();
		// avoid type narrowing, this ensures that the expected int type is the underlying type of the absl::LogSeverity enum class
		// int severity  {static_cast<std::underlying_type_t<decltype(severityEnumType)>>(severityEnumType)};
		if (severityEnumType < m_verbosity)
		{
			return;
		}

		std::scoped_lock lock(m_lock);

		auto sourceFile = std::string(entry.source_filename());
		int sourceLine = entry.source_line();
		auto message = std::string(entry.encoded_message()); // TODO: Work out what the different message functions do

		// avoid type narrowing, this ensures that the expected int type is the underlying type of the absl::LogSeverity enum class
		int severity  {static_cast<std::underlying_type_t<decltype(severityEnumType)>>(severityEnumType)};

		m_log.emplace_back(GrpcLogEntry(sourceFile, sourceLine, (gpr_log_severity)severity, message));

	  	while (m_log.size() > 10000) {
	  	  m_log.pop_front();
	  	}
	}

	void SetVerbosity(int verbosity)
	{
		m_verbosity = static_cast<absl::LogSeverity>(verbosity);
	}

	std::list<GrpcLogEntry> GetLogEntries()
	{
		std::scoped_lock lock(m_lock);

		std::list<GrpcLogEntry> result = std::move(m_log);
		m_log = std::list<GrpcLogEntry>();

		return result;
	}

private:
	absl::LogSeverity m_verbosity;
	std::mutex m_lock;
	std::list<GrpcLogEntry> m_log;
};

GrpcLogEntry::GrpcLogEntry(std::string file, int line, gpr_log_severity severity, std::string message)
  : file(std::move(file)),
    line(line),
    severity(severity),
    message(std::move(message)) {
}

std::once_flag GrpcLog::init_flag_;

LogSink* GrpcLog::log_sink = nullptr;

void GrpcLog::Initialize() {
  std::call_once(init_flag_, []() {
    if( log_sink != nullptr )
    {
		delete log_sink;
    }
  	log_sink = new LogSink();
  	absl::AddLogSink( log_sink );
  });
}

// cppcheck-suppress unusedFunction
void GrpcLog::SetLogLevel(gpr_log_severity level) {
  Initialize();
  // gpr_log_severity values are interchangeable with absl log verbosity values
  log_sink->SetVerbosity( level );
}

// cppcheck-suppress unusedFunction
std::list<GrpcLogEntry> GrpcLog::GetLogEntries() {
  Initialize();
  return log_sink->GetLogEntries();
}

}  // namespace monolith_grpc::client
