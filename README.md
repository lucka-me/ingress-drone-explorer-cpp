# Ingress Drone Explorer - C++

An offline CLI tool to analyze reachable Portals for Ingress Drone Mark I.

A Swift implementation is also [available](https://github.com/lucka-me/ingress-drone-explorer-swift).
The C++ implementation is faster but the code is much more complex.

### Requirements

- CMake
- Boost
    - JSON
    - Program Options
- CXX that supports C++20

## Build

Install the latest Swift, clone the repository and simply:

```sh
$ cmake --build ./build --target all
```

## Usage

### Prepare Files

All the files should be JSON.

1. Portal list file(s), should be an array of:
    ```jsonc
    {
        "guid": "GUID of Portal",
        "title": "Title of Portal",
        "lngLat": {
            "lng": 90.0,    // Longitude
            "lat": 45.0     // Latitude
        }
    }
    ```
2. Portal Key list file, should be an array of GUID (Not required but strongly recommended)

Maybe an IITC plugin like [this](https://github.com/lucka-me/toolkit/tree/master/Ingress/Portal-List-Exporter) can help.

### Run

```sh
$ ./bin/ingress-drone-explorer <path-to-portal-list-files> -s <start point>
```

Explore with key list:
```sh
$ ./bin/ingress-drone-explorer ... -k <path-to-key-list-file>
```

Output cells for IITC Draw tools:
```sh
$ ./bin/ingress-drone-explorer ... --output-drawn-items <path-to-output>
```

Help information:
```sh
$ ./bin/ingress-drone-explorer -h
```