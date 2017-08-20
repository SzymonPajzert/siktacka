//
// Created by svp on 17.08.17.
//

#ifndef SIKTACKA_TEST_HPP
#define SIKTACKA_TEST_HPP


#include <iostream>
#include <vector>

#include "def/util.hpp"


class Test {
public:

    Test() : test_run(0), test_correct(0) {};

    /** Test value and print the description as well as increase internal counters
     *
     * @param value
     */
    std::string WARN_UNUSED test(bool value) {
        test_run++;
        test_correct += int(value);
        return (value ? "OK" : "WRONG");
    }

    /** Print results of run tests
     *
     */
    Test & print_results(const std::string & description) {
        std::cout
                << std::endl

                << description << " results: " << test_correct << "/" << test_run
                << std::endl
                << std::endl;
        return *this;
    }

    bool ok() {
        return test_correct == test_run;
    }

    static Test merge_results(const std::vector<Test> tests) {
        int merge_test_run = 0, merge_test_correct = 0;
        for(auto & test : tests) {
            merge_test_run += test.test_run;
            merge_test_correct += test.test_correct;
        }

        return Test {merge_test_run, merge_test_correct};
    }

private:

    Test(int _test_run, int _test_correct) : test_run(_test_run), test_correct(_test_correct) {};

    int test_run = 0;
    int test_correct = 0;
};


#endif //SIKTACKA_TEST_HPP
