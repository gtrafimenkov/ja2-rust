#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "SGP/Types.h"
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
