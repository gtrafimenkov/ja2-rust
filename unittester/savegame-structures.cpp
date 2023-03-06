#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "SectorInfo.h"

#ifdef __cplusplus
}
#endif

// We want compatibility with the original game saved games on
// both Linux and Windows.  We need to make sure that data structured
// used for saving and loading the state have not changed.

TEST(SaveGameStructures, SectorInfo) { EXPECT_EQ(sizeof(SECTORINFO), 116); }
