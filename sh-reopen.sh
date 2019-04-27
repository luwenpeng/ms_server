################################################################################
# @File      : sh-reopen.sh
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

log_dir="/home/lwp/myserver/" # 建议使用绝对路径
old_log_dir="/home/lwp/myserver/" # 建议使用绝对路径
errlog_file="error.log"
acclog_file="access.log"
pid_file="/home/lwp/myserver/pid.log" # 建议使用绝对路径

rename_errlog=${errlog_file%.*}.$(date +"%4Y%2m%2d-%2H%M%S").log
rename_acclog=${acclog_file%.*}.$(date +"%4Y%2m%2d-%2H%M%S").log

cd $log_dir

# 重命名
mv $errlog_file ${old_log_dir}${rename_errlog}
mv $acclog_file ${old_log_dir}${rename_acclog}

cat ${pid_file} | xargs kill -s SIGINT

# 删除 3 天前的 log 文件
cd $old_log_dir
find . -mtime +3 -name "*20[1-9][1-9]*" | xargs rm -f
# -mtime n : File's data was last modified n*24 hours ago.
