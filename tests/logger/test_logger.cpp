#include <catch2/catch_test_macros.hpp>
#include "neocpp/logger.hpp"
#include <sstream>
#include <iostream>
#include <thread>
#include <vector>

using namespace neocpp;

TEST_CASE("Logger", "[logger]") {
    
    SECTION("Log levels") {
        // Save original level
        auto originalLevel = Logger::getLevel();
        
        // Test setting and getting log level
        Logger::setLevel(LogLevel::DEBUG);
        REQUIRE(Logger::getLevel() == LogLevel::DEBUG);
        
        Logger::setLevel(LogLevel::ERROR);
        REQUIRE(Logger::getLevel() == LogLevel::ERROR);
        
        // Restore original level
        Logger::setLevel(originalLevel);
    }
    
    SECTION("Color settings") {
        // Test color enable/disable
        Logger::setColorEnabled(false);
        // No direct way to test this without capturing output
        Logger::setColorEnabled(true);
    }
    
    SECTION("Logging at different levels") {
        auto originalLevel = Logger::getLevel();
        
        // Set to TRACE to ensure all messages would be logged
        Logger::setLevel(LogLevel::TRACE);
        
        // These should not throw
        REQUIRE_NOTHROW(Logger::trace("Test trace message"));
        REQUIRE_NOTHROW(Logger::debug("Test debug message"));
        REQUIRE_NOTHROW(Logger::info("Test info message"));
        REQUIRE_NOTHROW(Logger::warn("Test warning message"));
        REQUIRE_NOTHROW(Logger::error("Test error message"));
        REQUIRE_NOTHROW(Logger::fatal("Test fatal message"));
        
        // Test with file and line info
        REQUIRE_NOTHROW(Logger::trace("Test with location", __FILE__, __LINE__));
        
        // Test that messages below log level are filtered
        Logger::setLevel(LogLevel::ERROR);
        // These won't be printed but shouldn't throw
        REQUIRE_NOTHROW(Logger::debug("This won't be printed"));
        REQUIRE_NOTHROW(Logger::info("This won't be printed either"));
        
        // Restore original level
        Logger::setLevel(originalLevel);
    }
    
    SECTION("Logging macros") {
        auto originalLevel = Logger::getLevel();
        Logger::setLevel(LogLevel::TRACE);
        
        // Test macros (they include file and line automatically)
        REQUIRE_NOTHROW(LOG_TRACE("Macro trace test"));
        REQUIRE_NOTHROW(LOG_DEBUG("Macro debug test"));
        REQUIRE_NOTHROW(LOG_INFO("Macro info test"));
        REQUIRE_NOTHROW(LOG_WARN("Macro warn test"));
        REQUIRE_NOTHROW(LOG_ERROR("Macro error test"));
        REQUIRE_NOTHROW(LOG_FATAL("Macro fatal test"));
        
        Logger::setLevel(originalLevel);
    }
    
    SECTION("Thread safety") {
        // Test that multiple threads can log simultaneously
        auto originalLevel = Logger::getLevel();
        Logger::setLevel(LogLevel::INFO);
        
        std::vector<std::thread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([i]() {
                for (int j = 0; j < 10; ++j) {
                    Logger::info("Thread " + std::to_string(i) + " message " + std::to_string(j));
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        Logger::setLevel(originalLevel);
    }
}