#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "Utils/Text.h"

#ifdef __cplusplus
}
#endif

TEST(Language, Basic) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
  EXPECT_STREQ(L"Omerta", pTownNames[1]);
}
