// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

//Licensed to the Apache Software Foundation (ASF) under one
//or more contributor license agreements.  See the NOTICE file
//distributed with this work for additional information
//regarding copyright ownership.  The ASF licenses this file
//to you under the Apache License, Version 2.0 (the
//"License"); you may not use this file except in compliance
//with the License.  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing,
//software distributed under the License is distributed on an
//"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
//KIND, either express or implied.  See the License for the
//specific language governing permissions and limitations
//under the License.  


#include "doctest.h"

#if _WIN32
#include <Windows.h>
#endif

#include <vector>
#include <atomic>
#include <iostream>
#include <thread>

#include "utils/signal.hpp"
#include "utils/cs.hpp"

#ifdef __linux__
#include "cppguard.hpp"
#include "critical_section_linux.hpp"
#endif

//FIXME some variables could be optimized away in release?

TEST_CASE("test_cs_t1_a_a_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionA);

  LeaveCriticalSection(CriticalSectionA);
  LeaveCriticalSection(CriticalSectionA);
}

TEST_CASE("test_ok_cs_t1_a_b_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionB);

  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionA);
}

TEST_CASE("test_cs_t1_a_b_a_b_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionB);

  LeaveCriticalSection(CriticalSectionA);
  LeaveCriticalSection(CriticalSectionB);
}

TEST_CASE("test_cs_t1_a_b_b_a_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionB);

  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionA);
}


TEST_CASE("test_ok_cs_t1_a_b_bulk_exit_0")
{
  const size_t cs_count=20;

  std::vector<CRITICAL_SECTION> critical_sections;
  critical_sections.resize(cs_count);

  for(auto& cs:critical_sections)
  {
    InitializeCriticalSection(&cs);
  }

  for(auto& cs:critical_sections)
  {
    EnterCriticalSection(&cs);
  }

  for(auto& cs:critical_sections)
  {
    LeaveCriticalSection(&cs);
  }

  for(auto& cs:critical_sections)
  {
    DeleteCriticalSection(&cs);
  }

}

TEST_CASE("test_ok_cs_t1_a_b_bulk2_exit_0")
{
  cs_maker_t cs_maker;

  const size_t cs_count=20;

  std::vector<CRITICAL_SECTION> critical_sections;
  critical_sections.resize(cs_count);

  for(auto& cs:critical_sections)
  {
    InitializeCriticalSection(&cs);
  }

  for(size_t i=0; i<1000; ++i)
  {
    size_t j=0;
    for(auto& cs:critical_sections)
    {
      EnterCriticalSection(&cs);
      LeaveCriticalSection(&cs);
      j++;

    }
  }

  for(auto& cs:critical_sections)
  {
    DeleteCriticalSection(&cs);
  }

}


TEST_CASE("test_ok_cs_t1_a_b_t2_a_b_no_overlap_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();

  std::thread t([&]()
    {
      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);
    });

  std::thread t2([&]()
    {
      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

    });

  t.join();
  t2.join();

}

TEST_CASE("test_ok_cs_t1_a_b_t2_a_b_may_overlap_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();

  utils::event_t signal_t1;

  std::thread t([&]()
    {
      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

      signal_t1.notify_one();

    });

  std::thread t2([&]()
    {
      signal_t1.wait();

      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

    });

  t.join();
  t2.join();
}

void test_ok_cs_100t_10cs_bulk_thread_function(std::vector<CRITICAL_SECTION>& p_critical_sections, std::atomic_uint32_t& p_counter)
{
  for(size_t i=0; i<10; ++i)
  {
    for(auto& cs:p_critical_sections)
    {
      EnterCriticalSection(&cs);
    }

    p_counter++;

    for(auto& cs:p_critical_sections)
    {
      LeaveCriticalSection(&cs);
    }
  }
}

TEST_CASE("test_ok_cs_100t_10cs_bulk_exit_0")
{
  const size_t cs_count=20;
  const size_t thread_count=1000;
  std::atomic_uint32_t counter{0};

  std::vector<CRITICAL_SECTION> critical_sections;
  critical_sections.resize(cs_count);

  for(auto& cs:critical_sections)
  {
    InitializeCriticalSection(&cs);
  }

  std::vector<std::thread> threads;

  for(size_t i=0; i<thread_count; ++i)
  {
    threads.emplace_back(test_ok_cs_100t_10cs_bulk_thread_function, std::ref(critical_sections), std::ref(counter));
  }

  for(auto& t:threads)
  {
    t.join();
  }

  REQUIRE(counter==thread_count*10);

  for(auto& cs:critical_sections)
  {
    DeleteCriticalSection(&cs);
  }

}


TEST_CASE("test_cs_t1_a_b_t2_b_a_no_overlap_cs_ignored_exit_0")
{
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  WARN("skipped tsan active");
  return;
#endif
#else
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();


  utils::event_t signal_t1;

  std::thread t([&]()
    {
      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

      signal_t1.notify_one();

    });

  std::thread t2([&]()
    {
      signal_t1.wait();

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(2, 2);
#endif

      EnterCriticalSection(CriticalSectionB);
      EnterCriticalSection(CriticalSectionA);

      LeaveCriticalSection(CriticalSectionA);
      LeaveCriticalSection(CriticalSectionB);

    });

  t.join();
  t2.join();
#endif
}

TEST_CASE("test_cs_t1_a_b_t2_b_a_no_overlap_cs_ignored_both_threads_exit_0")
{
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  WARN("skipped tsan active");
  return;
#endif
#else
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();


  utils::event_t signal_t1;

  std::thread t([&]()
    {
#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(2, 2);
#endif

      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

      signal_t1.notify_one();

    });

  std::thread t2([&]()
    {
      signal_t1.wait();

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(2, 2);
#endif

      EnterCriticalSection(CriticalSectionB);
      EnterCriticalSection(CriticalSectionA);

      LeaveCriticalSection(CriticalSectionA);
      LeaveCriticalSection(CriticalSectionB);

    });

  t.join();
  t2.join();
#endif
}


TEST_CASE("test_cs_t1_a_b_t2_b_a_no_overlap_cs_ignored_both_threads_single_exit_0")
{
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  WARN("skipped tsan active");
  return;
#endif
#else
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();


  utils::event_t signal_t1;

  std::thread t([&]()
    {
#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(1, 0);
#endif
      EnterCriticalSection(CriticalSectionA);


#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(1, 0);
#endif
      EnterCriticalSection(CriticalSectionB);

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(0, 1);
#endif
      LeaveCriticalSection(CriticalSectionB);


#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(0, 1);
#endif
      LeaveCriticalSection(CriticalSectionA);

      signal_t1.notify_one();

    });

  std::thread t2([&]()
    {
      signal_t1.wait();

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(1, 0);
#endif
      EnterCriticalSection(CriticalSectionB);

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(1, 0);
#endif
      EnterCriticalSection(CriticalSectionA);

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(0, 1);
#endif
      LeaveCriticalSection(CriticalSectionA);

#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(0, 1);
#endif
      LeaveCriticalSection(CriticalSectionB);

    });

  t.join();
  t2.join();
#endif
}

TEST_CASE("test_cs_t1_double_leave_cs_disabled_exit_0")
{
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  WARN("skipped tsan active");
  return;
#endif
#else
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);

  LeaveCriticalSection(CriticalSectionA);

  //Double leave
#ifdef CPPGUARD_ENABLED
  cppguard_increase_ignore_counter(0, 1);
#endif
  LeaveCriticalSection(CriticalSectionA);
#endif
}

TEST_CASE("test_cs_leave_other_thread_cs_disabled_exit_0")
{
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  WARN("skipped tsan active");
  return;
#endif
#else
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();

#ifdef CPPGUARD_ENABLED
  cppguard_increase_ignore_counter(1, 0);
#endif
  EnterCriticalSection(CriticalSectionA);

  //leave in other thread
  std::thread t([&]()
    {
#ifdef CPPGUARD_ENABLED
      cppguard_increase_ignore_counter(0, 1);
#endif
      LeaveCriticalSection(CriticalSectionA);
    });

  t.join();
#endif
}

TEST_CASE("test_t1_aaa_bbb_ccc_recure_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionC=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionA);

  EnterCriticalSection(CriticalSectionB);
  EnterCriticalSection(CriticalSectionB);
  EnterCriticalSection(CriticalSectionB);

  EnterCriticalSection(CriticalSectionC);
  EnterCriticalSection(CriticalSectionC);
  EnterCriticalSection(CriticalSectionC);

  LeaveCriticalSection(CriticalSectionC);
  LeaveCriticalSection(CriticalSectionC);
  LeaveCriticalSection(CriticalSectionC);

  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionB);

  LeaveCriticalSection(CriticalSectionA);
  LeaveCriticalSection(CriticalSectionA);
  LeaveCriticalSection(CriticalSectionA);
}


TEST_CASE("test_t1_abc_bbb_abc_recure_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionC=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionB);
  EnterCriticalSection(CriticalSectionC);

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionB);
  EnterCriticalSection(CriticalSectionC);

  EnterCriticalSection(CriticalSectionA);
  EnterCriticalSection(CriticalSectionB);
  EnterCriticalSection(CriticalSectionC);

  LeaveCriticalSection(CriticalSectionC);
  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionA);

  LeaveCriticalSection(CriticalSectionC);
  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionA);

  LeaveCriticalSection(CriticalSectionC);
  LeaveCriticalSection(CriticalSectionB);
  LeaveCriticalSection(CriticalSectionA);
}


TEST_CASE("test_ok_cs_t1_a_b_t2_a_b_recurse_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();


  std::thread t([&]()
    {
      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

    });

  std::thread t2([&]()
    {
      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      EnterCriticalSection(CriticalSectionA);
      EnterCriticalSection(CriticalSectionB);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

      LeaveCriticalSection(CriticalSectionB);
      LeaveCriticalSection(CriticalSectionA);

    });

  t.join();
  t2.join();
}


TEST_CASE("test_ok_cs_tryenter_critical_section_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();

  EnterCriticalSection(CriticalSectionA);


  std::thread t([&]()
    {
      const bool got_lock=TryEnterCriticalSection(CriticalSectionA);
      REQUIRE_FALSE(got_lock);
    });

  t.join();

  LeaveCriticalSection(CriticalSectionA);

  std::thread t2([&]()
    {
      const bool got_lock=TryEnterCriticalSection(CriticalSectionA);
      REQUIRE(got_lock);
      LeaveCriticalSection(CriticalSectionA);
    });

  t2.join();

}


TEST_CASE("test_cs_try_enter_t1_a_b_t2_b_a_overlap_tryenter_on_second_exit_0")
{
  cs_maker_t cs_maker;

  CRITICAL_SECTION* CriticalSectionA=cs_maker.make_scoped();
  CRITICAL_SECTION* CriticalSectionB=cs_maker.make_scoped();



  utils::event_t signal_t1_cs_a;
  utils::event_t signal_t2_cs_b;
  utils::event_t signal3;
  utils::event_t signal4;

  std::thread t([&]()
    {
      const bool got=TryEnterCriticalSection(CriticalSectionA);
      REQUIRE(got);
      signal_t1_cs_a.notify_one();

      signal_t2_cs_b.wait();
      const bool got2=TryEnterCriticalSection(CriticalSectionB);
      REQUIRE_FALSE(got2);
      signal3.notify_one();

      signal4.wait();

      LeaveCriticalSection(CriticalSectionA);
    });

  std::thread t2([&]()
    {
      signal_t1_cs_a.wait();

      const bool got=TryEnterCriticalSection(CriticalSectionB);
      REQUIRE(got);
      signal_t2_cs_b.notify_one();

      signal3.wait();

      const bool got2=TryEnterCriticalSection(CriticalSectionA);
      REQUIRE_FALSE(got2);
      signal3.notify_one();

      signal4.notify_one();

      LeaveCriticalSection(CriticalSectionB);


    });

  t.join();
  t2.join();
}


CRITICAL_SECTION CriticalSectionPerfA;
CRITICAL_SECTION CriticalSectionPerfB;
CRITICAL_SECTION CriticalSectionPerfC;
CRITICAL_SECTION CriticalSectionPerfD;

int fib_perf(int n)
{
  if(n<=1)
    return n;
  return fib_perf(n-1)+fib_perf(n-2);
}

void test_cs_perf_thread_function(CRITICAL_SECTION* p_critical_sections, std::atomic_uint32_t& p_counter)
{
  for(int i=0; i<100000; i++)
  {
    EnterCriticalSection(&CriticalSectionPerfA);
    int res=fib_perf(1);
    p_counter+=res;

    EnterCriticalSection(&CriticalSectionPerfB);
    res=fib_perf(2);
    p_counter+=res;

    EnterCriticalSection(&CriticalSectionPerfC);
    res=fib_perf(3);
    p_counter+=res;

    EnterCriticalSection(&CriticalSectionPerfD);
    res=fib_perf(5);
    p_counter+=res;



    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    LeaveCriticalSection(&CriticalSectionPerfD);
    LeaveCriticalSection(&CriticalSectionPerfC);
    LeaveCriticalSection(&CriticalSectionPerfB);
    LeaveCriticalSection(&CriticalSectionPerfA);
  }
}


TEST_CASE("test_cs_delete_without_use_exit_0")
{
  CRITICAL_SECTION mu1;
  InitializeCriticalSection(&mu1);

  DeleteCriticalSection(&mu1);
}

TEST_CASE("test_cs_delete_with_use_exit_0")
{
  CRITICAL_SECTION mu1;
  InitializeCriticalSection(&mu1);

  EnterCriticalSection(&mu1);
  LeaveCriticalSection(&mu1);

  DeleteCriticalSection(&mu1);
}

TEST_CASE("test_cs_mass_delete_exit_0")
{
  for(size_t i=0; i<10000; ++i)
  {
    std::vector<CRITICAL_SECTION> critical_sections;
    critical_sections.resize(1000);

    for(auto& cs:critical_sections)
    {
      InitializeCriticalSection(&cs);
    }

    for(auto& critical_section:critical_sections)
    {
      EnterCriticalSection(&critical_section);
      LeaveCriticalSection(&critical_section);
    }

    for(auto& cs:critical_sections)
    {
      DeleteCriticalSection(&cs);
    }
  }
}

TEST_CASE("test_cs_perf_test_exit_0")
{
  std::atomic_uint32_t counter;

  InitializeCriticalSection(&CriticalSectionPerfA);
  InitializeCriticalSection(&CriticalSectionPerfB);
  InitializeCriticalSection(&CriticalSectionPerfC);
  InitializeCriticalSection(&CriticalSectionPerfD);

  const size_t thread_count=100;

  //while (true)
  {
    std::vector<std::thread> threads;

    for(size_t i=0; i<thread_count; ++i)
    {
      threads.emplace_back(test_cs_perf_thread_function, nullptr, std::ref(counter));
    }

    auto start=std::chrono::steady_clock::now();


    for(auto& t:threads)
    {
      t.join();
    }

    auto end=std::chrono::steady_clock::now();

    std::cout<<"elapsed time in milliseconds: "
      <<std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()<<" ms sec"<<std::endl;

  }

  DeleteCriticalSection(&CriticalSectionPerfA);
  DeleteCriticalSection(&CriticalSectionPerfB);
  DeleteCriticalSection(&CriticalSectionPerfC);
  DeleteCriticalSection(&CriticalSectionPerfD);

}







