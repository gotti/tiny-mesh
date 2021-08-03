#include <gtest/gtest.h>
#include "test.hpp"

TEST(main_test, sendreceive){
  constexpr int a = 1;
  constexpr int b = 1;
  printf("testing");
  EXPECT_EQ(a, b);
}
