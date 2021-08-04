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
#include <EventManager/Manager.hpp>
#include <EventManager/Participant.hpp>
#include <memory>
#include <limits>


const static std::uint32_t TEST_EVENT0 = 10;
const static std::uint32_t TEST_EVENT1 = 20;

class myParticipant : public EventManager::Participant
{
  private:
    bool receivedEvent_;

    std::uint32_t eventType_;

    std::uint32_t id_;

    void shutdown_() {
      _unsubscribe();
    }

    void schedule_() {
      std::shared_ptr<EventManager::Event> event = nullptr;

      _lockQueue();
      if (_hasEvents())
      {
        event = _fetchEvent();
      }
      _unlockQueue();
      if (event == nullptr)
      {
        return;
      }

      if (event->type() == EventManager::EVENT_TYPE_SHUTDOWN)
      {
        shutdown_();
      }
      else if (event->type() == eventType_)
      {
        receivedEvent_=true;
      }
    }
  public:
    myParticipant(std::uint32_t id,std::uint32_t eventType) : EventManager::Participant()
    {
      id_=id;
      receivedEvent_=false;
      eventType_=eventType;

    }

    bool eventReceived() const { return receivedEvent_;}

    void init() {
      _subscribe(eventType_);
      _enableScheduling();
    }
};


SCENARIO("Basic Usage of EventManager", "[Manager]")
{
  GIVEN("an EventManager::Manager and two participants")
  {

    std::shared_ptr<EventManager::Manager> manager;
    REQUIRE_NOTHROW([&]()
    {
      manager = std::make_shared<EventManager::Manager>();
    }());

    REQUIRE(manager->empty() == true);

    std::shared_ptr<myParticipant> participant0;
    REQUIRE_NOTHROW([&]()
    {
      participant0 = std::make_shared<myParticipant>(0,TEST_EVENT0);
      participant0->setManager(manager);
      participant0->init();
    }());

    REQUIRE(manager->empty() == false);

    std::shared_ptr<myParticipant> participant1;
    REQUIRE_NOTHROW([&]()
    {
      participant1 = std::make_shared<myParticipant>(1,TEST_EVENT1);
      participant1->setManager(manager);
      participant1->init();
    }());

    REQUIRE(manager->empty() == false);

    REQUIRE_NOTHROW([&]()
    {
      manager->start();
    }());

    REQUIRE(manager->isRunning() == true);

    WHEN("emitting shutdown event")
    {
      std::shared_ptr<EventManager::Event> shutdown = std::make_shared<EventManager::Event>(EventManager::EVENT_TYPE_SHUTDOWN);

      manager->emit(shutdown);
      manager->waitEmpty(3000);


      THEN("participants are shutting down")
      {
        REQUIRE(manager->empty() == true);

        REQUIRE_NOTHROW([&]()
        {
          manager->stop();
        }());

        REQUIRE(manager->isRunning() == false);
      }
    }

    WHEN("emitting events event")
    {
      std::shared_ptr<EventManager::Event> shutdown = std::make_shared<EventManager::Event>(EventManager::EVENT_TYPE_SHUTDOWN);
      std::shared_ptr<EventManager::Event> e0 = std::make_shared<EventManager::Event>(TEST_EVENT0);
      std::shared_ptr<EventManager::Event> e1 = std::make_shared<EventManager::Event>(TEST_EVENT1);
      manager->emit(e0);
      manager->emit(e1);
      manager->emit(shutdown);
      manager->waitEmpty(3000);


      THEN("participants recevied events and are shutting down")
      {
        REQUIRE(manager->empty() == true);
        REQUIRE(participant0->eventReceived());
        REQUIRE(participant1->eventReceived());

        REQUIRE_NOTHROW([&]()
        {
          manager->stop();
        }());

        REQUIRE(manager->isRunning() == false);
      }
    }
  }
}
