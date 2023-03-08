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
  EXPECT_EQ(GetSamSiteX(0), 2);
  EXPECT_EQ(GetSamSiteY(0), 4);
  EXPECT_EQ(GetSamSiteX(1), 15);
  EXPECT_EQ(GetSamSiteY(1), 4);
}
