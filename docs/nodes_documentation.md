# ROS 2 Nodes Documentation

## 1. Introduction

Turtle Navigator is organized into independent ROS 2 nodes, each responsible for a specific part of the application.

This modular structure separates user interaction, goal validation, navigation control, and monitoring.

The project contains four custom application nodes and one external simulation node.

## 2. Goal Input Node

**Source File:** `src/goal_input.cpp`

**Executable:** `goal_input`

**ROS 2 Node Name:** `goal_input`

**Technology:** C++ and Qt 6 Widgets

### 2.1 Overview

The Goal Input node provides the graphical interface used to interact with the navigation system.

It allows users to enter target coordinates, optionally specify a final orientation, validate goals, start navigation, and cancel active navigation.

### 2.2 Main Responsibilities

- Create and manage the Qt graphical interface.
- Accept target X and Y coordinates.
- Support optional final orientation.
- Communicate with the goal validation service.
- Submit navigation goals to the action server.
- Process action feedback and results.
- Request cancellation of active navigation.
- Display navigation status and measurements.

### 2.3 Communication Interfaces

| Interface | Name | Role |
|---|---|---|
| Service Client | `/validate_goal` | Validate requested goals. |
| Action Client | `/navigate_to_goal` | Submit and manage navigation goals. |

### 2.4 Execution Model

The Qt event loop runs in the main GUI thread, while a dedicated ROS 2 executor thread handles ROS communication.

Queued Qt invocations are used to transfer callback updates to the GUI thread.

This design helps maintain GUI responsiveness while navigation operations are running asynchronously.

## 3. Goal Validator Node

**Source File:** `src/goal_validator.cpp`

**Executable:** `goal_validator`

**ROS 2 Node Name:** `goal_validator`

### 3.1 Overview

The Goal Validator node provides a service for checking whether requested target coordinates satisfy the configured boundaries.

### 3.2 Main Responsibilities

- Receive validation requests.
- Read the current safety margin parameter.
- Check target coordinates.
- Return an acceptance result.
- Provide an explanation when a goal is rejected.

### 3.3 Communication Interfaces

| Interface | Name | Role |
|---|---|---|
| Service Server | `/validate_goal` | Process goal validation requests. |

### 3.4 Parameters

| Parameter | Type | Default |
|---|---|---:|
| `safety_margin` | Double | 1.0 |

The parameter can be updated at runtime.

### 3.5 Validation Logic

For a Turtlesim environment with coordinate limits from 0 to 11, the target must satisfy:

\[
m \leq x \leq 11-m
\]

\[
m \leq y \leq 11-m
\]

Where \(m\) represents the configured safety margin.

The service returns a Boolean acceptance result and an explanatory message.

## 4. Navigation Server Node

**Source File:** `src/navigation_server.cpp`

**Executable:** `navigation_server`

**ROS 2 Node Name:** `navigation_server`

### 4.1 Overview

The Navigation Server is responsible for executing navigation goals and controlling the turtle's motion.

It implements a ROS 2 Action Server and uses pose feedback from Turtlesim to calculate velocity commands.

### 4.2 Main Responsibilities

- Receive navigation goals.
- Accept or reject goal requests.
- Manage the active navigation goal.
- Subscribe to turtle pose updates.
- Calculate position and angular errors.
- Generate linear and angular velocity commands.
- Publish navigation feedback.
- Handle cancellation and goal replacement.
- Calculate navigation measurements.
- Publish navigation status.
- Return final action results.

### 4.3 Communication Interfaces

| Interface | Name | Role |
|---|---|---|
| Action Server | `/navigate_to_goal` | Execute navigation requests. |
| Publisher | `/turtle1/cmd_vel` | Send velocity commands. |
| Subscriber | `/turtle1/pose` | Receive turtle pose updates. |
| Publisher | `/navigation_status` | Publish navigation measurements and state. |

### 4.4 Main Execution Components

The implementation contains several important callbacks and operations:

- **Goal Handling:** Determines whether an incoming goal can be accepted.
- **Cancellation Handling:** Processes cancellation requests.
- **Accepted Goal Handling:** Initializes the newly accepted goal and navigation measurements.
- **Control Loop:** Calculates and publishes velocity commands.
- **Pose Callback:** Updates the current turtle pose.
- **Feedback Publishing:** Reports the remaining distance.
- **Result Handling:** Stops the turtle and reports the final outcome.

### 4.5 Motion Control

The controller uses the current turtle pose to calculate the required motion.

It calculates:

- Distance to the target.
- Desired heading.
- Angular error.
- Linear velocity.
- Angular velocity.

The controller first aligns the turtle toward the target and then commands forward movement.

It operates at approximately 20 Hz using a 50 ms timer interval.

### 4.6 Navigation Measurements

The server maintains:

- Remaining distance.
- Accumulated traveled distance.
- Elapsed navigation time.
- Pose sample count.
- Maximum distance between consecutive pose samples.

These measurements are used to report navigation progress and final results.

## 5. Results Monitor Node

**Source File:** `src/results_monitor.cpp`

**Executable:** `results_monitor`

**ROS 2 Node Name:** `results_monitor`

### 5.1 Overview

The Results Monitor node receives navigation status updates and reports the system's current navigation information.

### 5.2 Main Responsibilities

- Subscribe to navigation status updates.
- Detect navigation state changes.
- Convert numerical state values into readable labels.
- Display current and target positions.
- Report remaining and total distance.
- Report elapsed navigation time.
- Limit periodic console output.

### 5.3 Communication Interfaces

| Interface | Name | Role |
|---|---|---|
| Subscriber | `/navigation_status` | Receive navigation status messages. |

### 5.4 Monitoring Behavior

The node reports status changes immediately.

While navigation is active, periodic measurements are displayed approximately once per second.

The node uses transient-local durability to receive the most recently published status when it joins the topic.

## 6. Turtlesim Simulation Node

**Package:** `turtlesim`

**Executable:** `turtlesim_node`

**Node Name:** `turtlesim`

### 6.1 Overview

The Turtlesim node provides the 2D simulation environment used by the project.

It represents the simulated robot and responds to velocity commands from the navigation server.

### 6.2 Communication Interfaces

| Interface | Name | Role |
|---|---|---|
| Subscriber | `/turtle1/cmd_vel` | Receive velocity commands. |
| Publisher | `/turtle1/pose` | Publish current turtle pose. |

### 6.3 Role in the System

Turtlesim acts as the simulated environment and provides feedback about the turtle's movement.

It is an external ROS 2 package and is not implemented as part of the Turtle Navigator source code.

## 7. Node Interaction Summary

| Node | Publishers | Subscribers | Services | Actions |
|---|---|---|---|---|
| `goal_input` | — | — | Client: `/validate_goal` | Client: `/navigate_to_goal` |
| `goal_validator` | — | — | Server: `/validate_goal` | — |
| `navigation_server` | `/turtle1/cmd_vel`, `/navigation_status` | `/turtle1/pose` | — | Server: `/navigate_to_goal` |
| `results_monitor` | — | `/navigation_status` | — | — |
| `turtlesim_node` | `/turtle1/pose` | `/turtle1/cmd_vel` | — | — |

## 8. Running Individual Nodes

After building and sourcing the workspace, individual nodes can be started using ROS 2 commands.

### Goal Validator

```bash
ros2 run turtle_navigator goal_validator
```

### Navigation Server

```bash
ros2 run turtle_navigator navigation_server
```

### Results Monitor

```bash
ros2 run turtle_navigator results_monitor
```

### Goal Input GUI

```bash
ros2 run turtle_navigator goal_input
```

The Turtlesim simulator can be started separately:

```bash
ros2 run turtlesim turtlesim_node
```

For normal operation, starting the complete application through the launch file is recommended.

## 9. Summary

The node-based design allows each component to perform a clearly defined task.

The Goal Input node manages user interaction, the Goal Validator checks requested coordinates, the Navigation Server executes motion control, the Results Monitor reports navigation measurements, and Turtlesim provides the simulation environment.

This separation makes the system easier to understand, test, maintain, and extend.

---

