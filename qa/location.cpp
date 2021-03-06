#include <cassert>
#include <cppunit/extensions/HelperMacros.h>
#include <dlfcn.h>
#include <iostream>
#include <unistd.h>

class location_test : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(location_test);
    CPPUNIT_TEST(test_all_extents);
    CPPUNIT_TEST(test_unsaved_all_extents);
    CPPUNIT_TEST(test_ast_node);
    CPPUNIT_TEST(test_unsaved_ast_node);
    CPPUNIT_TEST(test_extent);
    CPPUNIT_TEST(test_unsaved_extent);
    CPPUNIT_TEST_SUITE_END();

    void test_all_extents();
    void test_unsaved_all_extents();
    void test_ast_node();
    void test_unsaved_ast_node();
    void test_extent();
    void test_unsaved_extent();

    void* m_handle = nullptr;

  public:
    location_test();
    location_test(const location_test&) = delete;
    location_test& operator=(const location_test&) = delete;

    void setUp() override;
    void tearDown() override;
};

location_test::location_test() = default;

void location_test::setUp() {
    m_handle = dlopen("lib/libclang-vim.so", RTLD_NOW);
    if (!m_handle) {
        std::stringstream ss;
        ss << "dlopen() failed: ";
        ss << dlerror();
        CPPUNIT_FAIL(ss.str());
    }
}

void location_test::tearDown() {
    if (m_handle)
        dlclose(m_handle);
}

void location_test::test_all_extents() {
    auto vim_clang_get_all_extents_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_all_extents_at"));
    assert(vim_clang_get_all_extents_at);

    std::string expected_prefix =
        "[{'start':{'line':1,'column':13,'offset':12,'file':'qa/data/"
        "all-extents.cpp',},'end':{'line':4,'column':2,'offset':37,'file':'qa/"
        "data/all-extents.cpp',}},";
    std::string actual(
        vim_clang_get_all_extents_at("qa/data/all-extents.cpp:std=c++1y:3:1"));
    CPPUNIT_ASSERT_EQUAL(
        0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void location_test::test_unsaved_all_extents() {
    auto vim_clang_get_all_extents_at =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_all_extents_at"));
    assert(vim_clang_get_all_extents_at);

    std::string expected_prefix =
        "[{'start':{'line':1,'column':13,'offset':12,'file':'all-extents.cpp',}"
        ",'end':{'line':4,'column':2,'offset':37,'file':'all-extents.cpp',}},";
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_all_extents_at(
        "all-extents.cpp#../all-extents.cpp:-std=c++1y:3:1"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(
        0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void location_test::test_ast_node() {
    auto vim_clang_get_location_information =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_location_information"));
    assert(vim_clang_get_location_information);

    std::string expected_prefix = "{'spell':'y','type':'int',";
    std::string actual(vim_clang_get_location_information(
        "qa/data/current-function.cpp:std=c++1y:10:9"));
    CPPUNIT_ASSERT_EQUAL(
        0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void location_test::test_unsaved_ast_node() {
    auto vim_clang_get_location_information =
        reinterpret_cast<char const* (*)(char const*)>(
            dlsym(m_handle, "vim_clang_get_location_information"));
    assert(vim_clang_get_location_information);

    std::string expected_prefix = "{'spell':'y','type':'int',";
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_location_information(
        "current-function.cpp#../current-function.cpp:-std=c++1y:10:9"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(
        0, actual.compare(0, expected_prefix.size(), expected_prefix));
}

void location_test::test_extent() {
    auto vim_clang_get_extent_of_node_at_specific_location =
        reinterpret_cast<char const* (*)(char const*)>(dlsym(
            m_handle, "vim_clang_get_extent_of_node_at_specific_location"));
    assert(vim_clang_get_extent_of_node_at_specific_location);

    std::string expected = "{'start':{'line':11,'column':5,'offset':110,"
                           "'file':'qa/data/current-function.cpp',},"
                           "'end':{'line':11,'column':13,'offset':118,'"
                           "file':'qa/data/current-function.cpp',}}";
    std::string actual(vim_clang_get_extent_of_node_at_specific_location(
        "qa/data/current-function.cpp:-std=c++11:11:7"));
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

void location_test::test_unsaved_extent() {
    auto vim_clang_get_extent_of_node_at_specific_location =
        reinterpret_cast<char const* (*)(char const*)>(dlsym(
            m_handle, "vim_clang_get_extent_of_node_at_specific_location"));
    assert(vim_clang_get_extent_of_node_at_specific_location);

    std::string expected = "{'start':{'line':11,'column':5,'offset':110,"
                           "'file':'current-function.cpp',},"
                           "'end':{'line':11,'column':13,'offset':118,'"
                           "file':'current-function.cpp',}}";
    chdir("qa/data/unsaved");
    std::string actual(vim_clang_get_extent_of_node_at_specific_location(
        "current-function.cpp#../current-function.cpp:-std=c++1y:11:7"));
    chdir("../../..");
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

CPPUNIT_TEST_SUITE_REGISTRATION(location_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
