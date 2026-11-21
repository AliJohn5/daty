
#include "daty/daty.hpp"
#include <gtest/gtest.h>

#include <filesystem>

class DatyWalTest : public ::testing::Test {
  protected:
    std::filesystem::path path = std::filesystem::temp_directory_path() / "daty_wal_test.dat";

    void SetUp() override {
        std::filesystem::remove(path);
        std::filesystem::remove(path.string() + ".wal");
    }

    void TearDown() override {
        std::filesystem::remove(path);
        std::filesystem::remove(path.string() + ".wal");
    }
};

TEST_F(DatyWalTest, RecoversPutAfterRestart) {
    {
        daty::Daty<> db;
        db.open(path);

        db.put("name", "Ali");
        db.put("country", "Switzerland");
    }

    {
        daty::Daty<> db;
        db.open(path);

        EXPECT_EQ(db.get("name"), "Ali");
        EXPECT_EQ(db.get("country"), "Switzerland");
    }
}

TEST_F(DatyWalTest, RecoversDeleteAfterRestart) {
    {
        daty::Daty<> db;
        db.open(path);

        db.put("name", "Ali");
        db.remove("name");
    }

    {
        daty::Daty<> db;
        db.open(path);

        EXPECT_FALSE(db.contains("name"));
    }
}

TEST_F(DatyWalTest, RecoversClearAfterRestart) {
    {
        daty::Daty<> db;
        db.open(path);

        db.put("a", "1");
        db.put("b", "2");
        db.clear();
    }

    {
        daty::Daty<> db;
        db.open(path);

        EXPECT_TRUE(db.empty());
    }
}

TEST_F(DatyWalTest, RecoversFromSnapshotAndWal) {
    {
        daty::Daty<> db;
        db.open(path);

        db.put("name", "Ali");
        db.save(path.string());

        db.put("name", "Yousef");
    }

    {
        daty::Daty<> db;
        db.open(path);

        EXPECT_EQ(db.get("name"), "Yousef");
    }
}