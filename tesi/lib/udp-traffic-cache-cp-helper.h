#ifndef UDP_TRAFFIC_CACHE_CP_HELPER_H
#define UDP_TRAFFIC_CACHE_CP_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <stdint.h>

namespace ns3
{

class UdpCacheServerHelper
{
  public:
    /**
     * Create UdpCacheServerHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * \param port The port the server will wait on for incoming packets
     */
    UdpCacheServerHelper(Address contentSever, uint16_t portContentServer, uint16_t portClients);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Create a UdpCacheServerApplication on the specified Node.
     *
     * \param node The node on which to create the Application.  The node is
     *             specified by a Ptr<Node>.
     *
     * \returns An ApplicationContainer holding the Application created,
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Create a UdpCacheServerApplication on specified node
     *
     * \param nodeName The node on which to create the application.  The node
     *                 is specified by a node name previously registered with
     *                 the Object Name Service.
     *
     * \returns An ApplicationContainer holding the Application created.
     */
    ApplicationContainer Install(std::string nodeName) const;

    /**
     * \param c The nodes on which to create the Applications.  The nodes
     *          are specified by a NodeContainer.
     *
     * Create one udp cache server application on each of the Nodes in the
     * NodeContainer.
     *
     * \returns The applications created, one Application per Node in the
     *          NodeContainer.
     */
    ApplicationContainer Install(NodeContainer c) const;

  private:
    /**
     * Install an ns3::UdpCacheServer on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an UdpCacheServer will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory; //!< Object factory.
};

/**
 * \ingroup udpcache
 * \brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class UdpTrafficGeneratorHelper
{
  public:
    /**
     * Create UdpTrafficGeneratorHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * not include a port value (e.g., Ipv4Address and Ipv6Address).
     *
     * \param ip The IP address of the remote udp echo server
     * \param port The port number of the remote udp echo server
     */
    UdpTrafficGeneratorHelper(Address ip, uint16_t port);
    /**
     * Create UdpTrafficGeneratorHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
     *
     * \param addr The address of the remote udp echo server
     */
    UdpTrafficGeneratorHelper(Address addr);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Given a pointer to a UdpTrafficGenerator application, set the data fill of the
     * packet (what is sent as data to the server) to the contents of the fill
     * string (including the trailing zero terminator).
     *
     * \warning The size of resulting echo packets will be automatically adjusted
     * to reflect the size of the fill string -- this means that the PacketSize
     * attribute may be changed as a result of this call.
     *
     * \param app Smart pointer to the application (real type must be UdpTrafficGenerator).
     * \param fill The string to use as the actual echo data bytes.
     */
    void SetFill(Ptr<Application> app, std::string fill);

    /**
     * Create a udp echo client application on the specified node.  The Node
     * is provided as a Ptr<Node>.
     *
     * \param node The Ptr<Node> on which to create the UdpTrafficGeneratorApplication.
     *
     * \returns An ApplicationContainer that holds a Ptr<Application> to the
     *          application created
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Create a udp echo client application on the specified node.  The Node
     * is provided as a string name of a Node that has been previously
     * associated using the Object Name Service.
     *
     * \param nodeName The name of the node on which to create the UdpTrafficGeneratorApplication
     *
     * \returns An ApplicationContainer that holds a Ptr<Application> to the
     *          application created
     */
    ApplicationContainer Install(std::string nodeName) const;

    /**
     * \param c the nodes
     *
     * Create one udp echo client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
    ApplicationContainer Install(NodeContainer c) const;

  private:
    /**
     * Install an ns3::UdpTrafficGenerator on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an UdpTrafficGenerator will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;
    ObjectFactory m_factory; //!< Object factory.
};

class UdpContentProviderHelper
{
  public:
    /**
     * Create UdpContentProviderHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * \param port The port the server will wait on for incoming packets
     */
    UdpContentProviderHelper(uint16_t port);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Create a UdpContentProviderApplication on the specified Node.
     *
     * \param node The node on which to create the Application.  The node is
     *             specified by a Ptr<Node>.
     *
     * \returns An ApplicationContainer holding the Application created,
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Create a UdpContentProviderApplication on specified node
     *
     * \param nodeName The node on which to create the application.  The node
     *                 is specified by a node name previously registered with
     *                 the Object Name Service.
     *
     * \returns An ApplicationContainer holding the Application created.
     */
    ApplicationContainer Install(std::string nodeName) const;

    /**
     * \param c The nodes on which to create the Applications.  The nodes
     *          are specified by a NodeContainer.
     *
     * Create one udp cache server application on each of the Nodes in the
     * NodeContainer.
     *
     * \returns The applications created, one Application per Node in the
     *          NodeContainer.
     */
    ApplicationContainer Install(NodeContainer c) const;

  private:
    /**
     * Install an ns3::UdpContentProvider on the node configured with all the
     * attributes set with SetAttribute.
     *
     * \param node The node on which an UdpContentProvider will be installed.
     * \returns Ptr to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* UDP_TRAFFIC_CACHE_CP_HELPER_H */
