/*
 * jellyfish-queue.cc
 *
 *  Created on: 2014-03-17
 *      Author: Michel Laterman
 *
 * TODO: Specify reorder window size, time delay, drop probability
 */
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "jellyfish-queue.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("JellyFishQueue");

namespace ns3 {

  NS_OBJECT_ENSURE_REGISTERED (JellyFishQueue)
    ;

TypeId JellyFishQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::JellyFishQueue")
    .SetParent<Queue> ()
    .AddConstructor<JellyFishQueue> ()
    .AddAttribute ("Mode",
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&JellyFishQueue::SetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this JellyFishQueue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&JellyFishQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this JellyFishQueue.",
                   UintegerValue (100 * 65535),
                   MakeUintegerAccessor (&JellyFishQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DropProbability",
                   "The probability that JellyFishQueue drops a received packet",
                   DoubleValue (0),
                   MakeDoubleAccessor (&JellyFishQueue::m_dropProbability),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReorderWindowSize",
                   "The amount of packets to reorder at a time",
                   UintegerValue (0),
                   MakeUintegerAccessor (&JellyFishQueue::m_windowSize),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

JellyFishQueue::JellyFishQueue() :
Queue (),
m_packets (),
m_reorderWindow (),
m_dropProbability (0),
m_windowSize (0),
rand (0.0, 1.0),
m_bytesInQueue (0)
{
NS_LOG_FUNCTION (this);
}

JellyFishQueue::~JellyFishQueue()
{
  NS_LOG_FUNCTION (this);
}

void
JellyFishQueue::SetMode (JellyFishQueue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

JellyFishQueue::QueueMode
JellyFishQueue::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
JellyFishQueue::SetDropProbability (double dropProbability)
{
  NS_LOG_FUNCTION (this);
  m_dropProbability = dropProbability;
}

double
JellyFishQueue::GetDropProbability (void)
{
  NS_LOG_FUNCTION (this);
  return m_dropProbability;
}

void
JellyFishQueue::SetReorderWindowSize (uint32_t windowSize)
{
  NS_LOG_FUNCTION (this);
  m_windowSize = windowSize;
}

uint32_t
JellyFishQueue::GetReorderWindowSize (void)
{
  NS_LOG_FUNCTION (this);
  return m_windowSize;
}

bool
JellyFishQueue::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  if (m_mode == QUEUE_MODE_PACKETS && ((m_packets.size () + m_reorderWindow.size ()) >= m_maxPackets))
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
    {
      NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (m_dropProbability > 0)
    {
      if (rand.GetValue () < m_dropProbability)
        {
          NS_LOG_LOGIC ("JellyFish queue maliciously dropped pkt");
          Drop (p);
          return false;
        }
    }

  m_bytesInQueue += p->GetSize ();

  if (m_windowSize > 0)
    {
      m_reorderWindow.push (p);
      if (m_reorderWindow.size () == m_windowSize)
        {
          NS_LOG_LOGIC ("JellyFish reorder window flushed (limit reached)");
          for(uint32_t i = 0; i < m_windowSize; i++)
            {
              Ptr<Packet> temp = m_reorderWindow.top ();
              m_reorderWindow.pop ();
              m_packets.push (temp);
            }
        }
    }
  else
    {
      m_packets.push (p);
    }

  NS_LOG_LOGIC ("Number packets " << m_packets.size () << ", In reorder window " << m_reorderWindow.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return true;
}

Ptr<Packet>
JellyFishQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      /*
       * Seems to fail if I don't pull from the window if the queue is empty
       * it was working before (?)
       */
      if (m_reorderWindow.empty ())
        {
          NS_LOG_LOGIC ("Queue & reorder window empty");
          return 0;
        }
      else
        {
          NS_LOG_LOGIC("Single packet taken from reorder window");
          Ptr<Packet> temp = m_reorderWindow.top ();
          m_reorderWindow.pop ();
          m_packets.push (temp);
        }
    }

  Ptr<Packet> p = m_packets.front ();
  m_packets.pop ();
  m_bytesInQueue -= p->GetSize ();

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size () << ", In reorder window " << m_reorderWindow.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

Ptr<const Packet>
JellyFishQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_packets.empty ())
    {
      if (m_reorderWindow.empty ())
        {
          NS_LOG_LOGIC ("Queue & Reorder window empty");
          return 0;
        }
      else
        {
          NS_LOG_LOGIC ("Queue empty");
        }
    }

  Ptr<Packet> p = !(m_packets.empty ()) ? m_packets.front () : m_reorderWindow.top ();

  NS_LOG_LOGIC ("Number packets " << m_packets.size () << ", In reorder window " << m_reorderWindow.size ());
  NS_LOG_LOGIC ("Number bytes " << m_bytesInQueue);

  return p;
}

} /* namespace ns3 */
