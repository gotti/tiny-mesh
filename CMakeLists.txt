# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.21)
# プロジェクト名と使用する言語を設定
project(test_cmake CXX)
# a.outという実行ファイルをmain.cppとhello.cppから作成
add_executable(a.out main.cpp main.hpp net.cpp net.hpp)

