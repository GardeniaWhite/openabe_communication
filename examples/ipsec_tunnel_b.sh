#!/bin/bash
# ip_1是host-a的ip , ip_2是host-b的ip，要保证ip_1和ip_2互相能够ping通

ip_1=192.168.204.130
ip_2=192.168.204.131

ovs-vsctl add-br br-ipsec
ip addr add 192.0.0.2/24 dev br-ipsec
ip link set br-ipsec up

cat key.txt|while read key
do
ovs-vsctl add-port br-ipsec tun -- \
            set interface tun type=gre \
                          options:remote_ip=$ip_1 \
                          options:psk=$key
done
