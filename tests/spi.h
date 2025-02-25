#pragma once

#include "tests.h"

namespace tests {

add_test(spi, ([](test_case &) {
           bool result = true;
           cppgres::spi_executor spi;
           auto res = spi.query<std::tuple<std::optional<int64_t>>>(
               "select $1 + i from generate_series(1,100) i", 1LL);

           int i = 0;
           for (auto &re : res) {
             i++;
             result = result && _assert(std::get<0>(re) == i + 1);
           }
           result = result && _assert(std::get<0>(res.begin()[0]) == 2);
           return result;
         }));

add_test(spi_type_mismatch, ([](test_case &) {
           bool result = true;
           cppgres::spi_executor spi;

           bool exception_raised = false;
           try {
             auto res = spi.query<std::tuple<std::optional<bool>>>(
                 "select $1 + i from generate_series(1,100) i", 1LL);
           } catch (std::invalid_argument &e) {
             exception_raised = true;
           }

           result = result && _assert(exception_raised);

           return result;
         }));

add_test(spi_plan, ([](test_case &) {
           bool result = true;
           cppgres::spi_executor spi;
           auto plan = spi.plan<int64_t>("select $1 + i from generate_series(1,100) i");
           auto res = spi.query<std::tuple<std::optional<int64_t>>>(plan, 1LL);

           int i = 0;
           for (auto &re : res) {
             i++;
             result = result && _assert(std::get<0>(re) == i + 1);
           }
           result = result && _assert(std::get<0>(res.begin()[0]) == 2);
           return result;
         }));

add_test(spi_plan_mismatch, ([](test_case &) {
           bool result = true;
           cppgres::spi_executor spi;
           auto plan = spi.plan<int64_t>("select $1 + i from generate_series(1,100) i");

           bool exception_raised = false;
           try {
             auto res = spi.query<std::tuple<std::optional<bool>>>(plan, 1LL);
           } catch (std::invalid_argument &e) {
             exception_raised = true;
           }

           result = result && _assert(exception_raised);

           return result;
         }));

add_test(spi_plan_gone, ([](test_case &) {
           bool result = true;

           auto plan = ({
             cppgres::spi_executor spi;
             spi.plan<int64_t>("select $1 + i from generate_series(1,100) i");
           });

           cppgres::spi_executor spi;

           bool exception_raised = false;
           try {
             auto res = spi.query<std::tuple<std::optional<int64_t>>>(plan, 1LL);
           } catch (cppgres::pointer_gone_exception &e) {
             exception_raised = true;
           }

           result = result && _assert(exception_raised);

           return result;
         }));

add_test(spi_keep_plan, ([](test_case &) {
           bool result = true;

           ::SPIPlanPtr ptr = nullptr;
           {
             auto plan = ({
               cppgres::spi_executor spi;
               auto plan = spi.plan<int64_t>("select $1 + i from generate_series(1,100) i");
               plan.keep();
               std::move(plan);
             });

             ptr = plan;

             cppgres::spi_executor spi;
             auto res = spi.query<std::tuple<std::optional<int64_t>>>(plan, 1LL);

             int i = 0;
             for (auto &re : res) {
               i++;
               result = result && _assert(std::get<0>(re) == i + 1);
             }
             result = result && _assert(std::get<0>(res.begin()[0]) == 2);
           }

           // Now the plan should be gone – destroyed and inaccessible
           bool exception_raised = false;
           try {
             auto ctx = cppgres::memory_context::for_pointer(ptr);
           } catch (std::exception &e) {
             exception_raised = true;
           }
           result = result && _assert(exception_raised);

           return result;
         }));

} // namespace tests
