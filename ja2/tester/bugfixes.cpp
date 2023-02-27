#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "Strategic/StrategicMap.h"

#ifdef __cplusplus
}
#endif

TEST(Bugfixes, SectorName) {
  wchar_t buf[100];
  // EXPECT_EQ(1, 1);
  // EXPECT_TRUE(true);
  GetSectorIDString(1, 1, 0, buf, false);
  EXPECT_STREQ(L"Omerta", buf);
}
