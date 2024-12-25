#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "Sector.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/StrategicMap.h"
#include "rust_sam_sites.h"

#ifdef __cplusplus
}
#endif

TEST(Bugfixes, SectorName) {
  wchar_t buf[100];

  GetSectorIDString(1, 1, 0, buf, ARR_SIZE(buf), false);
  EXPECT_STREQ(L"A1: Urban", buf);
  ;
  ;

  GetSectorIDString(9, 1, 0, buf, ARR_SIZE(buf), false);
  EXPECT_STREQ(L"A9: Omerta", buf);

  GetSectorIDString(13, 2, 0, buf, ARR_SIZE(buf), false);
  EXPECT_STREQ(L"B13: Drassen", buf);

  GetSectorIDString(13, 2, 0, buf, ARR_SIZE(buf), true);
  EXPECT_STREQ(L"B13: Drassen Airport", buf);

  GetShortSectorString(9, 1, buf, ARR_SIZE(buf));
  EXPECT_STREQ(L"A9", buf);

  GetSectorIDString(15, 4, 0, buf, ARR_SIZE(buf), true);
  EXPECT_STREQ(L"D15: Woods", buf);
  SetSamSiteFound(SamSiteDrassen, true);
  GetSectorIDString(15, 4, 0, buf, ARR_SIZE(buf), true);
  EXPECT_STREQ(L"D15: Woods, SAM site", buf);

  // when world is not loaded
  GetLoadedSectorString(buf, ARR_SIZE(buf));
  EXPECT_STREQ(L"", buf);
}
