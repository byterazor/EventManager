/*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
* This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git
*/

 /** @file */
 #include <EventManager/Event.hpp>
 #include <random>

 EventManager::Event::Event(std::uint32_t type) : type_(type), responseId_(0), isResponse_(false), emitter_(nullptr)
 {
   std::random_device rd;
   std::mt19937 rng(rd());

   std::uniform_int_distribution<std::mt19937::result_type> dist(1,std::numeric_limits<int>::max());

   id_ = dist(rng);
 }

 EventManager::Event::Event(std::uint32_t type, const EventManager::Event &event) : Event(type)
 {
   responseId_=event.id();
   isResponse_=true;
 }

EventManager::Event::Event(std::uint32_t type, const std::shared_ptr<EventManager::Event> event) : Event(type)
{
  responseId_=event->id();
  isResponse_=true;
}
