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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/aodv-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MobileNetwork");

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nJF = 1;
  uint32_t windowSize = 0;
  double dropP = 0.0;
  std::string outName = "Output";

  CommandLine cmd;
  cmd.AddValue ("nJF", "Number of JellyFish devices", nJF);
  cmd.AddValue ("winSize", "JellyFish reorder window size", windowSize);
  cmd.AddValue ("dropP", "Jellyfish Drop probability", dropP);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("o", "Output File name", outName);

  cmd.Parse (argc,argv);

  if (nJF > 14)
    {
      std::cout << "Too many JellyFish nodes" << std::endl;
      exit (1);
    }
	LogComponentEnable ("AodvJFRequestQueue", LOG_LEVEL_ALL);
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer ServerNodes;
  ServerNodes.Create (5);
  NodeContainer AppNodes;
  AppNodes.Create (5);
  NodeContainer JFNodes;
  JFNodes.Create (nJF);
  NodeContainer OtherNodes;
  OtherNodes.Create (15-nJF);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (5.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (ServerNodes);
  mobility.Install (AppNodes);
  mobility.Install (JFNodes);
  mobility.Install (OtherNodes);

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  mac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  phy.SetChannel (channel.Create ());
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

  NetDeviceContainer ServerDev;
  ServerDev = wifi.Install (phy, mac, ServerNodes);
  NetDeviceContainer AppDev;
  AppDev = wifi.Install (phy, mac, AppNodes);
  NetDeviceContainer JFDev;
  JFDev = wifi.Install (phy, mac, JFNodes);
  NetDeviceContainer OtherDev;
  OtherDev = wifi.Install (phy, mac, OtherNodes);

  AodvHelper aodv;
  AodvJFHelper aodvJF;
  aodvJF.Set ("DropProbability", DoubleValue(dropP));
  aodvJF.Set ("WindowSize", UintegerValue(windowSize));
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodvJF);
  stack.Install (ServerNodes);
  stack.Install (AppNodes);
  stack.Install (OtherNodes);
  stack.SetRoutingHelper (aodvJF);
  stack.Install (JFNodes);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer ServerInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  ServerInterfaces = address.Assign (ServerDev);

  Ipv4InterfaceContainer AppInterfaces;
  AppInterfaces = address.Assign (AppDev);
  address.Assign (JFDev);
  address.Assign (OtherDev);


  //Install Applications
  uint16_t port = 8888;
  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (ServerInterfaces.GetAddress (0), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (AppNodes.Get (0));
  for (uint32_t i = 1; i < 5; i++)
    {
      BulkSendHelper appN ("ns3::TcpSocketFactory",
          InetSocketAddress (ServerInterfaces.GetAddress (i), port));
      appN.SetAttribute ("MaxBytes", UintegerValue (0));
      sourceApps.Add(appN.Install (AppNodes.Get (i)));
    }
  sourceApps.Start (Seconds (2.0));
  sourceApps.Stop (Seconds (10.0));
  //Set server sinks
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                           InetSocketAddress (ServerInterfaces.GetAddress (0), port));
  ApplicationContainer sinkApps = sink.Install (ServerNodes.Get (0));
  for (uint32_t i = 1; i < 5; i++)
    {
      PacketSinkHelper sinkS ("ns3::TcpSocketFactory",
                         InetSocketAddress (ServerInterfaces.GetAddress (i), port));
      sinkApps.Add (sinkS.Install (ServerNodes.Get (i)));
    }
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (10.0));

  //phy.EnablePcap ("aodvOutput", ServerNodes, false);
  phy.EnablePcap (outName, JFNodes, true);

  Simulator::Stop (Seconds (12.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
