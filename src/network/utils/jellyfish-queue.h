/*
 * jellyfish-queue.h
 *
 *  Created on: 2014-03-17
 *      Author: Michel Laterman
 */

#ifndef JELLYFISH_QUEUE_H_
#define JELLYFISH_QUEUE_H_

#include <queue>
#include <stack>
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/random-variable.h"

namespace ns3 {

  class TraceContainer;
  /**
   * \ingroup queue
   *
   * \brief A FIFO packet queue that drops tail-end packets on overflow, has malicious delay, dropping and reordering mechanisms
   */
  class JellyFishQueue : public ns3::Queue {
  public:
    static TypeId GetTypeId (void);
    /**
       * \brief JellyFishQueue Constructor
       *
       * Creates a jellyfish queue with a maximum size of 100 packets by default
       */
    JellyFishQueue();

    JellyFishQueue(uint32_t winSize, double dropP);

    virtual ~JellyFishQueue();

    /**
       * Set the operating mode of this device.
       *
       * \param mode The operating mode of this device.
       *
       */
      void SetMode (JellyFishQueue::QueueMode mode);

      /**
       * Get the encapsulation mode of this device.
       *
       * \returns The encapsulation mode of this device.
       */
      JellyFishQueue::QueueMode GetMode (void);

      void SetDropProbability (double dropProbability);
      double GetDropProbability (void);
      void SetReorderWindowSize (uint32_t windowSize);
      uint32_t GetReorderWindowSize (void);

    private:
      virtual bool DoEnqueue (Ptr<Packet> p);
      virtual Ptr<Packet> DoDequeue (void);
      virtual Ptr<const Packet> DoPeek (void) const;

      std::queue<Ptr<Packet> > m_packets;
      std::stack<Ptr<Packet> > m_reorderWindow;
      double m_dropProbability;  // [0, 1]
      uint32_t m_windowSize;  // if > 1 then packets are reordered
      UniformVariable rand;

      uint32_t m_maxPackets;
      uint32_t m_maxBytes;
      uint32_t m_bytesInQueue;
      QueueMode m_mode;
  };

} // namespace ns3

#endif /* JELLYFISH_QUEUE_H_ */
