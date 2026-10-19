#!/bin/bash

# Test script for concurrent proxy connections

# Set the proxy and target URL
PROXY="http://localhost:8080"
URL="http://example.com"
NUM_REQUESTS=50

echo "Starting $NUM_REQUESTS concurrent requests through proxy $PROXY..."

# Run concurrent curl requests
for i in $(seq 1 $NUM_REQUESTS); do
	echo "Starting request $i"
	curl -x "$PROXY" "$URL" &> /dev/null &
done

echo "All requests started in background"
wait
echo "All requests completed"
