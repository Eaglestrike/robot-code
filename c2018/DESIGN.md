# 2018 Code Design
*Note: This document is a work in progress. If you have questions, please ask!* 

## Subsystems
The code shall be divided into subsystems, which are our primary unit of encapsulation for conceptual, 
structural, and execution purposes. Each subsystem shall be given its own module within the subsystem package in the form of either a file or folder containing the system and it's relevant components. Each subsystem will be given it's own thread shortly after creation. 
This year if a task requires it's own thread, it should be considered a subsystem to better facilitate code layout and interfacing with the parent subsystem. Use this as a general rule of thumb, but there will very likely be exceptions which come up that make more sense from a concurrency prespective to separate.

### Subsystem Internals
Each subsystem should have a `new` associated function, which should take in all needed `Sender`s
and return a tuple of `Self` and the `Sender` for the associated command queue.

Each subsystem should have a `run` method, which takes `self` and runs the subsystem, looping
and sleeping (or, more probably, waiting for a new command or update) as necessary.

Each subsystem should have an enum containing all commands it knows how to process. Variants of this enum
will be what is sent over the subsystem's command queue.

Each subsystem will contain a bus broadcast channel to convery messages to other subsystems.
Each subsystem should have a `create_receiver` method which returns a `BusReader` connected to the
respective subsystem's broadcast channel. The bus should be created when `new` is called as to ensure
`create_receiver` can be used directly after creation.

### Components
Each subsystem may include, in it directory, multiple rust modules or structs, known as components.
For instance, the drive subsystem may own the `DriveSides`, which may in turn own the motors. These
components are not intended to "think", but merely to execute commands called in the form of methods.
All components must reside within a subsystem. Access should be private and relevant information can
be shared using the subsystem broadcast and instruction channels. While the definition of a component
is clarified in this section, there will be no component struct or trait as they are only contained within
their subsystems or some form of utility file and the current design has no need for extraneous abstraction.

### Layout
*Note: This section is even more indeterminate than everything else.* 

- Controller
  - Gets inputs from driver station and delegates instructions to the other subsystems
  - Manages when auton routines should be used and canceled
- Drive
  - Moves the robot
  - Monitors the position of the robot through dead reckoning
- Hatch
  - Hatch ejection system (unknown at this point)
  - Elevator
- Cargo
  - Intake the cargo balls
  - Shoot or eject the balls into goals
- Vision
  - Monitor the robot's relative position to targets and broadcasts the information to the relevant subsystems


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
