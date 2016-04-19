#include <iostream>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Main
#include <boost/test/unit_test.hpp>

#include <apr.h>
#include "akumuli.h"
#include "storage_engine/blockstore.h"
#include "storage_engine/volume.h"
#include "storage_engine/nbtree.h"
#include "log_iface.h"

void test_logger(aku_LogLevel tag, const char* msg) {
    BOOST_MESSAGE(msg);
}

struct AkumuliInitializer {
    AkumuliInitializer() {
        apr_initialize();
        Akumuli::Logger::set_logger(&test_logger);
    }
};

AkumuliInitializer initializer;

using namespace Akumuli;
using namespace Akumuli::StorageEngine;


static const std::vector<uint32_t> CAPACITIES = { 8, 8 };  // two 64KB volumes
static const std::vector<std::string> VOLPATH = { "volume0", "volume1" };
static const std::string METAPATH = "metavolume";


static void create_blockstore() {
    Volume::create_new(VOLPATH[0].c_str(), CAPACITIES[0]);
    Volume::create_new(VOLPATH[1].c_str(), CAPACITIES[1]);
    MetaVolume::create_new(METAPATH.c_str(), 2, CAPACITIES.data());
}

static std::shared_ptr<BlockStore> open_blockstore() {
    auto bstore = BlockStore::open(METAPATH, VOLPATH);
    return bstore;
}


static void delete_blockstore() {
    apr_pool_t* pool;
    apr_pool_create(&pool, nullptr);
    apr_file_remove(METAPATH.c_str(), pool);
    apr_file_remove(VOLPATH[0].c_str(), pool);
    apr_file_remove(VOLPATH[1].c_str(), pool);
    apr_pool_destroy(pool);
}

BOOST_AUTO_TEST_CASE(Test_nbtree_0) {
    delete_blockstore();
    create_blockstore();

    auto bstore = open_blockstore();
    NBTree tree(42, bstore);

    const int N = 100000;
    for (int i = 0; i < N; i++) {
        tree.append(i, i*0.1);
    }

    NBTreeCursor cursor(tree, 0, N);
    aku_Timestamp curr = 0ull;
    while(!cursor.is_eof()) {
        for (size_t ix = 0; ix < cursor.size(); ix++) {
            aku_Timestamp ts;
            double value;
            aku_Status status;
            std::tie(status, ts, value) = cursor.at(ix);

            BOOST_REQUIRE(status != AKU_SUCCESS);

            BOOST_REQUIRE_EQUAL(curr, ts);

            BOOST_REQUIRE_EQUAL(curr*0.1, value);

            curr++;
        }
    }
    BOOST_REQUIRE_NE(curr, 0ull);

    delete_blockstore();
}