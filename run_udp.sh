#!/bin/bash

python3 pcom_hw2_udp_client/udp_client.py --input_file pcom_hw2_udp_client/sample_wildcard_payloads.json --source-address 127.0.0.2 --source-port 12345 --mode all_once --count 1 --delay 1000 127.0.0.1 8888