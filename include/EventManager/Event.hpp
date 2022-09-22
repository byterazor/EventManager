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
 #include <cstdint>
 #include <atomic>
 #include <stdexcept>
 #include <memory>

 namespace EventManager
 {

   // forward declaration of a participant
   class Participant;

   /// Eventtype to notify all participants that a shutdown is immanent
   const static std::uint32_t EVENT_TYPE_SHUTDOWN = 0;
   /**
     * @class Event
     * @brief An Event is the element in the system that triggers actions from participants
     *
     * Derive own events from this class to sned e.g. also a payload to subscribing 
     * participants. 
     */
   class Event
   {
    private:
      /// the type of the event
      std::uint32_t type_;

      /// the id which uniquly identifies the event
      std::uint64_t id_;

      /// a possible response id, identifying if this event is in repsonse to another event
      std::uint64_t responseId_;

      /// identifies if this event is a response to another event
      std::atomic<bool> isResponse_;

      /// emitter of the event
      std::shared_ptr<EventManager::Participant> emitter_;

    public:
      /**
      * @brief constructor for creating a simple event
      *
      * @param type - what kind of event is this
      */
      Event(std::uint32_t type);

      /**
      * @brief Constructor to create a response event
      */
      Event(std::uint32_t type, const EventManager::Event &event);

      /**
      * @brief Constructor to create a response event
      */
      Event(std::uint32_t type, const std::shared_ptr<EventManager::Event> event);

      /**
      * @brief return the id of the event
      */
      std::uint64_t id() const { return id_;}

      /**
      * @brief return the response id if this event is a response
      */
      std::uint64_t responseId() const {
                        if (!isResponse_)
                        {
                          throw std::runtime_error("is not a response  event");
                        }
                        return responseId_;
                      }

      /**
      * @brief check if the event is a response
      */
      bool isResponse() const { return isResponse_;}

      /**
      * @brief return the type of the event
      */
      std::uint32_t type() const { return type_;}

      /**
      * @brief set the emitter of the event
      */
      void emitter(std::shared_ptr<EventManager::Participant> participant) { emitter_=participant;}

      /**
      * @brief return the emitter of the event
      */
      std::shared_ptr<EventManager::Participant> emitter() const {return emitter_;}

   }; //
 }; // namespace EventManager
