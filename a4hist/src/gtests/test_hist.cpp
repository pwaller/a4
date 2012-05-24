#include <iostream>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include <a4/histogram.h>
#include <a4/object_store.h>
using a4::store::ObjectStore;
using a4::store::ObjectBackStore;

#include <boost/thread.hpp>

using namespace a4::hist;

const size_t GRIND_REPETITIONS = 5000000;

const uint64_t    n_per_thread = 5e7; //1000000000; // 1e9
const uint64_t    n_threads    = 2; //1000000000; // 1e9
volatile uint64_t bignum       = 0;

void IncreaseBignum(const uint64_t n) {
    for (uint64_t i = 0; i < n; i++) {
        bignum++;
    }
}

#ifdef HAVE_ATOMIC
#include <atomic>

volatile std::atomic<uint64_t> atomic_bignum(0);

void AtomicIncreaseBignum(const uint64_t n) {
    for (uint64_t i = 0; i < n; i++) {
        atomic_bignum++;
    }
}

TEST(a4hist, atomicint) {
    boost::thread_group tg;

    for (uint64_t i = 0; i < n_threads; i++) {
        tg.add_thread(new boost::thread(AtomicIncreaseBignum, n_per_thread));
    }
    tg.join_all();
    ASSERT_EQ(n_per_thread * n_threads, atomic_bignum);
}
#endif

TEST(a4hist, nonatomicint) {
    boost::thread_group tg;

    for (uint64_t i = 0; i < n_threads; i++) {
        tg.add_thread(new boost::thread(IncreaseBignum, n_per_thread));
    }
    tg.join_all();
    // These won't be equal on multicore systems.
    // ASSERT_EQ(n_per_thread*n_threads, bignum);
}

TEST(a4hist, h1) {
    H1 h1;

    h1(100, 0, 1);
    h1.fill(0.1, 1.0);
    h1.fill(0.9, 1.0);
    ASSERT_EQ(2.0, h1.integral());
}

TEST(a4hist, h2) {
    H2 h2;

    h2("Title") (100, 0, 1, "x") (100, 0, 1, "y");
    h2.fill(0.1, 0.1);
    h2.fill(0.2, 0.1);
    ASSERT_EQ(2, h2.integral());
}

#ifdef HAVE_INITIALIZER_LISTS
TEST(a4hist, basic_variable_binning_check) {
    H1 h1;

    h1("Title") ({ 1., 2., 4., 8., 16. }
                 );

    std::vector<double> values = {
        1., 1.1, 4., 8., 9., 10., 11., 15.
    };

    unsigned int n = 0;
    foreach (double i, values) {
        h1.fill(i);
        n++;
    }

    ASSERT_EQ(n, h1.integral());

    // -------

    H2 h2;
    h2("Title") ({ 1., 2., 4., 8., 16. }
                 ) ({ 1., 2., 4., 8., 16. }
                    );

    n = 0;
    foreach (double i, values) {
        h2.fill(i, i);
        n++;
    }

    ASSERT_EQ(n, h2.integral());

    // -------

    H3 h3;
    h3("Title") ({ 1., 2., 4., 8., 16. }
                 ) ({ 1., 2., 4., 8., 16. }
                    ) ({ 1., 2., 4., 8., 16. }
                       );

    n = 0;
    foreach (double i, values) {
        h3.fill(i, i, i);
        n++;
    }

    ASSERT_EQ(n, h3.integral());
}
#endif

TEST(a4hist, test_h1_grind) {
    H1 h1;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h1(100, 0, 1).fill(i);
    }
}

TEST(a4hist, test_h1_grind_backstore) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        S.T<H1>("hist") (100, 0, 1).fill(i);
    }
}

#ifndef HAVE_INITIALIZER_LISTS
TEST(a4hist, test_h1_grind_backstore_variable) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        S.T<H1>("hist") ({ 10., 20., 1000., 1000000., 5000000. }
                         ).fill(i);
    }
}

TEST(a4hist, test_h1_grind_backstore_variable_label) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        S.T<H1>("hist") ({ 10., 20., 1000., 1000000., 5000000. }, "x").fill(i);
    }
}

TEST(a4hist, test_h1_grind_backstore_variable_int) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        S.T<H1>("hist") ({ 10, 20, 1000, 1000000, 5000000 }
                         ).fill(i);
    }
}
#endif // HAVE_INITIALIZER_LISTS

TEST(a4hist, test_h1_grind_backstore_many) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS / 10; i++) {
        S.T<H1>("hist0") (100, 0, 1).fill(i + 0);
        S.T<H1>("hist1") (100, 0, 1).fill(i + 1);
        S.T<H1>("hist2") (100, 0, 1).fill(i + 2);
        S.T<H1>("hist3") (100, 0, 1).fill(i + 3);
        S.T<H1>("hist4") (100, 0, 1).fill(i + 4);
        S.T<H1>("hist5") (100, 0, 1).fill(i + 5);
        S.T<H1>("hist6") (100, 0, 1).fill(i + 6);
        S.T<H1>("hist7") (100, 0, 1).fill(i + 7);
        S.T<H1>("hist8") (100, 0, 1).fill(i + 8);
        S.T<H1>("hist9") (100, 0, 1).fill(i + 9);
    }
}

void test_h1_grind_backstore_func_fill_histos(ObjectStore S, const int & i) {
    S.T<H1>("hist0") (100, 0, 1).fill(i + 0);
    S.T<H1>("hist1") (100, 0, 1).fill(i + 1);
    S.T<H1>("hist2") (100, 0, 1).fill(i + 2);
    S.T<H1>("hist3") (100, 0, 1).fill(i + 3);
    S.T<H1>("hist4") (100, 0, 1).fill(i + 4);
    S.T<H1>("hist5") (100, 0, 1).fill(i + 5);
    S.T<H1>("hist6") (100, 0, 1).fill(i + 6);
    S.T<H1>("hist7") (100, 0, 1).fill(i + 7);
    S.T<H1>("hist8") (100, 0, 1).fill(i + 8);
    S.T<H1>("hist9") (100, 0, 1).fill(i + 9);
}

TEST(a4hist, test_h1_grind_backstore_func) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS / 10 / 10; i++) {
        test_h1_grind_backstore_func_fill_histos(S("0/"), i);
        test_h1_grind_backstore_func_fill_histos(S("1/"), i);
        test_h1_grind_backstore_func_fill_histos(S("2/"), i);
        test_h1_grind_backstore_func_fill_histos(S("3/"), i);
        test_h1_grind_backstore_func_fill_histos(S("4/"), i);
        test_h1_grind_backstore_func_fill_histos(S("5/"), i);
        test_h1_grind_backstore_func_fill_histos(S("6/"), i);
        test_h1_grind_backstore_func_fill_histos(S("7/"), i);
        test_h1_grind_backstore_func_fill_histos(S("8/"), i);
        test_h1_grind_backstore_func_fill_histos(S("9/"), i);
    }
}

TEST(a4hist, test_h1_grind_dynamic_100) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        S.T<H1>("hist0", i % 100) (100, 0, 1).fill(i + 0);
    }
}

TEST(a4hist, test_h1_grind_dynamic_crazy) {
    ObjectBackStore backstore;
    auto            S = backstore("test/");

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        S.T<H1>("hist0", i % 100, i % 200, i % 500) (100, 0, 1).fill(i + 0);
    }
}

TEST(a4hist, test_h1_grind_labelled) {
    H1 h1;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h1("title") (100, 0, 1, "axis label").fill(i);
    }
}

#ifndef HAVE_INITIALIZER_LISTS
TEST(a4hist, test_h1_grind_variable) {
    H1 h1;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h1({ 1., 2., 3., 4., 5., 100000., 1000000., 5000000. }
           ).fill(i);
    }
    //std::cout << h1 << std::endl;
}
#endif // HAVE_INITIALIZER_LISTS

TEST(a4hist, test_h2_grind) {
    H2 h2;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h2(100, 0, 1) (100, 0, 1).fill(i, i);
    }
}

#ifndef HAVE_INITIALIZER_LISTS
TEST(a4hist, test_h2_grind_variable) {
    H2 h2;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h2({ 1., 2., 3., 4., 5., 100000., 1000000., 5000000. }
           )
            ({ 1., 2., 3., 4., 6., 100000., 1000000., 5000000. }
            )
        .fill(i, i);
    }

    //std::cout << h2 << std::endl;
}
#endif // HAVE_INITIALIZER_LISTS

TEST(a4hist, test_h3_grind) {
    H3 h3;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h3(100, 0, 1) (100, 0, 1) (100, 0, 1).fill(i, i, i);
    }
}

#ifndef HAVE_INITIALIZER_LISTS
TEST(a4hist, test_h3_grind_variable) {
    H3 h3;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h3({ 1., 2., 3., 4., 5., 100000., 1000000., 5000000. }
           )
            ({ 1., 2., 3., 4., 6., 100000., 1000000., 5000000. }
            )
            ({ 1., 2., 3., 4., 7., 100000., 1000000., 5000000. }
            )
        .fill(i, i, i);
    }

    //std::cout << h2 << std::endl;
}

TEST(a4hist, test_h3_grind_variable_titles) {
    H3 h3;

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h3("title")
            ({ 1., 2., 3., 4., 5., 100000., 1000000., 5000000. }, "axis 1")
            ({ 1., 2., 3., 4., 6., 100000., 1000000., 5000000. }, "axis 2")
            ({ 1., 2., 3., 4., 7., 100000., 1000000., 5000000. }, "axis 3")
        .fill(i, i, i);
    }

    //std::cout << h2 << std::endl;
}
#endif // HAVE_INITIALIZER_LISTS

#ifdef HAVE_CERN_ROOT_SYSTEM

#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

TEST(a4hist, test_th1d) {
    TH1D h1("test", "test", 100, 0, 1);

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h1.Fill(i);
    }
}

TEST(a4hist, test_th1d_many) {
    TH1D h10("test0", "test", 100, 0, 1);
    TH1D h11("test1", "test", 100, 0, 1);
    TH1D h12("test2", "test", 100, 0, 1);
    TH1D h13("test3", "test", 100, 0, 1);
    TH1D h14("test4", "test", 100, 0, 1);
    TH1D h15("test5", "test", 100, 0, 1);
    TH1D h16("test6", "test", 100, 0, 1);
    TH1D h17("test7", "test", 100, 0, 1);
    TH1D h18("test8", "test", 100, 0, 1);
    TH1D h19("test9", "test", 100, 0, 1);

    for (size_t i = 0; i < GRIND_REPETITIONS / 10; i++) {
        h10.Fill(i + 0);
        h11.Fill(i + 1);
        h12.Fill(i + 2);
        h13.Fill(i + 3);
        h14.Fill(i + 4);
        h15.Fill(i + 5);
        h16.Fill(i + 6);
        h17.Fill(i + 7);
        h18.Fill(i + 8);
        h19.Fill(i + 9);
    }
}

TEST(a4hist, test_th1d_many_ptr) {
    const int n = 10;
    TH1D    * h1s[n];

    for (int i = 0; i < n; i++) {
        std::stringstream s("test");
        s << i;
        h1s[i] = new TH1D(s.str().c_str(), "test", 100, 0, 1);
    }

    for (size_t i = 0; i < GRIND_REPETITIONS / n; i++) {
        for (int j = 0; j < n; j++) {
            h1s[j]->Fill(i + j);
        }
    }

    for (int i = 0; i < n; i++) {
        delete h1s[i];
    }
}

TEST(a4hist, test_th2d) {
    TH2F h2("test", "test", 100, 0, 1, 100, 0, 1);

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h2.Fill(i, i);
    }
}

TEST(a4hist, test_th3d) {
    TH3F h3("test", "test", 100, 0, 1, 100, 0, 1, 100, 0, 1);

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h3.Fill(i, i, i);
    }
}

TEST(a4hist, test_th1f_variable) {
    Double_t bins[] = {
        1., 2., 3., 4., 5., 100000., 1000000., 5000000.
    };
    TH1F     h1("test", "test", sizeof(bins) / sizeof(Double_t) - 1, bins);

    for (size_t i = 0; i < GRIND_REPETITIONS; i++) {
        h1.Fill(i);
    }
}

#endif
