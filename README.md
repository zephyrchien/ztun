## Feature
Dual Stack
<br>
Async DNS
<br>
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

# set the precision of timer and operation timeout
# value: 1~2147483647(INT_MAX) ms
# timer_intv = xxx, default is 500
# connect_timeout = xxx, default is 2000
# resolve_intv = xxx, default is 60000
# resolve_timeout = xxx, default is 2000
timer_intv = 500
connect_timeout = 2000
resolve_intv = 60000
resolve_timeout = 2000

# set preallocated memory
# one connection requires 2 pipe + 1 connector + 2 readwriter + at least 1 timer
# object| cost * ratio(hard coded):
# pipe 16b * 2,
# timer 40b * 2,
# connector 88b * 1,
# readwriter 112b * 2,
# total 424b
prealloc = 30 # 60 pipe + 60 timer + 30 connector + 60 readwriter

# group1
local = 127.0.0.1:5000
remote = 1.1.1.1:443

# group2 (use ipv6)
local = ::1:10000
remote = 2.2.2.2:8080

# group3 (use domain name)
local = :15000
remote = www.google.com:80

# and more...
```