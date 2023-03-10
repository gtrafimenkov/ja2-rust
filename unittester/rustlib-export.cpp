#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "rust_platform.h"
#include "rust_sam_sites.h"

#ifdef __cplusplus
}
#endif

TEST(RustExport, SamSites) {
  EXPECT_EQ(GetSamSiteCount(), 4);
  EXPECT_EQ(GetSamSiteX(SamSiteChitzena), 2);
  EXPECT_EQ(GetSamSiteY(SamSiteChitzena), 4);
  EXPECT_EQ(GetSamSiteX(SamSiteDrassen), 15);
  EXPECT_EQ(GetSamSiteY(SamSiteDrassen), 4);
}

TEST(RustExport, PlatformOps) {
  Str512 s1;
  bool res = Plat_FileBaseName("foo/bar.txt", &s1);
  EXPECT_EQ(res, true);
  EXPECT_STREQ(s1.buf, "bar.txt");
}

TEST(RustExport, GetStr) {
  char buf[100];
  bool res;

  // should be success because there is enough space
  res = GetStrTest_Foo(buf, 4);
  EXPECT_EQ(res, true);
  EXPECT_STREQ(buf, "Foo");

  // should be failure because there is not enough space
  res = GetStrTest_Foo(buf, 3);
  EXPECT_EQ(res, false);
  EXPECT_STREQ(buf, "");

  // should be success because there is enough space
  res = GetStrTest_HelloRus(buf, 20);
  EXPECT_EQ(res, true);
  EXPECT_STREQ(buf, "Привет");

  Str512 s1;
  res = GetStrTest_Hello512(&s1);
  EXPECT_EQ(res, true);
  EXPECT_STREQ(s1.buf, "Hello String512");
}
