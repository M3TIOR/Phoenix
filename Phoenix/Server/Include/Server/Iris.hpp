// Copyright 2019-20 Genten Studios
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

// This is needed because Windows https://github.com/skypjack/entt/issues/96
#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <Common/Input.hpp>
#include <Common/Util/BlockingQueue.hpp>

#include <enet/enet.h>
#include <entt/entt.hpp>

#include <array>

namespace phx::server::networking
{
	struct StateBundle
	{
		bool                                          ready;
		std::size_t                                   users;
		std::size_t                                   sequence;
		std::unordered_map<entt::entity*, InputState> states;
	};

	struct EventBundle
	{
		entt::entity* userRef;
	};

	struct MessageBundle
	{
		entt::entity* userRef;
		std::string   message;
	};

	class Iris
	{
	public:
		/**
		 * @brief Creates a networking object to handle listening for packets
		 *
		 * @param registry The shared EnTT registry
		 * @param running Pointer to a boolean, the threaded function only runs
		 * if this is true
		 */
		Iris(entt::registry* registry, bool* running);

		/**
		 * @brief Cleans up any internal only objects
		 */
		~Iris();

		/**
		 * @brief Loops listening to the netowork and populates queues for data
		 * consumption
		 */
		void run();

		void auth();
		/**
		 * @brief Actions taken when a user disconnects
		 *
		 * @param userRef The user who disconnected
		 */
		void disconnect(entt::entity* userRef);

		/**
		 * @brief Actions taken when an event is received
		 *
		 * @param userRef The user who sent the event packet
		 * @param data The data in the event packet
		 * @param dataLength The length of the data in the event packet
		 */
		void parseEvent(entt::entity* userRef, enet_uint8* data,
		                std::size_t dataLength);

		/**
		 * @brief Actions taken when a state is received
		 *
		 * @param userRef The user who sent the state packet
		 * @param data The data in the state packet
		 * @param dataLength The length of the data in the state packet
		 */
		void parseState(entt::entity* userRef, enet_uint8* data,
		                std::size_t dataLength);

		/**
		 * @brief Actions taken when a message is received
		 *
		 * @param userRef The user who sent the message packet
		 * @param data The data in the message packet
		 * @param dataLength The length of the data in the message packet
		 */
		void parseMessage(entt::entity* userRef, enet_uint8* data,
		                  std::size_t dataLength);

		/**
		 * @brief Sends an event packet to a client
		 *
		 * @param userRef The user to sent the event to
		 * @param data The event packet data
		 */
		void sendEvent(entt::entity* userRef, enet_uint8* data);

		/**
		 * @brief Sends a state packet to a client
		 *
		 * @param userRef The user to sent the state to
		 * @param data The state packet data
		 */
		void sendState(std::size_t sequence);

		/**
		 * @brief Sends a message packet to a client
		 *
		 * @param userRef The user to sent the message to
		 * @param data The message packet data
		 */
		void sendMessage(entt::entity* userRef, const std::string& message);

		/**
		 * @brief The Queue of bundled states received
		 */
		std::vector<StateBundle> currentBundles;
		BlockingQueue<StateBundle> stateQueue;
		/**
		 * @brief The Queue of events received
		 */
		std::list<EventBundle> eventQueue;
		/**
		 * @brief The Queue of messages received
		 */
		std::list<MessageBundle> messageQueue;

	private:
		/// @brief The main loop runs while this is true
		bool* m_running;

		/// @breif An EnTT registry to store various data in
		entt::registry* m_registry;

		ENetHost*   m_server;
		ENetEvent   m_event;
		ENetAddress m_address;
	};
} // namespace phx::server::networking