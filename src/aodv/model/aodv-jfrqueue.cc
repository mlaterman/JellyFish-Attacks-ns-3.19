/*
 * aodv-jfrqueue.cc
 *
 *  Created on: 2014-04-06
 *      Author: boots
 */

#include "aodv-jfrqueue.h"
#include <algorithm>
#include <functional>
#include "ns3/ipv4-route.h"
#include "ns3/socket.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("AodvJFRequestQueue");

namespace ns3
{
namespace aodv
{

  uint32_t
  JFRequestQueue::GetSize ()
  {
    Purge ();
    return m_queue.size ();
  }

  bool
  JFRequestQueue::Enqueue (JFQueueEntry & entry)
  {
    Purge ();
    for (std::vector<JFQueueEntry>::const_iterator i = m_queue.begin (); i
         != m_queue.end (); ++i)
      {
        if ((i->GetPacket ()->GetUid () == entry.GetPacket ()->GetUid ())
            && (i->GetIpv4Header ().GetDestination ()
                == entry.GetIpv4Header ().GetDestination ()))
          return false;
      }
    entry.SetExpireTime (m_queueTimeout);
    if (m_queue.size () == m_maxLen)
      {
        Drop (m_queue.front (), "Drop the most aged packet"); // Drop the most aged packet
        m_queue.erase (m_queue.begin ());
      }
    if (m_dropProbability > 0)
      {
        if (rand.GetValue () < m_dropProbability)
          {
            NS_LOG_INFO ("Routing Enqueue Dropped - Malicious");
            return false;
          }
      }
    if (m_reorderSize > 1)
      {
		NS_LOG_LOGIC ("Request pushed to reorder buffer");
        m_reorderWindow.push (entry);
        if (m_reorderWindow.size () == m_reorderSize)
          {
           NS_LOG_LOGIC("Reorder queue flushed");
           for (uint32_t i = 0; i < m_reorderSize; i++)
             {
               JFQueueEntry temp = m_reorderWindow.top ();
               m_reorderWindow.pop ();
               m_queue.push_back (temp);
             }
          }
        return true;
      }

    m_queue.push_back (entry);
    return true;
  }

  void
  JFRequestQueue::DropPacketWithDst (Ipv4Address dst)
  {
    NS_LOG_FUNCTION (this << dst);
    Purge ();
    for (std::vector<JFQueueEntry>::iterator i = m_queue.begin (); i
         != m_queue.end (); ++i)
      {
        if (IsEqual (*i, dst))
          {
            Drop (*i, "DropPacketWithDst ");
          }
      }
    m_queue.erase (std::remove_if (m_queue.begin (), m_queue.end (),
                                   std::bind2nd (std::ptr_fun (JFRequestQueue::IsEqual), dst)), m_queue.end ());
  }

  bool
  JFRequestQueue::Dequeue (Ipv4Address dst, JFQueueEntry & entry)
  {
    Purge ();
    for (std::vector<JFQueueEntry>::iterator i = m_queue.begin (); i != m_queue.end (); ++i)
      {
        if (i->GetIpv4Header ().GetDestination () == dst)
          {
            entry = *i;
            m_queue.erase (i);
            return true;
          }
      }
    return false;
  }

  bool
  JFRequestQueue::Find (Ipv4Address dst)
  {
    for (std::vector<JFQueueEntry>::const_iterator i = m_queue.begin (); i
         != m_queue.end (); ++i)
      {
        if (i->GetIpv4Header ().GetDestination () == dst)
          return true;
      }
    return false;
  }

  struct IsExpired
  {
    bool
    operator() (JFQueueEntry const & e) const
    {
      return (e.GetExpireTime () < Seconds (0));
    }
  };

  void
  JFRequestQueue::Purge ()
  {
    IsExpired pred;
    for (std::vector<JFQueueEntry>::iterator i = m_queue.begin (); i
         != m_queue.end (); ++i)
      {
        if (pred (*i))
          {
            Drop (*i, "Drop outdated packet ");
          }
      }
    m_queue.erase (std::remove_if (m_queue.begin (), m_queue.end (), pred),
                   m_queue.end ());
  }

  void
  JFRequestQueue::Drop (JFQueueEntry en, std::string reason)
  {
    NS_LOG_LOGIC (reason << en.GetPacket ()->GetUid () << " " << en.GetIpv4Header ().GetDestination ());
    en.GetErrorCallback () (en.GetPacket (), en.GetIpv4Header (),
                            Socket::ERROR_NOROUTETOHOST);
    return;
  }


}
} /* namespace ns3 */
