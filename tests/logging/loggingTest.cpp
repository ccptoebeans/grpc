// Copyright © 2026 Fenris Creations ehf.

#include <gtest/gtest.h>
#include <carbongrpc/client/grpc_log.h>
#include "absl/log/log.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"

class TestEnv : public ::testing::Test
{
public:
	static void SetUpTestSuite()
	{
		absl::InitializeLog();
		monolith_grpc::client::GrpcLog::Initialize();
	}
};

TEST_F (TestEnv, SetLogLevelInfo)
{
	monolith_grpc::client::GrpcLog::SetLogLevel( GPR_LOG_SEVERITY_DEBUG );
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 0 );
	LOG(INFO) << "INFO" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 1 );
	LOG(WARNING) << "WARNING" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 1 );
	LOG(ERROR) << "ERROR" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 1 );

	auto entries = monolith_grpc::client::GrpcLog::GetLogEntries();

	for (const auto& entry : entries)
	{
		std::cout << entry.message << std::endl;
	}
}

TEST_F (TestEnv, SetLogLevelWarn)
{
	monolith_grpc::client::GrpcLog::SetLogLevel( GPR_LOG_SEVERITY_INFO );
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 0 );
	LOG(INFO) << "INFO" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 0 );
	LOG(WARNING) << "WARNING" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 1 );
	LOG(ERROR) << "ERROR" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 1 );
	auto entries = monolith_grpc::client::GrpcLog::GetLogEntries();

	for (const auto& entry : entries)
	{
		std::cout << entry.message << std::endl;
	}
}

TEST_F (TestEnv, SetLogLevelErr)
{
	monolith_grpc::client::GrpcLog::SetLogLevel( GPR_LOG_SEVERITY_ERROR );
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 0 );
	LOG(INFO) << "INFO" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 0 );
	LOG(WARNING) << "WARNING" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 0 );
	LOG(ERROR) << "ERROR" << std::endl;
	ASSERT_EQ( monolith_grpc::client::GrpcLog::GetLogEntries().size(), 1 );

}