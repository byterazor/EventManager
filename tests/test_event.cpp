/*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
* This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git
*/

 /** @file */
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <EventManager/Event.hpp>
#include <memory>
#include <limits>

SCENARIO("Use Event Class", "[Event]")
{
  GIVEN("nothing")
  {
    WHEN("creating an Event from scratch")
    {
      std::unique_ptr<EventManager::Event> e = std::make_unique<EventManager::Event>(10);

      THEN("the attributes can be correctly fetched")
      {
        REQUIRE(e->id() >= std::numeric_limits<std::uint64_t>::min());
        REQUIRE(e->id() <= std::numeric_limits<std::uint64_t>::max());
        REQUIRE(e->isResponse() == false);
        REQUIRE_THROWS([&]()
        {
          e->responseId();
        }());
      }
    }
    WHEN("creating an response Event")
    {
      std::unique_ptr<EventManager::Event> e = std::make_unique<EventManager::Event>(10);
      std::unique_ptr<EventManager::Event> r = std::make_unique<EventManager::Event>(10, *e);

      THEN("the attributes can be correctly fetched")
      {
        REQUIRE(r->id() >= std::numeric_limits<std::uint64_t>::min());
        REQUIRE(r->id() <= std::numeric_limits<std::uint64_t>::max());
        REQUIRE(r->isResponse() == true);
        std::uint64_t rid=0;
        REQUIRE_NOTHROW([&]()
        {
          rid=r->responseId();
        }());
        REQUIRE(rid == e->id());
      }
    }
  }
}
