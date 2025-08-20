#include <catch2/catch_session.hpp>

// Main test runner for NeoCpp
int main(int argc, char* argv[]) {
    // Run tests with Catch2
    int result = Catch::Session().run(argc, argv);
    return result;
}