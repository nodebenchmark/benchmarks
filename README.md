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

**Node.js** The first step to run any Node.js application is to have the Node.js executable built. This repo contains the Node.js source code, but does *not* build the exeutable for you. To build a node binary, enter the `node/` directory, and follow the build instructons. If you want to use other versions of Node.js, feel free to replace the `node/` directory with other forks.

**Applications** All the Node.js applications in this suite are open-source projects on github. Some applications are slightly instrumented to be able to work with the load generator. All the applications contain a `node_module/` directory, which should give you all the necessary dependencies to run each application. In case an application does not work, take a look at their corresponding directories and follow the installation steps closely (typically just a `npm install`).

**Load Generator** The client-side load generator is based on the (awesome) [wrk2](https://github.com/giltene/wrk2). wrk2 is integrated as a submodule of this repo so it is always in sync with the upstream. You need build to the wrk binary to use the load generator. Simply type `make` in the `wrk2/` directory to do so. Note that additional system dependencies such as openssl might be needed.

**Harness** There is no installation action needed to use the harness script `nodebench` except Python 2.7 is a prerequisite.

## Basic Usage
Once you have the Node.js executable built and each individuall applications installed, the `nodebench` python script is all you need to run the benchmark applications. `nodebench` starts a given application and launches its corresponding client-side load generator, which issues client requests in a parameterized manner. For a more detailed usage of the script take a look at the script itself, but here is a quick summary:
* Run `./nodebench -h` to get a basic usage of the scipt.
* Run `./nodebench -l` to show a list of support applications.
* To run a supported application, run `./nodebench app_name`, which will first launch the specified server application and then isssue a set of client requests to the server. There are three optional parameters used to control the load generation. `-c` specifies the number of clients; `-d` specifies the duration of the load testing; `-r` specifies the expected throughput in terms of request-per-second. By default, `-c` is set to "1"; `-d` is set to "5s"; `-r` is set to "10". The duration argument needs to include a time unit (e.g., 2s, 2m, 2h). All the clients issue requests in parallel.

Here is a simple example: `./nodebench -b nodejs-todo -c 5 -d 15s -r 100` will launch the `nodejs-todo` application and simulate 5 clients that simultaneously issue requests for 15 seconds at a rate of 100 requests per second.

## Advanced Usage

The three parameteres will be trasnslated to wrk2's parameters. So take a look at wrk2's [readme](https://github.com/giltene/wrk2/tree/c4250acb6921c13f8dccfc162d894bd7135a2979) for more detailed information. Here are two important things that you might want to keep in mind. First, the script is going to launch the same amount of threads as the number of clients with each thread handling one client. Each client will open exactly one HTTP connection.

## Publication
Kindly please cite the following paper if you use our workload suite for your work.

Y. Zhu, D. Richins, M. Halpern, and V. J. Reddi, "[Microarchitectural Implications of Event-driven Server-side Web Applications](http://yuhaozhu.com/pubs/micro15.pdf)", In Proc. of MICRO, 2015.

## Contributors
[Yuhao Zhu](http://yuhaozhu.com/), Daniel Richins, Wenzhi Cui, [Matthew Halpern](http://matthewhalpern.com/), [Vijay Janapa Reddi](http://3nity.io/~vj/)

We welcome contributors!

## License
This workload suite is under the [Creative Commons CC BY license](https://creativecommons.org/licenses/by/4.0/). In a few words, this means you can freely share this material (copy and redistribute the material in any medium or format) as well as adapt it (remix, transform, and build upon the material for any purpose, even commercially) provided that you credit the contributors (e.g., citing the above MICRO paper) as well as that you not in any way suggests that we endorse you or your use of this material.
