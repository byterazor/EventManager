/*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
* This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git
*/

 /** @file */
 #include <EventManager/Manager.hpp>
 #include <EventManager/Participant.hpp>
 #include <iostream>

 void EventManager::Manager::startMain_()
 {

   if (isMainThreadRunning_)
   {
     throw std::runtime_error("Main thread already running");
   }

   stopMainThread_=false;

   mainThread_ = std::make_unique<std::thread>(&EventManager::Manager::mainProcess_,this);

   std::int32_t timeout = 6000;

   while(!isMainThreadRunning_ && timeout > 0)
   {
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
     timeout-=100;
   }

   if (timeout <= 0)
   {
      stopMainThread_=true;
      throw std::runtime_error("EventManager: can not start main thread");
   }

 }

 void EventManager::Manager::startScheduling_()
 {

   if (isSchedulingThreadRunning_)
   {
     throw std::runtime_error("Scheduling thread already running");
   }

   stopSchedulingThread_=false;

   schedulingThread_ = std::make_unique<std::thread>(&EventManager::Manager::schedulingProcess_,this);

   std::int32_t timeout = 6000;

   while(!isSchedulingThreadRunning_ && timeout > 0)
   {
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
     timeout-=100;
   }

   if (timeout <= 0)
   {
      stopSchedulingThread_=true;
      throw std::runtime_error("EventManager: can not start scheduling thread");
   }

 }


 void EventManager::Manager::stopMain_()
 {
   std::int32_t timeout = 6000;
   stopMainThread_=true;
   newEventInQueue_.notify_one();

   while(isMainThreadRunning_ && timeout > 0)
   {
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
     timeout-=100;
   }

   if (timeout <= 0)
   {
     throw std::runtime_error("can not stop main thread");
   }

   mainThread_->join();
 }

 void EventManager::Manager::stopScheduling_()
 {
   std::int32_t timeout = 6000;
   stopSchedulingThread_=true;

   while(isSchedulingThreadRunning_ && timeout > 0)
   {
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
     timeout-=100;
   }

   if (timeout <= 0)
   {
     throw std::runtime_error("can not stop scheduling thread");
   }

   schedulingThread_->join();
 }

 void EventManager::Manager::start()
 {
   startMain_();
   try {
     startScheduling_();
   } catch (std::exception &e)
   {
     stopMain_();
     throw e;
   }
 }

 void EventManager::Manager::stop()
 {
   stopMain_();
   stopScheduling_();
 }

 EventManager::Manager::~Manager()
 {
   if (isMainThreadRunning_)
   {
     stopMain_();
   }

   if (isSchedulingThreadRunning_)
   {
     stopScheduling_();
   }
 }

 void EventManager::Manager::processEvent(const std::shared_ptr<EventManager::Event> event)
 {

   auto it = eventMap_.find(event->type());

   if (it != eventMap_.end())
   {

     for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
     {
       if (event->emitter() != *it2)
       {
         (*it2)->emit(event);
       }
     }

   }
 }

 void EventManager::Manager::mainProcess_()
 {
   isMainThreadRunning_=true;

   while(!stopMainThread_)
   {
     std::unique_lock<std::mutex> lock(mutexEventQueue_);
     newEventInQueue_.wait(lock);
     while(!eventQueue_.empty())
     {
       std::shared_ptr<EventManager::Event> event = eventQueue_.front();
       eventQueue_.pop();
       processEvent(event);
     }

     lock.unlock();

   }

   isMainThreadRunning_=false;

 }

 void EventManager::Manager::schedulingProcess_()
 {
   isSchedulingThreadRunning_=true;

   while(!stopSchedulingThread_)
   {
     mutexSchedulingParticipants_.lock();
     if (!schedulingParticipants_.empty())
     {
       for (auto it = schedulingParticipants_.begin(); it != schedulingParticipants_.end(); ++it)
       {
         (*it)->schedule();
       }
     }
     mutexSchedulingParticipants_.unlock();
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   isSchedulingThreadRunning_=false;

 }


 void EventManager::Manager::subscribe(std::uint32_t type, std::shared_ptr<EventManager::Participant> participant)
 {

   // check if participant is already registered
   auto it = eventMap_.find(type);

   if (it != eventMap_.end())
   {

     auto it2 = std::find(it->second.begin(), it->second.end(),participant);

     if (it2 == it->second.end())
     {
       it->second.push_back(participant);
     }

   }
   else
   {
     eventMap_[type].push_back(participant);
   }

 }

 void EventManager::Manager::unsubscribe(std::uint32_t type, std::shared_ptr<EventManager::Participant> participant)
 {

   auto it = eventMap_.find(type);

   if (it == eventMap_.end())
   {
     return;
   }

   auto it2 = std::find(it->second.begin(), it->second.end(),participant);

   if (it2 != it->second.end())
   {
     it->second.erase(it2);
   }

 }

 void EventManager::Manager::unsubscribe(std::shared_ptr<EventManager::Participant> participant)
 {

   for (auto it = eventMap_.begin(); it != eventMap_.end(); ++it)
   {
     unsubscribe(it->first,participant);
   }

 }


 void EventManager::Manager::emit(const std::shared_ptr<EventManager::Event> event)
 {
   {
     std::lock_guard<std::mutex> lock(mutexEventQueue_);
     eventQueue_.push(event);
   }
   newEventInQueue_.notify_one();

 }

 bool EventManager::Manager::isRunning()
 {

   if (isMainThreadRunning_ && isSchedulingThreadRunning_)
   {
     return true;
   }

   return false;
 }

 bool EventManager::Manager::empty() const
 {
   bool isEmpty=true;

   for (auto it = eventMap_.begin(); it != eventMap_.end(); ++it)
   {
     if ( !(*it).second.empty())
     {
       isEmpty=false;
     }
   }

   return isEmpty;
 }

 bool EventManager::Manager::waitEmpty(std::uint32_t timeoutMS) const
 {
   std::uint32_t timeout=timeoutMS;

   while(!empty() && timeout > 0)
   {
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
     timeout-=100;
   }

   if (timeout == 0)
   {
     return false;
   }

   return true;

 }

 void EventManager::Manager::schedule(std::shared_ptr<EventManager::Participant> participant)
 {
   std::lock_guard<std::mutex> guard(mutexSchedulingParticipants_);

   auto it = std::find(schedulingParticipants_.begin(), schedulingParticipants_.end(), participant);

   if (it == schedulingParticipants_.end())
   {
     schedulingParticipants_.push_back(participant);
   }
 }


 void EventManager::Manager::connect(std::shared_ptr<EventManager::Participant> participant)
 {
   std::lock_guard<std::mutex> guard(mutexParticipants_);
   particpants_.push_back(participant);
   participant->setManager(shared_from_this());
   participant->init();
 }

 void EventManager::Manager::disconnect(std::shared_ptr<EventManager::Participant> participant)
 {

   disconnect(participant);
   
   std::lock_guard<std::mutex> guard(mutexParticipants_);
   std::list<std::shared_ptr<EventManager::Participant>>::iterator it;

   it = std::find(particpants_.begin(), particpants_.end(),participant);

   if (it != particpants_.end())
   {
     particpants_.erase(it);
   }



   participant->setManager(nullptr);
 }
