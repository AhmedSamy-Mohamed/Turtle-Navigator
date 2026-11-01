# Testing and Validation

## 1. Introduction

This document describes the functional testing and validation activities performed on the Turtle Navigator project.

The tests focus on verifying the communication between ROS 2 nodes, goal validation, navigation execution, cancellation, goal replacement, and navigation monitoring.

Testing was performed using the Turtlesim simulation environment.

## 2. Testing Environment

| Component | Configuration |
|---|---|
| Operating System | Ubuntu 26.04 LTS |
| ROS 2 Distribution | Lyrical |
| Programming Language | C++ |
| GUI Framework | Qt 6 |
| Build System | CMake |
| ROS 2 Build Tool | colcon |
| Simulation | Turtlesim |
| Development Environment | Visual Studio Code |

## 3. Build and Dependency Validation

The project dependencies were checked using `rosdep`.

Command:

```bash
rosdep check --from-paths src/turtle_navigator src/turtle_navigator_interfaces --ignore-src
```

Result:

```text
All system dependencies have been satisfied.
```

The project was built using:

```bash
colcon build --packages-up-to turtle_navigator --event-handlers console_direct+
```

Result:

```text
2 packages finished
```

Both the custom interfaces package and the main application package compiled successfully.

## 4. Node and Communication Validation

The project was launched and its ROS 2 communication interfaces were inspected.

The following components were verified:

- Goal input GUI node.
- Goal validator node.
- Navigation server node.
- Results monitor node.
- Turtlesim simulation node.

The expected communication interfaces include:

| Interface | Type | Purpose |
|---|---|---|
| `/turtle1/cmd_vel` | Topic | Velocity commands |
| `/turtle1/pose` | Topic | Turtle pose feedback |
| `/navigation_status` | Topic | Navigation status monitoring |
| `/validate_goal` | Service | Goal boundary validation |
| `/navigate_to_goal` | Action | Navigation execution |

The communication flow was inspected using ROS 2 command-line tools.

## 5. Functional Test Cases

### Test 1: Successful Navigation

**Objective:** Verify that the turtle can navigate to a valid target position.

**Test scenario:**

- Submit a navigation goal at `(8, 8)`.
- Validate the goal.
- Execute the navigation action.
- Monitor the turtle's movement and final result.

**Observed result:**

| Measurement | Observed Value |
|---|---:|
| Target X | 8.0 |
| Target Y | 8.0 |
| Total traveled distance | 3.384 m |
| Elapsed time | 3.56 s |
| Remaining distance | Approximately 0.10 |
| Pose samples | 212 |
| Maximum step distance | 0.024 m |

**Outcome:** The navigation action completed successfully within the configured position tolerance.

### Test 2: Invalid Goal Rejection

**Objective:** Verify that the goal validator rejects targets outside the permitted safety boundaries.

**Test scenario:**

- Submit a target that violates the configured safety margin.
- Observe the validation service response.

**Outcome:** The validator rejected the invalid goal.

### Test 3: Dynamic Safety Margin Update

**Objective:** Verify that the safety margin parameter can be modified at runtime.

**Test scenario:**

1. Start with the default safety margin of `1.0`.
2. Change the parameter to `2.0`.
3. Submit a goal affected by the updated boundary.
4. Verify the validation response.
5. Restore the parameter to `1.0`.

**Outcome:** The validator applied the updated parameter value to subsequent requests.

### Test 4: Goal Replacement

**Objective:** Verify the behavior when a new navigation goal is submitted while another goal is active.

**Test scenario:**

1. Start navigation toward `(3, 3)`.
2. Submit a new goal at `(8, 6)` before the previous goal finishes.
3. Observe the previous and new action results.

**Observed result:**

- The previous goal was aborted.
- The new goal was accepted.
- The new navigation task completed successfully.

The new goal produced the following measurements:

| Measurement | Observed Value |
|---|---:|
| Target X | 8.0 |
| Target Y | 6.0 |
| Total traveled distance | 3.610 m |
| Elapsed time | 4.83 s |

**Outcome:** Goal replacement behavior was verified.

### Test 5: Navigation Cancellation

**Objective:** Verify that an active navigation goal can be canceled.

**Test scenario:**

1. Start navigation toward `(3, 9)`.
2. Request cancellation from the GUI.
3. Observe the action status and turtle movement.

**Observed result:**

| Measurement | Observed Value |
|---|---:|
| Final action status | CANCELED |
| Total traveled distance | 1.01 m |
| Elapsed time | 1.74 s |
| Remaining distance | 4.87 m |

**Outcome:** The cancellation request was accepted, the turtle stopped, and the GUI displayed the canceled state.

## 6. Monitoring Validation

The `results_monitor` node was verified to receive navigation status messages.

The monitoring functionality includes:

- Navigation state.
- Current turtle position.
- Target position.
- Remaining distance.
- Total traveled distance.
- Elapsed navigation time.

The monitor uses transient local QoS to support receiving the most recently published status when a subscriber joins.

## 7. Testing Summary

| Test | Description | Result |
|---|---|---|
| 1 | Successful navigation | Passed |
| 2 | Invalid goal rejection | Passed |
| 3 | Dynamic safety margin update | Passed |
| 4 | Goal replacement | Passed |
| 5 | Navigation cancellation | Passed |
| 6 | Build and dependency validation | Passed |
| 7 | Node and communication inspection | Verified |
| 8 | Navigation status monitoring | Verified |

## 8. Testing Limitations

The current testing was performed in Turtlesim.

The results demonstrate functional behavior in simulation, but they do not establish real-world robot performance.

The following activities have not been claimed as completed:

- Physical robot testing.
- Real-world localization accuracy evaluation.
- Obstacle avoidance testing.
- Automated unit-test coverage.
- Large-scale performance benchmarking.

## 9. Conclusion

The performed tests verified the main functional capabilities of Turtle Navigator.

The project successfully demonstrated goal validation, point-to-point navigation, runtime parameter updates, action cancellation, goal replacement, and real-time status monitoring in a ROS 2 simulation environment.

These results provide a functional validation baseline for future improvements.

---

