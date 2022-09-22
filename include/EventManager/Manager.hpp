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

    /**
      * @brief command types
      */
    enum class commandType :uint16_t
    {
      // command to connect a participant to the manager
      CONNECT,
      // command to disconnect a participant from the manager
      DISCONNECT,
      // command to schedule start scheduling a participant
      ENABLE_SCHEDULING,
      // command to disable scheduling for a participant (not schedule it anymore)
      DISABLE_SCHEDULING
    }; // commandType

   // forward declaration of EventManager::Participant
   class Participant;

   /**
     * @class Manager
     *
     * If you use the manager it has to be a shared pointer. Otherwise you will 
     * get a mem error.
     *
     * To add participants to the manager call class function connect.
     * Calling start method will start the manager.
     *
     * Depending on your concept you can first connect all participants to 
     * the manager and then start it. Or you can start in first and then 
     * connect the participants. In the first example all participant will
     * be started at the same time (when calling start from the manager.)
     * In the second example they will be started when they are connected.
     * That means if you have one starting event that all participants need
     * to receive you would choose example 1.
     */
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

     /// mutex to protect list schedulingParticipants_
     std::mutex mutexSchedulingParticipants_;

     /// list of all participants connected
     std::list<std::shared_ptr<EventManager::Participant>> participants_;

     /// mutex to protect participants_
     std::mutex mutexParticipants_;

     /// the id for the next participant connecting
     std::uint32_t nextParticipantID_;

     /// queue for different command requests like e.g. connecting a participant to the manager
     std::queue<std::pair<EventManager::commandType,std::shared_ptr<EventManager::Participant>>> commandQueue_;

     /// mutex to protect the commandQueue_
     std::mutex mutexCommandQueue_;

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
     * @brief processes the commands in the commandQueue depending on their type 
     */
     void processCommands_();
     
     /**
     * @brief adds the queued participants to the list of connected participants
     *
     * The connectionQueue_ contains the participants that should be connected to
     * the manager. All connected participants are stored in the list participants_.
     * This class function adds the queued participants to the list participants_
     * and removes them from the queue.
     */
     void processConnect_( std::shared_ptr<EventManager::Participant> participant );
     
     /**
     * @brief removes the given participant form the list of connected participants
     *
     * The participants_ list contains the participants that are connected to
     * the manager. This class function removes the queued participants from the list 
     * participants_.
     */
     void processDisconnect_( std::shared_ptr<EventManager::Participant> participant );

     /**
     * @brief processes the command to enable scheduling for the given participant
     * 
     * This class function adds the given participant to the schedulingParticipants_ list
     * where the participants are scheduled by calling their schedule class function.
     */
     void processEnableScheduling_( std::shared_ptr<EventManager::Participant> participant );

     /**
     * @brief disables the scheduling of the requested participants
     *
     * Removes the participants from the schedulingParticipants_ list that
     * should not be scheduled anymore.
     */
     void processDisableScheduling_( std::shared_ptr<EventManager::Participant> participant );

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
      * Just initializes all attributes to their starting values
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
