[![Build Status](https://jenkins.hbx-root.de/buildStatus/icon?job=pioe)](https://jenkins.hbx-root.de/job/pioe/)

### Building from Source

#### Getting the source

You can either download the source package here or clone this repository.

### Build Environment

#### Linux

##### Debian

```
$ sudo apt-get install -y git cmake gcc make python3 ruby ruby-dev python3-dev gengetopt doxygen
$ git clone https://git.hiddenbox.org/lotherk/pioengine.git /tmp/pioe
$ mkdir /tmp/pioe/build; cd /tmp/pioe/build
$ cmake .. && make && make test
```

#### Windows

##### Easy way with chocolatey.

Install chocolatey.

Install via chocolatey:
```
choco install mingw msys2 git cmake gnuwin ruby python -y --execution-timeout 9999
```

