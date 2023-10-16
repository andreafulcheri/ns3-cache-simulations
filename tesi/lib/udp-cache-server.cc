#include "udp-cache-server.h"

#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

#include <regex>
#include <fstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UdpCacheServerApplication");

NS_OBJECT_ENSURE_REGISTERED(UdpCacheServer);

TypeId
UdpCacheServer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::UdpCacheServer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpCacheServer>()
            .AddAttribute("CacheSize",
                          "Size of the cache",
                          UintegerValue(100),
                          MakeUintegerAccessor(&UdpCacheServer::m_cacheSize),
                          MakeUintegerChecker<u_int32_t>())
            .AddAttribute("rttCacheMiss",
                          "RTT for cache miss",
                          TimeValue(MilliSeconds(500)),
                          MakeTimeAccessor(&UdpCacheServer::m_RTTCacheMiss),
                          MakeTimeChecker())
            .AddAttribute("PortClients",
                          "Port on which we listen for incoming request from clients.",
                          UintegerValue(9),
                          MakeUintegerAccessor(&UdpCacheServer::m_port_clients),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("PortContentServer",
                          "Port on which we listen for incoming packets from content server.",
                          UintegerValue(15),
                          MakeUintegerAccessor(&UdpCacheServer::m_port_server),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("IpContentServer",
                          "Ip address on which we listen for incoming packets from content server.",
                          AddressValue(),
                          MakeAddressAccessor(&UdpCacheServer::contentServerAddress),
                          MakeAddressChecker())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&UdpCacheServer::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&UdpCacheServer::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

UdpCacheServer::UdpCacheServer()
{
    NS_LOG_FUNCTION(this);
}

UdpCacheServer::~UdpCacheServer()
{
    NS_LOG_FUNCTION(this);
    m_socket_clients = nullptr;
    m_socket_server = nullptr;
}

void
UdpCacheServer::DoDispose()
{
    NS_LOG_FUNCTION(this);
    printOut();
    Application::DoDispose();
}

void
UdpCacheServer::StartApplication()
{
    NS_LOG_FUNCTION(this);
    hitcount = 0;
    accesscount = 0;

    if(contentServerAddress.IsInvalid()){
        NS_FATAL_ERROR("Fatal Error: Content server address not valid");
    }

    if (!m_socket_clients)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket_clients = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port_clients);
        if (m_socket_clients->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket clients");
        }
    }

    m_socket_clients->SetRecvCallback(MakeCallback(&UdpCacheServer::HandleReadClients, this));


    if (!m_socket_server)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket_server = Socket::CreateSocket(GetNode(), tid);

        if (Ipv4Address::IsMatchingType(contentServerAddress) == true)
        {
            if (m_socket_server->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            contentServerAddress = InetSocketAddress(Ipv4Address::ConvertFrom(contentServerAddress), m_port_server);
            if(m_socket_server->Connect(contentServerAddress)==-1){
                NS_FATAL_ERROR("Failed to connect socket to content server");
            }
        }
        else if (InetSocketAddress::IsMatchingType(contentServerAddress) == true)
        {
            if (m_socket_server->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket_server->Connect(contentServerAddress);
        }
        else
        {
            NS_ASSERT_MSG(false, "Incompatible address type: " << contentServerAddress);
        }
    }

    m_socket_server->SetRecvCallback(MakeCallback(&UdpCacheServer::HandleReadServer, this));
}

void
UdpCacheServer::StopApplication()
{
    NS_LOG_FUNCTION(this);
    printOut();

    if (m_socket_clients)
    {
        m_socket_clients->Close();
        m_socket_clients->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }

    if (m_socket_server)
    {
        m_socket_server->Close();
        m_socket_server->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
UdpCacheServer::HandleReadClients(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " cache received " << packet->GetSize() << " bytes from client " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " on port " << InetSocketAddress::ConvertFrom(from).GetPort());
        }

        /* packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags(); */

        uint32_t value_from_pkt = getIdPacket(packet);
        accesscount++;

        NS_LOG_LOGIC("Check in the cache if the packet with random value " << value_from_pkt << " is present");
        if (cacheContains(value_from_pkt))
        {
            // Serve the packet from cache
            sendPacketBackToClient(value_from_pkt, from);
            hitcount++;

            if (InetSocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("Cache hit: Serving packet with random value " << value_from_pkt);
                NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " cache sent " << packet->GetSize() << " bytes to client " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " on port " << InetSocketAddress::ConvertFrom(from).GetPort());
            }
        }
        else
        {
            if (InetSocketAddress::IsMatchingType(contentServerAddress))
            {
                NS_LOG_INFO("Cache miss: Requesting packet with random value " << value_from_pkt << " to the content server " << InetSocketAddress::ConvertFrom(contentServerAddress).GetIpv4() << " on port " << InetSocketAddress::ConvertFrom(contentServerAddress).GetPort());
            }
            requestPacketToContentServer(value_from_pkt);
            prefetchData(value_from_pkt);
            requestQueue.insert(std::pair<uint32_t, Address>(value_from_pkt, from));
        }

    }
}

void
UdpCacheServer::HandleReadServer(Ptr<Socket> socketP2P)
{
    NS_LOG_FUNCTION(this << socketP2P);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;

    while ((packet = socketP2P->RecvFrom(from)))
    {
        socketP2P->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " cache received " << packet->GetSize() << " bytes from content server " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
        }

        /* packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags(); */

        uint32_t value_from_pkt = getIdPacket(packet);

        NS_LOG_LOGIC("Check in the cache if the packet with random value " << value_from_pkt << " is present");
        if (!cacheContains(value_from_pkt))
        {
            pushInCache(value_from_pkt);
        }
        // get the list of clients that requested the packet from the requestQueue, iterate it and send the packet to each client

        std::multimap<uint32_t, ns3::Address>::iterator requestQueueIter = requestQueue.find(value_from_pkt);
        
        while (requestQueueIter != requestQueue.end() && requestQueueIter->first == value_from_pkt)
        {
            sendPacketBackToClient(value_from_pkt, requestQueueIter->second);
            // remove the client from the requestQueue
            requestQueue.erase(requestQueueIter++);
        }

        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " cache sent " << packet->GetSize() << " bytes to " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port " << InetSocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

uint32_t
UdpCacheServer::getIdPacket(Ptr<Packet> packet)
{
    uint8_t* buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string payload = std::string(reinterpret_cast<char*>(buffer), packet->GetSize());
    std::regex six_digit_number_regex(R"(\b\d{1,6}\b)");

    std::smatch matches;
    if (std::regex_search(payload, matches, six_digit_number_regex))
    {
        std::string id = matches[0];
        return static_cast<uint32_t>(std::stoul(id));
    }
    else
    {
        // handle whenever the id is not found
        return 0;
    }
}

void UdpCacheServer::sendPacketBackToClient(uint32_t value_to_send, Address to){

    std::string message = "{ \"sender\": \"cache\", \"type\": \"response\", \"id\": " + std::to_string(value_to_send) + " }";

    uint32_t dataSize = message.size() + 1;
    uint8_t* m_data = new uint8_t[dataSize];

    memcpy(m_data, message.c_str(), dataSize);

    Ptr<Packet> packet = Create<Packet>(m_data, dataSize);
    m_socket_clients->SendTo(packet, 0, to);
}

void UdpCacheServer::requestPacketToContentServer(uint32_t value_to_send){


    std::string message = "{ \"sender\": \"cache\", \"type\": \"request\", \"id\": " + std::to_string(value_to_send) + " }";
    uint32_t dataSize = message.size() + 1;
    uint8_t* m_data = new uint8_t[dataSize];

    memcpy(m_data, message.c_str(), dataSize);

    Ptr<Packet> packet = Create<Packet>(m_data, dataSize);
    m_socket_server->Send(packet);
}

void UdpCacheServer::pushInCache(const uint32_t& item) {
    if (m_cache.size() >= m_cacheSize) {
        m_cache.pop_front();
    }
    m_cache.push_back(item);
}

bool UdpCacheServer::cacheContains(const uint32_t& item) {
    for (const auto& val : m_cache) {
        if (val == item) {
            return true;
        }
    }
    return false;
}

bool UdpCacheServer::pushIfNotContained(const uint32_t& item) {
    if (!cacheContains(item)) {
        pushInCache(item);
        return false;
    }
    return true;
}

void UdpCacheServer::prefetchData(uint32_t value) {
    
    for (size_t i = 1; i < 3 && value-i > 0; i++)
    {
        if(!cacheContains(value-i)){
            requestPacketToContentServer(value-i);
        }
    }
    for (size_t i = 1; i < 3 && value+i < 101; i++)
    {
        if(!cacheContains(value+i)){
            requestPacketToContentServer(value+i);
        }
    }
}

void UdpCacheServer::printOut(){

    std::ofstream outputFile("output/cachestats.txt");
    
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file cachestats.txt" << std::endl;
        return;
    }
    outputFile << "cachehits:" << hitcount << ";" << "cacheaccess:" << accesscount << ";" << std::endl;
    outputFile.close();
}

} // Namespace ns3

