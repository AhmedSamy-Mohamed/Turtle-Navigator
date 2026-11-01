# ROS 2 Concepts Used in Turtle Navigator

## 1. Introduction

Turtle Navigator demonstrates several fundamental ROS 2 concepts through a practical C++ application.

The project uses a distributed node-based architecture and communicates through Topics, Services, Actions, and Parameters.

This document explains these concepts and how they are implemented in the project.

## 2. ROS 2 Nodes

A ROS 2 node is an independent process or component responsible for a specific function within a ROS 2 system.

Nodes communicate with each other through ROS 2 communication interfaces.

### Nodes in Turtle Navigator

| Node | Function |
|---|---|
| `goal_input` | Provides the Qt graphical interface. |
| `goal_validator` | Validates requested navigation goals. |
| `navigation_server` | Executes navigation and controls turtle motion. |
| `results_monitor` | Monitors navigation status and measurements. |
| `turtlesim_node` | Simulates the turtle and its environment. |

Each application node is implemented as a separate C++ executable.

## 3. ROS 2 Topics

### 3.1 What Is a Topic?

A ROS 2 Topic is a communication mechanism used to exchange messages between nodes.

Topics follow a publish-subscribe model:

- A publisher sends messages.
- One or more subscribers receive those messages.
- Communication is asynchronous.

Topics are suitable for continuously updated information such as sensor measurements, robot position, and velocity commands.

### 3.2 Topics Used in the Project

| Topic | Message Type | Publisher | Subscriber |
|---|---|---|---|
| `/turtle1/cmd_vel` | `geometry_msgs/msg/Twist` | `navigation_server` | `turtlesim_node` |
| `/turtle1/pose` | `turtlesim_msgs/msg/Pose` | `turtlesim_node` | `navigation_server` |
| `/navigation_status` | `turtle_navigator_interfaces/msg/NavigationStatus` | `navigation_server` | `results_monitor` |

### 3.3 Velocity Commands

The navigation server publishes velocity commands to:

`/turtle1/cmd_vel`

The message type is:

`geometry_msgs/msg/Twist`

The controller uses:

- `linear.x` for forward or backward movement.
- `angular.z` for rotation around the vertical axis.

Example ROS 2 command:

```bash
ros2 topic echo /turtle1/cmd_vel
```

### 3.4 Turtle Pose

The simulator publishes the turtle's current pose through:

`/turtle1/pose`

The navigation server subscribes to this topic to obtain the current position and orientation.

Example:

```bash
ros2 topic echo /turtle1/pose
```

### 3.5 Navigation Status

The navigation server publishes application-specific status information through:

`/navigation_status`

The message contains the current state, turtle position, target position, remaining distance, total traveled distance, and elapsed time.

Example:

```bash
ros2 topic echo /navigation_status
```

## 4. ROS 2 Services

### 4.1 What Is a Service?

A ROS 2 Service provides request-response communication between nodes.

Unlike Topics, a service is generally used when a client needs a specific response from another node.

A service consists of:

- Request: Data sent by the client.
- Response: Data returned by the server.

### 4.2 Goal Validation Service

The project implements a custom service:

`/validate_goal`

Service type:

`turtle_navigator_interfaces/srv/ValidateGoal`

The client is `goal_input`, and the server is `goal_validator`.

The request contains a target goal, while the response contains:

- `accepted`: Indicates whether the goal is valid.
- `reason`: Provides an explanation.

### 4.3 Inspecting the Service

List available services:

```bash
ros2 service list -t
```

Inspect the service definition:

```bash
ros2 interface show turtle_navigator_interfaces/srv/ValidateGoal
```

Inspect the custom goal message:

```bash
ros2 interface show turtle_navigator_interfaces/msg/Goal
```

## 5. ROS 2 Actions

### 5.1 What Is an Action?

A ROS 2 Action is designed for long-running operations that require feedback and may need cancellation.

An action provides three communication components:

1. Goal: The requested operation.
2. Feedback: Progress updates while the operation is running.
3. Result: The final outcome.

Actions are particularly useful for robotics tasks such as navigation, manipulation, and other operations that take time to complete.

### 5.2 Navigation Action Server

Turtle Navigator implements:

`/navigate_to_goal`

Action type:

`turtle_navigator_interfaces/action/NavigateToGoal`

The action client is `goal_input`, while `navigation_server` implements the action server.

The action definition contains:

**Goal:**

- Target position and orientation.

**Feedback:**

- Remaining distance to the target.

**Result:**

- Success status.
- Total traveled distance.
- Total navigation time.

### 5.3 Goal Cancellation and Replacement

The navigation server supports cancellation of an active goal.

It also handles a newly accepted goal while another goal is active by terminating the previous goal and starting the new one.

### 5.4 Inspecting the Action

```bash
ros2 action list -t
```

Inspect the action interface:

```bash
ros2 interface show turtle_navigator_interfaces/action/NavigateToGoal
```

Inspect action server information:

```bash
ros2 action info /navigate_to_goal
```

## 6. ROS 2 Parameters

### 6.1 What Is a Parameter?

ROS 2 Parameters provide a way to configure node behavior at runtime.

Parameters can be inspected and modified using ROS 2 command-line tools.

### 6.2 Safety Margin Parameter

The `goal_validator` node defines:

| Parameter | Type | Default |
|---|---|---:|
| `safety_margin` | Double | 1.0 |

The parameter determines the minimum permitted distance between a target coordinate and the simulation boundaries.

### 6.3 Inspecting and Updating Parameters

List parameters:

```bash
ros2 param list /goal_validator
```

Read the current value:

```bash
ros2 param get /goal_validator safety_margin
```

Update the value:

```bash
ros2 param set /goal_validator safety_margin 2.0
```

Restore the default value:

```bash
ros2 param set /goal_validator safety_margin 1.0
```

The change takes effect at runtime without rebuilding the application.

## 7. Custom ROS 2 Interfaces

The project uses a separate interface package:

`turtle_navigator_interfaces`

It defines the following interfaces:

| Interface | Type | Purpose |
|---|---|---|
| `Goal` | Message | Represents a requested target. |
| `NavigationStatus` | Message | Reports navigation state and measurements. |
| `ValidateGoal` | Service | Validates a requested target. |
| `NavigateToGoal` | Action | Executes a navigation operation. |

Separating interfaces into their own package makes them available to other ROS 2 packages that depend on them.

## 8. Quality of Service (QoS)

ROS 2 uses Quality of Service policies to control communication behavior.

The project applies a transient-local durability policy to the navigation status publisher and subscriber.

### Transient-Local Durability

Transient-local durability allows a publisher to retain samples for late-joining subscribers, subject to the QoS history and depth settings.

In Turtle Navigator, this helps the results monitor receive the most recently published navigation status when it subscribes.

The status publisher and subscriber use compatible QoS settings.

## 9. ROS 2 Launch System

ROS 2 Launch allows multiple nodes to be started together through a single launch description.

The project uses:

`launch/turtle_navigator.launch.py`

To launch the complete system:

```bash
ros2 launch turtle_navigator turtle_navigator.launch.py
```

This starts the simulator and the four application nodes.

## 10. Inspecting the Running ROS 2 Graph

The ROS 2 command-line interface provides useful tools for inspecting the system.

### List Nodes

```bash
ros2 node list
```

### List Topics

```bash
ros2 topic list -t
```

### List Services

```bash
ros2 service list -t
```

### List Actions

```bash
ros2 action list -t
```

### Inspect a Node

```bash
ros2 node info /navigation_server
```

These commands help verify that the expected nodes and communication interfaces are active.

## 11. Summary

Turtle Navigator combines the main ROS 2 communication mechanisms in one integrated application:

- **Nodes** organize the application into independent components.
- **Topics** exchange continuous data such as pose, velocity, and navigation status.
- **Services** provide request-response goal validation.
- **Actions** manage long-running navigation tasks with feedback and cancellation.
- **Parameters** allow runtime configuration.
- **Custom Interfaces** define application-specific communication structures.
- **Launch** starts the complete system.

These concepts form the foundation of the project's architecture and demonstrate practical ROS 2 development using C++.

---

