# Node Benchmarks
A collection of NodeJS benchmarks, along with testing harnesses.

## Installation
The first step to run any NodeJS application is to have the NodeJS executable built. This repo contains a direct fork of NodeJS source code v.10.38, but does *not* build the exeutable for you. To do that, enter the `node` directory, and follow the build instructons. If you want to use other versions of NodeJS, feel free to replace the `node` directory with other forks.

The workload suite itself is merely a collection of open source NodeJS applications with some wrapper scripts. There is no installation action needed to use the wrapper scripts except Python 2.7 is a prerequisite. However, to run each individual application, take a look at their corresponding directories and follow the corresponding installation steps closely.

## Basic Usage
Once you have the NodeJS executable built and each individuall applications installed, you could launch an application using the following command:

`sh node.sh application_name`, in which currently supported `application_name` includes: etherpad-lite, lets-chat, lighter, nodejs-mud, nodejs-todo, word-finder.

This command will launch the corresponding NodeJS server application as specified by `application_name` as well as a client simulator that sequentially issues requests to stress the application. For detailed options, take a look at nodebench and node.sh.

## Publication
Please cite the following paper if you use our workload suite for your work.

[Microarchitectural Implications of Event-driven Server-side Web Applications](http://yuhaozhu.com/pubs/micro15.pdf) (MICRO 2015)
