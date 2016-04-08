# Node Benchmarks
A collection of Node.js-based server applications along with a client-side load generator. The ultimate goal of this workload suite is to collect representative applications to enable research on designing better server systems in light of the asynchronous, event-driven execution model that is increasingly prevalent in today's cloud environment.

This is a project from the [Electrical and Computer Engineering Department](http://www.ece.utexas.edu/) at [The University of Texas at Austin](http://www.utexas.edu/). Please see the contributor list at the end for a list of people who have been contributing to this project.

## Repo Structure
The high-level structure of this repo is as follows:

* `node/`: The [Node.js](https://github.com/nodejs/node) source code. It is a direct fork of v.10.38 with some minor instrumentations for observing the libuv event loop.
* `wrk2/`: The [wrk2](https://github.com/giltene/wrk2) load generator. It is integrated as a git submodule.
* `applications/`: Node.js-based server applications.
* `loadgen/`: Contains the Lua scripts used by wrk2 to generate client-side loads for each application.
* `nodebench`: The harness script that launches a given server application and its load generator.

## Installation
The workflow has been tested on Linux machines, both Ubuntu and CentOS.

* [Add a new SSH key to your github account]() to check out submodules.
* Check out the repository recursively:<br>
`git clone --recursively https://github.com/nodebenchmark/benchmarks.git`
* Or if you already checkout without `--recursive` option:<br>
`git submodule update --init --recursive`
* Build dependencies.

**Node.js** The first step to run any Node.js application is to have the Node.js executable built. This repo contains the Node.js source code, but does *not* build the exeutable for you. To build a node binary, enter the `node/` directory, and follow the build instructons. If you want to use other versions of Node.js, feel free to replace the `node/` directory with other forks.

**Applications** All the Node.js applications in this suite are open-source projects on github. Some applications are slightly instrumented to be able to work with the load generator. All the applications contain a `node_module/` directory, which should give you all the necessary dependencies to run each application. In case an application does not work, take a look at their corresponding directories and follow the installation steps closely (typically just a `npm install`).

**Load Generator** The client-side load generator is based on the (awesome) [wrk2](https://github.com/giltene/wrk2). wrk2 is integrated as a submodule of this repo so it is always in sync with the upstream. You need build to the wrk binary to use the load generator. Simply type `make` in the `wrk2/` directory to do so. Note that additional system dependencies such as openssl might be needed.

**Harness** There is no installation action needed to use the harness script `nodebench` except Python 2.7 is a prerequisite.

## Basic Usage
Once you have the Node.js executable built and each individuall applications installed, the `nodebench` python script is all you need to run the benchmark applications. `nodebench` starts a given application and launches its corresponding client-side load generator, which issues client requests in a parameterized manner. For a more detailed usage of the script take a look at the script itself, but here is a quick summary:
* Run `./nodebench -h` to get a basic usage of the scipt.
* Run `./nodebench -l` to show a list of support applications.
* To run a supported application, run `./nodebench app_name`, which will first launch the specified server application and then isssue a set of client requests to the server. There are three optional parameters used to control the load generation. `-c` specifies the number of clients; `-d` specifies the duration of the load testing; `-r` specifies the expected throughput in terms of request-per-second. By default, `-c` is set to "1"; `-d` is set to "5s"; `-r` is set to "10". The duration argument needs to include a time unit (e.g., 2s, 2m, 2h). All the clients issue requests in parallel.
* Make sure `-r` divided by `-c` is greater than 1 otherwise wrk2 would think that the expected RPS per client is 0 (because wrk2 does simple integer division) and hang--but with ~100% CPU utilization!.

Here is a simple example: `./nodebench -b nodejs-todo -c 5 -d 15s -r 100` will launch the `nodejs-todo` application and simulate 5 clients that *simultaneously* issue requests for 15 seconds at a rate of 100 requests per second.

## Advanced Usage

We discuss a few advanced usages here. It is always more fruitful to read the code directly ;)

#### Customize Load Parameters
Sometime we want to to customize the load generator parameters to put different levels of stress on the server. The three load generator related parameteres will be trasnslated to wrk2's parameters. So take a look at wrk2's [readme](https://github.com/giltene/wrk2/tree/c4250acb6921c13f8dccfc162d894bd7135a2979) for more detailed information. Here are a few tips:

1. The `nodebench` script takes in a `-c` argument indicating the number of clients. wrk2 however does not have the concept of "client". Rather it has two concepts: "connection" and "thread". A connection is a HTTP connection that sends a request to the server and waits for the response before sending another request. All the connections are evenly distributed to each thread. By specifying the `-c` argument, `nodebench` is going to launch the same amount of threads as the number of connections (as specified by the value of `-c`) with each thread handling exactly one client. This is a preferred way of generating client loads as it is simple and clear--after all, the number of threads, not the number of connections, dictates the client-side concurrency. However, wrk2 does allows more flexible configurations such as more than one connection per thread if you want. Modify the `nodebench` script to do so.
2. If you want to assign more than one connections to each thread, keep in mind that different connections within a thread are interleaved such that connection 2's request might go out before connection 1's response is received. You might run into unexpected bugs in the Lua script if not careful about this.

#### Customize Client Behavior
The exact client behavior of each application is specified in the Lua scripts in the `loadgen/` directory. We have hard-coded some representative behaviors for each application, but sometimes you might want to generate your own client-side behaviors. Modify the corresponding Lua script to do that. Here are a few tips:

1. The Lua script is thread local so all the variables are local to a thread.
2. The `request()` function returns an HTTP request message that will be issued. The `response()` function is a callback function that will be called every time the client receives a response from the server. The `init()` function is executed once and only once before any requests is sent.
3. For each connection, `request()` is blocking in the sense that `response()` is guaranteed to be executed after the client receives the response from the server. However as noted above, `request()` of a different connection in the same thread might execute earlier.
4. The best way to learn what kind of HTTP request to send to simulate a particular client-side behavior is to intercept HTTP requests in a browser. [Chrome's DevTools](https://developer.chrome.com/devtools/docs/network) is a good friend for this purpose.
5. wrk2 only issues HTTP requests. So if an Node.js application relies on websocket (e.g., socket.io), wrk2 will not work and you need to find another load generator.

## Publication
Kindly please cite the following paper if you use our workload suite for your work.

Y. Zhu, D. Richins, M. Halpern, and V. J. Reddi, "[Microarchitectural Implications of Event-driven Server-side Web Applications](http://yuhaozhu.com/pubs/micro15.pdf)", In Proc. of MICRO, 2015.

## Contributors
[Yuhao Zhu](http://yuhaozhu.com/), Daniel Richins, Wenzhi Cui, [Matthew Halpern](http://matthewhalpern.com/), [Vijay Janapa Reddi](http://3nity.io/~vj/)

We welcome contributors!

## License
This workload suite is under the [Creative Commons CC BY license](https://creativecommons.org/licenses/by/4.0/). In a few words, this means you can freely share this material (copy and redistribute the material in any medium or format) as well as adapt it (remix, transform, and build upon the material for any purpose, even commercially) provided that you credit the contributors (e.g., citing the above MICRO paper) as well as that you not in any way suggests that we endorse you or your use of this material.
