#!/usr/bin/env bash

set -e

IFACE=$(ip route get "8.8.8.8" | sed -n 's/.*dev \([^\ ]*\).*/\1/p')
IP=$(ip addr show dev "${IFACE}" | grep -Po 'inet \K[\d.]+')
NSNAME="bad"

VETHA="veth-a"
VETHB="veth-b"
VETHPREFIX="192.168.163.0/24"
VETHAIP="192.168.163.1"
VETHBIP="192.168.163.254"

function create() {
  ip netns add ${NSNAME}

  ip link add ${VETHA} type veth peer name ${VETHB}
  ip link set ${VETHA} netns ${NSNAME}

  ip netns exec ${NSNAME} ip addr add ${VETHAIP}/24 dev ${VETHA}
  ip netns exec ${NSNAME} ip link set ${VETHA} up

  ip addr add ${VETHBIP}/24 dev ${VETHB}
  ip link set ${VETHB} up

  ip netns exec ${NSNAME} ip route add default via ${VETHBIP} dev ${VETHA}

  echo 1 > /proc/sys/net/ipv4/ip_forward

  iptables -t nat -A POSTROUTING -s ${VETHPREFIX} -o ${IFACE} -j SNAT --to-source ${IP}

  tc qdisc add dev ${VETHB} root netem delay 200ms 40ms 25% loss 15.3% 25% duplicate 1% corrupt 0.1% reorder 5% 50%
}

function destroy() {
  ip netns del ${NSNAME}
}

if [[ "$1" = "create" ]]; then
  create
elif [[ "$1" = "destroy" ]]; then
  destroy
fi

# sudo ip netns exec bad ping 10.0.0.102
