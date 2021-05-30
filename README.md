## Feature
Non-blocking IO
<br>
Zero Copy

## Build
```
git clone https://github.com/zephyrchien/ZTUN
cd ZTUN
mkdir build && cd build
cmake ..
make
```
## Usage
```
ztun -l <local> -r <remote> # from args
ztun -c <conf> # from config file
```
## Config
```
# comment, starting with #

# set log level and output path
# level = none|warn|info|debug, default is info
# file = filename, default is stderr
log_level = info
log_file = ztun.log

# set operation timeout and the precision of timer
# timer_intv = 1~2147483647 (ms), default is 500
# connect_timeout = 1~2147483647 (ms), default is 2000
timer_intv = 500
connect_timeout = 2000

# group1
local = :5000
remote = 1.1.1.1:443

# group2
local = :10000
remote = 2.2.2.2:8080

# and more...
```