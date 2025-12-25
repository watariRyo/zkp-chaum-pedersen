cmake_minimum_required(VERSION 3.15)
project(zkp)

# C++の標準バージョン
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ライブラリを探す
find_package(Boost CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# 実行ファイルの設定
add_executable(zkp main.cpp)

# ライブラリをリンク
target_link_libraries(zkp PRIVATE Boost::boost)

## テスト用ターゲット
add_executable(unit_tests chaum_pedersen_test.cpp)
target_link_libraries(unit_tests PRIVATE 
    Boost::boost 
    GTest::gtest 
    GTest::gtest_main
)