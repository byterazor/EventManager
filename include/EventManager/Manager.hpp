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
 #include <thread>
 #include <list>
 #include <map>
 #include <condition_variable>

 #include <EventManager/Event.hpp>

 namespace EventManager
 {

   // forward declaration of EventManager::Participant
   class Participant;

   class Manager : public std::enable_shared_from_this<Manager>
   {
     /// the thread the event manager is transmitting events in
     std::unique_ptr<std::thread> mainThread_;

     /// is the main thread running
     std::atomic<bool> isMainThreadRunning_;

     /// stop the main thread
     std::atomic<bool> stopMainThread_;

     /// the thread the event manager is scheduling plugins in
     std::unique_ptr<std::thread> schedulingThread_;

     /// is the scheduling thread running
     std::atomic<bool> isSchedulingThreadRunning_;

     /// stop the scheduling thread
     std::atomic<bool> stopSchedulingThread_;

     /// map holding all the event type and plugin combinations
     std::map<std::uint32_t, std::list<std::shared_ptr<EventManager::Participant>>> eventMap_;
    
     /// mutex to protect the eventMap
     std::mutex mutexEventMap_;

     /// queue for incomng events
     std::queue<std::shared_ptr<EventManager::Event>> eventQueue_;

     /// mutex to protect the event queue
     std::mutex mutexEventQueue_;

     /// condition variable to wake of thread on new emit
     std::condition_variable newEventInQueue_;

     /// list of all plugins requiring scheduling
     std::list<std::shared_ptr<EventManager::Participant>> schedulingParticipants_;

     /// mutex to protect schedulingPlugins_
     std::mutex mutexSchedulingParticipants_;

     /// list of all participants connected
     std::list<std::shared_ptr<EventManager::Participant>> particpants_;

     /// mutex to protect participants_
     std::mutex mutexParticipants_;

     /// the id for the next participant connecting
     std::uint32_t nextParticipantID_;

     /*
     * all private methods
     */

     /**
     * @brief the method running in the main thread
     */
     void mainProcess_();

     /**
     * @brief the method running in the scheduling thread
     */
     void schedulingProcess_();

     /**
     * @brief process one event (call all the participants)
     */
     void processEvent(const std::shared_ptr<EventManager::Event> event);

     /**
     * @brief start the main thread
     */
     void startMain_();

     /**
     * @brief stop the main thread
     */
     void stopMain_();

     /**
     * @brief start the scheduling thread
     */
     void startScheduling_();

     /**
     * @brief stop the scheduling thread
     */
     void stopScheduling_();


    public:
      /**
      * @brief The constructor for the event manager
      *
      * Just initializes all attributes to its starting value
      */
      Manager() : mainThread_(nullptr), isMainThreadRunning_(false),
                       stopMainThread_(false), schedulingThread_(nullptr),
                       isSchedulingThreadRunning_(false), stopSchedulingThread_(false),
                       nextParticipantID_(1){}


      ~Manager();
      /**
      * @brief start the event manager
      */
      void start();

      /**
      * @brief stop the event manager
      */
      void stop();

      /**
      * @brief check if the eventmanager is running
      */
      bool isRunning();

      /**
      * @brief emit an event and make sure it is delivered to all subscribed plugins
      *
      * @param event - the event to emit
      */
      void emit(std::shared_ptr<EventManager::Event> event);

      /**
      * @brief subscribe a plugin for the given event type
      *
      * @param type     - the event type to subscribe to
      * @param plugin   - shared pointer to the plugin which subscribes
      */
      void subscribe(std::uint32_t type, std::shared_ptr<EventManager::Participant> plugin);

      /**
      * @brief unsubscribe a plugin from the given event type
      *
      * @param type     - the event type to unsubscribe from
      * @param plugin   - shared pointer to the plugin which unsubscribes
      */
      void unsubscribe(std::uint32_t type, std::shared_ptr<EventManager::Participant> plugin);

      /**
      * @brief unsubscribe a plugin from the all event types
      *
      * @param plugin   - shared pointer to the plugin which unsubscribes
      */
      void unsubscribe(std::shared_ptr<EventManager::Participant> plugin);

      /**
      * @brief check if there are any subscriptions within the event manager
      */
      bool empty() const;

      /**
      * @brief Wait for the EventManager to become empty.
      *
      * @param timeout - how many milliseconds to wait for EventManager becoming empty
      *
      * @return true  - EventManager is empty
      * @return false - EventManager is not empty
      */
      bool waitEmpty(std::uint32_t timeoutMS) const;

      /**
      * @brief schedule the given plugin regularly
      *
      * @param plugin - the plugin to schedule
      */
      void schedule(std::shared_ptr<EventManager::Participant> plugin);

      /**
       * @brief remove a participant from scheduling
       * 
       * @param participant  - the participant to remove
       */
      void unschedule(std::shared_ptr<EventManager::Participant> participant);

      /**
        * @brief method to connect a particpant to the manager
        */
      void connect(std::shared_ptr<EventManager::Participant> participant);

      /**
      * @brief method to disconnect a particpant from the manager
      */
      void disconnect(std::shared_ptr<EventManager::Participant> participant);

   }; // class Manager

 }; //namespace EventManager
