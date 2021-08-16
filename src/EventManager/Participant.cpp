/*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
* This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git
*/

 /** @file */
 #include <EventManager/Participant.hpp>
 #include <EventManager/Manager.hpp>
 #include <iostream>
 #include <chrono>
 using namespace std::chrono_literals;


EventManager::Participant::Participant() : manager_(nullptr),
                                  isScheduledByManager_(false), isQueueLocked_(false)
{

}

void EventManager::Participant::connect(std::shared_ptr<EventManager::Participant> participant)
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  manager_->connect(participant);
}

void EventManager::Participant::disconnect(std::shared_ptr<EventManager::Participant> participant)
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  manager_->disconnect(participant);
}


void EventManager::Participant::emit(std::shared_ptr<EventManager::Event> event)
{
  {
    std::lock_guard<std::mutex> lock(mutexEventQueue_);
    eventQueue_.push(event);
  }
  newEventInQueue_.notify_one();
}

bool EventManager::Participant::_hasEvents()
{
  if (isQueueLocked_)
  {
    return !eventQueue_.empty();
  }

  std::lock_guard<std::mutex> guard(mutexEventQueue_);


  return !eventQueue_.empty();
}

void EventManager::Participant::_lockQueue()
{
  mutexEventQueue_.lock();
  isQueueLocked_=true;
}

void EventManager::Participant::_unlockQueue()
{
  mutexEventQueue_.unlock();
  isQueueLocked_=false;
}

std::shared_ptr<EventManager::Event> EventManager::Participant::_fetchEvent()
{
  if (!isQueueLocked_)
  {
    throw std::runtime_error("queue not locked");
  }

  std::shared_ptr<EventManager::Event> event = eventQueue_.front();
  eventQueue_.pop();

  return event;
}

void EventManager::Participant::_waitForEvent()
{
  std::unique_lock<std::mutex> lock(mutexEventQueue_);
  newEventInQueue_.wait(lock);
  isQueueLocked_=true;
}

bool EventManager::Participant::_waitForEvent(std::uint32_t timeoutMS)
{
  std::unique_lock<std::mutex> lock(mutexEventQueue_);
  if (newEventInQueue_.wait_for(lock,timeoutMS*1ms)==std::cv_status::no_timeout)
  {
    isQueueLocked_=true;
    return true;
  }

  return false;

}


void EventManager::Participant::_enableScheduling()
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  manager_->schedule(shared_from_this());
  isScheduledByManager_=true;
}

void EventManager::Participant::_subscribe(std::uint32_t type)
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  manager_->subscribe(type, shared_from_this());
}

void EventManager::Participant::_unsubscribe(std::uint32_t type)
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  manager_->unsubscribe(type, shared_from_this());
}

void EventManager::Participant::_unsubscribe()
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  manager_->unsubscribe(shared_from_this());
}

void EventManager::Participant::_emit(std::shared_ptr<EventManager::Event> event)
{
  if (manager_ == nullptr)
  {
    throw std::runtime_error("no event manager set yet");
  }
  event->emitter(shared_from_this());
  manager_->emit(event);
}
