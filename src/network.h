/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file network.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_NETWORK_H_
#define  __RTCBASE_NETWORK_H_

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "memcheck.h"
#include "ipaddress.h"
#include "network_constants.h"
#include "sigslot.h"

struct ifaddrs;

namespace rtcbase {

class IfAddrsConverter;
class Network;

static const uint16_t k_network_cost_max = 999;
static const uint16_t k_network_cost_high = 900;
static const uint16_t k_network_cost_unknown = 50;
static const uint16_t k_network_cost_low = 10;
static const uint16_t k_network_cost_min = 0;

// By default, ignore loopback interfaces on the host.
const int k_default_network_ignore_mask = ADAPTER_TYPE_LOOPBACK;

// Makes a string key for this network. Used in the network manager's maps.
// Network objects are keyed on interface name, network prefix and the
// length of that prefix.
std::string make_network_key(const std::string& name, const IPAddress& prefix,
        int prefix_length);

// Utility function that attempts to determine an adapter type by an interface
// name (e.g., "wlan0"). Can be used by NetworkManager subclasses when other
// mechanisms fail to determine the type.
AdapterType get_adapter_type_from_name(const char* network_name);

class DefaultLocalAddressProvider {
public:
    virtual ~DefaultLocalAddressProvider() = default;
    // The default local address is the local address used in multi-homed endpoint
    // when the any address (0.0.0.0 or ::) is used as the local address. It's
    // important to check the return value as a IP family may not be enabled.
    virtual bool get_default_local_address(int family, IPAddress* ipaddr) const = 0;
};

class NetworkManager : public DefaultLocalAddressProvider {
public:
    typedef std::vector<Network*> NetworkList;
    
    // This enum indicates whether adapter enumeration is allowed.
    enum EnumerationPermission {
        ENUMERATION_ALLOWED,  // Adapter enumeration is allowed. Getting 0 network
                              // from GetNetworks means that there is no network
                              // available.
        ENUMERATION_BLOCKED,  // Adapter enumeration is disabled.
                              // GetAnyAddressNetworks() should be used instead.
    }; 

    NetworkManager();
    ~NetworkManager() override;
    
    // Called when network list is updated.
    Signal0<> signal_networks_changed;

    // Indicates a failure when getting list of network interfaces.
    Signal0<> signal_error;

    virtual void start_updating() = 0;
    virtual void stop_updating() = 0;

    // Returns the current list of networks available on this machine.
    // StartUpdating() must be called before this method is called.
    // It makes sure that repeated calls return the same object for a
    // given network, so that quality is tracked appropriately. Does not
    // include ignored networks.
    virtual void get_networks(NetworkList* networks) const = 0;    

    // return the current permission state of GetNetworks()
    virtual EnumerationPermission enumeration_permission() const;    
    
    // "AnyAddressNetwork" is a network which only contains single "any address"
    // IP address.  (i.e. INADDR_ANY for IPv4 or in6addr_any for IPv6). This is
    // useful as binding to such interfaces allow default routing behavior like
    // http traffic.
    // TODO(guoweis): remove this body when chromium implements this.
    virtual void get_any_address_networks(NetworkList* networks) { (void)networks; }

    // Dumps the current list of networks in the network manager.
    virtual void dump_networks() {}
    bool get_default_local_address(int family, IPAddress* ipaddr) const override;

public:
    struct Stats {
        int ipv4_network_count;
        int ipv6_network_count;
        Stats() {
            ipv4_network_count = 0;
            ipv6_network_count = 0;
        }
    };
}; 

// Base class for NetworkManager implementations.
class NetworkManagerBase : public NetworkManager {
public:
    NetworkManagerBase();
    ~NetworkManagerBase() override;
   
    void get_networks(NetworkList* networks) const override; 
    bool ipv6_enabled() const { return _ipv6_enabled; }

protected:
    typedef std::map<std::string, Network*> NetworkMap;
    
    // Updates |networks_| with the networks listed in |list|. If
    // |network_map_| already has a Network object for a network listed
    // in the |list| then it is reused. Accept ownership of the Network
    // objects in the |list|. |changed| will be set to true if there is
    // any change in the network list.
    void merge_network_list(const NetworkList& list, bool* changed);

    // |stats| will be populated even if |*changed| is false.
    void merge_network_list(const NetworkList& list,
            bool* changed,
            NetworkManager::Stats* stats);

private:
    NetworkList _networks;
    int _max_ipv6_networks;
    
    NetworkMap _networks_map;
    bool _ipv6_enabled;

    // We use 16 bits to save the bandwidth consumption when sending the network
    // id over the Internet. It is OK that the 16-bit integer overflows to get a
    // network id 0 because we only compare the network ids in the old and the new
    // best connections in the transport channel.
    uint16_t _next_available_network_id = 1;
};

// Basic implementation of the NetworkManager interface that gets list
// of networks using OS APIs.
class BasicNetworkManager : public NetworkManagerBase {
public:
    BasicNetworkManager();
    ~BasicNetworkManager() override;

    void start_updating() override;
    void stop_updating() override; 
    
    void dump_networks() override;

protected:
    void convert_if_addrs(ifaddrs* interfaces,
            IfAddrsConverter* converter,
            bool include_ignored,
            NetworkList* networks) const;
    
    // Creates a network object for each network available on the machine.
    bool create_networks(bool include_ignored, NetworkList* networks) const;

    // Determines if a network should be ignored. This should only be determined
    // based on the network's property instead of any individual IP.
    bool is_ignored_network(const Network& network) const;

private:
    // Only updates the networks; does not reschedule the next update.
    void update_networks_once();

private:
    bool _sent_first_update;
    std::vector<std::string> _network_ignore_list;
    bool _ignore_non_default_routes;
};

// Represents a Unix-type network interface, with a name and single address.
class Network : public MemCheck {
public:
    Network(const std::string& name,
            const std::string& description,
            const IPAddress& prefix,
            int prefix_length);

    Network(const std::string& name,
            const std::string& description,
            const IPAddress& prefix,
            int prefix_length,
            AdapterType type);
    ~Network();
    
    void set_default_local_address_provider(
            const DefaultLocalAddressProvider* provider) 
    {
        _default_local_address_provider = provider;
    }
    
    // Returns the name of the interface this network is associated wtih.
    const std::string& name() const { return _name; }
    
    // Returns the OS-assigned name for this network. This is useful for
    // debugging but should not be sent over the wire (for privacy reasons).
    const std::string& description() const { return _description; }

    // Returns the prefix for this network.
    const IPAddress& prefix() const { return _prefix; }
    // Returns the length, in bits, of this network's prefix.
    int prefix_length() const { return _prefix_length; }

    // |key_| has unique value per network interface. Used in sorting network
    // interfaces. Key is derived from interface name and it's prefix.
    std::string key() const { return _key; }    

    // Returns the Network's current idea of the 'best' IP it has.
    // Or return an unset IP if this network has no active addresses.
    // Here is the rule on how we mark the IPv6 address as ignorable for WebRTC.
    // 1) return all global temporary dynamic and non-deprecrated ones.
    // 2) if #1 not available, return global ones.
    // 3) if #2 not available, use ULA ipv6 as last resort. (ULA stands
    // for unique local address, which is not route-able in open
    // internet but might be useful for a close WebRTC deployment.

    // TODO(guoweis): rule #3 actually won't happen at current
    // implementation. The reason being that ULA address starting with
    // 0xfc 0r 0xfd will be grouped into its own Network. The result of
    // that is WebRTC will have one extra Network to generate candidates
    // but the lack of rule #3 shouldn't prevent turning on IPv6 since
    // ULA should only be tried in a close deployment anyway.

    // Note that when not specifying any flag, it's treated as case global
    // IPv6 address
    IPAddress get_best_IP() const;
    
    IPAddress ip() const { return get_best_IP(); }

    // Adds an active IP address to this network. Does not check for duplicates.
    void add_IP(const InterfaceAddress& ip) { _ips.push_back(ip); }     
   
    // Sets the network's IP address list. Returns true if new IP addresses were
    // detected. Passing true to already_changed skips this check.
    bool set_IPs(const std::vector<InterfaceAddress>& ips, bool already_changed); 
    // Get the list of IP Addresses associated with this network.
    const std::vector<InterfaceAddress>& get_IPs() const { return _ips;}

    // Returns the scope-id of the network's address.
    // Should only be relevant for link-local IPv6 addresses.
    int scope_id() const { return _scope_id; }
    void set_scope_id(int id) { _scope_id = id; }

    // Indicates whether this network should be ignored, perhaps because
    // the IP is 0, or the interface is one we know is invalid.
    bool ignored() const { return _ignored; }
    void set_ignored(bool ignored) { _ignored = ignored; }     
    
    AdapterType type() const { return _type; }
    void set_type(AdapterType type) {
        if (_type == type) {
            return;
        }
        _type = type;
        //SignalTypeChanged(this);
    }
    
    uint16_t get_cost() const {
        switch (_type) {
            case rtcbase::ADAPTER_TYPE_ETHERNET:
            case rtcbase::ADAPTER_TYPE_LOOPBACK:
                return k_network_cost_min;
            case rtcbase::ADAPTER_TYPE_WIFI:
            case rtcbase::ADAPTER_TYPE_VPN:
                return k_network_cost_low;
            case rtcbase::ADAPTER_TYPE_CELLULAR:
                return k_network_cost_high;
            default:
                return k_network_cost_unknown;
        }
    }

    // A unique id assigned by the network manager, which may be signaled
    // to the remote side in the candidate.
    uint16_t id() const { return _id; }
    void set_id(uint16_t id) { _id = id; }
    
    int preference() const { return _preference; }
    void set_preference(int preference) { _preference = preference; }

    // When we enumerate networks and find a previously-seen network is missing,
    // we do not remove it (because it may be used elsewhere). Instead, we mark
    // it inactive, so that we can detect network changes properly.
    bool active() const { return _active; }
    void set_active(bool active) {
        if (_active != active) {
            _active = active;
        }
    } 

    // Debugging description of this network
    std::string to_string() const;

private:
    const DefaultLocalAddressProvider* _default_local_address_provider = nullptr;
    std::string _name;
    std::string _description;
    IPAddress _prefix;
    int _prefix_length;
    std::string _key;
    std::vector<InterfaceAddress> _ips;
    int _scope_id;
    bool _ignored;
    AdapterType _type;
    int _preference;
    bool _active = true;
    uint16_t _id = 0;

    friend class NetworkManager;
};

} // rtcbase

#endif  //__RTCBASE_NETWORK_H_


