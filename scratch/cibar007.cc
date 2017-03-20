/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Sébastien Deronne
 *
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
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */
//#include <string.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
//#include "src/wifi/model/dcf-manager.h"

// This example considers two hidden stations in an 802.11n network which supports MPDU aggregation.
// The user can specify whether RTS/CTS is used and can set the number of aggregated MPDUs.
//
// Example: ./waf --run "simple-ht-hidden-stations --enableRts=1 --nMpdus=8"
//
// Network topology:
//
//   Wifi 192.168.1.0
//
//        AP
//   *    *    *
//   |    |    |
//   n1   n2   n3
//
// Packets in this simulation aren't marked with a QosTag so they are considered
// belonging to BestEffort Access Class (AC_BE).

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("SimplesHtHiddenStations");

uint32_t countEchoCSend9 = 0;
uint32_t countEchoSReceive9 = 0;

uint32_t countEchoCSend10 = 0;
uint32_t countEchoSReceive10 = 0;

uint32_t countEchoCSend11 = 0;
uint32_t countEchoSReceive11 = 0;

uint32_t countEchoCSend12 = 0;
uint32_t countEchoSReceive12 = 0;

void
UdpEchoClientTrace (std::string context, Ptr<const Packet> packet)
{
	if (context == "/NodeList/0/ApplicationList/0/$ns3::UdpEchoClient/Tx"){
		countEchoCSend9++;}
	else if (context == "/NodeList/1/ApplicationList/0/$ns3::UdpEchoClient/Tx"){
		countEchoCSend10++;}
	else if (context == "/NodeList/2/ApplicationList/0/$ns3::UdpEchoClient/Tx"){
		countEchoCSend11++;}
	else if (context == "/NodeList/3/ApplicationList/0/$ns3::UdpEchoClient/Tx"){
		countEchoCSend12++;}
}

void
UdpEchoServerTrace (std::string context, Ptr<const Packet> packet)
{
	if (context == "/NodeList/4/ApplicationList/0/$ns3::UdpEchoServer/Rx"){
		countEchoSReceive9++;}	
	else if (context == "/NodeList/4/ApplicationList/1/$ns3::UdpEchoServer/Rx"){
		countEchoSReceive10++;}
	else if (context == "/NodeList/4/ApplicationList/2/$ns3::UdpEchoServer/Rx"){
		countEchoSReceive11++;}
	else if (context == "/NodeList/4/ApplicationList/3/$ns3::UdpEchoServer/Rx"){
		countEchoSReceive12++;}

}


//void

//void UdpClientTrace(std::string context, Ptr,

int main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; //bytes
  uint64_t simulationTime = 10; //seconds
  uint32_t nMpdus = 1;
  uint32_t maxAmpduSize = 0;
  bool enableRts = 0;
  std::string packetInterval = "1";

  CommandLine cmd;
  cmd.AddValue ("nMpdus", "Number of aggregated MPDUs", nMpdus);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("enableRts", "Enable RTS/CTS", enableRts); // 1: RTS/CTS enabled; 0: RTS/CTS disabled
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("packetInterval", "Packets per second", packetInterval);
  cmd.Parse (argc, argv);

  if (!enableRts)
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
    }
  else
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
    }

  //if(string) 
  //{
   // Config::SetDefault
  //}

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("990000"));

  //Set the maximum size for A-MPDU with regards to the payload size
  maxAmpduSize = nMpdus * (payloadSize + 200);

  // Set the maximum wireless range to 5 meters in order to reproduce a hidden nodes scenario, i.e. the distance between hidden stations is larger than 5 meters
  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (5));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (4);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  channel.AddPropagationLoss ("ns3::RangePropagationLossModel"); //wireless range limited to 5 meters!

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue ("HtMcs0"));
  WifiMacHelper mac;

  Ssid ssid = Ssid ("simple-mpdu-aggregation");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false),
               "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconInterval", TimeValue (MicroSeconds (102400)),
               "BeaconGeneration", BooleanValue (true),
               "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (phy, mac, wifiApNode);

  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  // AP is between the two stations, each station being located at 5 meters from the AP.
  // The distance between the two stations is thus equal to 10 meters.
  // Since the wireless range is limited to 5 meters, the two stations are hidden from each other.
  positionAlloc->Add (Vector (5.0, 5.0, 0.0));

  positionAlloc->Add (Vector (5.0, 10.0, 0.0));
  positionAlloc->Add (Vector (0.0, 5.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  positionAlloc->Add (Vector (10.0, 5.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterface;
  StaInterface = address.Assign (staDevices);
  Ipv4InterfaceContainer ApInterface;
  ApInterface = address.Assign (apDevice);

  // Setting applications
  UdpEchoServerHelper myServer9 (9);
  ApplicationContainer serverApp1 = myServer9.Install (wifiApNode);
  serverApp1.Start (Seconds (0.0));
  serverApp1.Stop (Seconds (simulationTime + 1));

  // Setting applications
  UdpEchoServerHelper myServer10 (10);
  ApplicationContainer serverApp2 = myServer10.Install (wifiApNode);
  serverApp2.Start (Seconds (0.0));
  serverApp2.Stop (Seconds (simulationTime + 1));

  // Setting applications
  UdpEchoServerHelper myServer11 (11);
  ApplicationContainer serverApp3 = myServer11.Install (wifiApNode);
  serverApp3.Start (Seconds (0.0));
  serverApp3.Stop (Seconds (simulationTime + 1));

  // Setting applications
  UdpEchoServerHelper myServer12 (12);
  ApplicationContainer serverApp4 = myServer12.Install (wifiApNode);
  serverApp4.Start (Seconds (0.0));
  serverApp4.Stop (Seconds (simulationTime + 1));

  UdpEchoClientHelper myClient1 (ApInterface.GetAddress (0), 9);
  myClient1.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  myClient1.SetAttribute ("Interval", TimeValue (Time ("0.00002"))); //packets/s
  myClient1.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  // Saturated UDP traffic from stations to AP
  ApplicationContainer clientApp9 = myClient1.Install (wifiStaNodes.Get(0));
  clientApp9.Start (Seconds (1.0));
  clientApp9.Stop (Seconds (simulationTime + 1));
  
  UdpEchoClientHelper myClient2 (ApInterface.GetAddress (0), 10);
  myClient2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  myClient2.SetAttribute ("Interval", TimeValue (Time ("0.00002"))); //packets/s
  myClient2.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  // Saturated UDP traffic from stations to AP
  ApplicationContainer clientApp10 = myClient2.Install (wifiStaNodes.Get(1));
  clientApp10.Start (Seconds (1.0));
  clientApp10.Stop (Seconds (simulationTime + 1));
  
  UdpEchoClientHelper myClient3 (ApInterface.GetAddress (0), 11);
  myClient3.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  myClient3.SetAttribute ("Interval", TimeValue (Time ("0.00002"))); //packets/s
  myClient3.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  // Saturated UDP traffic from stations to AP
  ApplicationContainer clientApp11 = myClient3.Install (wifiStaNodes.Get(2));
  clientApp11.Start (Seconds (1.0));
  clientApp11.Stop (Seconds (simulationTime + 1));
  
  UdpEchoClientHelper myClient4 (ApInterface.GetAddress (0), 12);
  myClient4.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  myClient4.SetAttribute ("Interval", TimeValue (Time ("0.00002"))); //packets/s
  myClient4.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  // Saturated UDP traffic from stations to AP
  ApplicationContainer clientApp12 = myClient4.Install (wifiStaNodes.Get(3));
  clientApp12.Start (Seconds (1.0));
  clientApp12.Stop (Seconds (simulationTime + 1));

  phy.EnablePcap ("SimpleHtHiddenStations_Ap", apDevice.Get (0));
  phy.EnablePcap ("SimpleHtHiddenStations_Sta1", staDevices.Get (0));
  phy.EnablePcap ("SimpleHtHiddenStations_Sta2", staDevices.Get (1));
  phy.EnablePcap ("SimpleHtHiddenStations_Sta3", staDevices.Get (2));
  phy.EnablePcap ("SimpleHtHiddenStations_Sta4", staDevices.Get (3));

//change the 0's to * just in case it doesn't work
  Config::Connect ("NodeList/*/ApplicationList/*/$ns3::UdpEchoClient/Tx", MakeCallback(&UdpEchoClientTrace));
  Config::Connect ("NodeList/*/ApplicationList/*/$ns3::UdpEchoServer/Rx", MakeCallback(&UdpEchoServerTrace));
  

  Simulator::Stop (Seconds (simulationTime + 1));

  Simulator::Run ();
  Simulator::Destroy ();

  //MY_DEBUG(apDevice.Get(0));

  NS_LOG_UNCOND ("Number of Packets sent from client 9: " << countEchoCSend9);
  NS_LOG_UNCOND ("Number of Packets received at server 9: " << countEchoSReceive9);
  std::cout << "Lost Packets from client 9 to server 9: " << countEchoCSend9 - countEchoSReceive9 << "\n\n";

  NS_LOG_UNCOND ("Number of Packets sent from client 10: " << countEchoCSend10);
  NS_LOG_UNCOND ("Number of Packets received at server 10: " << countEchoSReceive10);
  std::cout << "Lost Packets from client 10 to server 10: " << countEchoCSend10 - countEchoSReceive10 << "\n\n";

  NS_LOG_UNCOND ("Number of Packets sent from client 11: " << countEchoCSend11);
  NS_LOG_UNCOND ("Number of Packets received at server 11: " << countEchoSReceive11);
  std::cout << "Lost Packets from client 11 to server 11: " << countEchoCSend11 - countEchoSReceive11 << "\n\n";

  NS_LOG_UNCOND ("Number of Packets sent from client 12: " << countEchoCSend12);
  NS_LOG_UNCOND ("Number of Packets received at server 12: " << countEchoSReceive12);
  std::cout << "Lost Packets from client 9 to server 12: " << countEchoCSend12 - countEchoSReceive12 << "\n\n";


  //uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp1.Get (0))->GetReceived ();
  double throughput9 = ((countEchoSReceive9 * payloadSize * 8)) / (simulationTime * 1000000.0);
  double packetloss9 = (double) (countEchoCSend9 - countEchoSReceive9) * 100/ (double) countEchoCSend9;
  
  double throughput10 = ((countEchoSReceive10 * payloadSize * 8)) / (simulationTime * 1000000.0);
  double packetloss10 = (double) (countEchoCSend10 - countEchoSReceive10) * 100/ (double) countEchoCSend10;
  
  double throughput11 = ((countEchoSReceive11 * payloadSize * 8)) / (simulationTime * 1000000.0);
  double packetloss11 = (double) (countEchoCSend11 - countEchoSReceive11) * 100/ (double) countEchoCSend11;

  double throughput12 = ((countEchoSReceive12 * payloadSize * 8)) / (simulationTime * 1000000.0);
  double packetloss12 = (double) (countEchoCSend12 - countEchoSReceive12) * 100/ (double) countEchoCSend12;
 

  std::cout << "Throughput client 9: " << throughput9 << " Mbits/s\n";
  std::cout << "Packet loss client 9: " << packetloss9 << " %\n\n";

  std::cout << "Throughput client 10: " << throughput10 << " Mbits/s\n";
  std::cout << "Packet loss client 10: " << packetloss10 << " %\n\n";

  std::cout << "Throughput client 11: " << throughput11 << " Mbits/s\n";
  std::cout << "Packet loss client 11: " << packetloss11 << " %\n\n";

  std::cout << "Throughput client 12: " << throughput12 << " Mbits/s\n";
  std::cout << "Packet loss client 12: " << packetloss12 << " %\n\n";




  //std::cout << "Total Packets: " << totalPacketThrough << " Packets" << '\n';
  //double throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0);
  //std::cout << "Throughput: " << throughput9 << " Mbit/s" << '\n';

  return 0;
}
