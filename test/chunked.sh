#!/bin/bash

# Define the data for each chunk
chunk1="This is the first chunk"
chunk2="This is the second chunk"
chunk3="And here's the third chunk"

# Create a chunked data request using echo and pipe it to Curl
(echo -ne "$chunk1" ; echo -ne "$chunk2" ; echo -ne "$chunk3") | \
curl -X POST \
  --url localhost:7700 \
  --header "Transfer-Encoding: chunked" \
  --data-raw @-
