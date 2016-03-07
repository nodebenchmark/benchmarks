# Node Benchmarks
A collection of Node.js-based server applications along with a client-side load generator. The ultimate goal of this workload suite is to collect representative applications to enable research on designing better server systems in light of of the asynchronous, event-driven execution model that is increasingly prevalent in today's cloud environment.

This is a project from the [Electrical and Computer Engineering Department](http://www.ece.utexas.edu/) at [The University of Texas at Austin](http://www.utexas.edu/). Please see the contributor list at the end for a list of people who have been contributing to this project.

## Installation
The first step to run any Node.js application is to have the NodeJS executable built. This repo contains a direct fork of [Node.js](https://github.com/nodejs/node) source tree v.10.38 (with some minor instrumentations for observing the libuv event loop), but does *not* build the exeutable for you. To build a node binary, enter the `node` directory, and follow the build instructons. If you want to use other versions of Node.js, feel free to replace the `node` directory with other forks.

The workload suite itself is a collection of open source Node.js server applications with a client-side load generator. Some applications are slightly instrumented to be able to work with the load generator. There is no installation action needed to use the wrapper scripts except Python 2.7 is a prerequisite. However, to run each individual application, take a look at their corresponding directories and follow the corresponding installation steps closely.

The workflow has been tested on Linux machines.

## Basic Usage
Once you have the Node.js executable built and each individuall applications installed, you could launch an application using the following command:

`sh node.sh application_name`, in which currently supported `application_name` includes: etherpad-lite, lets-chat, lighter, nodejs-mud, nodejs-todo, word-finder. A NodeJS application needs to be instrumented to be able to run with out test harness. Unsupported applications such as NodeBB and nodejs-chat are not instrumented. The detailed instrumentation instructions are work in progress.

This command will launch the corresponding NodeJS server application as specified by `application_name` as well as a client simulator that sequentially issues requests to stress the application. For detailed options, take a look at nodebench and node.sh.

## Publication
Please cite the following paper if you use our workload suite for your work.

Y. Zhu, D. Richins, M. Halpern, and V. J. Reddi, "[Microarchitectural Implications of Event-driven Server-side Web Applications](http://yuhaozhu.com/pubs/micro15.pdf)", In Proc. of MICRO, 2015.

## Contributors
[Yuhao Zhu](http://yuhaozhu.com/)
Daniel Richins
Wenzhi Cui
[Matthew Halpern](http://matthewhalpern.com/)
[Vijay Janapa Reddi](http://3nity.io/~vj/)

We welcome contributors!
