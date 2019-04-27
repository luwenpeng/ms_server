################################################################################
# @File      : sh-wrk.sh
# @Copyright : 2018 lwp Corporation, All Rights Reserved.
#
# @Author    : lwp
#
# @Brief     : 
#
#--------------------------- Revision History ----------------------------------
#  No      Version     Date        Revised By      Item        Description
# @1
#
################################################################################

#!bin/bash

wrk -t 12 -c  100 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  200 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  300 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  400 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  500 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  600 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  700 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  800 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c  900 -d 10s --latenc "http://127.0.0.1:9999"
wrk -t 12 -c 1000 -d 10s --latenc "http://127.0.0.1:9999"
