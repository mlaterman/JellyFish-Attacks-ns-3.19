/*
 * aodv-jfrqueue.h
 *
 *  Created on: 2014-04-06
 *      Author: Michel Laterman
 *
 *  attempted to implement a JF attack (reordering, dropping, and delaying packets) in the aodv module
 *  module does not work and is not being developed
 *  released to allow others to work off of a possible starting point.
 */

#ifndef AODV_JFRQUEUE_H_
#define AODV_JFRQUEUE_H_

#include <vector>
#include <stack>
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"

namespace ns3 {
namespace aodv {

  /**
   * \ingroup aodv
   * \brief AODV Queue Entry
   */
  class JFQueueEntry
  {
  public:
    typedef Ipv4RoutingProtocol::UnicastForwardCallback UnicastForwardCallback;
    typedef Ipv4RoutingProtocol::ErrorCallback ErrorCallback;
    /// c-tor
    JFQueueEntry (Ptr<const Packet> pa = 0, Ipv4Header const & h = Ipv4Header (),
                UnicastForwardCallback ucb = UnicastForwardCallback (),
                ErrorCallback ecb = ErrorCallback (), Time exp = Simulator::Now ()) :
      m_packet (pa), m_header (h), m_ucb (ucb), m_ecb (ecb),
      m_expire (exp + Simulator::Now ())
    {}

    /**
     * Compare queue entries
     * \return true if equal
     */
    bool operator== (JFQueueEntry const & o) const
    {
      return ((m_packet == o.m_packet) && (m_header.GetDestination () == o.m_header.GetDestination ()) && (m_expire == o.m_expire));
    }
    ///\name Fields
    //\{
    UnicastForwardCallback GetUnicastForwardCallback () const { return m_ucb; }
    void SetUnicastForwardCallback (UnicastForwardCallback ucb) { m_ucb = ucb; }
    ErrorCallback GetErrorCallback () const { return m_ecb; }
    void SetErrorCallback (ErrorCallback ecb) { m_ecb = ecb; }
    Ptr<const Packet> GetPacket () const { return m_packet; }
    void SetPacket (Ptr<const Packet> p) { m_packet = p; }
    Ipv4Header GetIpv4Header () const { return m_header; }
    void SetIpv4Header (Ipv4Header h) { m_header = h; }
    void SetExpireTime (Time exp) { m_expire = exp + Simulator::Now (); }
    Time GetExpireTime () const { return m_expire - Simulator::Now (); }
    //\}
  private:
    /// Data packet
    Ptr<const Packet> m_packet;
    /// IP header
    Ipv4Header m_header;
    /// Unicast forward callback
    UnicastForwardCallback m_ucb;
    /// Error callback
    ErrorCallback m_ecb;
    /// Expire time for queue entry
    Time m_expire;
  };
  class JFRequestQueue
  {
  public:
    /// Default c-tor
    JFRequestQueue (uint32_t maxLen, Time routeToQueueTimeout) :
      m_maxLen (maxLen), m_queueTimeout (routeToQueueTimeout), m_dropProbability (0), m_reorderSize (0), rand (0.0, 1.0)
    {
    }
    /// Push entry in queue, if there is no entry with the same packet and destination address in queue.
    bool Enqueue (JFQueueEntry & entry);
    /// Return first found (the earliest) entry for given destination
    bool Dequeue (Ipv4Address dst, JFQueueEntry & entry);
    /// Remove all packets with destination IP address dst
    void DropPacketWithDst (Ipv4Address dst);
    /// Finds whether a packet with destination dst exists in the queue
    bool Find (Ipv4Address dst);
    /// Number of entries
    uint32_t GetSize ();
    ///\name Fields
    //\{
    uint32_t GetMaxQueueLen () const { return m_maxLen; }
    void SetMaxQueueLen (uint32_t len) { m_maxLen = len; }
    Time GetQueueTimeout () const { return m_queueTimeout; }
    void SetQueueTimeout (Time t) { m_queueTimeout = t; }
    double GetDropProbability () const { return m_dropProbability; }
    void SetDropProbability (double p) { m_dropProbability = p; }
    uint32_t GetReorderWindowSize () const { return m_reorderSize; }
    void SetReorderWindowSize (uint32_t s) { m_reorderSize = s; }
    //\}

  private:
    std::vector<JFQueueEntry> m_queue;
    std::stack<JFQueueEntry> m_reorderWindow;
    /// Remove all expired entries
    void Purge ();
    /// Notify that packet is dropped from queue by timeout
    void Drop (JFQueueEntry en, std::string reason);
    /// The maximum number of packets that we allow a routing protocol to buffer.
    uint32_t m_maxLen;
    /// The maximum period of time that a routing protocol is allowed to buffer a packet for, seconds.
    Time m_queueTimeout;
    double m_dropProbability;
    uint32_t m_reorderSize;
    UniformVariable rand;

    static bool IsEqual (JFQueueEntry en, const Ipv4Address dst) { return (en.GetIpv4Header ().GetDestination () == dst); }
  };

}
} /* namespace ns3 */

#endif /* AODV_JFRQUEUE_H_ */
