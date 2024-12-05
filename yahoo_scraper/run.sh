#!/usr/bin/env bash
./fetch_options >> log.txt
rclone copy option_chains/ gdrive:/option_chains