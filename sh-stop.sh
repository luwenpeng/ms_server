################################################################################
# @File      : sh-stop.sh
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

pidlog="/home/lwp/myserver/pid.log"
cat ${pidlog} | xargs kill -s SIGQUIT
