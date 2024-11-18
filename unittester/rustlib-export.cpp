#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_platform.h"
#include "rust_sam_sites.h"
#include "rust_sector.h"
#include "rust_towns.h"

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

TEST(RustExport, TownSectors) {
  EXPECT_EQ(BLANK_SECTOR, GetTownIdForSector(1, 1));
  EXPECT_EQ(OMERTA, GetTownIdForSector(9, 1));
}

TEST(RustExport, FileMan) {
  File_RegisterSlfLibraries("tools/editor");

  {
    auto id = File_OpenForReading("Editor/EXITGRIDBUT.STI");
    EXPECT_NE(id, FILE_ID_ERR);
    EXPECT_TRUE(File_Close(id));
  }

  {
    auto id = File_OpenForReading("Editor/EXITGRIDBUT.STI");
    EXPECT_NE(id, FILE_ID_ERR);
    uint8_t buf[10];
    uint32_t read_bytes;
    auto res = File_Read(id, buf, sizeof(buf), &read_bytes);
    EXPECT_TRUE(res);
    EXPECT_EQ(10, read_bytes);
    EXPECT_EQ('S', buf[0]);
    EXPECT_EQ('T', buf[1]);
    EXPECT_EQ('C', buf[2]);
  }

  {
    auto id = File_OpenForReading("JA2.sln");
    EXPECT_NE(id, FILE_ID_ERR);
    uint8_t buf[4];
    uint32_t read_bytes;
    auto res = File_Read(id, buf, sizeof(buf), &read_bytes);
    EXPECT_TRUE(res);
    EXPECT_EQ(sizeof(buf), read_bytes);
    EXPECT_EQ('M', buf[0]);
    EXPECT_EQ('i', buf[1]);
    EXPECT_EQ('c', buf[2]);
    EXPECT_EQ('r', buf[3]);

    // seek backward from current position is not supported (because distance is u32)

    // seek forward
    {
      EXPECT_TRUE(File_Read(id, buf, 1, &read_bytes));
      EXPECT_EQ(1, read_bytes);
      EXPECT_EQ('o', buf[0]);

      // skip "sof"
      EXPECT_TRUE(File_Seek(id, 3, FILE_SEEK_CURRENT));
      EXPECT_TRUE(File_Read(id, buf, 1, &read_bytes));
      EXPECT_EQ(1, read_bytes);
      EXPECT_EQ('t', buf[0]);
    }

    EXPECT_TRUE(File_Close(id));
  }
}

TEST(RustExport, FileSize) {
  EXPECT_EQ(3444529, File_Size("tools/editor/Editor.slf"));

  File_RegisterSlfLibraries("tools/editor");
  EXPECT_EQ(5712, File_Size("Editor\\EXITGRIDBUT.STI"));
}

TEST(RustExport, FileManRW) {
  // first write
  {
    auto id = File_OpenForWriting("./tmp-test-write01.txt");
    EXPECT_NE(id, FILE_ID_ERR);
    const char *message = "Hello there 1\n";
    uint32_t written_bytes;
    auto res = File_Write(id, message, strlen(message), &written_bytes);
    EXPECT_TRUE(res);
    EXPECT_EQ(strlen(message), written_bytes);
    EXPECT_TRUE(File_Close(id));
  }

  // second write
  {
    auto id = File_OpenForWriting("./tmp-test-write01.txt");
    EXPECT_NE(id, FILE_ID_ERR);
    const char *message = "Hello there 2\n";
    uint32_t written_bytes;
    auto res = File_Write(id, message, strlen(message), &written_bytes);
    EXPECT_TRUE(res);
    EXPECT_EQ(strlen(message), written_bytes);
    EXPECT_TRUE(File_Close(id));
  }

  // append
  {
    auto id = File_OpenForAppending("./tmp-test-write01.txt");
    EXPECT_NE(id, FILE_ID_ERR);
    const char *message = "Hello there 3\n";
    uint32_t written_bytes;
    auto res = File_Write(id, message, strlen(message), &written_bytes);
    EXPECT_TRUE(res);
    EXPECT_EQ(strlen(message), written_bytes);
    EXPECT_TRUE(File_Close(id));
  }

  // read
  {
    char buf[100];
    auto id = File_OpenForReading("./tmp-test-write01.txt");
    EXPECT_NE(id, FILE_ID_ERR);
    uint32_t read_bytes;
    auto res = File_Read(id, buf, sizeof(buf) - 1, &read_bytes);
    EXPECT_TRUE(res);
    EXPECT_EQ(28, read_bytes);
    EXPECT_TRUE(File_Close(id));

    buf[read_bytes] = 0;
    EXPECT_TRUE(strcmp(buf, "Hello there 2\nHello there 3\n") == 0);
  }

  Plat_DeleteFile("./tmp-test-write01.txt");
}

TEST(RustExport, ReadingGameRes) {
  if (Plat_DirectoryExists("./ReleaseWithDebug/Data")) {
    EXPECT_TRUE(File_RegisterSlfLibraries("./ReleaseWithDebug/Data"));
    EXPECT_TRUE(File_Exists("FONTS\\LARGEFONT1.sti"));
    EXPECT_TRUE(File_Exists("Anims\\VEHICLES\\JEEP.STI"));
    EXPECT_TRUE(File_Exists("GENBUTN.STI"));
    EXPECT_TRUE(File_Exists("TILESETS\\9\\trapdoor.sti"));
  }
}

// TEST(RustExport, DebugLog) {
//   EXPECT_TRUE(DebugLogWrite("hello there"));
//   EXPECT_TRUE(DebugLogWrite("hello there again"));
// }

#include "TileEngine/StructureInternals.h"

TEST(FileMan, LoadStructureData) {
  if (Plat_DirectoryExists("./ReleaseWithDebug/Data")) {
    EXPECT_TRUE(File_RegisterSlfLibraries("./ReleaseWithDebug/Data"));

    const char *file_name = "ANIMS\\STRUCTDATA\\M_CROUCH.JSD";

    FileID hInput = FILE_ID_ERR;
    hInput = File_OpenForReading(file_name);
    EXPECT_TRUE(hInput != 0);

    STRUCTURE_FILE_HEADER Header;
    uint32_t uiBytesRead;
    BOOLEAN fOk = File_Read(hInput, &Header, sizeof(STRUCTURE_FILE_HEADER), &uiBytesRead);
    EXPECT_TRUE(fOk);
    EXPECT_EQ(16, uiBytesRead);
  }
}

TEST(RustExport, FileManParallelRead) {
  // It should be possible to open file two times and read content independently.
  if (Plat_DirectoryExists("./ReleaseWithDebug/Data")) {
    EXPECT_TRUE(File_RegisterSlfLibraries("./ReleaseWithDebug/Data"));

    const char *file_name = "ANIMS\\STRUCTDATA\\M_CROUCH.JSD";

    {
      uint8_t buf[400];
      FileID file_id = File_OpenForReading(file_name);
      EXPECT_TRUE(file_id != 0);
      uint32_t bytesRead = 0xffffffff;
      EXPECT_TRUE(File_Read(file_id, &buf, sizeof(buf), &bytesRead));
      EXPECT_EQ(400, bytesRead);
    }

    {
      uint8_t buf[400];
      FileID file_id = File_OpenForReading(file_name);
      EXPECT_TRUE(file_id != 0);
      uint32_t bytesRead = 0xffffffff;
      EXPECT_TRUE(File_Read(file_id, &buf, sizeof(buf), &bytesRead));
      EXPECT_EQ(400, bytesRead);
    }
  }
}
