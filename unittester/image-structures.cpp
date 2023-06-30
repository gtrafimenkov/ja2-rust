#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "SGP/ImgFmt.h"

#ifdef __cplusplus
}
#endif

TEST(ImageStructures, STCIHeader) { EXPECT_EQ(sizeof(STCIHeader), 64); }
