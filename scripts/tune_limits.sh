#!/bin/bash

# Display the file-descriptor limits currently configured for this shell.
echo "Current soft limit: $(ulimit -Sn)"
echo "Current hard limit: $(ulimit -Hn)"

# Raise the soft limit for commands started from this shell.
ulimit -n 65536

echo "New soft limit for this shell: $(ulimit -Sn)"

# These kernel settings can increase the pending connection queues before the
# Week 2 load test. Applying them requires administrator privileges.
# sudo sysctl -w net.core.somaxconn=65536
# sudo sysctl -w net.ipv4.tcp_max_syn_backlog=65536

# Containers can receive an equivalent file-descriptor limit at startup:
# docker run --ulimit nofile=200000:200000 ...