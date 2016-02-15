CLibvenice
==========

[![Swift 2.2](https://img.shields.io/badge/Swift-2.2-orange.svg?style=flat)](https://developer.apple.com/swift/)
[![Platforms Linux](https://img.shields.io/badge/Platforms-Linux-lightgray.svg?style=flat)](https://developer.apple.com/swift/)
[![License MIT](https://img.shields.io/badge/License-MIT-blue.svg?style=flat)](https://tldrlegal.com/license/mit-license)
[![Slack Status](https://zewo-slackin.herokuapp.com/badge.svg)](https://slack.zewo.io)

**CSP** for **Swift 2.2**.

## Installation

- Install [`libvenice`](https://github.com/Zewo/libvenice)

### Homebrew 
```bash
$ brew tap zewo/tap
$ brew install libvenice
```

### Ubuntu/Debian
```bash
$ echo "deb [trusted=yes] http://apt.zewo.io/deb ./" | sudo tee --append /etc/apt/sources.list
$ sudo apt-get update
$ sudo apt-get install libvenice
```

### Source
```bash
$ git clone https://github.com/Zewo/libvenice.git && cd libvenice
$ make
$ (sudo) make install
```

- Add `CLibvenice` to your `Package.swift`

```swift
import PackageDescription

let package = Package(
	dependencies: [
		.Package(url: "https://github.com/Zewo/CLibvenice.git", majorVersion: 0, minor: 2)
	]
)

```

## Community

[![Slack](http://s13.postimg.org/ybwy92ktf/Slack.png)](https://zewo-slackin.herokuapp.com)

Join us on [Slack](https://zewo-slackin.herokuapp.com).

License
-------

**CLibvenice** is released under the MIT license. See LICENSE for details.
