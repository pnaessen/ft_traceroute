# ft_traceroute

> A low-level network topology discovery tool written in C, replicating the core routing analysis behavior of the classic `traceroute` utility.

## 📖 Overview

This project explores **IP routing mechanisms** and **network diagnostics**. By dynamically manipulating the IP header's Time-To-Live (TTL) field and listening for specific ICMP error messages, this tool maps the exact path data packets take across the Internet to reach a target destination.

## 🛠️ Key Technical Features

* **Dynamic TTL Manipulation**: Programmatic control of the `IP_TTL` socket option to force intermediate routers to drop packets and reveal their identities.
* **Deep ICMP Parsing**: Intercepting and unpacking low-level ICMP control messages, specifically `Time Exceeded` (Type 11) and `Destination/Port Unreachable` (Type 3).
* **Raw Sockets (`SOCK_RAW`)**: Utilizing raw network sockets with elevated privileges to capture and filter inbound ICMP responses directly from the kernel.
* **High-Precision Timers**: Calculating accurate Round-Trip Time (RTT) latency for each network hop using `gettimeofday`.
* **Reverse DNS Resolution**: Implementing `getnameinfo` to resolve the hostnames of intermediate network nodes and gateways.
