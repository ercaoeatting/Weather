#启动守护进程
/home/han/CPP/Weather/tools/bin/procctl 10 /home/han/CPP/Weather/tools/mytest/checkproc  /tmp/log/checkproc.log

# 生成气象数据，每分钟运行一次
/home/han/CPP/Weather/tools/bin/procctl 60 /home/han/CPP/Weather/idc/cpp/crtsurfdata /home/han/CPP/Weather/idc/ini/stcode.ini  /tmp/idc/surfdata  /log/idc/crtsurfdata.log csv,xml,json