# 2018 Code Design
*Note: This document is a work in progress. If you have questions, please ask!* 

## Subsystems
The code shall be divided into subsystems, which are our primary unit of encapsulation for conceptual, 
structural, and execution purposes. Each subsystem shall be given its own module (probably in a new
directory) and its own thread.

### Subsystem Internals
Each subsystem should have a `new` associated function, which should take in all needed `Sender`s
and return a tuple of `Self`, the `Sender` for the associated command queue, and a `Receiver` for
the associated bus broadcast channel.

Each subsystem should have a `run` method, which takes `self` and runs the subsystem, looping
and sleeping (or, more probably, waiting for a new command or update) as necessary.

Each subsystem should have an enum containing all commands it knows how to process. Variants of this enum
will be what is sent over the subsystem's command queue.

### Components
Each subsystem may include, in it directory, multiple rust modules or structs, known as components.
For instance, the drive subsystem may own the `DriveSides`, which may in turn own the motors. These
components are not intended to "think", but merely to execute commands called in the form of methods.

### Layout
*Note: This section is even more indeterminate than everything else.* 

- Controller (takes controls and tells others what to do)
- Drive (moves robot)
- Hatch
- Cargo (ball)
- Vision?


### Communication
#### Command Queue
Subsystems shall communicate using channels. Each subsystem will have have its own mpsc channel
(we will be using the [crossbeam-channel] crate, which are actually mpmc, but we will be using them
with only one consumer). For the most part, only **commands** from the Controller to the subsystem
should be sent via this queue. 

#### Broadcast Channels
For cases where events need to be broadcast to all intrested subsystems, we will use the [bus] crate.
This should be done for all **data** sent from one peer subsystem to one or more other peer subsystems. 

## Initialization

There will be a master robot struct, which owns all other structs. It will be in charge of establishing
the thread for each subsystem.

### Injection
*Note: If you don't understand this section, you can probably ignore it.*

Dependency injection shall be used only for external dependencies (i.e. channels) at the subsystem
level. All components should be created by the `new` method of the subsystem. At lower levels,
all dependencies shall be injected.



## Configuration
All configuration (e.g. hardware channel numbers, physical constants of design) shall be handled 
in a `config` module by declaring constants. Subsystems may depend on these constants. 


[crossbeam-channel]: https://github.com/crossbeam-rs/crossbeam/tree/master/crossbeam-channel
[bus]: https://github.com/jonhoo/bus
