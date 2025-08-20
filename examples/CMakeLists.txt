# Examples for NeoCpp

# Basic wallet example
add_executable(wallet_example wallet_example.cpp)
target_link_libraries(wallet_example PRIVATE neocpp)

# Transaction example
add_executable(transaction_example transaction_example.cpp)
target_link_libraries(transaction_example PRIVATE neocpp)

# Crypto example
add_executable(crypto_example crypto_example.cpp)
target_link_libraries(crypto_example PRIVATE neocpp)