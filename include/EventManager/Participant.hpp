#pragma once
/*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
* Copyright 2021 Dominik Meyer <dmeyer@federationhq.de>
* This file is part of the EventManager distribution hosted at https://gitea.federationhq.de/byterazor/EventManager.git
*/

 /** @file */
 #include <memory>
 #include <atomic>
 #include <stdexcept>
 #include <string>
 #include <mutex>
 #include <queue>
 #include <condition_variable>

 #include <EventManager/Event.hpp>

 namespace EventManager
 {

   // forward declaration for the EventManager::Manager
   class Manager;

   /**
   * @brief The entity participating in the event system.
   */
   class Participant : public std::enable_shared_from_this<Participant>
   {
    private:
      /// a unique id for this participant, helpful for debugging
      std::uint32_t id_;

      /// pointer to the event manager
      std::shared_ptr<EventManager::Manager> manager_;

      /// is the participant scheduled by the EventManager::Manager
      std::atomic<bool> isScheduledByManager_;

      /// queue for incomng events
      std::queue<std::shared_ptr<EventManager::Event>> eventQueue_;

      /// mutex to protect the event queue
      std::mutex mutexEventQueue_;

      /// condition variable to wake of thread on new trigger
      std::condition_variable newEventInQueue_;

      /// has the participant locked the queue itself already
      std::atomic<bool> isQueueLocked_;

      /*
      * all private methods
      */

      /**
      * @brief Method called if the participant is scheduled by EventManager::Manager
      *
      * This method needs to be implemented by the child class.
      * Please make sure this method returns as fast as possible!
      * No endless loops are supported.
      * Just process some incoming events and then return!
      */
      virtual void schedule_() { throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not implemented");}

      /**
      * @brief method called by the EventManager::Manager on connecting this particpant
      *
      * This method need to be implemented by each child class of EventManager::Participant
      * Its the best location for subscribing to events and enable scheduling if required.
      */
      virtual void init_() {throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " not implemented");}


    protected:

      /**
      * @brief check if events are available
      */
      bool _hasEvents();

      /**
      * @brief Lock the queue to process all events
      */
      void _lockQueue();

      /**
      * @brief UnLock the queue to process all events
      */
      void _unlockQueue();

      /**
      * @brief fetch one event from the queue
      */
      std::shared_ptr<EventManager::Event> _fetchEvent();

      /**
      * @brief wait for a new event
      */
      void _waitForEvent();

      /**
      * @brief wait for a new event with timeout
      *
      * @return true  - new event available and queue locked
      * @return false - no new event, queue not locked, timeout reached
      */
      bool _waitForEvent(std::uint32_t timeoutMS);

      /**
      * @brief This method subscribes the participant to an event type
      *
      * @param type - the event type to subscribe this participant to
      */
      void _subscribe(std::uint32_t type);

      /**
      * @brief unsubscribe the participant from the given event type
      */
      void _unsubscribe(std::uint32_t type);

      /**
      * @brief unsubscribe the participant from the all event types
      */
      void _unsubscribe();

      /**
      * @brief Method to emit an event to the event manager
      *
      * @param event - the event to emit
      */
      void _emit(std::shared_ptr<EventManager::Event> event);

      /**
      * @brief enable scheduling of this particpant through the EventManager::Manager
      */
      void _enableScheduling();

      /**
      * @brief check if the participant is scheduled by event manager
      */
      bool isScheduledByManager() const {return isScheduledByManager_;}

      /**
      * @brief connect a new participant through another participant
      */
      void connect(std::shared_ptr<EventManager::Participant> participant);

      /**
      * @brief disconnect a participant through another participant
      */
      void disconnect(std::shared_ptr<EventManager::Participant> participant);

    public:
      /**
      * @brief Constructor setting the participant up for use
      */
      Participant();

      /**
       * @brief Method to set the Manager object
       *
       * This method is in general only used by the EventManager::Manager!
       * Only use this method if you really know what you are doing!
       *
       * @param manager - the manager to set
       */
      void setManager(std::shared_ptr<EventManager::Manager> manager) { manager_=manager;_subscribe(EVENT_TYPE_SHUTDOWN);}

      /**
       * @brief Method to set the unique id of the participant
       * 
       * This method is in general only used by the EventManager::Manager!
       * Only use this method if you really know what you are doing!
       *
       * @param id - the id to set
       */
      void setID(const std::uint32_t id) { id_=id;}

      /**
       * @brief Method to return the unique id of the participant
       * 
       * @return std::uint32_t 
       */
      std::uint32_t getID() const { return id_;}

      /**
      * @brief Method called by the EventManager::Manager to schedule the particpant
      *
      */
      void schedule() {schedule_();};

      /**
      * @brief method called by the EventManager::Manager when connecting the particpant
      *
      * method calls virtual _init method which has to be implemented by each particpants child class
      */
      void init() {init_();}

      /**
      * @brief emit an event to this participant
      */
      void emit(std::shared_ptr<EventManager::Event> event);

   };//

 }; // namespace EventManager
