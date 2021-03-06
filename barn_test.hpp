/******************************************************************************
* @file Contains tools for unit testing.
*
*
* TODO make_FunctionTest for not having to write such a tedious constructor.
*      aka get rid of explicitly writing the template arguments.
*
* if it really pays off:
* TODO test-functions for functions with side-effects. test_sideeffects(...)
* TODO test-functions that test for exceptions. test_exceptions(...)
* TODO test-function that aggregates all test functions. test_function(...)
*
*
* Write test series as follows:
---------------------------------------------------------------------------------------------------

// function to be tested
std::vector<int> fun(int i, int j) {
    return {1, i, j};
}


// comparison function to check actual and expected result
auto comp = [](const std::vector<int>& a, const std::vector<int>& b) {return std::equal(a.begin(), a.end(), b.begin()); };
auto tostring = [](const std::vector<int>& v) { return to_string(v, ", "); };

FunctionTest<std::vector<int>, int, int> tester(fun, comp, tostring);

tester.test("Run 1", { 1,13,15 }, 13, 15);
tester.test("Run 2", { 1,13,15 }, 13, 99);

---------------------------------------------------------------------------------------------------
*
*
* @author langenhagen
* @version 160107
******************************************************************************/
#pragma once

#include <chrono>
#include <exception>
#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// DEFINES and MACROS


///////////////////////////////////////////////////////////////////////////////
// NAMESPACE, CONSTANTS, TYPE DECLARATIONS/IMPLEMENTATIONS and FUNCTIONS


/** FunctionTest unit testing class wich is capable of invoking arbitrary functions with a return value
and comparing them to anticipated values. Measures the run-time of the function and writes unit test results to a stream.
*/
template< typename ResultType /*return type of the given function*/, typename... ArgTypes /*argument types of the given function*/>
class FunctionTest {

    using FunctionType           = const std::function<ResultType(ArgTypes...)>;
    using ComparatorFunctionType = const std::function<bool(const ResultType&, const ResultType&)>;
    using ToStringFunctionType   = const std::function<std::string(const ResultType&)>;

private: // vars

    const FunctionType fun_;                          ///< The function.
    const ComparatorFunctionType comp_;               ///< The result comparison function.
    const ToStringFunctionType to_string_function_;   ///< The to-string function for ResultType.
    std::ostream& os_;                                ///< The output stream.

public: // vars

    bool verbose = true;                    ///< Whether or not show detailed info in case of failures.    
    unsigned int output_line_length = 60;   ///< The number of dots that is shown in the printed lines.

public: // constructors

    /** Constructor #1 for the function tester. Use with complex return types.
    Sets the verbose flag to TRUE.
    @param function A function that must return a value.
    @param comparator A comparison function that compares the actual invocation-results
    with given expected results. The comparison function must have the form
    bool(ResultType, ResultType) or similar.
    @param to_string_function The to-string function for the function's return type. 
    */
    FunctionTest(FunctionType function, ComparatorFunctionType comparator, ToStringFunctionType to_string_function, std::ostream& os = std::cout)
        : fun_(function),
        comp_(comparator), 
        to_string_function_(to_string_function),
        os_(os)
    {}


    /** Constructor #2 for the function tester. Use with complex return types.
    Sets the to-string function for the function's return type to a standard function 
    and sets the verbose flag to FALSE.
    @param function A function that must return a value.
    @param comparator A comparison function that compares the actual invocation-results
    with given expected results. The comparison function must have the form
    bool(ResultType, ResultType) or similar.
    */
    FunctionTest(FunctionType function, ComparatorFunctionType comparator, std::ostream& os = std::cout)
        : fun_(function),
        comp_(comparator), 
        to_string_function_([](const ResultType& r) { return "<to-string function not specified>"; }),
        os_(os),
        verbose(false)
    {}


    /** Constructor #3 for the function tester. Use with simple return types.
    Sets the comparison function to a simple "==".
    Sets the to-string function for the function's return type to a standard function
    and sets the verbose flag to TRUE.
    @param function A function that must return a value.
    */
    FunctionTest(FunctionType function, std::ostream& os = std::cout)
        : fun_(function),
        comp_([](const ResultType& a, const ResultType& b) { return a == b; }),
        to_string_function_([](const ResultType& r) { return std::to_string(r); }),
        os_(os)
    {}


public: // methods

    /** Unit test on the function that is connected to the tester.
    Tests whether the return-value of a given function invoked with given parameters is equal to a given value
    and takes care of possibly occuring exceptions.
    Also measures the time the function execution takes and writes the results of the test to a given output-stream.
    Checks also for exceptions and reports them to the output stream.
    In case of error the object's flag .verbose in conjunction with a valid .result_to_string_function
    can be used to write more sophisticated output.
    @param test_name A human-readable alias of the test that will be written into the stream.
    @param expected_result The anticipated return value of the tested function.
    @param args The arguments that will be passed to the function on invocation.
    @return A pair<bool, ResultType>. The first value represents whether the test was successful or not.
    The second value is a copy of the actual result of the function invocation.
    */
    std::pair<bool, ResultType> test( const std::string& test_name,
                                      const ResultType& expected_result,
                                      const ArgTypes&... args) const {

        std::pair<bool, ResultType> ret;
        ret.first = false;
        std::string output = "TESTING " + test_name + ": ";
        output.resize(output_line_length, '.');
        os_ << output << " ";

        try {
            using namespace std::chrono;
            const auto clock_start = steady_clock::now();
            const ResultType result = fun_(args...);
            ret.second = result;
            const auto ms = duration_cast< milliseconds>(steady_clock::now() - clock_start);

            if (comp_(result, expected_result)) {
                os_ << "OK (" << ms.count() << " ms)\n";
                ret.first = true;
            }
            else {
                os_ << "FAILED (" << ms.count() << " ms)\n";

                if (verbose) {
                    os_ << " RESULT:   " << to_string_function_(result) << "\n" <<
                           " EXPECTED: " << to_string_function_(expected_result) << "\n" <<
                           ".\n";
                }
            }
        }
        catch (std::exception& ex) {
            os_ << "EXCEPTION\n" <<
                   typeid(ex).name() << ":\n" << ex.what() << std::endl;
        }
        catch (...) {
            os_ << "EXCEPTION\n" <<
                   "unknown\n";
        }
        return ret;
    }
};
