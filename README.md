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
The workflow has been tested on Linux machines, both Ubuntu and Centos.

**Node.js** The first step to run any Node.js application is to have the Node.js executable built. This repo contains the Node.js source code, but does *not* build the exeutable for you. To build a node binary, enter the `node/` directory, and follow the build instructons. If you want to use other versions of Node.js, feel free to replace the `node/` directory with other forks.

**Applications** All the Node.js applications in this suite are open-source projects on github. Some applications are slightly instrumented to be able to work with the load generator. All the applications contain a `node_module/` directory, which should give you all the necessary dependencies to run each application. In case an application does not work, take a look at their corresponding directories and follow the installation steps closely (typically just a `npm install`).

**Load Generator** The client-side load generator is based on the (awesome) [wrk2](https://github.com/giltene/wrk2). wrk2 is integrated as a submodule of this repo so it is always in sync with the upstream. You need build to the wrk binary to use the load generator. Simply type `make` in the `wrk2/` directory to do so. Note that additional system dependencies such as openssl might be needed.

**Harness** There is no installation action needed to use the harness script `nodebench` except Python 2.7 is a prerequisite.

## Basic Usage
The `nodebench` python script is all your need to use the benchmark. Once you have the Node.js executable built and each individuall applications installed, you could launch an application using the following command:

`sh node.sh application_name`, in which currently supported `application_name` includes: etherpad-lite, lets-chat, lighter, nodejs-mud, nodejs-todo, word-finder. A NodeJS application needs to be instrumented to be able to run with out test harness. Unsupported applications such as NodeBB and nodejs-chat are not instrumented. The detailed instrumentation instructions are work in progress.

This command will launch the corresponding NodeJS server application as specified by `application_name` as well as a client simulator that sequentially issues requests to stress the application. For detailed options, take a look at nodebench and node.sh.

## Publication
Kindly please cite the following paper if you use our workload suite for your work.

Y. Zhu, D. Richins, M. Halpern, and V. J. Reddi, "[Microarchitectural Implications of Event-driven Server-side Web Applications](http://yuhaozhu.com/pubs/micro15.pdf)", In Proc. of MICRO, 2015.

## Contributors
[Yuhao Zhu](http://yuhaozhu.com/), Daniel Richins, Wenzhi Cui, [Matthew Halpern](http://matthewhalpern.com/), [Vijay Janapa Reddi](http://3nity.io/~vj/)

We welcome contributors!
