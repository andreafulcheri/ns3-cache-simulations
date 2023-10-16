#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "lib/udp-traffic-cache-cp-helper.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UdpCacheExample");

int
main(int argc, char* argv[])
{

#if 1
  LogComponentEnable ("UdpCacheExample", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpTrafficGenerator", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpContentProviderApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpCacheServerApplication", LOG_LEVEL_INFO);
#endif

    NS_LOG_INFO("Create nodes.");
    NodeContainer c;
    c.Create(6);

    NodeContainer nodeContainer_01;
    nodeContainer_01.Add(c.Get(0));
    nodeContainer_01.Add(c.Get(1));

    NodeContainer nodeContainer_12;
    nodeContainer_12.Add(c.Get(1));
    nodeContainer_12.Add(c.Get(2));
    
    NodeContainer nodeContainer_14;
    nodeContainer_14.Add(c.Get(1));
    nodeContainer_14.Add(c.Get(4));

    NodeContainer nodeContainer_24;
    nodeContainer_24.Add(c.Get(2));
    nodeContainer_24.Add(c.Get(4));

    NodeContainer nodeContainer_23;
    nodeContainer_23.Add(c.Get(2));
    nodeContainer_23.Add(c.Get(3));

    NodeContainer nodeContainer_45;
    nodeContainer_45.Add(c.Get(4));
    nodeContainer_45.Add(c.Get(5));

    InternetStackHelper internet;
    internet.Install(c);

    NS_LOG_INFO("Create channels.");
    CsmaHelper csma_01;
    csma_01.SetChannelAttribute("DataRate", DataRateValue(DataRate("1Mb/s")));
    csma_01.SetChannelAttribute("Delay", TimeValue(MilliSeconds(5)));
    csma_01.SetDeviceAttribute("Mtu", UintegerValue(1400));
    NetDeviceContainer dev01 = csma_01.Install(nodeContainer_01);

    CsmaHelper csma_23;
    csma_23.SetChannelAttribute("DataRate", DataRateValue(DataRate("1Mb/s")));
    csma_23.SetChannelAttribute("Delay", TimeValue(MilliSeconds(5)));
    csma_23.SetDeviceAttribute("Mtu", UintegerValue(1400));
    NetDeviceContainer dev23 = csma_23.Install(nodeContainer_23);

    CsmaHelper csma_45;
    csma_45.SetChannelAttribute("DataRate", DataRateValue(DataRate("1Mb/s")));
    csma_45.SetChannelAttribute("Delay", TimeValue(MilliSeconds(5)));
    csma_45.SetDeviceAttribute("Mtu", UintegerValue(1400));
    NetDeviceContainer dev45 = csma_45.Install(nodeContainer_45);

    PointToPointHelper p2p_14;
    p2p_14.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gb/s")));
    p2p_14.SetChannelAttribute("Delay", TimeValue(MilliSeconds(15)));
    NetDeviceContainer dev14 = p2p_14.Install(nodeContainer_14);

    PointToPointHelper p2p_12;
    p2p_12.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gb/s")));
    p2p_12.SetChannelAttribute("Delay", TimeValue(MilliSeconds(100)));
    NetDeviceContainer dev12 = p2p_12.Install(nodeContainer_12);
    
    PointToPointHelper p2p_24;
    p2p_24.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gb/s")));
    p2p_24.SetChannelAttribute("Delay", TimeValue(MilliSeconds(15)));
    NetDeviceContainer dev24 = p2p_24.Install(nodeContainer_24);

    NS_LOG_INFO("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipInterf14 = ipv4.Assign(dev14);

    ipv4.SetBase("10.3.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipInterf12 = ipv4.Assign(dev12);

    ipv4.SetBase("10.4.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipInterf24 = ipv4.Assign(dev24);

    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipInterf45 = ipv4.Assign(dev45);

    ipv4.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ipInterf23 = ipv4.Assign(dev23);

    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ipInterf01 = ipv4.Assign(dev01);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    //Address cacheAddress = Address(ipInterf45.GetAddress(1));
    Address contentServerAddress = Address(ipInterf23.GetAddress(1));
    //NS_LOG_INFO("CACHE ADDRESS "<< ipInterf45.GetAddress(1));
    NS_LOG_INFO("CONTENT SERVER ADDRESS "<< ipInterf23.GetAddress(1));

    NS_LOG_INFO("Create Applications.");
    ApplicationContainer apps;
    uint16_t content_port = 15;
    UdpContentProviderHelper content_Server(content_port);
    apps = content_Server.Install(nodeContainer_23.Get(1));
    apps.Start(Seconds(0.0));

    /* uint16_t port = 9; // well-known echo port number
    uint32_t cache_size = 20;
    UdpCacheServerHelper cache(contentServerAddress, content_port, port);
    cache.SetAttribute("CacheSize", UintegerValue(cache_size));
    apps = cache.Install(nodeContainer_45.Get(1));
    apps.Start(Seconds(1.0)); */

    uint32_t maxPacketCount = 100;
    Time interPacketInterval = MilliSeconds(49);
    uint32_t variance = 100;
    uint32_t mean = 50;
    UdpTrafficGeneratorHelper client(contentServerAddress, content_port);
    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("NormalVariance", UintegerValue(variance));
    client.SetAttribute("NormalMean", UintegerValue(mean));
    apps = client.Install(nodeContainer_01.Get(0));
    apps.Start(Seconds(2.0));

    //AsciiTraceHelper ascii;
    //csma.EnableAsciiAll(ascii.CreateFileStream("udp-echo.tr"));
    if(system("mkdir output") != -1){
      csma_01.EnablePcapAll("output/udp-echo-csma", false);
    }

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}