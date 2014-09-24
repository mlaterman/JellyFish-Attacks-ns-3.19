/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * FOLLOWING THE TUTORIAL
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/jellyfish-queue.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   ...  nN
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0
//n0 is the receiver 'server'
//n1 is a 'router'
//n2-nN are end devices


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

/*
 * A more complex simulation to use the JF attack in
 * attacking nodes are either at the edge or within the network
 */

int
main (int argc, char *argv[])
{
  bool verbose = true;
  bool JFRouter = true;
  uint32_t nCsma = 1;
  uint32_t maxBytes = 0;
  uint32_t winSize = 0;
  double dropP = 0;
  std::string outName = "Output";

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of transmitting csma devices", nCsma);
  cmd.AddValue ("dropP", "The JF Drop probability", dropP);
  cmd.AddValue ("winSize", "The JF Reorder window max size", winSize);
  cmd.AddValue ("JFRouter", "Whether to implement the JFQ in the middle of the network", JFRouter);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("o", "Output pcap name", outName);

  cmd.Parse (argc,argv);
  std::cout << "Drop P " << dropP << std::endl;
  std::cout << outName << std::endl;

  if (verbose)
    {
      LogComponentEnable ("JellyFishQueue", LOG_LEVEL_INFO);
    }

  nCsma = nCsma == 0 ? 1 : nCsma;//At least one end device

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  pointToPoint.SetQueue("ns3::JellyFishQueue");//by default - unless drop or reorder is set it works as a DropTailQueue
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  if (JFRouter)//if we are using the router as the attack node
    {
      PointerValue tmp;
      p2pDevices.Get(1)->GetAttribute ("TxQueue", tmp);
      Ptr<Object> tQ = tmp.GetObject ();
      Ptr<JellyFishQueue> jfq = tQ->GetObject<JellyFishQueue> ();
      jfq->SetDropProbability (dropP);
      jfq->SetReorderWindowSize (winSize);
    }

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  csma.SetQueue("ns3::JellyFishQueue");
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  //CSMA JF queue altered only on devices with applications
  if(!JFRouter)
    {
      PointerValue tmp;
      csmaDevices.Get(1)->GetAttribute ("TxQueue", tmp); //start at index 1 (device 2)
      Ptr<Object> tQ = tmp.GetObject ();
      Ptr<JellyFishQueue> jfq = tQ->GetObject<JellyFishQueue> ();
      jfq->SetDropProbability (dropP);
      jfq->SetReorderWindowSize (winSize);
    }

  uint16_t port = 8888;  // well-known echo port number

  BulkSendHelper source ("ns3::TcpSocketFactory",
                           InetSocketAddress (p2pInterfaces.GetAddress (0), port));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    ApplicationContainer sourceApps = source.Install (csmaNodes.Get (1));
  for(uint32_t i = 2; i < csmaNodes.GetN(); i++)//for all other transmitting devices...
    {
      if(!JFRouter)//if they are the attack nodes...
          {
            PointerValue tmp;
            csmaDevices.Get(i)->GetAttribute ("TxQueue", tmp);
            Ptr<Object> tQ = tmp.GetObject ();
            Ptr<JellyFishQueue> jfq = tQ->GetObject<JellyFishQueue> ();
            jfq->SetDropProbability (dropP);
            jfq->SetReorderWindowSize (winSize);
          }
       sourceApps.Add (source.Install (csmaNodes.Get (i)));//install the application
    }

  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));

  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (p2pInterfaces.GetAddress (0), port));
  ApplicationContainer sinkApps = sink.Install (p2pNodes.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcap (outName, p2pNodes.Get (0)->GetId (), 0, true);//log from 'router'

  std::cout << "Start simulation" << std::endl;
  Simulator::Run ();
  Simulator::Destroy ();
  std::cout << "Stop simulation" << std::endl;
  return 0;
}
